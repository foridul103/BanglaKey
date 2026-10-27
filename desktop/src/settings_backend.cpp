#include "settings_backend.hpp"
#include "tray_manager.hpp"
#include <banglakey/banglakey.hpp>

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTextStream>
#include <QGuiApplication>
#include <QMenu>
#include <QAction>
#include <QDebug>

namespace banglakey::desktop {

SettingsBackend::SettingsBackend(QString customConfigPath, QObject* parent)
    : QObject(parent),
      custom_config_path_(std::move(customConfigPath)) {
    engine_ = banglakey::TypingEngine::create(banglakey::TypingMode::BijoyUnicode);
    loadConfig();
    checkFcitx5Status();
}

QString SettingsBackend::configFilePath() const {
    if (!custom_config_path_.isEmpty()) {
        return custom_config_path_;
    }
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/banglakey/config.ini";
}

bool SettingsBackend::validateMode(const QString& mode) const {
    return (mode.compare("Avro Phonetic", Qt::CaseInsensitive) == 0 ||
            mode.compare("Avro", Qt::CaseInsensitive) == 0 ||
            mode.compare("Phonetic", Qt::CaseInsensitive) == 0 ||
            mode.compare("Bijoy", Qt::CaseInsensitive) == 0 ||
            mode.compare("Bijoy Classic", Qt::CaseInsensitive) == 0 ||
            mode.compare("Bijoy Unicode", Qt::CaseInsensitive) == 0 ||
            mode.compare("Unicode", Qt::CaseInsensitive) == 0 ||
            mode.compare("English", Qt::CaseInsensitive) == 0);
}

bool SettingsBackend::validateAppearance(const QString& appearance) const {
    return (appearance.compare("system", Qt::CaseInsensitive) == 0 ||
            appearance.compare("dark", Qt::CaseInsensitive) == 0 ||
            appearance.compare("light", Qt::CaseInsensitive) == 0);
}

bool SettingsBackend::validateLanguage(const QString& lang) const {
    return (lang == "en" || lang == "bn");
}

bool SettingsBackend::validateShortcut(const QString& shortcut) const {
    return (shortcut == "F12" || shortcut == "Ctrl+Alt+M");
}

QString SettingsBackend::currentMode() const {
    return current_mode_;
}

void SettingsBackend::setCurrentMode(const QString& mode) {
    if (current_mode_ == mode) {
        return;
    }

    if (!validateMode(mode)) {
        status_message_ = tr("Invalid typing mode rejected: %1").arg(mode);
        emit statusMessageChanged();
        return;
    }

    // Normalize casing
    if (mode.compare("Bijoy Classic", Qt::CaseInsensitive) == 0) {
        current_mode_ = "Bijoy Classic";
    } else if (mode.compare("Bijoy Unicode", Qt::CaseInsensitive) == 0 ||
               mode.compare("Bijoy", Qt::CaseInsensitive) == 0 ||
               mode.compare("Unicode", Qt::CaseInsensitive) == 0) {
        current_mode_ = "Bijoy Unicode";
    } else if (mode.compare("Avro Phonetic", Qt::CaseInsensitive) == 0 ||
               mode.compare("Avro", Qt::CaseInsensitive) == 0 ||
               mode.compare("Phonetic", Qt::CaseInsensitive) == 0) {
        current_mode_ = "Avro Phonetic";
    } else if (mode.compare("English", Qt::CaseInsensitive) == 0) {
        current_mode_ = "English";
    } else {
        current_mode_ = "Bijoy Unicode";
    }
    updateEngineMode();

    emit currentModeChanged();
    saveConfig();
}

QString SettingsBackend::appearance() const {
    return appearance_;
}

void SettingsBackend::setAppearance(const QString& appearance) {
    if (appearance_ == appearance) {
        return;
    }

    if (!validateAppearance(appearance)) {
        status_message_ = tr("Invalid appearance option: %1").arg(appearance);
        emit statusMessageChanged();
        return;
    }

    appearance_ = appearance.toLower();
    emit appearanceChanged();
    saveConfig();
}

QString SettingsBackend::interfaceLanguage() const {
    return interface_language_;
}

void SettingsBackend::setInterfaceLanguage(const QString& lang) {
    if (interface_language_ == lang) {
        return;
    }

    if (!validateLanguage(lang)) {
        status_message_ = tr("Invalid language option: %1").arg(lang);
        emit statusMessageChanged();
        return;
    }

    interface_language_ = lang;
    emit interfaceLanguageChanged();
    saveConfig();
}

QString SettingsBackend::modeSwitchShortcut() const {
    return mode_switch_shortcut_;
}

void SettingsBackend::setModeSwitchShortcut(const QString& shortcut) {
    if (mode_switch_shortcut_ == shortcut) {
        return;
    }

    if (!validateShortcut(shortcut)) {
        status_message_ = tr("Invalid shortcut: %1").arg(shortcut);
        emit statusMessageChanged();
        return;
    }

    mode_switch_shortcut_ = shortcut;
    emit modeSwitchShortcutChanged();
    saveConfig();
}

bool SettingsBackend::autostart() const {
    return autostart_;
}

void SettingsBackend::setAutostart(bool enabled) {
    if (autostart_ == enabled) {
        return;
    }

    autostart_ = enabled;
    emit autostartChanged();
    saveConfig();
}

QString SettingsBackend::version() const {
    return QString::fromUtf8(BANGLAKEY_VERSION_STRING);
}

bool SettingsBackend::engineAvailable() const {
    return (engine_ != nullptr);
}

bool SettingsBackend::fcitx5Installed() const {
    return fcitx5_installed_;
}

bool SettingsBackend::fcitx5Running() const {
    return fcitx5_running_;
}

bool SettingsBackend::configFileValid() const {
    return config_file_valid_;
}

QString SettingsBackend::statusMessage() const {
    return status_message_;
}

bool SettingsBackend::isTrayAvailable() const {
    return TrayManager::isStatusNotifierWatcherAvailable() || QSystemTrayIcon::isSystemTrayAvailable();
}

QString SettingsBackend::trayStatusMessage() const {
    return TrayManager::trayStatusSummary();
}

bool SettingsBackend::appIndicatorInstalled() const {
    return TrayManager::isAppIndicatorExtensionInstalled();
}

bool SettingsBackend::appIndicatorEnabled() const {
    return TrayManager::isAppIndicatorExtensionEnabled();
}

QString SettingsBackend::sandboxCommit() const {
    return sandbox_commit_;
}

QString SettingsBackend::sandboxPreedit() const {
    return sandbox_preedit_;
}

void SettingsBackend::updateEngineMode() {
    if (!engine_) {
        return;
    }

    if (current_mode_ == "Bijoy Classic") {
        engine_->setMode(banglakey::TypingMode::BijoyClassic);
    } else if (current_mode_ == "Bijoy Unicode") {
        engine_->setMode(banglakey::TypingMode::BijoyUnicode);
    } else if (current_mode_ == "Avro Phonetic") {
        engine_->setMode(banglakey::TypingMode::AvroPhonetic);
    }
}

void SettingsBackend::saveWindowPosition(int x, int y) {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("Toolbar/x", x);
    settings.setValue("Toolbar/y", y);
    settings.sync();
}

int SettingsBackend::windowX() const {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    return settings.value("Toolbar/x", -1).toInt(); // -1 indicates unset / first launch
}

int SettingsBackend::windowY() const {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    return settings.value("Toolbar/y", 0).toInt(); // 0 indicates top edge
}

int SettingsBackend::dockPosition() const {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    return settings.value("Toolbar/dock", 1).toInt(); // Default 1: Center
}

void SettingsBackend::saveDockPosition(int dock) {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("Toolbar/dock", dock);
    settings.sync();
}

bool SettingsBackend::isWayland() const {
    return QGuiApplication::platformName().contains("wayland", Qt::CaseInsensitive);
}

void SettingsBackend::saveWindowVisible(bool visible) {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("Toolbar/visible", visible);
    settings.sync();
}

bool SettingsBackend::windowVisible() const {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);
    return settings.value("Toolbar/visible", true).toBool();
}

