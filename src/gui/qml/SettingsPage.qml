import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Settings: language, categories, data location, and version.
Item {
    id: page

    property string pendingCategoryId: ""
    property string pendingCategoryName: ""

    ScrollView {
        id: scroll

        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: scroll.availableWidth
            spacing: Theme.spacing

            Text {
                text: qsTr("Settings")
                color: Theme.textPrimary
                font.pixelSize: 26
                font.bold: true
            }

            Text {
                visible: App.readOnly
                text: qsTr("Read-only mode: changes are not saved.")
                color: "#ffcc66"
                font.pixelSize: 12
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: generalColumn.implicitHeight + 48
                radius: Theme.cardRadius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                ColumnLayout {
                    id: generalColumn
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 20

                    Text {
                        text: qsTr("General")
                        color: Theme.textPrimary
                        font.pixelSize: 16
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Column {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: qsTr("Language")
                                color: Theme.textPrimary
                                font.pixelSize: 14
                            }

                            Text {
                                text: qsTr("Changes apply immediately.")
                                color: Theme.textSecondary
                                font.pixelSize: 12
                            }
                        }

                        ComboBox {
                            id: languageBox
                            Layout.preferredWidth: 220
                            enabled: !App.readOnly

                            model: [
                                { code: "en", name: qsTr("English") },
                                { code: "ja", name: qsTr("Japanese") }
                            ]
                            textRole: "name"
                            valueRole: "code"

                            Component.onCompleted: currentIndex = indexOfValue(AppSettings.language)
                            onActivated: AppSettings.language = currentValue
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 16

                        Column {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: qsTr("Theme")
                                color: Theme.textPrimary
                                font.pixelSize: 14
                            }

                            Text {
                                text: qsTr("Dark is the only theme in this version.")
                                color: Theme.textSecondary
                                font.pixelSize: 12
                            }
                        }

                        Text {
                            text: qsTr("Dark")
                            color: Theme.textPrimary
                            font.pixelSize: 14
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: categoryColumn.implicitHeight + 48
                radius: Theme.cardRadius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                ColumnLayout {
                    id: categoryColumn
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Column {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                text: qsTr("Categories")
                                color: Theme.textPrimary
                                font.pixelSize: 16
                                font.bold: true
                            }

                            Text {
                                text: qsTr("Assign categories when editing an item.")
                                color: Theme.textSecondary
                                font.pixelSize: 12
                            }
                        }

                        Button {
                            text: qsTr("Add category")
                            enabled: !App.readOnly
                            onClicked: categoryDialog.openForAdd()
                        }
                    }

                    Text {
                        visible: App.categories.count === 0
                        text: qsTr("No categories yet.")
                        color: Theme.textSecondary
                        font.pixelSize: 13
                    }

                    ListView {
                        id: categoryList
                        Layout.fillWidth: true
                        Layout.preferredHeight: Math.min(contentHeight, 200)
                        visible: count > 0
                        clip: true
                        spacing: 2
                        model: App.categories

                        delegate: RowLayout {
                            width: ListView.view.width
                            height: 40
                            spacing: 8

                            Text {
                                Layout.fillWidth: true
                                text: model.categoryName
                                color: Theme.textPrimary
                                font.pixelSize: 14
                                elide: Text.ElideRight
                            }

                            ToolButton {
                                text: qsTr("Rename")
                                enabled: !App.readOnly
                                onClicked: categoryDialog.openForEdit(model.categoryId)
                            }

                            ToolButton {
                                text: qsTr("Delete")
                                enabled: !App.readOnly
                                onClicked: {
                                    page.pendingCategoryId = model.categoryId
                                    page.pendingCategoryName = model.categoryName
                                    deleteCategoryDialog.open()
                                }
                            }
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: dataColumn.implicitHeight + 48
                radius: Theme.cardRadius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                ColumnLayout {
                    id: dataColumn
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 12

                    Text {
                        text: qsTr("Data")
                        color: Theme.textPrimary
                        font.pixelSize: 16
                        font.bold: true
                    }

                    Text {
                        text: qsTr("Storage location")
                        color: Theme.textPrimary
                        font.pixelSize: 14
                    }

                    Text {
                        Layout.fillWidth: true
                        text: AppSettings.dataLocation
                        color: Theme.textSecondary
                        font.family: "Consolas"
                        font.pixelSize: 12
                        wrapMode: Text.WrapAnywhere
                    }

                    Text {
                        text: qsTr("Logs and application data stay on this device.")
                        color: Theme.textSecondary
                        font.pixelSize: 12
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: updateColumn.implicitHeight + 48
                radius: Theme.cardRadius
                color: Theme.surface
                border.width: 1
                border.color: Theme.border

                ColumnLayout {
                    id: updateColumn
                    anchors.fill: parent
                    anchors.margins: 24
                    spacing: 12

                    Text {
                        text: qsTr("Updates")
                        color: Theme.textPrimary
                        font.pixelSize: 16
                        font.bold: true
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: qsTr("Current version: %1").arg(Qt.application.version)
                            color: Theme.textSecondary
                            font.pixelSize: 12
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Text {
                            Layout.maximumWidth: 360
                            horizontalAlignment: Text.AlignRight
                            text: App.updates.statusText
                            color: App.updates.available ? Theme.accent : Theme.textSecondary
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                    }

                    HubCheckBox {
                        text: qsTr("Check for updates automatically on startup")
                        checked: AppSettings.checkForUpdates
                        enabled: !App.readOnly
                        onClicked: AppSettings.checkForUpdates = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Button {
                            text: qsTr("Check for updates")
                            enabled: !App.updates.busy
                            onClicked: App.updates.checkForUpdates(false)
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        Button {
                            text: qsTr("Download and install")
                            visible: App.updates.available && !App.updates.busy
                            onClicked: installConfirmDialog.open()
                        }
                    }

                    ProgressBar {
                        Layout.fillWidth: true
                        visible: App.updates.busy
                        from: 0
                        to: 100
                        indeterminate: App.updates.progress < 0
                        value: Math.max(0, App.updates.progress)
                    }

                    Text {
                        Layout.fillWidth: true
                        visible: App.updates.errorText.length > 0
                        text: App.updates.errorText
                        color: "#ff9b9b"
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }

                    Text {
                        visible: App.updates.releaseNotes.length > 0
                        text: qsTr("Release notes")
                        color: Theme.textPrimary
                        font.pixelSize: 13
                        font.bold: true
                    }

                    TextArea {
                        Layout.fillWidth: true
                        Layout.minimumHeight: 64
                        Layout.preferredHeight: Math.min(contentHeight, 160)
                        visible: App.updates.releaseNotes.length > 0
                        text: App.updates.releaseNotes
                        readOnly: true
                        wrapMode: TextArea.Wrap
                        selectByMouse: true
                        color: Theme.textSecondary
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Update checks contact GitHub only when enabled or requested. No personal data is sent.")
                        color: Theme.textSecondary
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                Text {
                    text: qsTr("Version")
                    color: Theme.textSecondary
                    font.pixelSize: 12
                }

                Text {
                    text: Qt.application.version
                    color: Theme.textSecondary
                    font.pixelSize: 12
                }

                Item {
                    Layout.fillWidth: true
                }
            }

            Item {
                Layout.preferredHeight: 8
            }
        }
    }

    CategoryDialog {
        id: categoryDialog
    }

    Dialog {
        id: deleteCategoryDialog
        modal: true
        anchors.centerIn: parent
        width: 460
        title: qsTr("Delete category")

        contentItem: Text {
            text: qsTr("Delete category \"%1\"? Items in it will become uncategorized.")
                      .arg(page.pendingCategoryName)
            color: Theme.textPrimary
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: deleteCategoryDialog.close()
            }

            Button {
                text: qsTr("Delete")
                DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                onClicked: {
                    App.deleteCategory(page.pendingCategoryId)
                    deleteCategoryDialog.close()
                }
            }
        }

        Shortcut {
            sequence: "Return"
            enabled: deleteCategoryDialog.visible
            onActivated: {
                App.deleteCategory(page.pendingCategoryId)
                deleteCategoryDialog.close()
            }
        }
    }

    Dialog {
        id: installConfirmDialog
        modal: true
        anchors.centerIn: parent
        width: 480
        padding: 24
        title: qsTr("Install update")

        contentItem: Text {
            width: installConfirmDialog.availableWidth
            text: qsTr("Desktop Hub will close and restart to install version %1.")
                      .arg(App.updates.latestVersion)
            color: Theme.textPrimary
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: installConfirmDialog.close()
            }

            Button {
                text: qsTr("Install")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    installConfirmDialog.close()
                    App.updates.downloadAndInstall()
                }
            }
        }

        Shortcut {
            sequence: "Return"
            enabled: installConfirmDialog.visible
            onActivated: {
                installConfirmDialog.close()
                App.updates.downloadAndInstall()
            }
        }
    }
}
