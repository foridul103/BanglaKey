pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Window {
    id: toolbarWindow
    visible: true
    width: 440
    height: 38
    minimumWidth: 400
    maximumWidth: 480
    minimumHeight: 38
    maximumHeight: 38
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"

    property var backend: null
    property var ipcManager: null

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
            if (sy > 0 && sy < Screen.height - 80) targetY = sy
            if (sx > 0 && sx < screenW - toolbarWindow.width) targetX = sx
        }
        toolbarWindow.x = targetX
        toolbarWindow.y = targetY
    }

    // Top horizontal bar frame (38px high)
    Rectangle {
        id: toolbarFrame
        anchors.fill: parent
        radius: 6
        border.color: "#0f3e7a"
        border.width: 1

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#256ec7" }
            GradientStop { position: 0.15; color: "#195cb0" }
            GradientStop { position: 0.85; color: "#10478e" }
            GradientStop { position: 1.0; color: "#0c3b77" }
        }

        // Native compositor move on dragging the blue bar
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

            // 2. Mode Selector Dropdown Button (Click opens native popup menu)
            Rectangle {
                id: modeBtn
                z: 10
                Layout.preferredWidth: 120
                Layout.preferredHeight: 26
                radius: 4
                color: modeBtnMouse.containsMouse ? "#f1f5f9" : "#ffffff"
                border.color: "#38bdf8"
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
                        text: "▼"
                        font.pixelSize: 9
                        color: "#475569"
                    }
                }

                MouseArea {
                    id: modeBtnMouse
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

            // 3. Quick-Switch Mode Buttons (1-Click Switch Directly on Toolbar)
            RowLayout {
                spacing: 3
                z: 10

                // English
                Rectangle {
                    id: qbEng
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "English"
                    color: active ? "#ffffff" : (qbEngM.containsMouse ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(1, 1, 1, 0.12))
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
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("English")
                        }
                    }
                }

                // Bijoy Classic
                Rectangle {
                    id: qbClassic
                    Layout.preferredWidth: 46
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Bijoy Classic"
                    color: active ? "#f59e0b" : (qbClassicM.containsMouse ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(1, 1, 1, 0.12))
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
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Bijoy Classic")
                        }
                    }
                }

                // Bijoy Unicode
                Rectangle {
                    id: qbBijoyUni
                    Layout.preferredWidth: 42
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Bijoy Unicode"
                    color: active ? "#10b981" : (qbBijoyUniM.containsMouse ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(1, 1, 1, 0.12))
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
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Bijoy Unicode")
                        }
                    }
                }

                // Unicode (Phonetic)
                Rectangle {
                    id: qbUni
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 22
                    radius: 3
                    property bool active: toolbarWindow.ipcManager && toolbarWindow.ipcManager.currentModeString === "Unicode"
                    color: active ? "#06b6d4" : (qbUniM.containsMouse ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(1, 1, 1, 0.12))
                    border.color: active ? "#ffffff" : "transparent"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "ইউনি"
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
                            if (toolbarWindow.ipcManager) toolbarWindow.ipcManager.setModeFromString("Unicode")
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            // 4. Dedicated System Tray Minimize Button
            Rectangle {
                id: btnTray
                z: 10
                Layout.preferredWidth: 24
                Layout.preferredHeight: 24
                radius: 3
                color: trayMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.3) : Qt.rgba(1, 1, 1, 0.12)
                border.color: Qt.rgba(1, 1, 1, 0.25)
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
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.saveWindowVisible(false)
                        }
                        toolbarWindow.hide()
                        toolbarWindow.hideRequested()
                    }
                }
            }

            // 5. Settings button (gear)
            Rectangle {
                id: btnSettings
                z: 10
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
                        if (toolbarWindow.backend) {
                            toolbarWindow.backend.requestOpenSettings()
                        }
                        toolbarWindow.openSettingsRequested()
                    }
                }
            }

            // 6. Close button (✕)
            Rectangle {
                id: btnClose
                z: 10
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
}
