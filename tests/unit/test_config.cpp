#include <gtest/gtest.h>
#include <banglakey/engine.hpp>

using namespace banglakey;

TEST(ConfigTest, DefaultConfigIsValid) {
    EngineConfig cfg;
    auto res = cfg.validate();
    EXPECT_TRUE(res.is_valid);
    EXPECT_TRUE(cfg.isValid());
}

TEST(ConfigTest, RejectInvalidBufferSize) {
    EngineConfig cfg;

    // Too small (< 8)
    cfg.max_buffer_size = 4;
    EXPECT_FALSE(cfg.isValid());
    EXPECT_FALSE(cfg.validate().is_valid);

    // Too large (> 4096)
    cfg.max_buffer_size = 5000;
    EXPECT_FALSE(cfg.isValid());

    // Valid boundary values
    cfg.max_buffer_size = 8;
    EXPECT_TRUE(cfg.isValid());

    cfg.max_buffer_size = 4096;
    EXPECT_TRUE(cfg.isValid());
}

TEST(ConfigTest, RejectInvalidCandidateCount) {
    EngineConfig cfg;

    // 0 candidates
    cfg.max_candidates = 0;
    EXPECT_FALSE(cfg.isValid());

    // > 30 candidates
    cfg.max_candidates = 50;
    EXPECT_FALSE(cfg.isValid());

    // Valid
    cfg.max_candidates = 1;
    EXPECT_TRUE(cfg.isValid());

    cfg.max_candidates = 30;
    EXPECT_TRUE(cfg.isValid());
}

TEST(ConfigTest, EngineRejectsInvalidConfigSafely) {
    auto engine = TypingEngine::create();
    ASSERT_NE(engine, nullptr);

    EngineConfig invalidCfg;
    invalidCfg.max_buffer_size = 1; // Invalid!

    bool result = engine->setConfig(invalidCfg);
    EXPECT_FALSE(result);

    // Existing config must be preserved
    EXPECT_NE(engine->config().max_buffer_size, 1u);

    // Valid config succeeds
    EngineConfig validCfg;
    validCfg.max_buffer_size = 256;
    EXPECT_TRUE(engine->setConfig(validCfg));
    EXPECT_EQ(engine->config().max_buffer_size, 256u);
}
