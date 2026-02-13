set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/config.h)
endif()

configure_file (cmake_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

set(LV2_MET_HEADERS
    midievent.h
    midimet.h
    midimet_lv2.h
)

set(LV2_MET_SOURCES
    midimet.cpp
    midimet_lv2.cpp
)
    
add_library (${PACKAGE_NAME} MODULE
  ${LV2_MET_HEADERS}
  ${LV2_MET_SOURCES}
)

set_target_properties (${PACKAGE_NAME} PROPERTIES CXX_STANDARD 11 PREFIX "")

if (UNIX AND NOT APPLE AND STRIP_DEBUG_SYMBOLS)
  add_custom_command(TARGET ${PACKAGE_NAME} POST_BUILD
    COMMAND strip ${PACKAGE_NAME}.so)
endif ()

if (UNIX AND NOT APPLE)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}.so
     DESTINATION ${CONFIG_LV2DIR}/${PACKAGE_NAME}.lv2)
endif ()

