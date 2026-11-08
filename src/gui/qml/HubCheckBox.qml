import QtQuick
import QtQuick.Controls

// Check box with an explicit check mark so the on/off state is visible in the
// dark theme without relying on color alone.
CheckBox {
    id: control

    indicator: Rectangle {
        implicitWidth: 18
        implicitHeight: 18
        x: control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        radius: 3

        color: control.checked ? Theme.accent : "transparent"
        border.width: 1.5
        border.color: control.checked ? Theme.accent : Theme.textSecondary

        Text {
            anchors.centerIn: parent
            visible: control.checked
            text: "✓"
            color: "#ffffff"
            font.pixelSize: 13
            font.bold: true
        }
    }

    contentItem: Text {
        leftPadding: control.indicator.width + control.spacing
        text: control.text
        font: control.font
        color: control.enabled ? Theme.textPrimary : Theme.textSecondary
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
    }
}
