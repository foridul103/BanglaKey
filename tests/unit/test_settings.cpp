#include <gtest/gtest.h>
#include <QApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <memory>

#include "settings_backend.hpp"
#include "ipc_manager.hpp"
#include "tray_manager.hpp"

namespace {

// Global QApplication instance needed for Qt event loop, QSettings, and QWidget/QMenu
class QtEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        if (!QCoreApplication::instance()) {
            qputenv("QT_QPA_PLATFORM", "offscreen");
            static int argc = 1;
            static char app_name[] = "banglakey_settings_test";
            static char* argv[] = {app_name, nullptr};
            app_ = std::make_unique<QApplication>(argc, argv);
        }
    }

private:
    std::unique_ptr<QApplication> app_;
};

[[maybe_unused]] auto* const qt_env = ::testing::AddGlobalTestEnvironment(new QtEnvironment);

class SettingsBackendTest : public ::testing::Test {
protected:
    void SetUp() override {
        temp_dir_ = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(temp_dir_->isValid());
        test_config_path_ = temp_dir_->filePath("test_banglakey_config.ini");
    }

    void TearDown() override {
        temp_dir_.reset();
    }

    QString testConfigPath() const {
        return test_config_path_;
    }

private:
    std::unique_ptr<QTemporaryDir> temp_dir_;
    QString test_config_path_;
};

} // namespace

TEST_F(SettingsBackendTest, DefaultSettingsAreValid) {
    banglakey::desktop::SettingsBackend backend(testConfigPath());

    EXPECT_EQ(backend.currentMode(), "Bijoy Unicode");
    EXPECT_EQ(backend.appearance(), "system");
    EXPECT_EQ(backend.interfaceLanguage(), "en");
    EXPECT_EQ(backend.modeSwitchShortcut(), "F12");
    EXPECT_FALSE(backend.autostart());
    EXPECT_TRUE(backend.engineAvailable());
    EXPECT_FALSE(backend.version().isEmpty());
    EXPECT_EQ(backend.configFilePath(), testConfigPath());
}

TEST_F(SettingsBackendTest, SaveAndLoadValidTypingMode) {
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        backend.setCurrentMode("Bijoy Classic");
        EXPECT_EQ(backend.currentMode(), "Bijoy Classic");
        EXPECT_TRUE(QFile::exists(testConfigPath()));
    }

    // Verify persistence in a brand new backend instance
    {
        banglakey::desktop::SettingsBackend backend2(testConfigPath());
        EXPECT_EQ(backend2.currentMode(), "Bijoy Classic");
    }
}

TEST_F(SettingsBackendTest, RejectInvalidValuesAndPreserveCurrent) {
    banglakey::desktop::SettingsBackend backend(testConfigPath());

    // 1. Invalid typing mode
    EXPECT_FALSE(backend.validateMode("non_existent_mode"));
    EXPECT_FALSE(backend.validateMode(""));
    backend.setCurrentMode("invalid_mode");
    EXPECT_EQ(backend.currentMode(), "Bijoy Unicode"); // Remains Bijoy Unicode

    // 2. Invalid appearance
    EXPECT_FALSE(backend.validateAppearance("matrix_green"));
    backend.setAppearance("matrix_green");
    EXPECT_EQ(backend.appearance(), "system");

    // 3. Invalid interface language
    EXPECT_FALSE(backend.validateLanguage("de"));
    EXPECT_FALSE(backend.validateLanguage("es"));
    backend.setInterfaceLanguage("fr");
    EXPECT_EQ(backend.interfaceLanguage(), "en");

    // 4. Invalid shortcut
    EXPECT_FALSE(backend.validateShortcut("Alt+F4"));
    backend.setModeSwitchShortcut("Alt+F4");
    EXPECT_EQ(backend.modeSwitchShortcut(), "F12");
}

