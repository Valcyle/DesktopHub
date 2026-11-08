pragma Singleton

import QtQuick

// Shared visual tokens for the dark theme.
// Keeping them in one place makes later UI work consistent.
QtObject {
    readonly property color windowBackground: "#1e1f22"
    readonly property color sidebarBackground: "#17181a"
    readonly property color surface: "#26272b"
    readonly property color surfaceHover: "#2f3136"
    readonly property color border: "#34363b"
    readonly property color textPrimary: "#f2f3f5"
    readonly property color textSecondary: "#a3a7ae"
    readonly property color accent: "#4c8dff"

    readonly property int sidebarWidth: 220
    readonly property int pagePadding: 28
    readonly property int cardRadius: 12
    readonly property int spacing: 16
    readonly property int smallSpacing: 8
}
