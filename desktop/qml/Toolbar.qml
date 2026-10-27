pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Window {
    id: toolbarWindow
    visible: false
    width: 480
    height: (trayTooltip && trayTooltip.opacity > 0) ? 76 : 38
    minimumWidth: 440
    maximumWidth: 540
    minimumHeight: 38
    maximumHeight: 76
    flags: Qt.Tool | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"

    property var backend: null
    property var ipcManager: null

    signal hideRequested()
    signal closeRequested()
    signal openSettingsRequested()

    // Enforce top-edge horizontal alignment upon completion and restore saved X
    Component.onCompleted: {
        var screenW = Screen.desktopAvailableWidth > 0 ? Screen.desktopAvailableWidth : (Screen.width > 0 ? Screen.width : 1920)
        var targetX = Math.max(0, (screenW - toolbarWindow.width) / 2)
        var targetY = 0

        if (toolbarWindow.backend) {
            var sx = toolbarWindow.backend.windowX()
            if (sx >= 0 && sx <= screenW - toolbarWindow.width) {
                targetX = sx
            } else {
                var dock = toolbarWindow.backend.dockPosition()
                if (dock === 0) targetX = 0
                else if (dock === 2) targetX = Math.max(0, screenW - toolbarWindow.width)
                else targetX = Math.max(0, (screenW - toolbarWindow.width) / 2)
            }
        }
        toolbarWindow.x = targetX
        toolbarWindow.y = targetY
        console.log("[TOOLBAR_INIT] Initialized toolbar at targetX=" + targetX + " targetY=" + targetY + " screenW=" + screenW)
    }

    // Re-clamp position if screen resolution changes
    Connections {
        target: Screen
        function onWidthChanged() {
            var screenW = Screen.desktopAvailableWidth > 0 ? Screen.desktopAvailableWidth : (Screen.width > 0 ? Screen.width : 1920)
            var clampedX = Math.max(0, Math.min(toolbarWindow.x, screenW - toolbarWindow.width))
            toolbarWindow.x = clampedX
            toolbarWindow.y = 0
            if (toolbarWindow.backend) {
                toolbarWindow.backend.saveWindowPosition(clampedX, 0)
            }
        }
    }

    // Main horizontal toolbar bar (38px high)
    Rectangle {
        id: toolbarFrame
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 38
        radius: 6
        border.color: "#38bdf8"
        border.width: 1

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#1e3a8a" }
            GradientStop { position: 0.15; color: "#1d4ed8" }
            GradientStop { position: 0.85; color: "#1e40af" }
            GradientStop { position: 1.0; color: "#172554" }
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8
            anchors.rightMargin: 8
            spacing: 6

            // 1. Logo / Emblem
            Image {
                id: toolbarLogo
                objectName: "toolbarLogo"
                Layout.preferredWidth: 26
                Layout.preferredHeight: 26
                source: "qrc:/qt/qml/banglakey/desktop/assets/banglakey-emblem.png"
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }

            // 2. Current Mode Label Badge (Visual pill indicating active layout)
            Rectangle {
                id: modeBadge
                Layout.preferredHeight: 24
                Layout.preferredWidth: 90
                radius: 4
                color: {
                    if (!toolbarWindow.ipcManager) return "#334155"
                    var cur = toolbarWindow.ipcManager.currentModeString
                    if (cur === "Bijoy Unicode") return "#059669" // Emerald
                    if (cur === "Bijoy Classic") return "#d97706" // Amber
                    if (cur === "Avro Phonetic") return "#0891b2" // Cyan
                    return "#475569" // Slate for English
                }
                border.color: Qt.rgba(1, 1, 1, 0.35)
                border.width: 1

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 4

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: "#ffffff"
                    }

                    Text {
                        text: {
                            if (!toolbarWindow.ipcManager) return "English"
                            var cur = toolbarWindow.ipcManager.currentModeString
                            if (cur === "Bijoy Unicode") return "বি-ইউনিকোড"
                            if (cur === "Bijoy Classic") return "বি-ক্লাসিক"
                            if (cur === "Avro Phonetic") return "অভ্রো"
                            return "English"
                        }
                        color: "#ffffff"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            // 3. Mode Selector Dropdown Button (Click opens full 4-mode dropdown popup)
            Rectangle {
                id: modeBtn
                objectName: "modeBtn"
                z: 10
                Layout.preferredWidth: 150
                Layout.preferredHeight: 26
                radius: 4
                color: modeBtnMouse.containsMouse ? "#f8fafc" : "#ffffff"
                border.color: "#38bdf8"
                border.width: 1.5

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    spacing: 4

                    Text {
                        Layout.fillWidth: true
                        text: {
                            if (!toolbarWindow.ipcManager) return "English"
                            var cur = toolbarWindow.ipcManager.currentModeString
                            if (cur === "Bijoy Unicode") return "Bijoy Unicode"
                            if (cur === "Bijoy Classic") return "Bijoy Classic"
                            if (cur === "Avro Phonetic") return "Avro Phonetic"
                            return "English"
                        }
                        font.pixelSize: 11
                        font.bold: true
                        color: "#0f172a"
                        elide: Text.ElideRight
                    }

                    Text {
                        text: "▼"
                        font.pixelSize: 8
                        color: "#64748b"
                    }
                }

                MouseArea {
                    id: modeBtnMouse
                    objectName: "modeBtnMouse"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.openLayoutMenu(
                                toolbarWindow.x + modeBtn.x,
                                toolbarWindow.y + toolbarWindow.height + 2
                            )
                        }
                    }
                }
            }

            // 4. Draggable Title & Grip Handle (Dedicated middle region for horizontal dragging)
            Rectangle {
                id: dragGrip
                objectName: "dragGrip"
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                radius: 4
                color: dragArea.containsMouse ? Qt.rgba(255, 255, 255, 0.12) : Qt.rgba(255, 255, 255, 0.05)
                border.color: dragArea.containsMouse ? "#38bdf8" : Qt.rgba(255, 255, 255, 0.15)
                border.width: 1

                RowLayout {
                    anchors.centerIn: parent
                    spacing: 4

                    Text {
                        text: "⋮⋮"
                        color: "#94a3b8"
                        font.pixelSize: 11
                        font.bold: true
                    }

                    Text {
                        text: "বাংলাকি"
                        color: "#e2e8f0"
                        font.pixelSize: 10
                        font.bold: true
                    }

                    Text {
                        text: "⋮⋮"
                        color: "#94a3b8"
                        font.pixelSize: 11
                        font.bold: true
                    }
                }

                MouseArea {
                    id: dragArea
                    objectName: "dragArea"
                    anchors.fill: parent
                    cursorShape: Qt.SizeHorCursor
                    acceptedButtons: Qt.LeftButton
                    hoverEnabled: true

                    property int startMouseX: 0
                    property int startWinX: 0

                    onPressed: function(mouse) {
                        dragArea.startMouseX = mouse.x
                        dragArea.startWinX = toolbarWindow.x
                        console.log("[POINTER_PRESS] mouseX=" + mouse.x + " mouseY=" + mouse.y +
                                    " winX=" + toolbarWindow.x + " winY=" + toolbarWindow.y)

                        // On native Wayland, delegate to compositor for smooth dragging
                        if (toolbarWindow.backend && toolbarWindow.backend.isWayland()) {
                            console.log("[WAYLAND_MOVE] Delegating to compositor startSystemMove()...")
                            toolbarWindow.startSystemMove()
                        }
                    }

                    onPositionChanged: function(mouse) {
                        if (mouse.buttons & Qt.LeftButton) {
                            var screenW = Screen.desktopAvailableWidth > 0 ? Screen.desktopAvailableWidth : (Screen.width > 0 ? Screen.width : 1920)
                            var deltaX = mouse.x - dragArea.startMouseX
                            var requestedX = dragArea.startWinX + deltaX
                            var clampedX = Math.max(0, Math.min(requestedX, screenW - toolbarWindow.width))

                            console.log("[POINTER_MOVE] deltaX=" + deltaX +
                                        " requestedWinX=" + clampedX +
                                        " actualWinX=" + toolbarWindow.x +
                                        " actualWinY=" + toolbarWindow.y)

                            // Under X11/XWayland and window managers supporting client coordinate moves
                            toolbarWindow.x = clampedX
                            toolbarWindow.y = 0 // Locked strictly to top edge
                        }
                    }

                    onReleased: function(mouse) {
                        console.log("[POINTER_RELEASE] finalWinX=" + toolbarWindow.x + " finalWinY=" + toolbarWindow.y)
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.saveWindowPosition(toolbarWindow.x, 0)
                        }
                    }
                }
            }

            // 5. Quick Top-Edge Docking Presets (Left, Center, Right)
            Rectangle {
                id: dockControls
                objectName: "dockControls"
                z: 10
                Layout.preferredHeight: 24
                Layout.preferredWidth: 60
                radius: 4
                color: Qt.rgba(0, 0, 0, 0.25)
                border.color: Qt.rgba(255, 255, 255, 0.15)
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    spacing: 1

                    // Dock Left
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 3
                        color: dockLeftMouse.containsMouse ? "#3b82f6" : "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "⮜"
                            color: "#ffffff"
                            font.pixelSize: 10
                        }
                        MouseArea {
                            id: dockLeftMouse
                            objectName: "dockLeftMouse"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var newX = 0
                                toolbarWindow.x = newX
                                toolbarWindow.y = 0
                                console.log("[DOCK_LEFT] Docked to Left: winX=" + newX + " winY=0")
                                if (toolbarWindow.backend) {
                                    toolbarWindow.backend.saveWindowPosition(newX, 0)
                                    toolbarWindow.backend.saveDockPosition(0)
                                }
                            }
                        }
                    }

                    // Dock Center
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 3
                        color: dockCenterMouse.containsMouse ? "#3b82f6" : "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "⯀"
                            color: "#ffffff"
                            font.pixelSize: 8
                        }
                        MouseArea {
                            id: dockCenterMouse
                            objectName: "dockCenterMouse"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var screenW = Screen.desktopAvailableWidth > 0 ? Screen.desktopAvailableWidth : (Screen.width > 0 ? Screen.width : 1920)
                                var newX = Math.max(0, (screenW - toolbarWindow.width) / 2)
                                toolbarWindow.x = newX
                                toolbarWindow.y = 0
                                console.log("[DOCK_CENTER] Docked to Center: winX=" + newX + " winY=0")
                                if (toolbarWindow.backend) {
                                    toolbarWindow.backend.saveWindowPosition(newX, 0)
                                    toolbarWindow.backend.saveDockPosition(1)
                                }
                            }
                        }
                    }

                    // Dock Right
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 3
                        color: dockRightMouse.containsMouse ? "#3b82f6" : "transparent"
                        Text {
                            anchors.centerIn: parent
                            text: "⮞"
                            color: "#ffffff"
                            font.pixelSize: 10
                        }
                        MouseArea {
                            id: dockRightMouse
                            objectName: "dockRightMouse"
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                var screenW = Screen.desktopAvailableWidth > 0 ? Screen.desktopAvailableWidth : (Screen.width > 0 ? Screen.width : 1920)
                                var newX = Math.max(0, screenW - toolbarWindow.width)
                                toolbarWindow.x = newX
                                toolbarWindow.y = 0
                                console.log("[DOCK_RIGHT] Docked to Right: winX=" + newX + " winY=0")
                                if (toolbarWindow.backend) {
                                    toolbarWindow.backend.saveWindowPosition(newX, 0)
                                    toolbarWindow.backend.saveDockPosition(2)
                                }
                            }
                        }
                    }
                }
            }

            // 4. Settings Button (Prominent, crisp gear icon with accessible clickable area)
            Rectangle {
                id: btnSettings
                objectName: "btnSettings"
                z: 10
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 4
                color: settingsMouse.pressed ? Qt.rgba(1, 1, 1, 0.35)
                     : settingsMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.2)
                     : Qt.rgba(1, 1, 1, 0.08)
                border.color: settingsMouse.containsMouse ? "#38bdf8" : Qt.rgba(1, 1, 1, 0.2)
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "⚙"
                    color: "#ffffff"
                    font.pixelSize: 16
                }

                MouseArea {
                    id: settingsMouse
                    objectName: "settingsMouse"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.requestOpenSettings()
                        }
                        toolbarWindow.openSettingsRequested()
                    }
                }
            }

            // 5. Hide to System Tray Button (⤓)
            Rectangle {
                id: btnTray
                objectName: "btnTray"
                z: 10
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 4
                color: trayMouse.pressed ? Qt.rgba(1, 1, 1, 0.35)
                     : trayMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.2)
                     : Qt.rgba(1, 1, 1, 0.08)
                border.color: trayMouse.containsMouse ? "#38bdf8" : Qt.rgba(1, 1, 1, 0.2)
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "⤓"
                    color: "#ffffff"
                    font.pixelSize: 14
                    font.bold: true
                }

                MouseArea {
                    id: trayMouse
                    objectName: "trayMouse"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.backend && !toolbarWindow.backend.isTrayAvailable) {
                            trayTooltip.show(toolbarWindow.backend.trayStatusMessage)
                            return
                        }
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.requestHideToolbar()
                        } else {
                            toolbarWindow.hide()
                        }
                    }
                }
            }

            // 6. Close button (✕)
            Rectangle {
                id: btnClose
                objectName: "btnClose"
                z: 10
                Layout.preferredWidth: 28
                Layout.preferredHeight: 28
                radius: 4
                color: closeMouse.pressed ? "#b91c1c"
                     : closeMouse.containsMouse ? "#dc2626"
                     : Qt.rgba(1, 1, 1, 0.08)
                border.color: closeMouse.containsMouse ? "#ef4444" : Qt.rgba(1, 1, 1, 0.2)
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "✕"
                    color: "#ffffff"
                    font.pixelSize: 12
                    font.bold: true
                }

                MouseArea {
                    id: closeMouse
                    objectName: "closeMouse"
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.requestExit()
                        } else {
                            toolbarWindow.close()
                        }
                    }
                }
            }
        }
    }

    // Status warning notification for unavailable system tray
    Rectangle {
        id: trayTooltip
        z: 100
        visible: opacity > 0
        opacity: 0.0
        anchors.top: toolbarFrame.bottom
        anchors.topMargin: 4
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(420, parent.width - 10)
        height: tipText.contentHeight + 12
        radius: 4
        color: "#1e293b"
        border.color: "#f59e0b"
        border.width: 1

        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }

        Text {
            id: tipText
            anchors.fill: parent
            anchors.margins: 6
            text: ""
            color: "#fef3c7"
            font.pixelSize: 10
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        Timer {
            id: tipTimer
            interval: 5000
            onTriggered: trayTooltip.opacity = 0.0
        }

        function show(msg) {
            tipText.text = "⚠️ " + msg
            trayTooltip.opacity = 1.0
            tipTimer.restart()
        }
    }
}
