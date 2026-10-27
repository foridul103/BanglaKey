cmake_minimum_required(VERSION 3.25)

project(BanglaKey
    VERSION 0.1.0
    DESCRIPTION "Native Bengali Typing Software for Linux"
    HOMEPAGE_URL "https://github.com/banglakey/banglakey-linux"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Common compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion)
endif()

include(GNUInstallDirs)

# Modular component build options
option(BANGLAKEY_BUILD_ENGINE  "Build core typing engine library" ON)
option(BANGLAKEY_BUILD_FCITX5  "Build Fcitx5 input method addon" ON)
option(BANGLAKEY_BUILD_DESKTOP "Build Qt 6 / QML settings application" ON)
option(BANGLAKEY_BUILD_TESTS   "Build test suites" ON)
option(BANGLAKEY_INSTALL       "Enable installation targets" ON)

# 1. Engine Library
if(BANGLAKEY_BUILD_ENGINE)
    add_subdirectory(engine)
endif()

# 2. Fcitx5 Addon
if(BANGLAKEY_BUILD_FCITX5)
    if(NOT TARGET BanglaKey::Engine)
        message(FATAL_ERROR "BANGLAKEY_BUILD_FCITX5 requires BANGLAKEY_BUILD_ENGINE=ON")
    endif()
    add_subdirectory(fcitx5)
endif()

# 3. Qt 6 / QML Desktop Application
if(BANGLAKEY_BUILD_DESKTOP)
    if(NOT TARGET BanglaKey::Engine)
        message(FATAL_ERROR "BANGLAKEY_BUILD_DESKTOP requires BANGLAKEY_BUILD_ENGINE=ON")
    endif()
    add_subdirectory(desktop)
endif()

# 4. Testing
if(BANGLAKEY_BUILD_TESTS)
    if(NOT TARGET BanglaKey::Engine)
        message(FATAL_ERROR "BANGLAKEY_BUILD_TESTS requires BANGLAKEY_BUILD_ENGINE=ON")
    endif()
    enable_testing()
    add_subdirectory(tests)
endif()

# 5. Packaging (CPack)
if(BANGLAKEY_INSTALL)
    set(CPACK_PACKAGE_NAME "banglakey")
    set(CPACK_PACKAGE_VENDOR "BanglaKey Team")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Native Bengali Typing Software for Linux")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    set(CPACK_GENERATOR "TGZ;TXZ")
    include(CPack)
endif()

# Summary
message(STATUS "--------------------------------------------------------")
message(STATUS " BanglaKey Linux Configuration Summary")
message(STATUS " Version:          ${PROJECT_VERSION}")
message(STATUS " Build Engine:     ${BANGLAKEY_BUILD_ENGINE}")
message(STATUS " Build Fcitx5:     ${BANGLAKEY_BUILD_FCITX5}")
message(STATUS " Build Desktop:    ${BANGLAKEY_BUILD_DESKTOP}")
message(STATUS " Build Tests:      ${BANGLAKEY_BUILD_TESTS}")
message(STATUS " C++ Standard:     ${CMAKE_CXX_STANDARD}")
message(STATUS " Install prefix:   ${CMAKE_INSTALL_PREFIX}")
message(STATUS "--------------------------------------------------------")

