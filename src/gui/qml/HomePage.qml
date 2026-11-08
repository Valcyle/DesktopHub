import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Home dashboard: Favorites first, then one section per category, followed by
// uncategorized items.
Item {
    id: page

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
                    text: qsTr("Home")
                    color: Theme.textPrimary
                    font.pixelSize: 26
                    font.bold: true
                }

                Text {
                    text: qsTr("%n item(s)", "", App.items.count)
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

            HubSearchField {
                id: searchField
                Layout.preferredWidth: 260
                Layout.minimumWidth: 160
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
                id: homeList

                anchors.fill: parent
                anchors.margins: 8
                clip: true
                spacing: 2
                model: App.home
                focus: true
                currentIndex: count > 0 ? 0 : -1

                Keys.onReturnPressed: {
                    const itemId = App.home.itemIdAt(currentIndex)
                    if (itemId.length > 0) {
                        App.launchItem(itemId)
                    }
                }
                Keys.onEnterPressed: {
                    const itemId = App.home.itemIdAt(currentIndex)
                    if (itemId.length > 0) {
                        App.launchItem(itemId)
                    }
                }

                delegate: ItemDelegate {
                    id: delegate

                    width: ListView.view.width
                    height: model.isHeader ? 40 : 64
                    enabled: !model.isHeader

                    onClicked: ListView.view.currentIndex = index
                    onDoubleClicked: App.launchItem(model.itemId)

                    contentItem: Item {
                        Text {
                            visible: model.isHeader
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.bottom: parent.bottom
                            anchors.bottomMargin: 6
                            text: model.title
                            color: Theme.textSecondary
                            font.pixelSize: 13
                            font.bold: true
                        }

                        RowLayout {
                            visible: !model.isHeader
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 8
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
                        }
                    }

                    background: Rectangle {
                        radius: 8
                        color: !model.isHeader && delegate.hovered
                               ? Theme.surfaceHover
                               : "transparent"
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                visible: App.home.count === 0

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
                          ? qsTr("Apps, folders, and websites you add will appear here.")
                          : qsTr("Try a different search.")
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
}