void SettingsBackend::checkFcitx5Status() {
    // 1. Check if Fcitx5 addon library or descriptor is installed
    const QStringList probePaths = {
        "/usr/lib64/fcitx5/libbanglakey-fcitx5.so",
        "/usr/local/lib/fcitx5/libbanglakey-fcitx5.so",
        "/usr/local/lib64/fcitx5/libbanglakey-fcitx5.so",
        QDir::homePath() + "/.local/lib/fcitx5/libbanglakey-fcitx5.so",
        QDir::homePath() + "/.local/lib64/fcitx5/libbanglakey-fcitx5.so",
        "/usr/share/fcitx5/addon/banglakey.conf",
        QDir::homePath() + "/.local/share/fcitx5/addon/banglakey.conf"
    };

    fcitx5_installed_ = false;
    for (const auto& path : probePaths) {
        if (QFile::exists(path)) {
            fcitx5_installed_ = true;
            break;
        }
    }

    // 2. Check if fcitx5 process is active
    int ret = QProcess::execute("pgrep", {"-x", "fcitx5"});
    fcitx5_running_ = (ret == 0);

    emit statusChanged();
}

void SettingsBackend::refreshDiagnostics() {
    checkFcitx5Status();
    loadConfig();
    status_message_ = tr("Diagnostics refreshed.");
    emit statusMessageChanged();
}

