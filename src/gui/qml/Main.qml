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
    property bool allowQuit: false

    width: 1200
    height: 760
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("Desktop Hub")
    color: Theme.windowBackground

    onClosing: (close) => {
        if (App.hasUnsavedChanges && !root.allowQuit) {
            close.accepted = false
            quitDialog.open()
        }
    }

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

    Connections {
        target: App.updates

        function onUpdateFound(automatic) {
            if (automatic) {
                updateDialog.open()
            }
        }

        function onErrorOccurred(message) {
            errorDialog.title = qsTr("Update failed")
            errorDialog.message = message
            errorDialog.retryable = false
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

    Shortcut {
        sequence: "Return"
        enabled: errorDialog.visible
        onActivated: errorDialog.close()
    }

    Dialog {
        id: quitDialog

        modal: true
        anchors.centerIn: parent
        width: 460
        padding: 24
        title: qsTr("Unsaved changes")
        closePolicy: Popup.CloseOnEscape

        contentItem: Text {
            width: quitDialog.availableWidth
            text: qsTr("Changes could not be saved. Quit without saving them?")
            color: Theme.textPrimary
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: quitDialog.close()
            }

            Button {
                text: qsTr("Quit")
                DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                onClicked: {
                    root.allowQuit = true
                    Qt.quit()
                }
            }
        }

        Shortcut {
            sequence: "Return"
            enabled: quitDialog.visible
            onActivated: {
                root.allowQuit = true
                Qt.quit()
            }
        }
    }

    Dialog {
        id: updateDialog

        modal: true
        anchors.centerIn: parent
        width: 540
        padding: 24
        title: qsTr("Update available")
        closePolicy: Popup.CloseOnEscape

        contentItem: ColumnLayout {
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: qsTr("Desktop Hub %1 is available. You are using %2.")
                          .arg(App.updates.latestVersion)
                          .arg(App.updates.currentVersion)
                color: Theme.textPrimary
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }

            Text {
                Layout.fillWidth: true
                visible: App.updates.releaseNotes.length > 0
                text: qsTr("Release notes")
                color: Theme.textPrimary
                font.pixelSize: 13
                font.bold: true
            }

            Text {
                Layout.fillWidth: true
                Layout.maximumHeight: 180
                visible: App.updates.releaseNotes.length > 0
                text: App.updates.releaseNotes
                color: Theme.textSecondary
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                clip: true
            }
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Later")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: updateDialog.close()
            }

            Button {
                text: qsTr("Install now")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    updateDialog.close()
                    App.updates.downloadAndInstall()
                }
            }
        }

        Shortcut {
            sequence: "Return"
            enabled: updateDialog.visible
            onActivated: {
                updateDialog.close()
                App.updates.downloadAndInstall()
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
