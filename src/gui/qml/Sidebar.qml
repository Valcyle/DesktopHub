import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Fixed-width navigation sidebar.
Rectangle {
    id: sidebar

    property int currentPage: 0
    signal pageRequested(int page)

    implicitWidth: Theme.sidebarWidth
    color: Theme.sidebarBackground

    component NavButton: ItemDelegate {
        id: control

        required property string label
        property int page: -1
        property bool selected: sidebar.currentPage === page

        text: label
        implicitHeight: 40
        leftPadding: 20
        rightPadding: 16

        contentItem: Text {
            text: control.text
            color: control.selected ? Theme.textPrimary : Theme.textSecondary
            font.pixelSize: 14
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        background: Rectangle {
            radius: 8
            color: control.selected
                   ? Theme.surface
                   : control.hovered ? Theme.surfaceHover : "transparent"

            // Accent bar makes the active page visible without relying on color alone.
            Rectangle {
                visible: control.selected
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 3
                height: 18
                radius: width / 2
                color: Theme.accent
            }
        }

        onClicked: sidebar.pageRequested(control.page)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 28
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.bottomMargin: 24
        spacing: 4

        Text {
            text: qsTr("Desktop Hub")
            color: Theme.textPrimary
            font.pixelSize: 18
            font.bold: true
            Layout.leftMargin: 8
            Layout.bottomMargin: 24
        }

        NavButton {
            Layout.fillWidth: true
            label: qsTr("Home")
            page: 0
        }

        NavButton {
            Layout.fillWidth: true
            label: qsTr("Apps")
            page: 1
        }

        NavButton {
            Layout.fillWidth: true
            label: qsTr("Settings")
            page: 2
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