void SettingsBackend::clearSandbox() {
    sandbox_commit_.clear();
    sandbox_preedit_.clear();
    if (engine_) {
        engine_->reset();
    }
    emit sandboxCommitChanged();
    emit sandboxPreeditChanged();
}

void SettingsBackend::processSandboxKey(int keycode, int keysym, int unicode, int modifiers, bool isRelease) {
    if (!engine_ || isRelease) {
        return;
    }

    uint32_t sym = static_cast<uint32_t>(keysym);
    if (sym == 0x01000003) { // Qt::Key_Backspace
        sym = 0xff08; // XK_BackSpace
    } else if (sym == 0x01000004 || sym == 0x01000005) { // Qt::Key_Return / Enter
        sym = 0xff0d; // XK_Return
    } else if (sym == 0x01000000) { // Qt::Key_Escape
        sym = 0xff1b; // XK_Escape
    } else if (sym == 0 && unicode > 0) {
        sym = static_cast<uint32_t>(unicode);
    }

    banglakey::KeyEvent bkEvent;
    bkEvent.keycode = static_cast<uint32_t>(keycode);
    bkEvent.keysym = sym;
    bkEvent.unicode = static_cast<uint32_t>(unicode);
    bkEvent.is_release = isRelease;

    banglakey::Modifier mod = banglakey::Modifier::None;
    if (modifiers & Qt::ShiftModifier) {
        mod = mod | banglakey::Modifier::Shift;
    }
    if (modifiers & Qt::ControlModifier) {
        mod = mod | banglakey::Modifier::Control;
    }
    if (modifiers & Qt::AltModifier) {
        mod = mod | banglakey::Modifier::Alt;
    }
    bkEvent.modifiers = mod;

    banglakey::EngineAction action = engine_->processKeyEvent(bkEvent);

    switch (action.type) {
        case banglakey::EngineActionType::Commit:
            sandbox_commit_.append(QString::fromStdString(action.commit_text));
            sandbox_preedit_.clear();
            emit sandboxCommitChanged();
            emit sandboxPreeditChanged();
            break;

        case banglakey::EngineActionType::Preedit:
            sandbox_preedit_ = QString::fromStdString(action.preedit_text);
            emit sandboxPreeditChanged();
            break;

        case banglakey::EngineActionType::Reset:
            sandbox_preedit_.clear();
            emit sandboxPreeditChanged();
            break;

        case banglakey::EngineActionType::ForwardKey:
        case banglakey::EngineActionType::None:
            break;
    }
}

bool SettingsBackend::loadConfig() {
    const QString path = configFilePath();
    QSettings settings(path, QSettings::IniFormat);

    config_file_valid_ = (settings.status() == QSettings::NoError);

    QString savedMode = settings.value("General/TypingMode", "Bijoy Unicode").toString();
    if (validateMode(savedMode)) {
        if (savedMode.compare("Bijoy Classic", Qt::CaseInsensitive) == 0) {
            current_mode_ = "Bijoy Classic";
        } else if (savedMode.compare("Bijoy Unicode", Qt::CaseInsensitive) == 0 ||
                   savedMode.compare("Bijoy", Qt::CaseInsensitive) == 0 ||
                   savedMode.compare("Unicode", Qt::CaseInsensitive) == 0) {
            current_mode_ = "Bijoy Unicode";
        } else if (savedMode.compare("Avro Phonetic", Qt::CaseInsensitive) == 0 ||
                   savedMode.compare("Avro", Qt::CaseInsensitive) == 0 ||
                   savedMode.compare("Phonetic", Qt::CaseInsensitive) == 0) {
            current_mode_ = "Avro Phonetic";
        } else if (savedMode.compare("English", Qt::CaseInsensitive) == 0) {
            current_mode_ = "English";
        } else {
            current_mode_ = "Bijoy Unicode";
        }
    } else {
        current_mode_ = "Bijoy Unicode";
    }

    QString savedApp = settings.value("General/Appearance", "system").toString();
    if (validateAppearance(savedApp)) {
        appearance_ = savedApp.toLower();
    } else {
        appearance_ = "system";
    }

    QString savedLang = settings.value("General/InterfaceLanguage", "en").toString();
    if (validateLanguage(savedLang)) {
        interface_language_ = savedLang;
    } else {
        interface_language_ = "en";
    }

    QString savedShortcut = settings.value("General/ModeSwitchShortcut", "F12").toString();
    if (validateShortcut(savedShortcut)) {
        mode_switch_shortcut_ = savedShortcut;
    } else {
        mode_switch_shortcut_ = "F12";
    }

    autostart_ = settings.value("General/Autostart", false).toBool();

    updateEngineMode();

    emit currentModeChanged();
    emit appearanceChanged();
    emit interfaceLanguageChanged();
    emit modeSwitchShortcutChanged();
    emit autostartChanged();
    emit statusChanged();

    return config_file_valid_;
}

