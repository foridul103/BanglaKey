find_package(GTest QUIET)

set(UNIT_TEST_SOURCES
    unit/test_smoke.cpp
    unit/test_unicode.cpp
    unit/test_bijoy.cpp
    unit/test_phonetic.cpp
    unit/test_composition.cpp
    unit/test_config.cpp
)

add_executable(banglakey_unit_tests
    ${UNIT_TEST_SOURCES}
)

target_link_libraries(banglakey_unit_tests
    PRIVATE
        BanglaKey::Engine
)

set_target_properties(banglakey_unit_tests PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(GTest_FOUND)
    message(STATUS "Found GoogleTest: ${GTest_VERSION} - building test suite with GoogleTest")
    target_compile_definitions(banglakey_unit_tests PRIVATE USE_GTEST=1)
    target_link_libraries(banglakey_unit_tests PRIVATE GTest::gtest GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(banglakey_unit_tests)
else()
    message(STATUS "GoogleTest not found - building standalone smoke test")
    add_test(NAME banglakey_smoke_test COMMAND banglakey_unit_tests)
endif()

# Fcitx5 Addon Integration Tests
find_package(Fcitx5Core QUIET)
if(BANGLAKEY_BUILD_FCITX5 AND Fcitx5Core_FOUND AND GTest_FOUND)
    add_executable(banglakey_fcitx5_tests
        unit/test_fcitx5_addon.cpp
        ${CMAKE_SOURCE_DIR}/fcitx5/src/banglakey_addon.cpp
    )

    target_include_directories(banglakey_fcitx5_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/fcitx5/src
    )

    target_link_libraries(banglakey_fcitx5_tests
        PRIVATE
            BanglaKey::Engine
            Fcitx5::Core
            GTest::gtest
            GTest::gtest_main
    )

    set_target_properties(banglakey_fcitx5_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )

    gtest_discover_tests(banglakey_fcitx5_tests)
endif()

# Standalone Engine Smoke Test (Phase 1 Deliverable)
add_executable(engine_smoke_test
    unit/engine_smoke_test.cpp
)

target_link_libraries(engine_smoke_test
    PRIVATE
        BanglaKey::Engine
)

set_target_properties(engine_smoke_test PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

add_test(NAME engine_smoke_test COMMAND engine_smoke_test)

# Desktop Settings Backend Tests
find_package(Qt6 QUIET COMPONENTS Core Network)
if(BANGLAKEY_BUILD_DESKTOP AND Qt6_FOUND AND GTest_FOUND)
    add_executable(banglakey_settings_tests
        unit/test_settings.cpp
        ${CMAKE_SOURCE_DIR}/desktop/src/settings_backend.cpp
        ${CMAKE_SOURCE_DIR}/desktop/src/ipc_manager.cpp
    )

    target_include_directories(banglakey_settings_tests
        PRIVATE
            ${CMAKE_SOURCE_DIR}/desktop/src
    )

    target_link_libraries(banglakey_settings_tests
        PRIVATE
            BanglaKey::Engine
            Qt6::Core
            Qt6::Network
            GTest::gtest
            GTest::gtest_main
    )

    set_target_properties(banglakey_settings_tests PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        AUTOMOC ON
    )

    gtest_discover_tests(banglakey_settings_tests)
endif()
