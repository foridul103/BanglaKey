#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
#include <fcitx/instance.h>
#include <fcitx-utils/keysymgen.h>

#include "banglakey_addon.hpp"

namespace banglakey::fcitx5::testing {

class MockInputContext : public fcitx::InputContextV2 {
public:
    explicit MockInputContext(fcitx::InputContextManager& mgr)
        : fcitx::InputContextV2(mgr, "banglakey-test-client") {}

    ~MockInputContext() override {
        destroy();
    }

    const char* frontend() const override { return "mock"; }

    void commitStringImpl(const std::string& text) override {
        committed_text += text;
        last_commit = text;
    }

    void deleteSurroundingTextImpl(int, unsigned int) override {}
    void forwardKeyImpl(const fcitx::ForwardKeyEvent&) override {}
    void updatePreeditImpl() override {}

    void commitStringWithCursorImpl(const std::string& text, size_t) override {
        committed_text += text;
        last_commit = text;
    }

    std::string committed_text{};
    std::string last_commit{};
};

class Fcitx5AddonTest : public ::testing::Test {
protected:
    std::string original_xdg_{};
    std::string test_runtime_dir_{};

    void SetUp() override {
        const char* xdg = getenv("XDG_RUNTIME_DIR");
        if (xdg) original_xdg_ = xdg;
        test_runtime_dir_ = "/tmp/banglakey_test_runtime_" + std::to_string(getpid());
        std::filesystem::create_directories(test_runtime_dir_ + "/banglakey");
        setenv("XDG_RUNTIME_DIR", test_runtime_dir_.c_str(), 1);

        char arg0[] = "banglakey-test";
        char* args[] = {arg0, nullptr};
        instance_ = std::make_unique<fcitx::Instance>(1, args);
        instance_->initialize();
        addon_ = std::make_unique<BanglaKeyAddon>(instance_.get());
        entry_ = std::make_unique<fcitx::InputMethodEntry>("banglakey", "BanglaKey", "bn", "banglakey");
    }

    void TearDown() override {
        addon_.reset();
        instance_.reset();
        std::error_code ec;
        std::filesystem::remove_all(test_runtime_dir_, ec);
        if (!original_xdg_.empty()) {
            setenv("XDG_RUNTIME_DIR", original_xdg_.c_str(), 1);
        } else {
            unsetenv("XDG_RUNTIME_DIR");
        }
    }

    void sendKey(MockInputContext& ic, fcitx::KeySym sym, fcitx::KeyStates states = fcitx::KeyStates()) {
        fcitx::Key key(sym, states);
        fcitx::KeyEvent pressEvent(&ic, key, false);
        addon_->keyEvent(*entry_, pressEvent);
        fcitx::KeyEvent releaseEvent(&ic, key, true);
        addon_->keyEvent(*entry_, releaseEvent);
    }

    std::unique_ptr<fcitx::Instance> instance_{nullptr};
    std::unique_ptr<BanglaKeyAddon> addon_{nullptr};
    std::unique_ptr<fcitx::InputMethodEntry> entry_{nullptr};
};

TEST_F(Fcitx5AddonTest, AddonRegistrationAndMetadata) {
    auto imList = addon_->listInputMethods();
    ASSERT_EQ(imList.size(), 1u);
    EXPECT_EQ(imList[0].uniqueName(), "banglakey");
    EXPECT_EQ(imList[0].name(), "BanglaKey");
    EXPECT_EQ(imList[0].languageCode(), "bn");
    EXPECT_EQ(imList[0].nativeName(), "বাংলাকি");
    EXPECT_EQ(imList[0].label(), "বাং");
}

TEST_F(Fcitx5AddonTest, InitialModeIsBijoyUnicode) {
    MockInputContext ic(instance_->inputContextManager());
    EXPECT_EQ(addon_->subMode(*entry_, ic), "Bijoy Unicode");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "বি-ইউ");
}

TEST_F(Fcitx5AddonTest, PhoneticTypingAndSpaceCommit) {
    MockInputContext ic(instance_->inputContextManager());

    // Switch to Avro Phonetic
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");

    // Type "amar" -> "আমার"
    sendKey(ic, FcitxKey_a);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আ");

    sendKey(ic, FcitxKey_m);
    sendKey(ic, FcitxKey_a);
    sendKey(ic, FcitxKey_r);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আমার");

    // Space commits word with space
    sendKey(ic, FcitxKey_space);
    EXPECT_EQ(ic.committed_text, "আমার ");
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());
}