bool SettingsBackend::saveConfig() {
    const QString path = configFilePath();
    QDir dir = QFileInfo(path).dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QSettings settings(path, QSettings::IniFormat);
    settings.setValue("General/TypingMode", current_mode_);
    settings.setValue("General/Appearance", appearance_);
    settings.setValue("General/InterfaceLanguage", interface_language_);
    settings.setValue("General/ModeSwitchShortcut", mode_switch_shortcut_);
    settings.setValue("General/Autostart", autostart_);
    settings.sync();

    bool success = (settings.status() == QSettings::NoError);
    config_file_valid_ = success;

    // Handle XDG autostart file if not in custom test mode
    if (custom_config_path_.isEmpty()) {
        QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/autostart";
        QString autostartFile = autostartDir + "/banglakey-settings.desktop";
        if (autostart_) {
            QDir().mkpath(autostartDir);
            QFile f(autostartFile);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&f);
                out << "[Desktop Entry]\n"
                    << "Type=Application\n"
                    << "Name=BanglaKey Settings\n"
                    << "Exec=banglakey-settings --minimized\n"
                    << "Icon=banglakey\n"
                    << "Comment=BanglaKey Input Method Settings\n"
                    << "Terminal=false\n";
            }
        } else {
            if (QFile::exists(autostartFile)) {
                QFile::remove(autostartFile);
            }
        }
    }

    if (success) {
        status_message_ = tr("Preferences saved successfully.");
    } else {
        status_message_ = tr("Error saving preferences to disk!");
    }
    emit statusMessageChanged();
    emit statusChanged();

    return success;
}

void SettingsBackend::restoreDefaults() {
    current_mode_ = "Bijoy Unicode";
    appearance_ = "system";
    interface_language_ = "en";
    mode_switch_shortcut_ = "F12";
    autostart_ = false;

    updateEngineMode();
    saveConfig();

    emit currentModeChanged();
    emit appearanceChanged();
    emit interfaceLanguageChanged();
    emit modeSwitchShortcutChanged();
    emit autostartChanged();

    status_message_ = tr("Settings restored to defaults.");
    emit statusMessageChanged();
}

QVariantMap SettingsBackend::getBijoyKeymap() const {
    QVariantMap map;
    // Base keys
    map["j"] = "ক";
    map["d"] = "ি";
    map["f"] = "া";
    map["g"] = "্ (Link)";
    map["h"] = "ব";
    map["k"] = "ত";
    map["l"] = "দ";
    map["u"] = "জ";
    map["i"] = "হ";
    map["o"] = "গ";
    map["p"] = "ড়";
    map["r"] = "প";
    map["s"] = "ু";
    map["w"] = "য";
    map["e"] = "ড";
    map["y"] = "চ";
    map["c"] = "ে";
    map["v"] = "র";
    map["b"] = "ন";
    map["n"] = "স";
    map["m"] = "ম";
    map["q"] = "ঙ";
    map["t"] = "ট";
    map["a"] = "ৃ";
    map["z"] = "্র";
    map["x"] = "ও";
    map["\\"] = "ৎ";

    // Shifted keys
    map["J"] = "খ";
    map["D"] = "ী";
    map["F"] = "অ";
    map["G"] = "। (দাঁড়ি)";
    map["H"] = "ভ";
    map["K"] = "থ";
    map["L"] = "ধ";
    map["U"] = "ঝ";
    map["I"] = "ঞ";
    map["O"] = "ঘ";
    map["P"] = "ঢ়";
    map["R"] = "ফ";
    map["S"] = "ূ";
    map["W"] = "য়";
    map["E"] = "ঢ";
    map["Y"] = "ছ";
    map["C"] = "ৈ";
    map["V"] = "ল";
    map["B"] = "ণ";
    map["N"] = "ষ";
    map["M"] = "শ";
    map["A"] = "র্";
    map["Q"] = "ং";
    map["T"] = "ঠ";
    map["Z"] = "্য";
    map["X"] = "ৌ";
    map["|"] = "ঃ";
    map["$"] = "৳";
    map["&"] = "ঁ";
    return map;
}

