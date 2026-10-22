pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Window {
    id: splashWindow
    visible: true
    width: 380
    height: 230
    flags: Qt.SplashScreen | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    color: "transparent"

    property var backend: null

    signal splashFinished()

    Component.onCompleted: {
        // Center on screen
        splashWindow.x = (Screen.width - splashWindow.width) / 2
        splashWindow.y = (Screen.height - splashWindow.height) / 2
        dismissTimer.start()
    }

    Timer {
        id: dismissTimer
        interval: 1800
        repeat: false
        onTriggered: {
            if (splashWindow.backend) {
                splashWindow.backend.onSplashFinished()
            }
            splashWindow.splashFinished()
            splashWindow.close()
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            dismissTimer.stop()
            if (splashWindow.backend) {
                splashWindow.backend.onSplashFinished()
            }
            splashWindow.splashFinished()
            splashWindow.close()
        }
    }

    Rectangle {
        id: splashCard
        anchors.fill: parent
        anchors.margins: 6
        radius: 14
        color: "#13233b"
        border.color: "#3b82f6"
        border.width: 1.5

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#162e52" }
            GradientStop { position: 0.6; color: "#0f1d33" }
            GradientStop { position: 1.0; color: "#0a1322" }
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            // Top Header: Logo + App Name
            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                // Stylized Logo Emblem
                Rectangle {
                    Layout.preferredWidth: 48
                    Layout.preferredHeight: 48
                    radius: 12
                    color: "#006a4e"
                    border.color: "#22c55e"
                    border.width: 1.5

                    Rectangle {
                        anchors.centerIn: parent
                        width: 32
                        height: 32
                        radius: 16
                        color: "#f42a41"

                        Text {
                            anchors.centerIn: parent
                            text: "ক"
                            color: "#ffffff"
                            font.pixelSize: 20
                            font.bold: true
                        }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: "BanglaKey Linux"
                        color: "#ffffff"
                        font.pixelSize: 18
                        font.bold: true
                    }

                    Text {
                        text: "বাংলাকি লিনাক্স • বিজয় ও ফোনেটিক"
                        color: "#93c5fd"
                        font.pixelSize: 12
                    }
                }

                Rectangle {
                    Layout.preferredWidth: 50
                    Layout.preferredHeight: 22
                    radius: 11
                    color: Qt.rgba(0.23, 0.51, 0.96, 0.25)
                    border.color: "#3b82f6"
                    border.width: 1

                    Text {
                        anchors.centerIn: parent
                        text: "v0.1.0"
                        color: "#bfdbfe"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Qt.rgba(1, 1, 1, 0.12)
            }

            // Description
            Text {
                Layout.fillWidth: true
                text: "Native Bengali Input System for Fedora Linux\nKDE Plasma 6 • Wayland & X11 • Fcitx5"
                color: "#cbd5e1"
                font.pixelSize: 11
                lineHeight: 1.3
                horizontalAlignment: Text.AlignHCenter
            }

            Item { Layout.fillHeight: true }

            // Loading Progress Bar
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    Text {
                        text: "Initializing typing engine..."
                        color: "#94a3b8"
                        font.pixelSize: 10
                    }
                    Item { Layout.fillWidth: true }
                    Text {
                        text: "Ready"
                        color: "#4ade80"
                        font.pixelSize: 10
                        font.bold: true
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 4
                    radius: 2
                    color: "#1e293b"

                    Rectangle {
                        id: progressBar
                        height: parent.height
                        radius: 2
                        color: "#3b82f6"

                        NumberAnimation on width {
                            from: 10
                            to: splashCard.width - 40
                            duration: 1700
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }
        }
    }
}