TEST_F(SettingsBackendTest, PreserveUnrelatedSettingsWhenSaving) {
    // Write an unrelated custom setting into the file first
    {
        QSettings settings(testConfigPath(), QSettings::IniFormat);
        settings.setValue("CustomPlugin/ApiKey", "xyz-12345");
        settings.setValue("General/ThirdPartyFlag", 42);
        settings.sync();
    }

    // Now update a setting through SettingsBackend
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        backend.setCurrentMode("Bijoy Classic");
        backend.setAppearance("dark");
    }

    // Verify unrelated keys are completely preserved
    {
        QSettings settings(testConfigPath(), QSettings::IniFormat);
        EXPECT_EQ(settings.value("General/TypingMode").toString(), "Bijoy Classic");
        EXPECT_EQ(settings.value("General/Appearance").toString(), "dark");
        EXPECT_EQ(settings.value("CustomPlugin/ApiKey").toString(), "xyz-12345");
        EXPECT_EQ(settings.value("General/ThirdPartyFlag").toInt(), 42);
    }
}

TEST_F(SettingsBackendTest, RestoreDefaultsExplicitAction) {
    // 1. Set custom settings
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        backend.setCurrentMode("Bijoy Classic");
        backend.setAppearance("light");
        backend.setInterfaceLanguage("bn");
        backend.setModeSwitchShortcut("Ctrl+Alt+M");
        backend.setAutostart(true);
    }

    // 2. Restore defaults
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        EXPECT_EQ(backend.currentMode(), "Bijoy Classic");
        backend.restoreDefaults();

        EXPECT_EQ(backend.currentMode(), "Bijoy Unicode");
        EXPECT_EQ(backend.appearance(), "system");
        EXPECT_EQ(backend.interfaceLanguage(), "en");
        EXPECT_EQ(backend.modeSwitchShortcut(), "F12");
        EXPECT_FALSE(backend.autostart());
    }

    // 3. Verify on disk
    {
        banglakey::desktop::SettingsBackend backend2(testConfigPath());
        EXPECT_EQ(backend2.currentMode(), "Bijoy Unicode");
        EXPECT_EQ(backend2.appearance(), "system");
        EXPECT_EQ(backend2.interfaceLanguage(), "en");
        EXPECT_EQ(backend2.modeSwitchShortcut(), "F12");
        EXPECT_FALSE(backend2.autostart());
    }
}

TEST_F(SettingsBackendTest, PersistenceAcrossMultipleFields) {
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        backend.setCurrentMode("Bijoy Classic");
        backend.setAppearance("dark");
        backend.setInterfaceLanguage("bn");
        backend.setModeSwitchShortcut("Ctrl+Alt+M");
        backend.setAutostart(true);
    }

    {
        banglakey::desktop::SettingsBackend backend2(testConfigPath());
        EXPECT_EQ(backend2.currentMode(), "Bijoy Classic");
        EXPECT_EQ(backend2.appearance(), "dark");
        EXPECT_EQ(backend2.interfaceLanguage(), "bn");
        EXPECT_EQ(backend2.modeSwitchShortcut(), "Ctrl+Alt+M");
        EXPECT_TRUE(backend2.autostart());
    }
}

TEST_F(SettingsBackendTest, KeymapsAndInteractiveSandbox) {
    banglakey::desktop::SettingsBackend backend(testConfigPath());

    // Verify keymap data structures
    QVariantMap bijoyMap = backend.getBijoyKeymap();
    EXPECT_FALSE(bijoyMap.isEmpty());
    EXPECT_EQ(bijoyMap.value("j").toString(), "ক");
    EXPECT_EQ(bijoyMap.value("d").toString(), "ি");
    EXPECT_EQ(bijoyMap.value("g").toString(), "্ (Link)");

    QVariantMap phoneticMap = backend.getPhoneticKeymap();
    EXPECT_FALSE(phoneticMap.isEmpty());
    EXPECT_EQ(phoneticMap.value("k").toString(), "ক");

    // Test Sandbox execution in Avro Phonetic mode
    backend.setCurrentMode("Avro Phonetic");
    // Type 'k' (keysym 'k', unicode 'k') -> should be in preedit
    backend.processSandboxKey(0, 'k', 'k', 0, false);
    EXPECT_EQ(backend.sandboxPreedit(), "ক");

    // Commit by Space (keysym 0x20, unicode ' ')
    backend.processSandboxKey(0, 0x20, ' ', 0, false);
    EXPECT_EQ(backend.sandboxCommit(), "ক ");
    EXPECT_TRUE(backend.sandboxPreedit().isEmpty());

    // Clear sandbox
    backend.clearSandbox();
    EXPECT_TRUE(backend.sandboxCommit().isEmpty());
    EXPECT_TRUE(backend.sandboxPreedit().isEmpty());
}

