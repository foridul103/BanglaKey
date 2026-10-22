pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 900
    height: 640
    minimumWidth: 780
    minimumHeight: 540
    title: qsTr("BanglaKey Settings")

    // Context property reference for typed/qualified access
    property var backend: null

    // System palette detection for genuine KDE Plasma theme integration
    SystemPalette {
        id: sysPalette
        colorGroup: SystemPalette.Active
    }

    // Dynamic theme colors that respect KDE system theme, or user dark/light preference
    readonly property bool isDark: {
        if (root.backend && root.backend.appearance === "dark") return true;
        if (root.backend && root.backend.appearance === "light") return false;
        // When system: evaluate luminance of sysPalette.window
        return (sysPalette.window.r * 0.299 + sysPalette.window.g * 0.587 + sysPalette.window.b * 0.114) < 0.5;
    }

    readonly property color windowBg: (root.backend && root.backend.appearance === "system")
        ? sysPalette.window
        : (isDark ? "#1e1e2e" : "#f6f8fa")

    readonly property color sidebarBg: (root.backend && root.backend.appearance === "system")
        ? (isDark ? Qt.darker(sysPalette.window, 1.08) : Qt.darker(sysPalette.window, 1.04))
        : (isDark ? "#181825" : "#ebeef2")

    readonly property color cardBg: (root.backend && root.backend.appearance === "system")
        ? (isDark ? Qt.lighter(sysPalette.window, 1.12) : "#ffffff")
        : (isDark ? "#2a2b40" : "#ffffff")

    readonly property color cardBorder: (root.backend && root.backend.appearance === "system")
        ? sysPalette.mid
        : (isDark ? "#393b56" : "#d0d7de")

    readonly property color surfaceBg: (root.backend && root.backend.appearance === "system")
        ? sysPalette.base
        : (isDark ? "#252538" : "#ffffff")

    readonly property color textPrimary: (root.backend && root.backend.appearance === "system")
        ? sysPalette.windowText
        : (isDark ? "#cdd6f4" : "#1f2328")

    readonly property color textSecondary: (root.backend && root.backend.appearance === "system")
        ? Qt.rgba(sysPalette.windowText.r, sysPalette.windowText.g, sysPalette.windowText.b, 0.7)
        : (isDark ? "#a6adc8" : "#656d76")

    readonly property color accentColor: (root.backend && root.backend.appearance === "system")
        ? sysPalette.highlight
        : (isDark ? "#89b4fa" : "#0969da")

    readonly property color accentText: (root.backend && root.backend.appearance === "system")
        ? sysPalette.highlightedText
        : "#ffffff"

    readonly property color successColor: isDark ? "#a6e3a1" : "#1a7f37"
    readonly property color warningColor: isDark ? "#f9e2af" : "#9a6700"
    readonly property color errorColor: isDark ? "#f38ba8" : "#cf222e"

    color: root.windowBg

    // Current navigation section index
    property int currentSection: 0

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ==========================================
        // 1. LEFT SIDEBAR NAVIGATION
        // ==========================================
        Rectangle {
            Layout.preferredWidth: 230
            Layout.fillHeight: true
            color: root.sidebarBg
            border.color: root.cardBorder
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                // Brand Header
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Rectangle {
                        Layout.preferredWidth: 42
                        Layout.preferredHeight: 42
                        radius: 10
                        color: Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.18)
                        border.color: root.accentColor
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "ক"
                            font.pixelSize: 22
                            font.bold: true
                            color: root.accentColor
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        Text {
                            text: qsTr("BanglaKey")
                            font.pixelSize: 17
                            font.bold: true
                            color: root.textPrimary
                        }
                        Text {
                            text: qsTr("Linux • KDE Plasma")
                            font.pixelSize: 11
                            color: root.textSecondary
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: root.cardBorder
                }

                // Nav Buttons
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Repeater {
                        model: [
                            { name: qsTr("Dashboard"), icon: "📊" },
                            { name: qsTr("Typing Modes"), icon: "⌨️" },
                            { name: qsTr("Layout Preview"), icon: "🗺️" },
                            { name: qsTr("Preferences"), icon: "⚙️" },
                            { name: qsTr("Diagnostics"), icon: "🔍" }
                        ]

                        Rectangle {
                            id: navItem
                            required property var modelData
                            required property int index
                            Layout.fillWidth: true
                            Layout.preferredHeight: 38
                            radius: 8
                            readonly property bool isSelected: root.currentSection === index
                            color: isSelected
                                ? Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.22)
                                : (navMouse.containsMouse ? Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.08) : "transparent")
                            border.color: isSelected ? root.accentColor : "transparent"
                            border.width: 1

                            MouseArea {
                                id: navMouse
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor
                                onClicked: root.currentSection = navItem.index
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12
                                spacing: 10

                                Text {
                                    text: navItem.modelData.icon
                                    font.pixelSize: 14
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: navItem.modelData.name
                                    font.pixelSize: 13
                                    font.bold: navItem.isSelected
                                    color: navItem.isSelected ? root.accentColor : root.textPrimary
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Quick Status Footer Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 88
                    radius: 10
                    color: root.cardBg
                    border.color: root.cardBorder
                    border.width: 1

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Active Mode:")
                                font.pixelSize: 11
                                color: root.textSecondary
                            }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                Layout.preferredWidth: 64
                                Layout.preferredHeight: 20
                                radius: 4
                                color: Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.2)
                                Text {
                                    anchors.centerIn: parent
                                    text: root.backend ? root.backend.currentMode : "Phonetic"
                                    font.pixelSize: 10
                                    font.bold: true
                                    color: root.accentColor
                                }
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: qsTr("Fcitx5 Service:")
                                font.pixelSize: 11
                                color: root.textSecondary
                            }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                Layout.preferredWidth: 8
                                Layout.preferredHeight: 8
                                radius: 4
                                color: (root.backend && root.backend.fcitx5Running) ? root.successColor : root.warningColor
                            }
                            Text {
                                text: (root.backend && root.backend.fcitx5Running) ? qsTr("Running") : qsTr("Stopped")
                                font.pixelSize: 10
                                font.bold: true
                                color: (root.backend && root.backend.fcitx5Running) ? root.successColor : root.warningColor
                            }
                        }

                        Text {
                            text: "v" + (root.backend ? root.backend.version : "0.1.0")
                            font.pixelSize: 10
                            color: root.textSecondary
                        }
                    }
                }
            }
        }

        // ==========================================
        // 2. MAIN CONTENT AREA (STACK LAYOUT)
        // ==========================================
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // Header Banner / Status Bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 52
                color: root.windowBg
                border.color: root.cardBorder
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 24
                    anchors.rightMargin: 24
                    spacing: 12

                    Text {
                        text: {
                            switch (root.currentSection) {
                                case 0: return qsTr("Dashboard & Status");
                                case 1: return qsTr("Typing Mode & Interactive Sandbox");
                                case 2: return qsTr("Keyboard Layout Preview");
                                case 3: return qsTr("Preferences & Configuration");
                                case 4: return qsTr("System Diagnostics");
                                default: return qsTr("Settings");
                            }
                        }
                        font.pixelSize: 16
                        font.bold: true
                        color: root.textPrimary
                    }

                    Item { Layout.fillWidth: true }

                    // Status Message Display
                    Rectangle {
                        visible: root.backend ? (root.backend.statusMessage.length > 0) : false
                        Layout.preferredHeight: 28
                        Layout.preferredWidth: statusText.implicitWidth + 24
                        radius: 14
                        color: Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.15)
                        border.color: root.accentColor
                        border.width: 1

                        Text {
                            id: statusText
                            anchors.centerIn: parent
                            text: root.backend ? root.backend.statusMessage : ""
                            font.pixelSize: 11
                            color: root.textPrimary
                        }
                    }
                }
            }

            // StackLayout for Views
            StackLayout {
                id: mainStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: root.currentSection

                // ------------------------------------------
                // VIEW 0: DASHBOARD
                // ------------------------------------------
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 20

                        // Status Cards Grid
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            // Card 1: Typing Engine
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 110
                                radius: 12
                                color: root.cardBg
                                border.color: root.cardBorder
                                border.width: 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 6

                                    RowLayout {
                                        Text { text: "⚙️"; font.pixelSize: 16 }
                                        Text { text: qsTr("Core Engine"); font.bold: true; color: root.textPrimary; font.pixelSize: 14 }
                                    }
                                    Text {
                                        text: (root.backend && root.backend.engineAvailable) ? qsTr("C++ Engine Active") : qsTr("Unavailable")
                                        color: (root.backend && root.backend.engineAvailable) ? root.successColor : root.errorColor
                                        font.bold: true
                                        font.pixelSize: 13
                                    }
                                    Text {
                                        text: qsTr("Bijoy & Phonetic modes built-in")
                                        color: root.textSecondary
                                        font.pixelSize: 11
                                    }
                                }
                            }

                            // Card 2: Fcitx5 Addon Integration
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 110
                                radius: 12
                                color: root.cardBg
                                border.color: root.cardBorder
                                border.width: 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 6

                                    RowLayout {
                                        Text { text: "🔌"; font.pixelSize: 16 }
                                        Text { text: qsTr("Fcitx5 Addon"); font.bold: true; color: root.textPrimary; font.pixelSize: 14 }
                                    }
                                    Text {
                                        text: (root.backend && root.backend.fcitx5Installed) ? qsTr("Addon Detected") : qsTr("Not In Standard Path")
                                        color: (root.backend && root.backend.fcitx5Installed) ? root.successColor : root.warningColor
                                        font.bold: true
                                        font.pixelSize: 13
                                    }
                                    Text {
                                        text: (root.backend && root.backend.fcitx5Running) ? qsTr("Daemon active (ready to type)") : qsTr("Fcitx5 daemon stopped")
                                        color: root.textSecondary
                                        font.pixelSize: 11
                                    }
                                }
                            }

                            // Card 3: Saved Typing Mode
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 110
                                radius: 12
                                color: root.cardBg
                                border.color: root.cardBorder
                                border.width: 1

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 16
                                    spacing: 6

                                    RowLayout {
                                        Text { text: "📝"; font.pixelSize: 16 }
                                        Text { text: qsTr("Configured Mode"); font.bold: true; color: root.textPrimary; font.pixelSize: 14 }
                                    }
                                    Text {
                                        text: root.backend ? root.backend.currentMode : "Phonetic"
                                        color: root.accentColor
                                        font.bold: true
                                        font.pixelSize: 13
                                    }
                                    Text {
                                        text: qsTr("Switch in apps via %1").arg(root.backend ? root.backend.modeSwitchShortcut : "F12")
                                        color: root.textSecondary
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }

                        // Mode Quick Select Card
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 130
                            radius: 12
                            color: root.cardBg
                            border.color: root.cardBorder
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10

                                Text {
                                    text: qsTr("Choose Typing Mode for BanglaKey")
                                    font.bold: true
                                    font.pixelSize: 14
                                    color: root.textPrimary
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Button {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 46
                                        text: qsTr("🔤 Phonetic (English to Bengali)")
                                        highlighted: root.backend && root.backend.currentMode === "Phonetic"
                                        onClicked: if (root.backend) root.backend.currentMode = "Phonetic"
                                    }

                                    Button {
                                        Layout.fillWidth: true
                                        Layout.preferredHeight: 46
                                        text: qsTr("⌨️ Bijoy-Compatible Layout")
                                        highlighted: root.backend && root.backend.currentMode === "Bijoy"
                                        onClicked: if (root.backend) root.backend.currentMode = "Bijoy"
                                    }
                                }

                                Text {
                                    text: qsTr("Preferences are saved to %1 and loaded automatically by the Fcitx5 addon.").arg(root.backend ? root.backend.configFilePath : "")
                                    font.pixelSize: 11
                                    color: root.textSecondary
                                }
                            }
                        }

                        // Fedora / KDE Plasma Setup Helper Card
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 170
                            radius: 12
                            color: root.cardBg
                            border.color: root.cardBorder
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 8

                                RowLayout {
                                    Text { text: "💡"; font.pixelSize: 16 }
                                    Text {
                                        text: qsTr("Fedora Linux 44 & KDE Plasma 6 Setup Guide")
                                        font.bold: true
                                        font.pixelSize: 14
                                        color: root.textPrimary
                                    }
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: qsTr("1. Ensure Fcitx5 is installed: <code>sudo dnf install fcitx5 fcitx5-configtool</code><br>" +
                                               "2. Open <b>KDE System Settings</b> → <b>Input Devices</b> → <b>Virtual Keyboard</b> and select <b>Fcitx 5</b>.<br>" +
                                               "3. In Fcitx5 Configuration, add <b>BanglaKey</b> under your active Input Methods.<br>" +
                                               "4. Press <b>Ctrl+Space</b> to activate Bengali typing, and press <b>%1</b> to switch between Bijoy and Phonetic modes.").arg(root.backend ? root.backend.modeSwitchShortcut : "F12")
                                    font.pixelSize: 12
                                    lineHeight: 1.3
                                    textFormat: Text.RichText
                                    color: root.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                // ------------------------------------------
                // VIEW 1: TYPING MODES & SANDBOX
                // ------------------------------------------
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 20

                        // Mode Comparison
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            // Phonetic Card
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 110
                                radius: 12
                                color: (root.backend && root.backend.currentMode === "Phonetic")
                                    ? Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.15)
                                    : root.cardBg
                                border.color: (root.backend && root.backend.currentMode === "Phonetic") ? root.accentColor : root.cardBorder
                                border.width: (root.backend && root.backend.currentMode === "Phonetic") ? 2 : 1

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: if (root.backend) root.backend.currentMode = "Phonetic"
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 4

                                    RowLayout {
                                        Text { text: "🔤"; font.pixelSize: 18 }
                                        Text { text: qsTr("Phonetic Mode"); font.bold: true; font.pixelSize: 14; color: root.textPrimary }
                                        Item { Layout.fillWidth: true }
                                        Text {
                                            visible: root.backend && root.backend.currentMode === "Phonetic"
                                            text: qsTr("SAVED DEFAULT")
                                            font.pixelSize: 9
                                            font.bold: true
                                            color: root.accentColor
                                        }
                                    }
                                    Text {
                                        text: qsTr("Type intuitively using English keys (e.g., 'amar' → 'আমার', 'bangla' → 'বাংলা').")
                                        font.pixelSize: 11
                                        color: root.textSecondary
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }
                            }

                            // Bijoy Card
                            Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 110
                                radius: 12
                                color: (root.backend && root.backend.currentMode === "Bijoy")
                                    ? Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.15)
                                    : root.cardBg
                                border.color: (root.backend && root.backend.currentMode === "Bijoy") ? root.accentColor : root.cardBorder
                                border.width: (root.backend && root.backend.currentMode === "Bijoy") ? 2 : 1

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: if (root.backend) root.backend.currentMode = "Bijoy"
                                }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 14
                                    spacing: 4

                                    RowLayout {
                                        Text { text: "⌨️"; font.pixelSize: 18 }
                                        Text { text: qsTr("Bijoy Compatible"); font.bold: true; font.pixelSize: 14; color: root.textPrimary }
                                        Item { Layout.fillWidth: true }
                                        Text {
                                            visible: root.backend && root.backend.currentMode === "Bijoy"
                                            text: qsTr("SAVED DEFAULT")
                                            font.pixelSize: 9
                                            font.bold: true
                                            color: root.accentColor
                                        }
                                    }
                                    Text {
                                        text: qsTr("Standard layout mapping with 'g' as conjunct link key and pre-vowel reordering.")
                                        font.pixelSize: 11
                                        color: root.textSecondary
                                        wrapMode: Text.WordWrap
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                        }

                        // Decoupling Notice
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 36
                            radius: 6
                            color: Qt.rgba(root.accentColor.r, root.accentColor.g, root.accentColor.b, 0.08)
                            border.color: root.cardBorder
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8
                                Text { text: "ℹ️"; font.pixelSize: 12 }
                                Text {
                                    text: qsTr("Selected mode is saved to disk as the default. When using Fcitx5, toggle mode anytime with %1.").arg(root.backend ? root.backend.modeSwitchShortcut : "F12")
                                    font.pixelSize: 11
                                    color: root.textSecondary
                                }
                            }
                        }

                        // Interactive Sandbox Area
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 320
                            radius: 12
                            color: root.cardBg
                            border.color: root.cardBorder
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                spacing: 10

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("Interactive Typing Sandbox")
                                        font.bold: true
                                        font.pixelSize: 14
                                        color: root.textPrimary
                                    }
                                    Item { Layout.fillWidth: true }
                                    Button {
                                        text: qsTr("Clear Sandbox")
                                        onClicked: {
                                            if (root.backend) root.backend.clearSandbox();
                                            sandboxInput.text = "";
                                            sandboxInput.forceActiveFocus();
                                        }
                                    }
                                }

                                Text {
                                    text: qsTr("Test typing directly inside this window in %1 mode without modifying any system settings:").arg(root.backend ? root.backend.currentMode : "Phonetic")
                                    font.pixelSize: 11
                                    color: root.textSecondary
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    radius: 8
                                    color: root.surfaceBg
                                    border.color: sandboxInput.activeFocus ? root.accentColor : root.cardBorder
                                    border.width: sandboxInput.activeFocus ? 2 : 1

                                    ScrollView {
                                        anchors.fill: parent
                                        anchors.margins: 10

                                        TextArea {
                                            id: sandboxInput
                                            placeholderText: qsTr("Click here and type to test %1 mode...").arg(root.backend ? root.backend.currentMode : "")
                                            placeholderTextColor: root.textSecondary
                                            color: root.textPrimary
                                            font.pixelSize: 16
                                            wrapMode: TextArea.Wrap
                                            focus: true

                                            Keys.onPressed: function(event) {
                                                if (!root.backend) return;
                                                var unicodeVal = (event.text.length > 0) ? event.text.charCodeAt(0) : 0;
                                                root.backend.processSandboxKey(event.nativeScanCode, event.key, unicodeVal, event.modifiers, false);

                                                if (root.backend.sandboxCommit.length > 0 || root.backend.sandboxPreedit.length > 0) {
                                                    sandboxInput.text = root.backend.sandboxCommit + (root.backend.sandboxPreedit.length > 0 ? " [" + root.backend.sandboxPreedit + "]" : "");
                                                    sandboxInput.cursorPosition = sandboxInput.text.length;
                                                    event.accepted = true;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // ------------------------------------------
                // VIEW 2: LAYOUT PREVIEW
                // ------------------------------------------
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 16

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            TabBar {
                                id: layoutTabBar
                                Layout.preferredWidth: 320
                                TabButton { text: qsTr("Bijoy Layout") }
                                TabButton { text: qsTr("Phonetic Rules") }
                            }

                            Item { Layout.fillWidth: true }

                            TextField {
                                id: keySearchFilter
                                Layout.preferredWidth: 200
                                placeholderText: qsTr("Filter mappings...")
                            }
                        }

                        // Mappings Container
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 12
                            color: root.cardBg
                            border.color: root.cardBorder
                            border.width: 1

                            StackLayout {
                                anchors.fill: parent
                                anchors.margins: 16
                                currentIndex: layoutTabBar.currentIndex

                                // Tab 0: Bijoy Keymap
                                ScrollView {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true

                                    Flow {
                                        width: parent.width
                                        spacing: 10

                                        Repeater {
                                            model: {
                                                if (!root.backend) return [];
                                                var map = root.backend.getBijoyKeymap();
                                                var list = [];
                                                var filter = keySearchFilter.text.toLowerCase();
                                                for (var k in map) {
                                                    var val = map[k];
                                                    if (filter.length === 0 || k.toLowerCase().indexOf(filter) !== -1 || val.indexOf(filter) !== -1) {
                                                        list.push({ key: k, bangla: val });
                                                    }
                                                }
                                                return list;
                                            }

                                            Rectangle {
                                                id: bijoyCell
                                                required property var modelData
                                                width: 80
                                                height: 54
                                                radius: 8
                                                color: root.surfaceBg
                                                border.color: root.cardBorder
                                                border.width: 1

                                                ColumnLayout {
                                                    anchors.centerIn: parent
                                                    spacing: 2
                                                    Text {
                                                        text: bijoyCell.modelData.key
                                                        font.bold: true
                                                        font.pixelSize: 12
                                                        color: root.accentColor
                                                        Layout.alignment: Qt.AlignHCenter
                                                    }
                                                    Text {
                                                        text: bijoyCell.modelData.bangla
                                                        font.pixelSize: 15
                                                        color: root.textPrimary
                                                        Layout.alignment: Qt.AlignHCenter
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }

                                // Tab 1: Phonetic Rules
                                ScrollView {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true

                                    Flow {
                                        width: parent.width
                                        spacing: 10

                                        Repeater {
                                            model: {
                                                if (!root.backend) return [];
                                                var map = root.backend.getPhoneticKeymap();
                                                var list = [];
                                                var filter = keySearchFilter.text.toLowerCase();
                                                for (var k in map) {
                                                    var val = map[k];
                                                    if (filter.length === 0 || k.toLowerCase().indexOf(filter) !== -1 || val.indexOf(filter) !== -1) {
                                                        list.push({ key: k, bangla: val });
                                                    }
                                                }
                                                return list;
                                            }

                                            Rectangle {
                                                id: phoneticCell
                                                required property var modelData
                                                width: 90
                                                height: 54
                                                radius: 8
                                                color: root.surfaceBg
                                                border.color: root.cardBorder
                                                border.width: 1

                                                ColumnLayout {
                                                    anchors.centerIn: parent
                                                    spacing: 2
                                                    Text {
                                                        text: phoneticCell.modelData.key
                                                        font.bold: true
                                                        font.pixelSize: 12
                                                        color: root.accentColor
                                                        Layout.alignment: Qt.AlignHCenter
                                                    }
                                                    Text {
                                                        text: phoneticCell.modelData.bangla
                                                        font.pixelSize: 15
                                                        color: root.textPrimary
                                                        Layout.alignment: Qt.AlignHCenter
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // ------------------------------------------
                // VIEW 3: PREFERENCES
                // ------------------------------------------
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 20

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 330
                            radius: 12
                            color: root.cardBg
                            border.color: root.cardBorder
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 16

                                Text {
                                    text: qsTr("User Preferences")
                                    font.bold: true
                                    font.pixelSize: 15
                                    color: root.textPrimary
                                }

                                // Appearance
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("Desktop Appearance:")
                                        font.pixelSize: 13
                                        color: root.textPrimary
                                        Layout.preferredWidth: 200
                                    }
                                    ComboBox {
                                        id: appearanceCombo
                                        Layout.preferredWidth: 240
                                        model: [
                                            { text: qsTr("System (Follow KDE Plasma)"), value: "system" },
                                            { text: qsTr("Dark Mode"), value: "dark" },
                                            { text: qsTr("Light Mode"), value: "light" }
                                        ]
                                        textRole: "text"
                                        currentIndex: {
                                            if (!root.backend) return 0;
                                            if (root.backend.appearance === "dark") return 1;
                                            if (root.backend.appearance === "light") return 2;
                                            return 0;
                                        }
                                        onActivated: function(index) {
                                            if (root.backend) {
                                                root.backend.appearance = appearanceCombo.model[index].value;
                                            }
                                        }
                                    }
                                }

                                // Interface Language
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("Interface Language:")
                                        font.pixelSize: 13
                                        color: root.textPrimary
                                        Layout.preferredWidth: 200
                                    }
                                    ComboBox {
                                        id: langCombo
                                        Layout.preferredWidth: 240
                                        model: [
                                            { text: qsTr("English (en)"), value: "en" },
                                            { text: qsTr("বাংলা (bn)"), value: "bn" }
                                        ]
                                        textRole: "text"
                                        currentIndex: (root.backend && root.backend.interfaceLanguage === "bn") ? 1 : 0
                                        onActivated: function(index) {
                                            if (root.backend) {
                                                root.backend.interfaceLanguage = langCombo.model[index].value;
                                            }
                                        }
                                    }
                                }

                                // Mode Switch Shortcut
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("Mode Switch Shortcut:")
                                        font.pixelSize: 13
                                        color: root.textPrimary
                                        Layout.preferredWidth: 200
                                    }
                                    ComboBox {
                                        id: shortcutCombo
                                        Layout.preferredWidth: 240
                                        model: ["F12", "Ctrl+Alt+M"]
                                        currentIndex: (root.backend && root.backend.modeSwitchShortcut === "Ctrl+Alt+M") ? 1 : 0
                                        onActivated: function(index) {
                                            if (root.backend) {
                                                root.backend.modeSwitchShortcut = shortcutCombo.model[index];
                                            }
                                        }
                                    }
                                }

                                // Autostart CheckBox
                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("Start on System Login:")
                                        font.pixelSize: 13
                                        color: root.textPrimary
                                        Layout.preferredWidth: 200
                                    }
                                    CheckBox {
                                        text: qsTr("Enable autostart desktop entry")
                                        checked: root.backend ? root.backend.autostart : false
                                        onToggled: {
                                            if (root.backend) root.backend.autostart = checked;
                                        }
                                    }
                                }

                                Item { Layout.fillHeight: true }

                                // Actions Row
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 12

                                    Button {
                                        text: qsTr("Save Preferences")
                                        highlighted: true
                                        onClicked: {
                                            if (root.backend) root.backend.saveConfig();
                                        }
                                    }

                                    Button {
                                        text: qsTr("Restore Defaults")
                                        onClicked: restoreDialog.open()
                                    }
                                }
                            }
                        }

                        Item { Layout.fillHeight: true }
                    }
                }

                // ------------------------------------------
                // VIEW 4: DIAGNOSTICS
                // ------------------------------------------
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: availableWidth

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 24
                        spacing: 20

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 380
                            radius: 12
                            color: root.cardBg
                            border.color: root.cardBorder
                            border.width: 1

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 20
                                spacing: 14

                                RowLayout {
                                    Layout.fillWidth: true
                                    Text {
                                        text: qsTr("BanglaKey System Diagnostics")
                                        font.bold: true
                                        font.pixelSize: 15
                                        color: root.textPrimary
                                    }
                                    Item { Layout.fillWidth: true }
                                    Button {
                                        text: qsTr("Refresh Diagnostics")
                                        onClicked: {
                                            if (root.backend) root.backend.refreshDiagnostics();
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    color: root.cardBorder
                                }

                                // Checklist
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 10

                                    // Item 1: Version
                                    RowLayout {
                                        Text { text: "📌"; font.pixelSize: 14 }
                                        Text { text: qsTr("BanglaKey Version: "); font.bold: true; color: root.textPrimary }
                                        Text { text: root.backend ? root.backend.version : ""; color: root.accentColor }
                                    }

                                    // Item 2: Core Engine
                                    RowLayout {
                                        Text { text: (root.backend && root.backend.engineAvailable) ? "✅" : "❌"; font.pixelSize: 14 }
                                        Text { text: qsTr("Core Typing Engine: "); font.bold: true; color: root.textPrimary }
                                        Text {
                                            text: (root.backend && root.backend.engineAvailable) ? qsTr("Available (In-process C++ Engine)") : qsTr("Unavailable")
                                            color: (root.backend && root.backend.engineAvailable) ? root.successColor : root.errorColor
                                        }
                                    }

                                    // Item 3: Fcitx5 Addon Installation
                                    RowLayout {
                                        Text { text: (root.backend && root.backend.fcitx5Installed) ? "✅" : "⚠️"; font.pixelSize: 14 }
                                        Text { text: qsTr("Fcitx5 Addon Library: "); font.bold: true; color: root.textPrimary }
                                        Text {
                                            text: (root.backend && root.backend.fcitx5Installed) ? qsTr("Installed in system/local library path") : qsTr("Not detected in standard paths")
                                            color: (root.backend && root.backend.fcitx5Installed) ? root.successColor : root.warningColor
                                        }
                                    }

                                    // Item 4: Fcitx5 Daemon Process
                                    RowLayout {
                                        Text { text: (root.backend && root.backend.fcitx5Running) ? "✅" : "⚠️"; font.pixelSize: 14 }
                                        Text { text: qsTr("Fcitx5 Daemon: "); font.bold: true; color: root.textPrimary }
                                        Text {
                                            text: (root.backend && root.backend.fcitx5Running) ? qsTr("Running (PID detected)") : qsTr("Not running (start with: fcitx5 -d)")
                                            color: (root.backend && root.backend.fcitx5Running) ? root.successColor : root.warningColor
                                        }
                                    }

                                    // Item 5: Config File
                                    RowLayout {
                                        Text { text: (root.backend && root.backend.configFileValid) ? "✅" : "❌"; font.pixelSize: 14 }
                                        Text { text: qsTr("Configuration File: "); font.bold: true; color: root.textPrimary }
                                        Text {
                                            text: root.backend ? root.backend.configFilePath : ""
                                            color: root.textSecondary
                                            elide: Text.ElideMiddle
                                            Layout.maximumWidth: 350
                                        }
                                    }
                                }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 1
                                    color: root.cardBorder
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: qsTr("Privacy Notice: BanglaKey processes all keystrokes strictly on your local machine. No keystrokes, personal text, or telemetry are ever recorded, saved to disk, or sent over a network.")
                                    font.pixelSize: 11
                                    color: root.textSecondary
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }

                    }
                }
            }
        }
    }

    // Confirmation Dialog for Restore Defaults
    Dialog {
        id: restoreDialog
        title: qsTr("Restore Default Settings?")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: Math.round((root.width - width) / 2)
        y: Math.round((root.height - height) / 2)

        Text {
            text: qsTr("Are you sure you want to restore all settings to their default values?")
            color: root.textPrimary
            font.pixelSize: 13
        }

        onAccepted: {
            if (root.backend) root.backend.restoreDefaults();
        }
    }
}
