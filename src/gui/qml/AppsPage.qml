import QtQuick
import QtQuick.Layouts

// Apps page placeholder. Item management is implemented in Phase 2.
Item {
    id: page

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: Theme.spacing

        Text {
            text: qsTr("Apps")
            color: Theme.textPrimary
            font.pixelSize: 26
            font.bold: true
        }

        Text {
            text: qsTr("All registered items will be listed here.")
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
                    text: qsTr("Item management is coming in a later milestone.")
                    color: Theme.textSecondary
                    font.pixelSize: 14
                }
            }
        }
    }
}
