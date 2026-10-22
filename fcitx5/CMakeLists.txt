find_package(Fcitx5Core 5.1.0 QUIET)

if(NOT Fcitx5Core_FOUND)
    message(WARNING "Fcitx5Core development package not found! Ensure fcitx5-devel and extra-cmake-modules are installed to build the Fcitx5 addon.")
    return()
endif()

message(STATUS "Found Fcitx5Core: ${Fcitx5Core_VERSION}")

add_library(banglakey-fcitx5 MODULE
    src/banglakey_addon.cpp
)

target_include_directories(banglakey-fcitx5
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(banglakey-fcitx5
    PRIVATE
        Fcitx5::Core
        BanglaKey::Engine
)

set_target_properties(banglakey-fcitx5 PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Install configuration files
if(BANGLAKEY_INSTALL)
    # Determine addon installation dir
    if(NOT DEFINED FCITX5_ADDON_INSTALL_DIR)
        set(FCITX5_ADDON_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/fcitx5")
    endif()

    install(TARGETS banglakey-fcitx5
        DESTINATION ${FCITX5_ADDON_INSTALL_DIR}
    )

    configure_file(banglakey.conf.in banglakey.conf @ONLY)
    configure_file(banglakey-addon.conf.in banglakey-im.conf @ONLY)

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/banglakey.conf
        DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/addon"
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/banglakey-im.conf
        DESTINATION "${CMAKE_INSTALL_DATADIR}/fcitx5/inputmethod"
        RENAME banglakey.conf
    )
endif()
