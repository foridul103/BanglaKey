#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <memory>

#include "settings_backend.hpp"

namespace {

// Global QCoreApplication instance needed for Qt event loop and QSettings
class QtEnvironment : public ::testing::Environment {
public:
    void SetUp() override {
        if (!QCoreApplication::instance()) {
            static int argc = 1;
            static char app_name[] = "banglakey_settings_test";
            static char* argv[] = {app_name, nullptr};
            app_ = std::make_unique<QCoreApplication>(argc, argv);
        }
    }

private:
    std::unique_ptr<QCoreApplication> app_;
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

    EXPECT_EQ(backend.currentMode(), "Phonetic");
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
        backend.setCurrentMode("Bijoy");
        EXPECT_EQ(backend.currentMode(), "Bijoy");
        EXPECT_TRUE(QFile::exists(testConfigPath()));
    }

    // Verify persistence in a brand new backend instance
    {
        banglakey::desktop::SettingsBackend backend2(testConfigPath());
        EXPECT_EQ(backend2.currentMode(), "Bijoy");
    }
}

TEST_F(SettingsBackendTest, RejectInvalidValuesAndPreserveCurrent) {
    banglakey::desktop::SettingsBackend backend(testConfigPath());

    // 1. Invalid typing mode
    EXPECT_FALSE(backend.validateMode("non_existent_mode"));
    EXPECT_FALSE(backend.validateMode(""));
    backend.setCurrentMode("invalid_mode");
    EXPECT_EQ(backend.currentMode(), "Phonetic"); // Remains Phonetic

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
        backend.setCurrentMode("Bijoy");
        backend.setAppearance("dark");
    }

    // Verify unrelated keys are completely preserved
    {
        QSettings settings(testConfigPath(), QSettings::IniFormat);
        EXPECT_EQ(settings.value("General/TypingMode").toString(), "Bijoy");
        EXPECT_EQ(settings.value("General/Appearance").toString(), "dark");
        EXPECT_EQ(settings.value("CustomPlugin/ApiKey").toString(), "xyz-12345");
        EXPECT_EQ(settings.value("General/ThirdPartyFlag").toInt(), 42);
    }
}

TEST_F(SettingsBackendTest, RestoreDefaultsExplicitAction) {
    // 1. Set custom settings
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        backend.setCurrentMode("Bijoy");
        backend.setAppearance("light");
        backend.setInterfaceLanguage("bn");
        backend.setModeSwitchShortcut("Ctrl+Alt+M");
        backend.setAutostart(true);
    }

    // 2. Restore defaults
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        EXPECT_EQ(backend.currentMode(), "Bijoy");
        backend.restoreDefaults();

        EXPECT_EQ(backend.currentMode(), "Phonetic");
        EXPECT_EQ(backend.appearance(), "system");
        EXPECT_EQ(backend.interfaceLanguage(), "en");
        EXPECT_EQ(backend.modeSwitchShortcut(), "F12");
        EXPECT_FALSE(backend.autostart());
    }

    // 3. Verify on disk
    {
        banglakey::desktop::SettingsBackend backend2(testConfigPath());
        EXPECT_EQ(backend2.currentMode(), "Phonetic");
        EXPECT_EQ(backend2.appearance(), "system");
        EXPECT_EQ(backend2.interfaceLanguage(), "en");
        EXPECT_EQ(backend2.modeSwitchShortcut(), "F12");
        EXPECT_FALSE(backend2.autostart());
    }
}

TEST_F(SettingsBackendTest, PersistenceAcrossMultipleFields) {
    {
        banglakey::desktop::SettingsBackend backend(testConfigPath());
        backend.setCurrentMode("Bijoy");
        backend.setAppearance("dark");
        backend.setInterfaceLanguage("bn");
        backend.setModeSwitchShortcut("Ctrl+Alt+M");
        backend.setAutostart(true);
    }

    {
        banglakey::desktop::SettingsBackend backend2(testConfigPath());
        EXPECT_EQ(backend2.currentMode(), "Bijoy");
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

    // Test Sandbox execution: Phonetic mode (default)
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
