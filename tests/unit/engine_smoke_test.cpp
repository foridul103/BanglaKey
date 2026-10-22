#include <iostream>
#include <cstdlib>

#include <banglakey/banglakey.hpp>

int main() {
    std::cout << "==================================================" << std::endl;
    std::cout << " BanglaKey Linux — Phase 1 Engine Smoke Test" << std::endl;
    std::cout << "==================================================" << std::endl;

    // 1. Verify Engine Creation & Factory
    auto engine = banglakey::TypingEngine::create(banglakey::TypingMode::Phonetic);
    if (!engine) {
        std::cerr << "FAIL: Failed to create BanglaKey TypingEngine instance!" << std::endl;
        return 1;
    }
    std::cout << "[PASS] Engine creation successful" << std::endl;

    // 2. Verify Initial Mode
    if (engine->mode() != banglakey::TypingMode::Phonetic) {
        std::cerr << "FAIL: Default mode should be Phonetic!" << std::endl;
        return 1;
    }
    std::cout << "[PASS] Default mode is Phonetic" << std::endl;

    // 3. Verify Mode Switching
    engine->setMode(banglakey::TypingMode::Bijoy);
    if (engine->mode() != banglakey::TypingMode::Bijoy) {
        std::cerr << "FAIL: Failed to set mode to Bijoy!" << std::endl;
        return 1;
    }
    std::cout << "[PASS] Switched mode to Bijoy" << std::endl;

    // 4. Verify Configuration Loading
    auto config = engine->config();
    if (config.max_buffer_size == 0) {
        std::cerr << "FAIL: Engine configuration invalid!" << std::endl;
        return 1;
    }
    std::cout << "[PASS] Engine configuration loaded (max_buffer_size: "
              << config.max_buffer_size << ")" << std::endl;

    // 5. Verify Context Reset
    engine->reset();
    const auto& st = engine->state();
    if (!st.isEmpty()) {
        std::cerr << "FAIL: State should be empty after reset!" << std::endl;
        return 1;
    }
    std::cout << "[PASS] State reset verified" << std::endl;

    std::cout << "--------------------------------------------------" << std::endl;
    std::cout << " ALL ENGINE SMOKE TESTS PASSED SUCCESSFULLY!" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    return 0;
}
