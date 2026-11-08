import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Adds a category or renames an existing one.
Dialog {
    id: dialog

    property string editId: ""

    modal: true
    anchors.centerIn: parent
    width: 460
    padding: 24
    title: editId.length > 0 ? qsTr("Rename category") : qsTr("Add category")
    closePolicy: Popup.CloseOnEscape

    function openForAdd() {
        editId = ""
        nameField.text = ""
        inlineError.text = ""
        open()
        nameField.forceActiveFocus()
    }

    function openForEdit(id) {
        const category = App.categories.category(id)
        if (!category.id) {
            return
        }

        editId = id
        nameField.text = category.name
        inlineError.text = ""
        open()
        nameField.forceActiveFocus()
    }

    function commit() {
        const name = nameField.text.trim()
        const error = editId.length > 0
            ? App.renameCategory(editId, name)
            : App.addCategory(name)

        if (error.length > 0) {
            inlineError.text = error
            return
        }

        close()
    }

    contentItem: ColumnLayout {
        spacing: 12

        Text {
            text: qsTr("Category name")
            color: Theme.textSecondary
            font.pixelSize: 13
        }

        TextField {
            id: nameField
            Layout.fillWidth: true
            selectByMouse: true
            placeholderText: qsTr("1 to 50 characters")
            onTextEdited: inlineError.text = ""
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

    Shortcut {
        sequence: "Return"
        enabled: dialog.visible
        onActivated: dialog.commit()
    }
}