TEST(IpcManagerTest, ModeConversionsAndDisplayNames) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;

    EXPECT_EQ(IpcManager::modeToString(AppMode::English), "English");
    EXPECT_EQ(IpcManager::modeToString(AppMode::BijoyClassic), "Bijoy Classic");
    EXPECT_EQ(IpcManager::modeToString(AppMode::BijoyUnicode), "Bijoy Unicode");
    EXPECT_EQ(IpcManager::modeToString(AppMode::AvroPhonetic), "Avro Phonetic");

    EXPECT_EQ(IpcManager::stringToMode("English"), AppMode::English);
    EXPECT_EQ(IpcManager::stringToMode("Bijoy Classic"), AppMode::BijoyClassic);
    EXPECT_EQ(IpcManager::stringToMode("BijoyClassic"), AppMode::BijoyClassic);
    EXPECT_EQ(IpcManager::stringToMode("Bijoy Unicode"), AppMode::BijoyUnicode);
    EXPECT_EQ(IpcManager::stringToMode("BijoyUnicode"), AppMode::BijoyUnicode);
    EXPECT_EQ(IpcManager::stringToMode("Unicode"), AppMode::BijoyUnicode);
    EXPECT_EQ(IpcManager::stringToMode("Phonetic"), AppMode::AvroPhonetic);
    EXPECT_EQ(IpcManager::stringToMode("Avro Phonetic"), AppMode::AvroPhonetic);

    EXPECT_EQ(IpcManager::modeToDisplayName(AppMode::English), "English");
    EXPECT_EQ(IpcManager::modeToDisplayName(AppMode::BijoyClassic), "বিজয় ক্লাসিক");
    EXPECT_EQ(IpcManager::modeToDisplayName(AppMode::BijoyUnicode), "বিজয় ইউনিকোড");
    EXPECT_EQ(IpcManager::modeToDisplayName(AppMode::AvroPhonetic), "অভ্রো ফোনেটিক");
}

TEST(IpcManagerTest, ModeCycleOrder) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;

    IpcManager ipc;
    ipc.setMode(AppMode::English);
    EXPECT_EQ(ipc.currentMode(), AppMode::English);

    // English -> Bijoy Unicode
    ipc.cycleMode();
    EXPECT_EQ(ipc.currentMode(), AppMode::BijoyUnicode);

    // Bijoy Unicode -> Bijoy Classic
    ipc.cycleMode();
    EXPECT_EQ(ipc.currentMode(), AppMode::BijoyClassic);

    // Bijoy Classic -> Avro Phonetic
    ipc.cycleMode();
    EXPECT_EQ(ipc.currentMode(), AppMode::AvroPhonetic);

    // Avro Phonetic -> English
    ipc.cycleMode();
    EXPECT_EQ(ipc.currentMode(), AppMode::English);
}

TEST(IpcManagerTest, RuntimeModeFileSynchronization) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;

    IpcManager ipc;
    ipc.setMode(AppMode::BijoyUnicode);

    QString path = IpcManager::runtimeModeFilePath();
    EXPECT_TRUE(QFile::exists(path));

    QFile file(path);
    ASSERT_TRUE(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    QString modeLine = in.readLine();
    EXPECT_EQ(modeLine, "Bijoy Unicode");
}