TEST_F(Fcitx5AddonTest, CandidateListGenerationAndHotKeySelection) {
    MockInputContext ic(instance_->inputContextManager());

    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);

    // Type "khoroch"
    const std::string word = "khoroch";
    for (char c : word) {
        sendKey(ic, static_cast<fcitx::KeySym>(c));
    }

    // Candidate list must be active
    auto candList = ic.inputPanel().candidateList();
    ASSERT_TRUE(candList != nullptr);
    ASSERT_GT(candList->size(), 0);

    // Primary candidate for khoroch is খরচ
    EXPECT_EQ(candList->candidate(0).text().toString(), "খরচ");

    // Select candidate 1 using number key '1'
    sendKey(ic, FcitxKey_1);
    EXPECT_EQ(ic.committed_text, "খরচ");
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());
}

TEST_F(Fcitx5AddonTest, CandidateSelectionViaCandidateWordMethod) {
    MockInputContext ic(instance_->inputContextManager());

    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);

    sendKey(ic, FcitxKey_k);
    sendKey(ic, FcitxKey_h);
    sendKey(ic, FcitxKey_o);

    auto candList = ic.inputPanel().candidateList();
    ASSERT_TRUE(candList != nullptr);
    ASSERT_GT(candList->size(), 0);

    // Directly trigger UI candidate selection callback
    candList->candidate(0).select(&ic);
    EXPECT_FALSE(ic.committed_text.empty());
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());
}

TEST_F(Fcitx5AddonTest, ModeSwitchViaF12PreservesUnfinishedText) {
    MockInputContext ic(instance_->inputContextManager());

    // Switch to Avro Phonetic
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->subMode(*entry_, ic), "Avro Phonetic");

    // Type "am" in phonetic
    sendKey(ic, FcitxKey_a);
    sendKey(ic, FcitxKey_m);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আম");

    // Hit F12: Cycle English -> Bijoy Unicode -> Bijoy Classic -> Avro Phonetic -> English
    // From Avro Phonetic, next is English
    sendKey(ic, FcitxKey_F12);

    EXPECT_EQ(addon_->activeMode(), "English");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "EN");
    EXPECT_EQ(ic.committed_text, "আম");
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());

    // Cycle next: Bijoy Unicode
    sendKey(ic, FcitxKey_F12);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "বি-ইউ");

    // Now in Bijoy Unicode: press 'k' -> 'ত'
    sendKey(ic, FcitxKey_k);
    EXPECT_EQ(ic.committed_text, "আমত");

    // Cycle next: Bijoy Classic
    sendKey(ic, FcitxKey_F12);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "ক্লাসিক");

    // Cycle next: Avro Phonetic
    sendKey(ic, FcitxKey_F12);
    EXPECT_EQ(addon_->subMode(*entry_, ic), "Avro Phonetic");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "অভ্রো");

    // Cycle next: English
    sendKey(ic, FcitxKey_F12);
    EXPECT_EQ(addon_->activeMode(), "English");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "EN");
}

TEST_F(Fcitx5AddonTest, MultiContextIsolation) {
    MockInputContext ic1(instance_->inputContextManager());
    MockInputContext ic2(instance_->inputContextManager());

    // ic1 switches to Avro Phonetic, ic2 switches to Bijoy Unicode
    sendKey(ic1, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    sendKey(ic2, FcitxKey_a, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->subMode(*entry_, ic1), "Avro Phonetic");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic1), "অভ্রো");
    EXPECT_EQ(addon_->subMode(*entry_, ic2), "Bijoy Unicode");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic2), "বি-ইউ");

    // Type in ic1: "a" -> "আ" (Avro Phonetic preedit)
    sendKey(ic1, FcitxKey_a);
    EXPECT_EQ(ic1.inputPanel().preedit().toString(), "আ");
    EXPECT_TRUE(ic2.inputPanel().preedit().toString().empty());

    // Type in ic2: "j" -> "ক" (Bijoy Unicode direct layout mapping)
    sendKey(ic2, FcitxKey_j);
    EXPECT_EQ(ic2.committed_text, "ক");
    EXPECT_EQ(ic1.inputPanel().preedit().toString(), "আ");
}

TEST_F(Fcitx5AddonTest, BackspaceAndEscapeHandling) {
    MockInputContext ic(instance_->inputContextManager());
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);

    sendKey(ic, FcitxKey_a);
    sendKey(ic, FcitxKey_m);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আম");

    // Backspace removes 'm'
    sendKey(ic, FcitxKey_BackSpace);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আ");

    // Escape cancels composition without committing
    sendKey(ic, FcitxKey_Escape);
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());
    EXPECT_TRUE(ic.committed_text.empty());
}

