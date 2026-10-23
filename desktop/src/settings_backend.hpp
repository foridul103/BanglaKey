#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <memory>

#include <banglakey/engine.hpp>

namespace banglakey::desktop {

class SettingsBackend : public QObject {
    Q_OBJECT

    // Core User Preferences
    Q_PROPERTY(QString currentMode READ currentMode WRITE setCurrentMode NOTIFY currentModeChanged)
    Q_PROPERTY(QString appearance READ appearance WRITE setAppearance NOTIFY appearanceChanged)
    Q_PROPERTY(QString interfaceLanguage READ interfaceLanguage WRITE setInterfaceLanguage NOTIFY interfaceLanguageChanged)
    Q_PROPERTY(QString modeSwitchShortcut READ modeSwitchShortcut WRITE setModeSwitchShortcut NOTIFY modeSwitchShortcutChanged)
    Q_PROPERTY(bool autostart READ autostart WRITE setAutostart NOTIFY autostartChanged)

    // Diagnostics & System Status
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(bool engineAvailable READ engineAvailable CONSTANT)
    Q_PROPERTY(bool fcitx5Installed READ fcitx5Installed NOTIFY statusChanged)
    Q_PROPERTY(bool fcitx5Running READ fcitx5Running NOTIFY statusChanged)
    Q_PROPERTY(QString configFilePath READ configFilePath CONSTANT)
    Q_PROPERTY(bool configFileValid READ configFileValid NOTIFY statusChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

    // Interactive Typing Sandbox
    Q_PROPERTY(QString sandboxCommit READ sandboxCommit NOTIFY sandboxCommitChanged)
    Q_PROPERTY(QString sandboxPreedit READ sandboxPreedit NOTIFY sandboxPreeditChanged)

public:
    explicit SettingsBackend(QString customConfigPath = QString(), QObject* parent = nullptr);
    ~SettingsBackend() override = default;

    // Getters and Setters
    [[nodiscard]] QString currentMode() const;
    void setCurrentMode(const QString& mode);

    [[nodiscard]] QString appearance() const;
    void setAppearance(const QString& appearance);

    [[nodiscard]] QString interfaceLanguage() const;
    void setInterfaceLanguage(const QString& lang);

    [[nodiscard]] QString modeSwitchShortcut() const;
    void setModeSwitchShortcut(const QString& shortcut);

    [[nodiscard]] bool autostart() const;
    void setAutostart(bool enabled);

    // Diagnostics
    [[nodiscard]] QString version() const;
    [[nodiscard]] bool engineAvailable() const;
    [[nodiscard]] bool fcitx5Installed() const;
    [[nodiscard]] bool fcitx5Running() const;
    [[nodiscard]] QString configFilePath() const;
    [[nodiscard]] bool configFileValid() const;
    [[nodiscard]] QString statusMessage() const;

    // Sandbox
    [[nodiscard]] QString sandboxCommit() const;
    [[nodiscard]] QString sandboxPreedit() const;

    // Q_INVOKABLE Methods for QML and Tests
    Q_INVOKABLE void processSandboxKey(int keycode, int keysym, int unicode, int modifiers, bool isRelease);
    Q_INVOKABLE void clearSandbox();
    Q_INVOKABLE bool saveConfig();
    Q_INVOKABLE bool loadConfig();
    Q_INVOKABLE void restoreDefaults();
    Q_INVOKABLE void refreshDiagnostics();
    Q_INVOKABLE QVariantMap getBijoyKeymap() const;
    Q_INVOKABLE QVariantMap getPhoneticKeymap() const;

    // Window position persistence
    Q_INVOKABLE void saveWindowPosition(int x, int y);
    Q_INVOKABLE int windowX() const;
    Q_INVOKABLE int windowY() const;
    Q_INVOKABLE void saveWindowVisible(bool visible);
    Q_INVOKABLE bool windowVisible() const;

    // Toolbar and Navigation requests
    Q_INVOKABLE void requestShowToolbar();
    Q_INVOKABLE void requestHideToolbar();
    Q_INVOKABLE void requestOpenSettings();
    Q_INVOKABLE void requestExit();
    Q_INVOKABLE void onSplashFinished();

    // Validation helpers
    [[nodiscard]] bool validateMode(const QString& mode) const;
    [[nodiscard]] bool validateAppearance(const QString& appearance) const;
    [[nodiscard]] bool validateLanguage(const QString& lang) const;
    [[nodiscard]] bool validateShortcut(const QString& shortcut) const;

signals:
    void currentModeChanged();
    void appearanceChanged();
    void interfaceLanguageChanged();
    void modeSwitchShortcutChanged();
    void autostartChanged();
    void statusChanged();
    void statusMessageChanged();
    void sandboxCommitChanged();
    void sandboxPreeditChanged();
    void showToolbarRequested();
    void hideToolbarRequested();
    void openSettingsRequested();
    void exitRequested();

private:
    void updateEngineMode();
    void checkFcitx5Status();

    std::unique_ptr<banglakey::TypingEngine> engine_{nullptr};
    QString custom_config_path_{};

    QString current_mode_{"Phonetic"};
    QString appearance_{"system"};
    QString interface_language_{"en"};
    QString mode_switch_shortcut_{"F12"};
    bool autostart_{false};

    bool fcitx5_installed_{false};
    bool fcitx5_running_{false};
    bool config_file_valid_{true};
    QString status_message_{"Ready"};

    QString sandbox_commit_{};
    QString sandbox_preedit_{};
};

} // namespace banglakey::desktop