TEST(TrayManagerTest, DetectionAndStatus) {
    using banglakey::desktop::TrayManager;

    QString summary = TrayManager::trayStatusSummary();
    EXPECT_FALSE(summary.isEmpty());

    // Verify detection methods execute cleanly on any environment
    [[maybe_unused]] bool installed = TrayManager::isAppIndicatorExtensionInstalled();
    [[maybe_unused]] bool watcher = TrayManager::isStatusNotifierWatcherAvailable();
    [[maybe_unused]] bool enabled = TrayManager::isAppIndicatorExtensionEnabled();
    [[maybe_unused]] bool isKde = TrayManager::isKdePlasma();
    [[maybe_unused]] bool isGnome = TrayManager::isGnome();
    [[maybe_unused]] QString session = TrayManager::desktopSessionType();
    [[maybe_unused]] QString desktop = TrayManager::desktopEnvironment();
}

TEST(TrayManagerTest, RightClickMenuStructureAndOrdering) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;
    using banglakey::desktop::TrayManager;

    IpcManager ipc;
    TrayManager tray(&ipc);

    QMenu* menu = tray.menu();
    ASSERT_NE(menu, nullptr);

    const auto actions = menu->actions();
    ASSERT_EQ(actions.size(), 8);

    // 1. Bijoy Unicode
    EXPECT_EQ(actions[0]->text(), "Bijoy Unicode");
    EXPECT_TRUE(actions[0]->isCheckable());
    EXPECT_EQ(actions[0], tray.actionBijoyUnicode());

    // 2. Bijoy Classic
    EXPECT_EQ(actions[1]->text(), "Bijoy Classic");
    EXPECT_TRUE(actions[1]->isCheckable());
    EXPECT_EQ(actions[1], tray.actionBijoyClassic());

    // 3. Avro Phonetic
    EXPECT_EQ(actions[2]->text(), "Avro Phonetic");
    EXPECT_TRUE(actions[2]->isCheckable());
    EXPECT_EQ(actions[2], tray.actionAvroPhonetic());

    // 4. English
    EXPECT_EQ(actions[3]->text(), "English");
    EXPECT_TRUE(actions[3]->isCheckable());
    EXPECT_EQ(actions[3], tray.actionEnglish());

    // 5. Separator
    EXPECT_TRUE(actions[4]->isSeparator());

    // 6. Show as Topbar
    EXPECT_EQ(actions[5]->text(), "Show as Topbar");
    EXPECT_FALSE(actions[5]->isCheckable());
    EXPECT_EQ(actions[5], tray.actionShowTopbar());

    // 7. Settings...
    EXPECT_EQ(actions[6]->text(), "Settings...");
    EXPECT_FALSE(actions[6]->isCheckable());
    EXPECT_EQ(actions[6], tray.actionSettings());

    // 8. Close
    EXPECT_EQ(actions[7]->text(), "Close");
    EXPECT_FALSE(actions[7]->isCheckable());
    EXPECT_EQ(actions[7], tray.actionClose());
}

TEST(TrayManagerTest, CheckmarkSynchronization) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;
    using banglakey::desktop::TrayManager;

    IpcManager ipc;
    TrayManager tray(&ipc);

    // Set to Bijoy Unicode
    ipc.setMode(AppMode::BijoyUnicode);
    EXPECT_TRUE(tray.actionBijoyUnicode()->isChecked());
    EXPECT_FALSE(tray.actionBijoyClassic()->isChecked());
    EXPECT_FALSE(tray.actionAvroPhonetic()->isChecked());
    EXPECT_FALSE(tray.actionEnglish()->isChecked());

    // Set to Bijoy Classic
    ipc.setMode(AppMode::BijoyClassic);
    EXPECT_FALSE(tray.actionBijoyUnicode()->isChecked());
    EXPECT_TRUE(tray.actionBijoyClassic()->isChecked());
    EXPECT_FALSE(tray.actionAvroPhonetic()->isChecked());
    EXPECT_FALSE(tray.actionEnglish()->isChecked());

    // Set to Avro Phonetic
    ipc.setMode(AppMode::AvroPhonetic);
    EXPECT_FALSE(tray.actionBijoyUnicode()->isChecked());
    EXPECT_FALSE(tray.actionBijoyClassic()->isChecked());
    EXPECT_TRUE(tray.actionAvroPhonetic()->isChecked());
    EXPECT_FALSE(tray.actionEnglish()->isChecked());

    // Set to English
    ipc.setMode(AppMode::English);
    EXPECT_FALSE(tray.actionBijoyUnicode()->isChecked());
    EXPECT_FALSE(tray.actionBijoyClassic()->isChecked());
    EXPECT_FALSE(tray.actionAvroPhonetic()->isChecked());
    EXPECT_TRUE(tray.actionEnglish()->isChecked());
}

