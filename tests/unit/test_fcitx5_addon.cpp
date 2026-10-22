#include <gtest/gtest.h>

#include <fcitx/instance.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputpanel.h>
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
    void SetUp() override {
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
    }

    void sendKey(MockInputContext& ic, fcitx::KeySym sym, fcitx::KeyStates states = fcitx::KeyStates()) {
        fcitx::Key key(sym, states);
        fcitx::KeyEvent event(&ic, key, false);
        addon_->keyEvent(*entry_, event);
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

TEST_F(Fcitx5AddonTest, InitialModeIsPhonetic) {
    MockInputContext ic(instance_->inputContextManager());
    EXPECT_EQ(addon_->subMode(*entry_, ic), "Phonetic");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "ফোনেটিক");
}

TEST_F(Fcitx5AddonTest, PhoneticTypingAndSpaceCommit) {
    MockInputContext ic(instance_->inputContextManager());

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

    EXPECT_EQ(addon_->subMode(*entry_, ic), "Phonetic");

    // Type "am" in phonetic
    sendKey(ic, FcitxKey_a);
    sendKey(ic, FcitxKey_m);
    EXPECT_EQ(ic.inputPanel().preedit().toString(), "আম");

    // Hit F12: switch to Bijoy without losing pending text
    sendKey(ic, FcitxKey_F12);

    EXPECT_EQ(addon_->subMode(*entry_, ic), "Bijoy");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "বিজয়");
    EXPECT_EQ(ic.committed_text, "আম");
    EXPECT_TRUE(ic.inputPanel().preedit().toString().empty());

    // Now in Bijoy: press 'k' -> 'ত'
    sendKey(ic, FcitxKey_k);
    EXPECT_EQ(ic.committed_text, "আমত");

    // Hit Ctrl+Alt+M: switch back to Phonetic
    sendKey(ic, FcitxKey_m, fcitx::KeyStates(fcitx::KeyState::Ctrl) | fcitx::KeyState::Alt);
    EXPECT_EQ(addon_->subMode(*entry_, ic), "Phonetic");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic), "ফোনেটিক");
}

TEST_F(Fcitx5AddonTest, MultiContextIsolation) {
    MockInputContext ic1(instance_->inputContextManager());
    MockInputContext ic2(instance_->inputContextManager());

    // ic1 stays in Phonetic, ic2 switches to Bijoy
    sendKey(ic2, FcitxKey_F12);
    EXPECT_EQ(addon_->subMode(*entry_, ic1), "Phonetic");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic1), "ফোনেটিক");
    EXPECT_EQ(addon_->subMode(*entry_, ic2), "Bijoy");
    EXPECT_EQ(addon_->subModeLabelImpl(*entry_, ic2), "বিজয়");

    // Type in ic1: "a" -> "আ"
    sendKey(ic1, FcitxKey_a);
    EXPECT_EQ(ic1.inputPanel().preedit().toString(), "আ");
    EXPECT_TRUE(ic2.inputPanel().preedit().toString().empty());

    // Type in ic2: "j" -> "ক"
    sendKey(ic2, FcitxKey_j);
    EXPECT_EQ(ic2.committed_text, "ক");
    EXPECT_EQ(ic1.inputPanel().preedit().toString(), "আ");
}

TEST_F(Fcitx5AddonTest, BackspaceAndEscapeHandling) {
    MockInputContext ic(instance_->inputContextManager());

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

} // namespace banglakey::fcitx5::testing
