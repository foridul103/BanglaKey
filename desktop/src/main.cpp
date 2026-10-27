#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QIcon>
#include <QScreen>
#include <QDebug>
#include <QTimer>
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

    // Load and set application icon
    QIcon appIcon = QIcon::fromTheme(QStringLiteral("banglakey"),
        QIcon(QStringLiteral(":/qt/qml/banglakey/desktop/assets/banglakey.png")));
    if (appIcon.isNull()) {
        appIcon = QIcon(QStringLiteral(BANGLAKEY_QML_SOURCE_DIR "/assets/banglakey.png"));
    }
    app.setWindowIcon(appIcon);

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
            settingsWindow->setIcon(appIcon);
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
                toolbarWindow->setIcon(appIcon);
                toolbarWindow->setProperty("backend", QVariant::fromValue(backend.get()));
                toolbarWindow->setProperty("ipcManager", QVariant::fromValue(ipcManager.get()));

                // Configure tool window flags
                toolbarWindow->setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

                // Restore saved coordinates attached to the top edge of active monitor
                QScreen* primaryScreen = QGuiApplication::primaryScreen();
                QRect availGeom = primaryScreen ? primaryScreen->availableGeometry() : QRect(0, 0, 1920, 1080);
                int winWidth = toolbarWindow->width() > 0 ? toolbarWindow->width() : 480;
                int winHeight = toolbarWindow->height() > 0 ? toolbarWindow->height() : 38;

                int savedX = backend->windowX();
                int savedY = availGeom.y();

                // If unset or out of screen bounds, center horizontally by default
                if (savedX < availGeom.x() || savedX > availGeom.x() + availGeom.width() - winWidth) {
                    int dock = backend->dockPosition();
                    if (dock == 0) {
                        savedX = availGeom.x();
                    } else if (dock == 2) {
                        savedX = availGeom.x() + availGeom.width() - winWidth;
                    } else {
                        savedX = availGeom.x() + (availGeom.width() - winWidth) / 2;
                    }
                }

                toolbarWindow->setGeometry(savedX, savedY, winWidth, winHeight);
                toolbarWindow->setX(savedX);
                toolbarWindow->setY(savedY);

                qInfo() << "BanglaKey: Toolbar positioned at top edge: x=" << savedX << " y=" << savedY
                        << " w=" << winWidth << " h=" << winHeight
                        << " screen=" << (primaryScreen ? primaryScreen->name() : "default");

                // Dynamically re-clamp when monitor resolution or geometry changes
                if (primaryScreen) {
                    QObject::connect(primaryScreen, &QScreen::availableGeometryChanged, toolbarWindow,
                                     [toolbarWindow, backend = backend.get()](const QRect& newGeom) {
                        int currentX = toolbarWindow->x();
                        int clampedX = std::max(newGeom.x(), std::min(currentX, newGeom.x() + newGeom.width() - toolbarWindow->width()));
                        toolbarWindow->setGeometry(clampedX, newGeom.y(), toolbarWindow->width(), toolbarWindow->height());
                        toolbarWindow->setX(clampedX);
                        toolbarWindow->setY(newGeom.y());
                        backend->saveWindowPosition(clampedX, newGeom.y());
                        qInfo() << "BanglaKey: Screen geometry changed. Toolbar re-clamped to x=" << clampedX << " y=" << newGeom.y();
                    });
                }

                // Automatically persist X changes
                QObject::connect(toolbarWindow, &QQuickWindow::xChanged, toolbarWindow, [toolbarWindow, backend = backend.get()]() {
                    backend->saveWindowPosition(toolbarWindow->x(), toolbarWindow->y());
                });

                // Ensure Y strictly remains anchored to top edge
                QObject::connect(toolbarWindow, &QQuickWindow::yChanged, toolbarWindow, [toolbarWindow]() {
                    QScreen* s = toolbarWindow->screen();
                    if (s) {
                        int topY = s->availableGeometry().y();
                        if (toolbarWindow->y() != topY) {
                            toolbarWindow->setY(topY);
                        }
                    }
                });
            } else {
                qWarning() << "BanglaKey: Failed to create Toolbar window:" << toolbarComp.errors();
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
    QObject::connect(trayManager.get(), &banglakey::desktop::TrayManager::trayAvailabilityChanged,
                     backend.get(), &banglakey::desktop::SettingsBackend::trayAvailabilityChanged);

    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::showToolbarRequested,
                     &app, showToolbarWindow);

    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::hideToolbarRequested,
                     &app, [&]() {
        if (trayManager->isTrayAvailable()) {
            trayManager->show();
            if (toolbarWindow) toolbarWindow->hide();
            backend->saveWindowVisible(false);
        } else {
            qWarning() << "BanglaKey: Toolbar remains visible because system tray is unavailable on this desktop:"
                       << banglakey::desktop::TrayManager::trayStatusSummary();
            if (toolbarWindow) toolbarWindow->show();
            backend->saveWindowVisible(true);
        }
    });

    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::openSettingsRequested,
                     &app, showSettingsWindow);

    QObject::connect(backend.get(), &banglakey::desktop::SettingsBackend::exitRequested,
                     &app, &QCoreApplication::quit);

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
        // Immediately show the compact floating toolbar attached to the top edge
        showToolbarWindow();
    }

    return app.exec();
}