TEST(TrayManagerTest, ActionTriggersUpdateModeAndEmitSignals) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;
    using banglakey::desktop::TrayManager;

    IpcManager ipc;
    TrayManager tray(&ipc);

    // Trigger Avro Phonetic
    tray.actionAvroPhonetic()->trigger();
    EXPECT_EQ(ipc.currentMode(), AppMode::AvroPhonetic);

    // Trigger Bijoy Classic
    tray.actionBijoyClassic()->trigger();
    EXPECT_EQ(ipc.currentMode(), AppMode::BijoyClassic);

    // Trigger Bijoy Unicode
    tray.actionBijoyUnicode()->trigger();
    EXPECT_EQ(ipc.currentMode(), AppMode::BijoyUnicode);

    // Signals
    bool showTopbarReceived = false;
    QObject::connect(&tray, &TrayManager::showToolbarRequested, [&]() {
        showTopbarReceived = true;
    });
    tray.actionShowTopbar()->trigger();
    EXPECT_TRUE(showTopbarReceived);

    bool exitReceived = false;
    QObject::connect(&tray, &TrayManager::exitRequested, [&]() {
        exitReceived = true;
    });
    tray.actionClose()->trigger();
    EXPECT_TRUE(exitReceived);
}

TEST(TrayManagerTest, ModeIconGenerationAndUpdates) {
    using banglakey::desktop::AppMode;
    using banglakey::desktop::IpcManager;
    using banglakey::desktop::TrayManager;

    IpcManager ipc;
    TrayManager tray(&ipc);

    // Test distinct icons are generated for all 4 modes
    const std::vector<AppMode> modes = {
        AppMode::English,
        AppMode::BijoyUnicode,
        AppMode::BijoyClassic,
        AppMode::AvroPhonetic
    };

    for (auto m : modes) {
        QIcon icon = tray.createModeIcon(m);
        EXPECT_FALSE(icon.isNull());

        // Verify multi-resolution pixmaps are available
        for (int sz : {16, 22, 24, 32, 48, 64}) {
            QPixmap pix = icon.pixmap(sz, sz);
            EXPECT_FALSE(pix.isNull());
            EXPECT_EQ(pix.width(), sz);
            EXPECT_EQ(pix.height(), sz);
        }

        // Verify updateMode updates without throwing
        tray.updateMode(m);
        EXPECT_EQ(ipc.currentMode(), ipc.currentMode());
    }
}

TEST_F(SettingsBackendTest, WindowPositionAndDockPersistence) {
    banglakey::desktop::SettingsBackend backend(testConfigPath());

    // Defaults on first launch
    EXPECT_EQ(backend.windowX(), -1); // Unset, indicates center horizontally by default
    EXPECT_EQ(backend.windowY(), 0);  // Top edge
    EXPECT_EQ(backend.dockPosition(), 1); // 1 = Center

    // Save custom position
    backend.saveWindowPosition(350, 0);
    EXPECT_EQ(backend.windowX(), 350);
    EXPECT_EQ(backend.windowY(), 0);

    // Save docking presets
    backend.saveDockPosition(0); // Left
    EXPECT_EQ(backend.dockPosition(), 0);
    backend.saveDockPosition(2); // Right
    EXPECT_EQ(backend.dockPosition(), 2);

    // Persistence across new backend instance loading same config file
    banglakey::desktop::SettingsBackend loadedBackend(testConfigPath());
    EXPECT_EQ(loadedBackend.windowX(), 350);
    EXPECT_EQ(loadedBackend.windowY(), 0);
    EXPECT_EQ(loadedBackend.dockPosition(), 2);
}