TEST_F(Fcitx5AddonTest, ShortcutPassthroughAndCommit) {
    MockInputContext ic(instance_->inputContextManager());
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);

    // Type preedit
    sendKey(ic, FcitxKey_a);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আ");

    // Hit Ctrl+C
    fcitx::Key keyCtrlC(FcitxKey_c, fcitx::KeyState::Ctrl);
    fcitx::KeyEvent evCtrlC(&ic, keyCtrlC, false);
    addon_->keyEvent(*entry_, evCtrlC);

    // Preedit was committed to avoid loss, and event was forwarded (not filtered)
    EXPECT_EQ(ic.committed_text, "আ");
    EXPECT_FALSE(evCtrlC.filtered());
}

TEST_F(Fcitx5AddonTest, ModeSwitchFromDiskPreservesUnfinishedText) {
    MockInputContext ic(instance_->inputContextManager());
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);

    // Type "am" in phonetic
    sendKey(ic, FcitxKey_a);
    sendKey(ic, FcitxKey_m);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আম");

    // Write "Bijoy Classic" to runtime mode file
    auto ctxId = reinterpret_cast<banglakey::ContextId>(&ic);
    addon_->writeActiveModeToDisk("Bijoy Classic");

    // Sync triggers commit of pending text
    addon_->syncActiveModeFromDisk(&ic, ctxId);

    EXPECT_EQ(ic.committed_text, "আম");
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");
}

// ============================================================================
// Unit Tests: Shortcut Matching and Mode-Switching Logic
// NOTE: These are simulated mock input-context unit tests for testing addon
// shortcut parsing and state transitions. Real physical-keyboard tests require
// testing via the running desktop environment (Task 9).
// ============================================================================

