#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <memory>

#include "ipc_manager.hpp"

namespace banglakey::desktop {

class TrayManager : public QObject {
    Q_OBJECT

public:
    explicit TrayManager(IpcManager* ipcManager, QObject* parent = nullptr);
    ~TrayManager() override = default;

    void show();
    void hide();

    [[nodiscard]] bool isVisible() const;

signals:
    void showToolbarRequested();
    void hideToolbarRequested();
    void openSettingsRequested();
    void exitRequested();

public slots:
    void updateMode(AppMode mode);

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void buildMenu();
    void updateChecks();
    QIcon createTrayIcon(AppMode mode);
    QIcon createSquareIcon(const QColor& color, bool rounded = true);
    QIcon createAppIcon();
    QIcon createRestoreIcon();
    QIcon createCloseIcon();

    IpcManager* ipc_manager_{nullptr};
    std::unique_ptr<QSystemTrayIcon> tray_icon_{nullptr};
    std::unique_ptr<QMenu> tray_menu_{nullptr};

    QAction* action_unicode_{nullptr};
    QAction* action_bijoy_unicode_{nullptr};
    QAction* action_bijoy_classic_{nullptr};
    QAction* action_english_{nullptr};
    QAction* action_show_topbar_{nullptr};
    QAction* action_close_{nullptr};
};

} // namespace banglakey::desktop
