pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: toolbarWindow
    visible: true
    width: 410
    height: 38
    minimumWidth: 380
    maximumWidth: 460
    minimumHeight: 38
    maximumHeight: 38
    flags: Qt.Window | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"

    property var backend: null
    property var ipcManager: null

    signal hideRequested()
    signal closeRequested()
    signal openSettingsRequested()

    // Drag-to-move handling
    property int dragStartX: 0
    property int dragStartY: 0

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

        // Draggable background
        MouseArea {
            id: dragArea
            anchors.fill: parent
            cursorShape: Qt.SizeAllCursor
            acceptedButtons: Qt.LeftButton

            onPressed: function(mouse) {
                toolbarWindow.dragStartX = mouse.x
                toolbarWindow.dragStartY = mouse.y
            }

            onPositionChanged: function(mouse) {
                if (pressed) {
                    var dx = mouse.x - toolbarWindow.dragStartX
                    var dy = mouse.y - toolbarWindow.dragStartY
                    toolbarWindow.x += dx
                    toolbarWindow.y += dy
                }
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
            spacing: 8

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

            // 2. Mode Indicator
            Text {
                text: {
                    if (!toolbarWindow.ipcManager) return "English"
                    return toolbarWindow.ipcManager.currentDisplayName
                }
                color: "#ffffff"
                font.pixelSize: 12
                font.bold: true
                Layout.preferredWidth: 70
                elide: Text.ElideRight
            }

            // 3. Dropdown ComboBox (English, Bijoy Classic, Bijoy Unicode, Unicode)
            ComboBox {
                id: modeCombo
                Layout.fillWidth: true
                Layout.preferredHeight: 26
                model: ["English", "Bijoy Classic", "Bijoy Unicode", "Unicode"]

                currentIndex: {
                    if (!toolbarWindow.ipcManager) return 0
                    var cur = toolbarWindow.ipcManager.currentModeString
                    for (var i = 0; i < modeCombo.model.length; ++i) {
                        if (modeCombo.model[i] === cur) return i
                    }
                    return 0
                }

                onActivated: function(index) {
                    var selected = modeCombo.model[index]
                    if (toolbarWindow.ipcManager) {
                        toolbarWindow.ipcManager.setModeFromString(selected)
                    }
                }

                // Compact native-like styling for toolbar dropdown
                background: Rectangle {
                    color: "#ffffff"
                    radius: 3
                    border.color: modeCombo.activeFocus ? "#38bdf8" : "#94a3b8"
                    border.width: 1
                }

                contentItem: Text {
                    leftPadding: 6
                    rightPadding: 16
                    text: modeCombo.displayText
                    font.pixelSize: 11
                    color: "#0f172a"
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }

            // 4. Settings button (gear)
            Rectangle {
                id: btnSettings
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                radius: 3
                color: settingsMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "⚙"
                    color: "#ffffff"
                    font.pixelSize: 13
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

                ToolTip.visible: settingsMouse.containsMouse
                ToolTip.text: qsTr("Settings & Diagnostics")
                ToolTip.delay: 400
            }

            // 5. Minimize / Hide to Tray button (_)
            Rectangle {
                id: btnMinimize
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
                radius: 3
                color: minMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.25) : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "—"
                    color: "#ffffff"
                    font.pixelSize: 11
                    font.bold: true
                }

                MouseArea {
                    id: minMouse
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

                ToolTip.visible: minMouse.containsMouse
                ToolTip.text: qsTr("Hide toolbar to system tray")
                ToolTip.delay: 400
            }

            // 6. Close button (✕)
            Rectangle {
                id: btnClose
                Layout.preferredWidth: 22
                Layout.preferredHeight: 22
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

                ToolTip.visible: closeMouse.containsMouse
                ToolTip.text: qsTr("Exit BanglaKey")
                ToolTip.delay: 400
            }
        }
    }
}
