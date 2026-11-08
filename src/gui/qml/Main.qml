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

    // All user-facing failures are reported through this single signal.
    Connections {
        target: App

        function onErrorOccurred(title, message, retryable) {
            errorDialog.title = title
            errorDialog.message = message
            errorDialog.retryable = retryable
            errorDialog.open()
        }
    }

    Dialog {
        id: errorDialog

        property string message: ""
        property bool retryable: false

        modal: true
        anchors.centerIn: parent
        width: 480
        padding: 24
        title: qsTr("Notice")
        closePolicy: Popup.CloseOnEscape

        contentItem: Text {
            width: errorDialog.availableWidth
            text: errorDialog.message
            color: Theme.textPrimary
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Retry")
                visible: errorDialog.retryable
                DialogButtonBox.buttonRole: DialogButtonBox.ActionRole
                onClicked: {
                    errorDialog.close()
                    App.save()
                }
            }

            Button {
                text: qsTr("Close")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: errorDialog.close()
            }
        }

        // A fatal data problem (for example a newer schema version) stops the
        // application after the user has read the message.
        onClosed: {
            if (App.fatalError) {
                Qt.quit()
            }
        }
    }

    Component.onCompleted: {
        if (App.startupWarning.length > 0) {
            errorDialog.title = qsTr("Data notice")
            errorDialog.message = App.startupWarning
            errorDialog.retryable = false
            errorDialog.open()
            App.clearStartupWarning()
        }
    }
}
