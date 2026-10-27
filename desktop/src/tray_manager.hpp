#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QIcon>
#include <QString>
#include <memory>

#include "ipc_manager.hpp"

class QDBusServiceWatcher;

namespace banglakey::desktop {

class TrayManager : public QObject {
    Q_OBJECT

public:
    explicit TrayManager(IpcManager* ipcManager, QObject* parent = nullptr);
    ~TrayManager() override = default;

    void show();
    void hide();

    [[nodiscard]] bool isVisible() const;
    [[nodiscard]] bool isTrayAvailable() const;

    // Static desktop & D-Bus detection helpers
    [[nodiscard]] static bool isStatusNotifierWatcherAvailable();
    [[nodiscard]] static bool isAppIndicatorExtensionInstalled();
    [[nodiscard]] static bool isAppIndicatorExtensionEnabled();
    [[nodiscard]] static bool isKdePlasma();
    [[nodiscard]] static bool isGnome();
    [[nodiscard]] static QString desktopSessionType();
    [[nodiscard]] static QString desktopEnvironment();
    [[nodiscard]] static QString trayStatusSummary();

    // Inspection methods for automated unit testing
    [[nodiscard]] QMenu* menu() const noexcept { return tray_menu_.get(); }
    [[nodiscard]] QAction* actionBijoyUnicode() const noexcept { return action_bijoy_unicode_; }
    [[nodiscard]] QAction* actionBijoyClassic() const noexcept { return action_bijoy_classic_; }
    [[nodiscard]] QAction* actionAvroPhonetic() const noexcept { return action_avro_phonetic_; }
    [[nodiscard]] QAction* actionUnicode() const noexcept { return action_unicode_; }
    [[nodiscard]] QAction* actionEnglish() const noexcept { return action_english_; }
    [[nodiscard]] QAction* actionShowTopbar() const noexcept { return action_show_topbar_; }
    [[nodiscard]] QAction* actionSettings() const noexcept { return action_settings_; }
    [[nodiscard]] QAction* actionClose() const noexcept { return action_close_; }
    QIcon createModeIcon(AppMode mode);

signals:
    void showToolbarRequested();
    void hideToolbarRequested();
    void openSettingsRequested();
    void exitRequested();
    void trayAvailabilityChanged(bool available);

public slots:
    void updateMode(AppMode mode);

private slots:
    void onTrayActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void buildMenu();
    void updateChecks();
    QIcon createAppIcon();
    QIcon createRestoreIcon();
    QIcon createCloseIcon();

    IpcManager* ipc_manager_{nullptr};
    std::unique_ptr<QSystemTrayIcon> tray_icon_{nullptr};
    std::unique_ptr<QMenu> tray_menu_{nullptr};
    QActionGroup* mode_group_{nullptr};
    QDBusServiceWatcher* service_watcher_{nullptr};

    QAction* action_bijoy_unicode_{nullptr};
    QAction* action_bijoy_classic_{nullptr};
    QAction* action_avro_phonetic_{nullptr};
    QAction* action_unicode_{nullptr};
    QAction* action_english_{nullptr};
    QAction* action_show_topbar_{nullptr};
    QAction* action_settings_{nullptr};
    QAction* action_close_{nullptr};
};

} // namespace banglakey::desktop
