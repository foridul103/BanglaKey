#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QIcon>
#include <QScreen>
#include <QDebug>
#include <memory>

#include "ipc_manager.hpp"
#include "tray_manager.hpp"
#include "settings_backend.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setOrganizationName("BanglaKey");
    app.setOrganizationDomain("banglakey.org");
    app.setApplicationName("BanglaKey");
    app.setApplicationVersion("0.1.0");

    // Important: Do not exit when toolbar window is closed/hidden, as app lives in system tray
    app.setQuitOnLastWindowClosed(false);

    // 1. Single Instance Check via IPC
    auto ipcManager = std::make_unique<banglakey::desktop::IpcManager>();
    if (!ipcManager->initSingleInstance()) {
        // Existing instance was notified to show itself. Exit cleanly.
        return 0;
    }

    // 2. Initialize Settings Backend
    auto backend = std::make_unique<banglakey::desktop::SettingsBackend>();

    // Bidirectional sync between IpcManager and SettingsBackend
    QObject::connect(ipcManager.get(), &banglakey::desktop::IpcManager::modeChanged,
                     backend.get(), [backend = backend.get()](banglakey::desktop::AppMode mode) {
        backend->setCurrentMode(banglakey::desktop::IpcManager::modeToString(mode));
    });

    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::currentModeChanged,
                     ipcManager.get(), [ipc = ipcManager.get(), backend = backend.get()]() {
        ipc->setModeFromString(backend->currentMode());
    });

    // 3. Initialize System Tray Manager
    auto trayManager = std::make_unique<banglakey::desktop::TrayManager>(ipcManager.get());
    trayManager->show();

    // 4. Initialize QML Engine
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("backend", backend.get());
    engine.rootContext()->setContextProperty("ipcManager", ipcManager.get());

    // Window pointers
    QQuickWindow* toolbarWindow = nullptr;
    QQuickWindow* settingsWindow = nullptr;

    // Helper lambda to load and display settings window
    auto showSettingsWindow = [&]() {
        if (!settingsWindow) {
            QQmlComponent settingsComp(&engine, QUrl(QStringLiteral("qrc:/qt/qml/banglakey/desktop/qml/Main.qml")));
            if (settingsComp.isError()) {
                settingsComp.loadUrl(QUrl::fromLocalFile(QStringLiteral(BANGLAKEY_QML_SOURCE_DIR "/qml/Main.qml")));
            }
            QObject* obj = settingsComp.create(engine.rootContext());
            settingsWindow = qobject_cast<QQuickWindow*>(obj);
        }
        if (settingsWindow) {
            settingsWindow->show();
            settingsWindow->raise();
            settingsWindow->requestActivate();
        }
    };

    // Helper lambda to load and display floating toolbar
    auto showToolbarWindow = [&]() {
        if (!toolbarWindow) {
            QQmlComponent toolbarComp(&engine, QUrl(QStringLiteral("qrc:/qt/qml/banglakey/desktop/qml/Toolbar.qml")));
            if (toolbarComp.isError()) {
                toolbarComp.loadUrl(QUrl::fromLocalFile(QStringLiteral(BANGLAKEY_QML_SOURCE_DIR "/qml/Toolbar.qml")));
            }
            QObject* obj = toolbarComp.create(engine.rootContext());
            toolbarWindow = qobject_cast<QQuickWindow*>(obj);

            if (toolbarWindow) {
                // Restore saved coordinates
                int savedX = backend->windowX();
                int savedY = backend->windowY();

                QScreen* primaryScreen = QGuiApplication::primaryScreen();
                if (primaryScreen) {
                    QRect screenGeom = primaryScreen->availableGeometry();
                    if (savedX <= 0 || savedX > screenGeom.width() - 100) {
                        savedX = screenGeom.x() + (screenGeom.width() - toolbarWindow->width()) / 2;
                    }
                    if (savedY <= 0 || savedY > screenGeom.height() - 50) {
                        savedY = screenGeom.y() + 40;
                    }
                }

                toolbarWindow->setX(savedX);
                toolbarWindow->setY(savedY);

            }
        }

        if (toolbarWindow) {
            toolbarWindow->show();
            toolbarWindow->raise();
            toolbarWindow->requestActivate();
            backend->saveWindowVisible(true);
        }
    };

    // Connect Tray, Backend, and IPC signals to Toolbar & Settings
    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::showToolbarRequested,
                     &app, showToolbarWindow);

    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::openSettingsRequested,
                     &app, showSettingsWindow);

    QObject::connect(trayManager.get(), &banglakey::desktop::TrayManager::showToolbarRequested,
                     &app, showToolbarWindow);

    QObject::connect(trayManager.get(), &banglakey::desktop::TrayManager::openSettingsRequested,
                     &app, showSettingsWindow);

    QObject::connect(trayManager.get(), &banglakey::desktop::TrayManager::exitRequested,
                     &app, &QCoreApplication::quit);

    QObject::connect(ipcManager.get(), &banglakey::desktop::IpcManager::showToolbarRequested,
                     &app, showToolbarWindow);

    // Check command line arguments
    bool startMinimized = app.arguments().contains("--minimized") || app.arguments().contains("--tray");
    bool openSettingsOnly = app.arguments().contains("--settings");

    if (openSettingsOnly) {
        showSettingsWindow();
    } else if (startMinimized) {
        // Keep hidden in tray
        qInfo() << "BanglaKey: Started minimized in system tray.";
    } else {
        // 5. Show Startup Splash Screen, then transition to Toolbar
        QQmlComponent splashComp(&engine, QUrl(QStringLiteral("qrc:/qt/qml/banglakey/desktop/qml/Splash.qml")));
        if (splashComp.isError()) {
            splashComp.loadUrl(QUrl::fromLocalFile(QStringLiteral(BANGLAKEY_QML_SOURCE_DIR "/qml/Splash.qml")));
        }
        QObject* splashObj = splashComp.create(engine.rootContext());
        auto* splashWindow = qobject_cast<QQuickWindow*>(splashObj);

        if (splashWindow) {
            splashWindow->show();
        } else {
            // Fallback: If splash failed to create, show toolbar directly
            showToolbarWindow();
        }
    }

    return app.exec();
}
