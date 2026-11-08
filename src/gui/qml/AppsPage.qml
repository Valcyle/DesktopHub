import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Full item list with add, edit, delete, and launch actions.
Item {
    id: page

    property string pendingDeleteId: ""
    property string pendingDeleteName: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: Theme.spacing

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                spacing: 4

                Text {
                    text: qsTr("Apps")
                    color: Theme.textPrimary
                    font.pixelSize: 26
                    font.bold: true
                }

                Text {
                    text: App.items.count === 0
                          ? qsTr("No items registered yet.")
                          : qsTr("%n item(s)", "", App.items.count)
                    color: Theme.textSecondary
                    font.pixelSize: 13
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Add item")
                onClicked: itemDialog.openForAdd()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 8
            radius: Theme.cardRadius
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            ListView {
                id: listView

                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 2
                model: App.items
                focus: true
                currentIndex: count > 0 ? 0 : -1

                Keys.onReturnPressed: {
                    if (currentIndex >= 0) {
                        App.launchItem(App.items.itemIdAt(currentIndex))
                    }
                }
                Keys.onEnterPressed: {
                    if (currentIndex >= 0) {
                        App.launchItem(App.items.itemIdAt(currentIndex))
                    }
                }

                delegate: ItemDelegate {
                    id: delegate

                    width: ListView.view.width
                    height: 66

                    onClicked: ListView.view.currentIndex = index
                    onDoubleClicked: App.launchItem(model.itemId)

                    contentItem: RowLayout {
                        spacing: 12

                        Image {
                            source: "image://itemicon/" + model.itemId + "/" + model.revision
                            sourceSize: Qt.size(32, 32)
                            Layout.preferredWidth: 32
                            Layout.preferredHeight: 32
                            fillMode: Image.PreserveAspectFit
                            smooth: true
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                Layout.fillWidth: true
                                text: model.name
                                color: Theme.textPrimary
                                font.pixelSize: 14
                                elide: Text.ElideRight
                            }

                            Text {
                                Layout.fillWidth: true
                                text: model.target
                                color: Theme.textSecondary
                                font.pixelSize: 12
                                elide: Text.ElideMiddle
                            }
                        }

                        Text {
                            visible: model.favorite
                            text: "★"
                            color: Theme.accent
                            font.pixelSize: 16
                            Accessible.name: qsTr("Favorite")
                        }

                        ToolButton {
                            text: qsTr("Edit")
                            onClicked: itemDialog.openForEdit(model.itemId)
                        }

                        ToolButton {
                            text: qsTr("Delete")
                            onClicked: {
                                page.pendingDeleteId = model.itemId
                                page.pendingDeleteName = model.name
                                deleteDialog.open()
                            }
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                visible: App.items.count === 0

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No items yet")
                    color: Theme.textPrimary
                    font.pixelSize: 18
                    font.bold: true
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Add an application, folder, or website to get started.")
                    color: Theme.textSecondary
                    font.pixelSize: 14
                }
            }
        }
    }

    ItemDialog {
        id: itemDialog
    }

    Dialog {
        id: deleteDialog
        modal: true
        anchors.centerIn: parent
        width: 460
        title: qsTr("Delete registration")

        contentItem: Text {
            text: qsTr("Delete \"%1\"? The original file will not be deleted.")
                      .arg(page.pendingDeleteName)
            color: Theme.textPrimary
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: deleteDialog.close()
            }

            Button {
                text: qsTr("Delete")
                DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                onClicked: {
                    App.items.removeItem(page.pendingDeleteId)
                    deleteDialog.close()
                }
            }
        }
    }
}
