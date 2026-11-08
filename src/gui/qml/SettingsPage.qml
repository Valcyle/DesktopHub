import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Settings page. Phase 1 covers the language switch, the fixed theme display,
// and the storage location. Category management follows in Phase 3.
Item {
    id: page

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.pagePadding
        spacing: Theme.spacing

        Text {
            text: qsTr("Settings")
            color: Theme.textPrimary
            font.pixelSize: 26
            font.bold: true
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
            Layout.fillHeight: true
        }
    }
}