QVariantMap SettingsBackend::getPhoneticKeymap() const {
    QVariantMap map;
    map["k"] = "ক";
    map["kh"] = "খ";
    map["g"] = "গ";
    map["gh"] = "ঘ";
    map["ng"] = "ঙ";
    map["c/ch"] = "চ";
    map["ch/chh"] = "ছ";
    map["j"] = "জ";
    map["jh"] = "ঝ";
    map["T"] = "ট";
    map["Th"] = "ঠ";
    map["D"] = "ড";
    map["Dh"] = "ঢ";
    map["N"] = "ণ";
    map["t"] = "ত";
    map["th"] = "থ";
    map["d"] = "দ";
    map["dh"] = "ধ";
    map["n"] = "ন";
    map["p"] = "প";
    map["ph/f"] = "ফ";
    map["b"] = "ব";
    map["bh/v"] = "ভ";
    map["m"] = "ম";
    map["z/j"] = "য";
    map["r"] = "র";
    map["l"] = "ল";
    map["sh/S"] = "শ";
    map["sh/ss"] = "ষ";
    map["s"] = "স";
    map["h"] = "হ";
    map["R"] = "ড়";
    map["Rh"] = "ঢ়";
    map["y"] = "য়";
    map["t`"] = "ৎ";
    map["ng/ngg"] = "ং";
    map["::"] = "ঃ";
    map["^"] = "ঁ";
    return map;
}

void SettingsBackend::requestShowToolbar() {
    emit showToolbarRequested();
}

void SettingsBackend::requestHideToolbar() {
    emit hideToolbarRequested();
}

void SettingsBackend::requestOpenSettings() {
    emit openSettingsRequested();
}

void SettingsBackend::requestExit() {
    emit exitRequested();
}

void SettingsBackend::onSplashFinished() {
    emit showToolbarRequested();
}

void SettingsBackend::openLayoutMenu(int globalX, int globalY) {
    QMenu menu;
    menu.setStyleSheet(
        "QMenu {"
        "  background-color: #ffffff;"
        "  color: #1a1a1a;"
        "  border: 1px solid #c0c0c0;"
        "  padding: 4px;"
        "  font-size: 13px;"
        "}"
        "QMenu::item {"
        "  padding: 6px 28px 6px 14px;"
        "  margin: 1px 0px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: #e3f2fd;"
        "  color: #0d47a1;"
        "}"
    );

    auto* actEnglish = menu.addAction(QString::fromUtf8("English (ইংরেজি)"));
    auto* actBijoyUni = menu.addAction(QString::fromUtf8("Bijoy Unicode (বিজয় ইউনিকোড)"));
    auto* actClassic = menu.addAction(QString::fromUtf8("Bijoy Classic (বিজয় ক্লাসিক)"));
    auto* actAvro = menu.addAction(QString::fromUtf8("Avro Phonetic (অভ্রো ফোনেটিক)"));

    actEnglish->setCheckable(true);
    actBijoyUni->setCheckable(true);
    actClassic->setCheckable(true);
    actAvro->setCheckable(true);

    QString cur = currentMode();
    actEnglish->setChecked(cur == "English");
    actBijoyUni->setChecked(cur == "Bijoy Unicode");
    actClassic->setChecked(cur == "Bijoy Classic");
    actAvro->setChecked(cur == "Avro Phonetic");

    menu.addSeparator();
    auto* actTray = menu.addAction(QString::fromUtf8("Minimize to System Tray (ট্রে-তে পাঠান)"));
    auto* actClose = menu.addAction(QString::fromUtf8("Exit BanglaKey (অ্যাপ বন্ধ করুন)"));

    QAction* selected = menu.exec(QPoint(globalX, globalY));
    if (!selected) return;

    if (selected == actEnglish) {
        setCurrentMode("English");
    } else if (selected == actBijoyUni) {
        setCurrentMode("Bijoy Unicode");
    } else if (selected == actClassic) {
        setCurrentMode("Bijoy Classic");
    } else if (selected == actAvro) {
        setCurrentMode("Avro Phonetic");
    } else if (selected == actTray) {
        emit hideToolbarRequested();
    } else if (selected == actClose) {
        emit exitRequested();
    }
}

} // namespace banglakey::desktop
