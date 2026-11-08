import QtQuick
import QtQuick.Controls

// Search field shared by Home and Apps. Filtering is paused while an IME
// composition is active, as required for Japanese input.
TextField {
    id: field

    placeholderText: qsTr("Search")
    selectByMouse: true

    // Do not bind "text" to App.searchText: an editable TextField keeps its own
    // editing state. Initialize it once and sync only while it is not focused.
    Component.onCompleted: text = App.searchText

    onTextChanged: {
        // Do not update filtering while an IME composition is in progress.
        if (!inputMethodComposing) {
            App.searchText = text
        }
    }

    onInputMethodComposingChanged: {
        if (!inputMethodComposing) {
            App.searchText = text
        }
    }

    Connections {
        target: App

        function onSearchTextChanged() {
            if (!field.activeFocus && field.text !== App.searchText) {
                field.text = App.searchText
            }
        }
    }

    Keys.onEscapePressed: {
        field.text = ""
        App.searchText = ""
    }
}
