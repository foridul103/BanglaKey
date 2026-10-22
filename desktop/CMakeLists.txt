find_package(Qt6 6.5 QUIET COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Network)

if(NOT Qt6_FOUND)
    message(WARNING "Qt 6 (Core, Gui, Qml, Quick, Widgets, Network) development packages not found! Ensure qt6-qtbase-devel and qt6-qtdeclarative-devel are installed to build the desktop settings application.")
    return()
endif()

message(STATUS "Found Qt6: ${Qt6_VERSION}")

if(COMMAND qt_policy)
    qt_policy(SET QTP0001 NEW)
    qt_policy(SET QTP0004 NEW)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(banglakey-settings
    src/main.cpp
    src/settings_backend.hpp
    src/settings_backend.cpp
    src/ipc_manager.hpp
    src/ipc_manager.cpp
    src/tray_manager.hpp
    src/tray_manager.cpp
)

qt_add_qml_module(banglakey-settings
    URI banglakey.desktop
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Splash.qml
        qml/Toolbar.qml
)

target_compile_definitions(banglakey-settings
    PRIVATE
        BANGLAKEY_QML_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(banglakey-settings
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        Qt6::Network
        BanglaKey::Engine
)

set_target_properties(banglakey-settings PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

if(BANGLAKEY_INSTALL)
    install(TARGETS banglakey-settings
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()
