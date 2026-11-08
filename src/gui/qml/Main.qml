import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Top-level window: a fixed sidebar on the left and the active page on the right.
ApplicationWindow {
    id: root

    // Page indexes must match the order of the pages in the StackLayout below.
    readonly property int pageHome: 0
    readonly property int pageApps: 1
    readonly property int pageSettings: 2

    property int currentPage: pageHome

    width: 1200
    height: 760
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("Desktop Hub")
    color: Theme.windowBackground

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            Layout.fillHeight: true
            currentPage: root.currentPage
            onPageRequested: (page) => root.currentPage = page
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: root.currentPage

            HomePage {}
            AppsPage {}
            SettingsPage {}
        }
    }
}
