add_library(banglakey_engine STATIC
    src/engine.cpp
    src/bijoy_mode.cpp
    src/phonetic_mode.cpp
    src/unicode.cpp
    src/composition_state.cpp
    src/config.cpp
)

add_library(BanglaKey::Engine ALIAS banglakey_engine)

target_include_directories(banglakey_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/rules
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(banglakey_engine PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

if(BANGLAKEY_INSTALL)
    install(TARGETS banglakey_engine
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(DIRECTORY include/banglakey
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()
