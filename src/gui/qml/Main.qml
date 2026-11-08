import QtQuick

// Phase 0 placeholder window. The real dashboard layout is added in Phase 1.
Window {
    id: root

    width: 1200
    height: 760
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: qsTr("Desktop Hub")
    color: "#1e1f22"

    Text {
        anchors.centerIn: parent
        text: qsTr("Desktop Hub — Phase 0 build OK")
        color: "#f2f3f5"
        font.pixelSize: 20
    }
}
