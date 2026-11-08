import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Full item list with search, category filter, add/edit/delete, and launch.
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
                          : qsTr("%n item(s)", "", App.filteredItems.count)
                    color: Theme.textSecondary
                    font.pixelSize: 13
                }

                Text {
                    visible: App.readOnly
                    text: qsTr("Read-only mode: changes are not saved.")
                    color: "#ffcc66"
                    font.pixelSize: 12
                }
            }

            Item {
                Layout.fillWidth: true
            }

            Button {
                text: qsTr("Add item")
                enabled: !App.readOnly
                onClicked: itemDialog.openForAdd()
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            HubSearchField {
                id: searchField
                Layout.fillWidth: true
                Layout.minimumWidth: 160
            }

            ComboBox {
                id: categoryFilter
                Layout.preferredWidth: 200

                model: {
                    const options = [
                        { id: "", name: qsTr("All categories") },
                        { id: "__uncategorized__", name: qsTr("Uncategorized") }
                    ]
                    const categories = App.categoryOptions
                    for (let i = 0; i < categories.length; ++i) {
                        options.push(categories[i])
                    }
                    return options
                }
                textRole: "name"
                valueRole: "id"
                onActivated: App.filteredItems.categoryId = currentValue
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
                model: App.filteredItems
                focus: true
                currentIndex: count > 0 ? 0 : -1

                Keys.onReturnPressed: {
                    if (currentIndex >= 0) {
                        App.launchItem(App.filteredItems.itemIdAt(currentIndex))
                    }
                }
                Keys.onEnterPressed: {
                    if (currentIndex >= 0) {
                        App.launchItem(App.filteredItems.itemIdAt(currentIndex))
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
                                text: model.categoryName.length > 0
                                      ? model.categoryName + " · " + model.target
                                      : model.target
                                color: Theme.textSecondary
                                font.pixelSize: 12
                                elide: Text.ElideMiddle
                            }
                        }

                        ToolButton {
                            enabled: !App.readOnly
                            onClicked: App.items.setFavorite(model.itemId, !model.favorite)
                            Accessible.name: qsTr("Favorite")

                            contentItem: Text {
                                text: model.favorite ? "★" : "☆"
                                color: model.favorite ? Theme.accent : Theme.textSecondary
                                font.pixelSize: 18
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                        }

                        ToolButton {
                            text: qsTr("Edit")
                            enabled: !App.readOnly
                            onClicked: itemDialog.openForEdit(model.itemId)
                        }

                        ToolButton {
                            text: qsTr("Delete")
                            enabled: !App.readOnly
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
                visible: App.filteredItems.count === 0

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: App.items.count === 0 ? qsTr("No items yet") : qsTr("No matching items")
                    color: Theme.textPrimary
                    font.pixelSize: 18
                    font.bold: true
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: App.items.count === 0
                          ? qsTr("Add an application, folder, or website to get started.")
                          : qsTr("Try a different search or category filter.")
                    color: Theme.textSecondary
                    font.pixelSize: 14
                }
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+F"
        enabled: page.visible
        onActivated: searchField.forceActiveFocus()
    }

    Connections {
        target: App

        function onCategoryOptionsChanged() {
            categoryFilter.currentIndex = 0
            App.filteredItems.categoryId = ""
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

        Shortcut {
            sequence: "Return"
            enabled: deleteDialog.visible
            onActivated: {
                App.items.removeItem(page.pendingDeleteId)
                deleteDialog.close()
            }
        }
    }
}
