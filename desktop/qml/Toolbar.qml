pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: toolbarWindow
    visible: true
    width: 460
    height: dropdownOpen ? 210 : 38
    minimumWidth: 420
    maximumWidth: 500
    minimumHeight: 38
    maximumHeight: 220
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"

    property var backend: null
    property var ipcManager: null
    property bool dropdownOpen: false

    signal hideRequested()
    signal closeRequested()
    signal openSettingsRequested()

    Component.onCompleted: {
        var screenW = Screen.width > 0 ? Screen.width : 1920
        var targetX = Math.max(0, (screenW - toolbarWindow.width) / 2)
        var targetY = 36

        if (toolbarWindow.backend) {
            var sx = toolbarWindow.backend.windowX()
            var sy = toolbarWindow.backend.windowY()
            if (sy > 0 && sy < Screen.height - 100) targetY = sy
            if (sx > 0 && sx < screenW - toolbarWindow.width) targetX = sx
        }
        toolbarWindow.x = targetX
        toolbarWindow.y = targetY
    }

    // Top horizontal bar frame (38px high)
    Rectangle {
        id: toolbarFrame
        width: parent.width
        height: 38
        radius: 6
        border.color: "#0f3e7a"
        border.width: 1

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#256ec7" }
            GradientStop { position: 0.15; color: "#195cb0" }
            GradientStop { position: 0.85; color: "#10478e" }
            GradientStop { position: 1.0; color: "#0c3b77" }
        }

        // Native compositor move on drag
        MouseArea {
            id: dragArea
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor
            acceptedButtons: Qt.LeftButton

            onPressed: function(mouse) {
                toolbarWindow.startSystemMove()
            }

            onReleased: {
                if (toolbarWindow.backend) {
                    toolbarWindow.backend.saveWindowPosition(toolbarWindow.x, toolbarWindow.y)
                }
            }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 6

            // 1. Logo / Emblem
            Rectangle {
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 12
                color: "#006a4e"
                border.color: "#4ade80"
                border.width: 1

                Rectangle {
                    anchors.centerIn: parent
                    width: 16
                    height: 16
                    radius: 8
                    color: "#f42a41"

                    Text {
                        anchors.centerIn: parent
                        text: "ক"
                        color: "#ffffff"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            // 2. Mode Selector Button (Click opens full Dropdown menu)
            Rectangle {
                id: modeBtn
                Layout.preferredWidth: 125
                Layout.preferredHeight: 26
                radius: 4
                color: modeBtnMouse.containsMouse ? "#f1f5f9" : "#ffffff"
                border.color: toolbarWindow.dropdownOpen ? "#38bdf8" : "#94a3b8"
                border.width: 1.5

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 4

                    Text {
                        Layout.fillWidth: true
                        text: {
                            if (!toolbarWindow.ipcManager) return "English"
                            var cur = toolbarWindow.ipcManager.currentModeString
                            if (cur === "Bijoy Classic") return "বিজয় ক্লাসিক"
                            if (cur === "Bijoy Unicode") return "বিজয় ইউনিকোড"
                            if (cur === "Unicode") return "ইউনিকোড"
                            return "English"
                        }
                        font.pixelSize: 11
                        font.bold: true
                        color: "#0f172a"
                        elide: Text.ElideRight
                    }

                    Text {
                        text: toolbarWindow.dropdownOpen ? "▲" : "▼"
                        font.pixelSize: 9
                        color: "#64748b"
                    }
                }

                MouseArea {
                    id: modeBtnMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        toolbarWindow.dropdownOpen = !toolbarWindow.dropdownOpen
                    }
                }

                ToolTip.visible: modeBtnMouse.containsMouse && !toolbarWindow.dropdownOpen
                ToolTip.text: qsTr("Click to choose Bengali / English layout")
                ToolTip.delay: 300
            }

            // 3. Quick-Switch Mode Buttons (1-Click Switch Directly on Toolbar)
            RowLayout {
                spacing: 3

                // English
                Rectangle {
                    id: qbEng
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "English"
                    color: active ? "#ffffff" : (qbEngM.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(1, 1, 1, 0.12))
                    border.color: active ? "#38bdf8" : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "EN"
                        font.pixelSize: 10
                        font.bold: true
                        color: qbEng.active ? "#1565C0" : "#ffffff"
                    }

                    MouseArea {
                        id: qbEngM
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            toolbarWindow.dropdownOpen = false
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("English")
                        }
                    }

                    ToolTip.visible: qbEngM.containsMouse
                    ToolTip.text: qsTr("English Mode")
                    ToolTip.delay: 300
                }

                // Bijoy Classic
                Rectangle {
                    id: qbClassic
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Bijoy Classic"
                    color: active ? "#f59e0b" : (qbClassicM.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(1, 1, 1, 0.12))
                    border.color: active ? "#ffffff" : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "ক্লাসিক"
                        font.pixelSize: 10
                        font.bold: true
                        color: "#ffffff"
                    }

                    MouseArea {
                        id: qbClassicM
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            toolbarWindow.dropdownOpen = false
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Bijoy Classic")
                        }
                    }

                    ToolTip.visible: qbClassicM.containsMouse
                    ToolTip.text: qsTr("Bijoy Classic (Ctrl+B)")
                    ToolTip.delay: 300
                }

                // Bijoy Unicode
                Rectangle {
                    id: qbBijoyUni
                    Layout.preferredWidth: 44
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Bijoy Unicode"
                    color: active ? "#10b981" : (qbBijoyUniM.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(1, 1, 1, 0.12))
                    border.color: active ? "#ffffff" : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "বি-ইউ"
                        font.pixelSize: 10
                        font.bold: true
                        color: "#ffffff"
                    }

                    MouseArea {
                        id: qbBijoyUniM
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            toolbarWindow.dropdownOpen = false
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Bijoy Unicode")
                        }
                    }

                    ToolTip.visible: qbBijoyUniM.containsMouse
                    ToolTip.text: qsTr("Bijoy Unicode (Ctrl+Alt+B)")
                    ToolTip.delay: 300
                }

                // Unicode (Phonetic)
                Rectangle {
                    id: qbUni
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Unicode"
                    color: active ? "#06b6d4" : (qbUniM.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : Qt.rgba(1, 1, 1, 0.12))
                    border.color: active ? "#ffffff" : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "ইউনিকোড"
                        font.pixelSize: 10
                        font.bold: true
                        color: "#ffffff"
                    }

                    MouseArea {
                        id: qbUniM
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            toolbarWindow.dropdownOpen = false
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Unicode")
                        }
                    }

                    ToolTip.visible: qbUniM.containsMouse
                    ToolTip.text: qsTr("Unicode / Phonetic (Ctrl+V)")
                    ToolTip.delay: 300
                }
            }

            Item { Layout.fillWidth: true }

            // 4. Dedicated System Tray Minimize Button
            Rectangle {
                id: btnTray
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 3
                color: trayMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(1, 1, 1, 0.12)
                border.color: Qt.rgba(1, 1, 1, 0.2)
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "⤓"
                    color: "#ffffff"
                    font.pixelSize: 13
                    font.bold: true
                }

                MouseArea {
                    id: trayMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        toolbarWindow.dropdownOpen = false
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.saveWindowVisible(false)
                        }
                        toolbarWindow.hide()
                        toolbarWindow.hideRequested()
                    }
                }

                ToolTip.visible: trayMouse.containsMouse
                ToolTip.text: qsTr("Minimize to System Tray (ট্রে-তে পাঠান)")
                ToolTip.delay: 300
            }

            // 5. Settings button (gear)
            Rectangle {
                id: btnSettings
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 3
                color: settingsMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "⚙"
                    color: "#ffffff"
                    font.pixelSize: 12
                }

                MouseArea {
                    id: settingsMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        toolbarWindow.dropdownOpen = false
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.requestOpenSettings()
                        }
                        toolbarWindow.openSettingsRequested()
                    }
                }

                ToolTip.visible: settingsMouse.containsMouse
                ToolTip.text: qsTr("Settings & Diagnostics (সেটিংস)")
                ToolTip.delay: 300
            }

            // 6. Close button (✕)
            Rectangle {
                id: btnClose
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 3
                color: closeMouse.containsMouse ? "#dc2626" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: "#ffffff"
                    font.pixelSize: 11
                    font.bold: true
                }

                MouseArea {
                    id: closeMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        toolbarWindow.dropdownOpen = false
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.requestExit()
                        } else {
                            toolbarWindow.close()
                        }
                    }
                }

                ToolTip.visible: closeMouse.containsMouse
                ToolTip.text: qsTr("Exit BanglaKey (অ্যাপ বন্ধ করুন)")
                ToolTip.delay: 300
            }
        }
    }

    // Dropdown Card (Expands downward directly below the mode selector)
    Rectangle {
        id: dropdownCard
        visible: toolbarWindow.dropdownOpen
        x: modeBtn.x + 8
        y: 40
        width: 250
        height: 160
        radius: 6
        color: "#ffffff"
        border.color: "#cbd5e1"
        border.width: 1

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 2

            // Option 1: Unicode
            Rectangle {
                id: rowUni
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                radius: 4
                property bool isCurrent: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Unicode"
                color: itemUniM.containsMouse ? "#f1f5f9" : (rowUni.isCurrent ? "#e0f2fe" : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 2
                        color: "#06b6d4"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: "Unicode (ইউনিকোড)"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#0f172a"
                        }
                        Text {
                            text: "Phonetic / Avro layout (Ctrl+V)"
                            font.pixelSize: 9
                            color: "#64748b"
                        }
                    }

                    Text {
                        text: "✓"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#0284c7"
                        visible: rowUni.isCurrent
                    }
                }

                MouseArea {
                    id: itemUniM
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Unicode")
                        toolbarWindow.dropdownOpen = false
                    }
                }
            }

            // Option 2: Bijoy Unicode
            Rectangle {
                id: rowBijoyUni
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                radius: 4
                property bool isCurrent: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Bijoy Unicode"
                color: itemBijoyUniM.containsMouse ? "#f1f5f9" : (rowBijoyUni.isCurrent ? "#dcfce7" : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 2
                        color: "#10b981"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: "Bijoy Unicode (বিজয় ইউনিকোড)"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#0f172a"
                        }
                        Text {
                            text: "Bijoy layout with Unicode text"
                            font.pixelSize: 9
                            color: "#64748b"
                        }
                    }

                    Text {
                        text: "✓"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#059669"
                        visible: rowBijoyUni.isCurrent
                    }
                }

                MouseArea {
                    id: itemBijoyUniM
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Bijoy Unicode")
                        toolbarWindow.dropdownOpen = false
                    }
                }
            }

            // Option 3: Bijoy Classic
            Rectangle {
                id: rowClassic
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                radius: 4
                property bool isCurrent: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Bijoy Classic"
                color: itemClassicM.containsMouse ? "#f1f5f9" : (rowClassic.isCurrent ? "#fef3c7" : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 2
                        color: "#f59e0b"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: "Bijoy Classic (বিজয় ক্লাসিক)"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#0f172a"
                        }
                        Text {
                            text: "ANSI SutonnyMJ legacy (Ctrl+B)"
                            font.pixelSize: 9
                            color: "#64748b"
                        }
                    }

                    Text {
                        text: "✓"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#d97706"
                        visible: rowClassic.isCurrent
                    }
                }

                MouseArea {
                    id: itemClassicM
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Bijoy Classic")
                        toolbarWindow.dropdownOpen = false
                    }
                }
            }

            // Option 4: English
            Rectangle {
                id: rowEng
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                radius: 4
                property bool isCurrent: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "English"
                color: itemEngM.containsMouse ? "#f1f5f9" : (rowEng.isCurrent ? "#ede9fe" : "transparent")

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 8

                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: 10
                        radius: 2
                        color: "#3b82f6"
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Text {
                            text: "English (ইংরেজি)"
                            font.pixelSize: 11
                            font.bold: true
                            color: "#0f172a"
                        }
                        Text {
                            text: "Standard English keyboard"
                            font.pixelSize: 9
                            color: "#64748b"
                        }
                    }

                    Text {
                        text: "✓"
                        font.pixelSize: 12
                        font.bold: true
                        color: "#4f46e5"
                        visible: rowEng.isCurrent
                    }
                }

                MouseArea {
                    id: itemEngM
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("English")
                        toolbarWindow.dropdownOpen = false
                    }
                }
            }
        }
    }
}
