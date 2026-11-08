import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

// Add/edit dialog shared by the Apps page.
Dialog {
    id: dialog

    property string editId: ""
    property bool nameTouched: false
    property bool duplicateConfirmed: false

    modal: true
    anchors.centerIn: parent
    width: 580
    padding: 24
    title: editId.length > 0 ? qsTr("Edit item") : qsTr("Add item")
    closePolicy: Popup.CloseOnEscape

    function selectedType() {
        return ["app", "folder", "url"][typeBox.currentIndex]
    }

    function resolvedType() {
        const type = selectedType()
        if (type !== "app") {
            return type
        }
        return targetField.text.toLowerCase().endsWith(".lnk") ? "lnk" : "exe"
    }

    function typeIndexFor(itemType) {
        if (itemType === "folder") {
            return 1
        }
        if (itemType === "url") {
            return 2
        }
        return 0
    }

    function resetFields() {
        inlineError.text = ""
        duplicateConfirmed = false
        nameTouched = false
    }

    function openForAdd() {
        editId = ""
        typeBox.currentIndex = 0
        targetField.text = ""
        nameField.text = ""
        favoriteBox.checked = false
        resetFields()
        open()
    }

    function openForEdit(id) {
        const item = App.items.item(id)
        if (!item.id) {
            return
        }

        editId = id
        typeBox.currentIndex = typeIndexFor(item.type)
        targetField.text = item.target
        nameField.text = item.name
        favoriteBox.checked = item.favorite
        resetFields()
        nameTouched = true
        open()
    }

    function commit() {
        const type = resolvedType()
        const name = nameField.text.trim()
        const target = targetField.text.trim()

        if (name.length === 0 || name.length > 100) {
            inlineError.text = qsTr("Name must be 1 to 100 characters.")
            return
        }
        if (target.length === 0) {
            inlineError.text = qsTr("Select a target first.")
            return
        }

        if (!duplicateConfirmed && App.items.hasTarget(type, target, editId)) {
            duplicateDialog.open()
            return
        }

        const error = editId.length > 0
            ? App.items.updateItem(editId, name, type, target, favoriteBox.checked)
            : App.items.addItem(name, type, target, favoriteBox.checked)

        if (error.length > 0) {
            inlineError.text = error
            return
        }

        close()
    }

    contentItem: ColumnLayout {
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: qsTr("Type")
                color: Theme.textSecondary
                font.pixelSize: 13
                Layout.preferredWidth: 90
            }

            ComboBox {
                id: typeBox
                Layout.fillWidth: true
                model: [
                    { label: qsTr("Application") },
                    { label: qsTr("Folder") },
                    { label: qsTr("Web URL") }
                ]
                textRole: "label"

                onActivated: {
                    targetField.text = ""
                    nameField.text = ""
                    dialog.nameTouched = false
                    dialog.resetFields()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: qsTr("Target")
                color: Theme.textSecondary
                font.pixelSize: 13
                Layout.preferredWidth: 90
                Layout.alignment: Qt.AlignTop
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                TextField {
                    id: targetField
                    Layout.fillWidth: true
                    readOnly: dialog.selectedType() !== "url"
                    placeholderText: dialog.selectedType() === "url"
                                     ? qsTr("https://example.com")
                                     : qsTr("Choose a file or folder")
                    selectByMouse: true

                    onTextEdited: dialog.duplicateConfirmed = false
                    onEditingFinished: {
                        if (dialog.selectedType() === "url"
                                && dialog.editId === ""
                                && !dialog.nameTouched
                                && text.trim().length > 0) {
                            nameField.text = App.nameFromTarget("url", text)
                        }
                    }
                }

                Button {
                    text: qsTr("Browse…")
                    visible: dialog.selectedType() !== "url"
                    onClicked: {
                        if (dialog.selectedType() === "folder") {
                            folderDialog.open()
                        } else {
                            applicationDialog.open()
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                text: qsTr("Name")
                color: Theme.textSecondary
                font.pixelSize: 13
                Layout.preferredWidth: 90
            }

            TextField {
                id: nameField
                Layout.fillWidth: true
                selectByMouse: true
                onTextEdited: {
                    dialog.nameTouched = true
                    dialog.duplicateConfirmed = false
                }
            }
        }

        CheckBox {
            id: favoriteBox
            text: qsTr("Add to favorites")
        }

        Text {
            id: inlineError
            Layout.fillWidth: true
            visible: text.length > 0
            color: "#ff9b9b"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    footer: DialogButtonBox {
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            onClicked: dialog.close()
        }

        Button {
            text: qsTr("Save")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            onClicked: dialog.commit()
        }
    }

    FileDialog {
        id: applicationDialog
        title: qsTr("Select an application")
        nameFilters: [qsTr("Applications (*.exe *.lnk)"), qsTr("All files (*)")]
        onAccepted: {
            const path = App.localFile(selectedFile)
            targetField.text = path
            dialog.duplicateConfirmed = false
            if (dialog.editId === "" && !dialog.nameTouched) {
                nameField.text = App.nameFromTarget("exe", path)
            }
        }
    }

    FolderDialog {
        id: folderDialog
        title: qsTr("Select a folder")
        onAccepted: {
            const path = App.localFile(selectedFolder)
            targetField.text = path
            dialog.duplicateConfirmed = false
            if (dialog.editId === "" && !dialog.nameTouched) {
                nameField.text = App.nameFromTarget("folder", path)
            }
        }
    }

    Dialog {
        id: duplicateDialog
        modal: true
        anchors.centerIn: parent
        width: 460
        title: qsTr("Already registered")

        contentItem: Text {
            text: qsTr("This target is already registered. Register it again?")
            color: Theme.textPrimary
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: duplicateDialog.close()
            }

            Button {
                text: qsTr("Register anyway")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    dialog.duplicateConfirmed = true
                    duplicateDialog.close()
                    dialog.commit()
                }
            }
        }
    }
}