TEST_F(Fcitx5AddonTest, EveryShortcutTogglesAssignedModeAndEnglish) {
    MockInputContext ic(instance_->inputContextManager());

    // Switch to English first
    sendKey(ic, FcitxKey_v, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");

    // 1. Ctrl+Alt+B: 1st press -> Bijoy Classic, 2nd press -> English, 3rd press -> Bijoy Classic
    sendKey(ic, FcitxKey_b, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");
    sendKey(ic, FcitxKey_b, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");
    sendKey(ic, FcitxKey_b, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");

    // 2. Ctrl+Alt+A: 1st press -> Bijoy Unicode, 2nd press -> English, 3rd press -> Bijoy Unicode
    sendKey(ic, FcitxKey_a, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");
    sendKey(ic, FcitxKey_a, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");
    sendKey(ic, FcitxKey_a, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");

    // 3. Ctrl+Alt+O: 1st press -> Avro Phonetic, 2nd press -> English, 3rd press -> Avro Phonetic
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");

    // 4. Ctrl+Alt+V: explicitly assigned to Bijoy Unicode / English toggle
    // 1st press -> Bijoy Unicode, 2nd press -> English, 3rd press -> Bijoy Unicode
    sendKey(ic, FcitxKey_v, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");
    sendKey(ic, FcitxKey_v, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");
    sendKey(ic, FcitxKey_v, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");
}

TEST_F(Fcitx5AddonTest, SwitchingFromAnotherBengaliModeToShortcutAssignedMode) {
    MockInputContext ic(instance_->inputContextManager());

    // Start in Avro Phonetic
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");

    // Pressing Ctrl+Alt+B from Avro Phonetic switches directly to Bijoy Classic
    sendKey(ic, FcitxKey_b, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");

    // Pressing Ctrl+Alt+A from Bijoy Classic switches directly to Bijoy Unicode
    sendKey(ic, FcitxKey_a, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");

    // Pressing Ctrl+Alt+O from Bijoy Unicode switches directly to Avro Phonetic
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");

    // Pressing Ctrl+Alt+V from Avro Phonetic switches directly to Bijoy Unicode
    sendKey(ic, FcitxKey_v, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");
}

TEST_F(Fcitx5AddonTest, RepeatedKeyEventsDoNotTriggerDuplicateToggles) {
    MockInputContext ic(instance_->inputContextManager());

    // Switch to English first
    sendKey(ic, FcitxKey_v, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");

    // Press Ctrl+Alt+B: switches to Bijoy Classic
    fcitx::Key keyB(FcitxKey_b, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    fcitx::KeyEvent evB1(&ic, keyB, false);
    addon_->keyEvent(*entry_, evB1);
    EXPECT_TRUE(evB1.filtered());
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");

    // Hold key: auto-repeat sends another press event WITHOUT an intervening release!
    fcitx::KeyEvent evBRepeat(&ic, keyB, false);
    addon_->keyEvent(*entry_, evBRepeat);
    EXPECT_TRUE(evBRepeat.filtered());
    // Mode MUST REMAIN Bijoy Classic, NOT oscillate back to English!
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");

    // Another repeat event
    fcitx::KeyEvent evBRepeat2(&ic, keyB, false);
    addon_->keyEvent(*entry_, evBRepeat2);
    EXPECT_TRUE(evBRepeat2.filtered());
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");

    // Now release key
    fcitx::KeyEvent evBRelease(&ic, keyB, true);
    addon_->keyEvent(*entry_, evBRelease);

    // Subsequent physical press toggles to English!
    fcitx::KeyEvent evBNextPress(&ic, keyB, false);
    addon_->keyEvent(*entry_, evBNextPress);
    EXPECT_TRUE(evBNextPress.filtered());
    EXPECT_EQ(addon_->activeMode(), "English");
}

TEST_F(Fcitx5AddonTest, ShortcutMatchingHandlesUppercaseKeysymsAndNumLock) {
    MockInputContext ic(instance_->inputContextManager());

    // Test with uppercase FcitxKey_B and NumLock active (Mod2 = NumLock = 16)
    auto ctrlAltNumLock = fcitx::KeyStates(fcitx::KeyState::Ctrl) |
                          fcitx::KeyState::Alt |
                          fcitx::KeyState::NumLock;

    sendKey(ic, FcitxKey_B, ctrlAltNumLock);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Classic");

    // Test with uppercase FcitxKey_A and NumLock active
    sendKey(ic, FcitxKey_A, ctrlAltNumLock);
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");

    // Test with uppercase FcitxKey_O and NumLock active
    sendKey(ic, FcitxKey_O, ctrlAltNumLock);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");
}

TEST_F(Fcitx5AddonTest, PlainCtrlVNeverSwitchesModeAndAllowsPaste) {
    MockInputContext ic(instance_->inputContextManager());

    // Initial state is Bijoy Unicode: Plain Ctrl+V is NOT filtered and does NOT switch mode!
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");

    fcitx::Key keyCtrlV(FcitxKey_v, fcitx::KeyState::Ctrl);
    fcitx::KeyEvent evCtrlV(&ic, keyCtrlV, false);
    addon_->keyEvent(*entry_, evCtrlV);

    EXPECT_FALSE(evCtrlV.filtered());
    EXPECT_EQ(addon_->activeMode(), "Bijoy Unicode");

    // Switch to Avro Phonetic mode: Plain Ctrl+V is NOT filtered and does NOT switch mode!
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");

    fcitx::KeyEvent evCtrlVPhonetic(&ic, keyCtrlV, false);
    addon_->keyEvent(*entry_, evCtrlVPhonetic);

    EXPECT_FALSE(evCtrlVPhonetic.filtered());
    EXPECT_EQ(addon_->activeMode(), "Avro Phonetic");

    // Toggle from Avro Phonetic to English mode: Plain Ctrl+V is NOT filtered
    sendKey(ic, FcitxKey_o, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->activeMode(), "English");

    fcitx::KeyEvent evCtrlVEN(&ic, keyCtrlV, false);
    addon_->keyEvent(*entry_, evCtrlVEN);

    EXPECT_FALSE(evCtrlVEN.filtered());
    EXPECT_EQ(addon_->activeMode(), "English");
}

TEST_F(Fcitx5AddonTest, EnglishModePassesKeysThrough) {
    MockInputContext ic(instance_->inputContextManager());

    // Switch to English mode
    addon_->writeActiveModeToDisk("English");
    auto ctxId = reinterpret_cast<banglakey::ContextId>(&ic);
    addon_->syncActiveModeFromDisk(&ic, ctxId);
    EXPECT_EQ(addon_->activeMode(), "English");

    // Type 'a'
    fcitx::Key keyA(FcitxKey_a);
    fcitx::KeyEvent evA(&ic, keyA, false);
    addon_->keyEvent(*entry_, evA);

    // In English mode, keys are NOT filtered (passed through untouched)
    EXPECT_FALSE(evA.filtered());
    EXPECT_TRUE(ic.committed_text.empty());
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());
}

} // namespace banglakey::fcitx5::testing
