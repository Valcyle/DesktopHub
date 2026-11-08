import QtQuick
import QtQuick.Layouts

// Home page. Phase 1 shows the empty state; favorites and categories follow
// once the item model exists.
Item {
    id: page

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: Theme.spacing

        Text {
            text: qsTr("Home")
            color: Theme.textPrimary
            font.pixelSize: 26
            font.bold: true
        }

        Text {
            text: qsTr("Favorites and categories at a glance.")
            color: Theme.textSecondary
            font.pixelSize: 14
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 8
            radius: Theme.cardRadius
            color: Theme.surface
            border.width: 1
            border.color: Theme.border

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No items yet")
                    color: Theme.textPrimary
                    font.pixelSize: 18
                    font.bold: true
                }

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Apps, folders, and websites you add will appear here.")
                    color: Theme.textSecondary
                    font.pixelSize: 14
                }
            }
        }
    }
}
