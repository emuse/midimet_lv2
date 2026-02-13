cmake_minimum_required(VERSION 3.10)
project(midimet VERSION 0.0.1
        DESCRIPTION "LV2 Audio and MIDI Metronome with host transport capability"
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set (PACKAGE "midimet")
set (PACKAGE_NAME "midimet")
set (PACKAGE_VERSION "${PROJECT_VERSION}")
set (PACKAGE_BUGREPORT "emuse@users.sourceforge.net")
set (PACKAGE_STRING "${PACKAGE_NAME}-${PACKAGE_VERSION}")
set (PACKAGE_TARNAME "midimet")
set (VERSION "${PROJECT_VERSION}")

set(STRIP_DEBUG_SYMBOLS ON CACHE BOOL "Strip debug symbols (default=yes)")
if (CMAKE_BUILD_TYPE MATCHES "Debug")
  set(STRIP_DEBUG_SYMBOLS OFF)
endif ()

include (GNUInstallDirs)

# Fix for new CMAKE_REQUIRED_LIBRARIES policy.
if (POLICY CMP0075)
  cmake_policy (SET CMP0075 NEW)
endif ()

# Fix for CXX_VISIBILITY_PRESET policy.
if (POLICY CMP0063)
  cmake_policy (SET CMP0063 NEW)
  set (CMAKE_CXX_VISIBILITY_PRESET hidden)
  set (CMAKE_VISIBILITY_INLINES_HIDDEN 1)
endif ()

include (CheckIncludeFileCXX)
include (CheckIncludeFiles)
include (CheckIncludeFile)
include (CheckFunctionExists)
include (CheckLibraryExists)

# Checks for header files.
if (UNIX AND NOT APPLE)
  check_include_files ("fcntl.h;unistd.h;signal.h" HAVE_SIGNAL_H)
endif ()

# Find package modules
find_package (PkgConfig REQUIRED)

# Check for LV2 support.
pkg_check_modules (LV2 lv2)
if (LV2_FOUND)
if (CMAKE_INSTALL_PREFIX MATCHES $ENV{HOME})
  set (CONFIG_LV2DIR "${CMAKE_INSTALL_PREFIX}/.lv2" CACHE STRING "LV2 Path Configuration")
else ()
  pkg_get_variable (LV2_SYSTEM_LV2DIR lv2 lv2dir)
  set (CONFIG_LV2DIR "${CMAKE_INSTALL_FULL_LIBDIR}/lv2" CACHE STRING "LV2 Path Configuration")
  if (NOT LV2_SYSTEM_LV2DIR STREQUAL CONFIG_LV2DIR)
    message (WARNING "*** System LV2 Path is ${LV2_SYSTEM_LV2DIR}, mismatch with \${CONFIG_LV2DIR}=\"${CONFIG_LV2DIR}\"!")
  endif ()
endif ()
include_directories (${LV2_INCLUDE_DIRS})
link_directories (${LV2_LIBRARY_DIRS})
link_libraries (${LV2_LIBRARIES})
# Check for LV2 Atom support.
check_include_file (lv2/lv2plug.in/ns/ext/atom/atom.h HAVE_LV2_ATOM_H)
if (NOT HAVE_LV2_ATOM_H)
  set (CONFIG_LV2_ATOM 0)
else ()
  set (CONFIG_LV2_ATOM 1)
  set (BUILD_LV2 1)
  message ("Will build LV2 plugins")
endif ()
set (CONFIG_LV2_ATOM_FORGE_OBJECT ${CONFIG_LV2_ATOM})
set (CONFIG_LV2_ATOM_FORGE_KEY ${CONFIG_LV2_ATOM})
else ()
message (WARNING "*** LV2 headers not found.")
set (CONFIG_LV2 0)
endif ()

add_subdirectory(src)
add_subdirectory(midimet.lv2)

if (EXISTS ${CMAKE_SOURCE_DIR}/configure)
    set(ADD_AUTOCONF_FILES  --add-file=${CMAKE_SOURCE_DIR}/configure
                            --add-file=${CMAKE_SOURCE_DIR}/aclocal.m4
                            --add-file=${CMAKE_SOURCE_DIR}/compile
                            --add-file=${CMAKE_SOURCE_DIR}/config.guess
                            --add-file=${CMAKE_SOURCE_DIR}/config.sub
                            --add-file=${CMAKE_SOURCE_DIR}/depcomp
                            --add-file=${CMAKE_SOURCE_DIR}/install-sh
                            --add-file=${CMAKE_SOURCE_DIR}/ltmain.sh
                            --add-file=${CMAKE_SOURCE_DIR}/missing
                            --add-file=${CMAKE_SOURCE_DIR}/Makefile.in
                            --prefix=${PACKAGE_STRING}/m4/
                            --add-file=${CMAKE_SOURCE_DIR}/m4/libtool.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/ltoptions.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/ltsugar.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/ltversion.m4
                            --add-file=${CMAKE_SOURCE_DIR}/m4/lt~obsolete.m4
                            --prefix=${PACKAGE_STRING}/src/
                            --add-file=${CMAKE_SOURCE_DIR}/src/config.h.in
                            --add-file=${CMAKE_SOURCE_DIR}/src/Makefile.in)

else ()
    set(ADD_AUTOCONF_FILES )
    message("Warning: No configure file found, will be missing when creating a dist archive")
endif ()

set(DIST_TARCOMMAND git --git-dir=${CMAKE_SOURCE_DIR}/.git archive 
                        --format=tar 
                        --prefix=${PACKAGE_STRING}/ ${ADD_AUTOCONF_FILES}
                        ${PACKAGE_STRING}^{tree} -o ${PACKAGE_STRING}.tar)

add_custom_target(dist-bz2 COMMAND ${DIST_TARCOMMAND} && bzip2 -f ${PACKAGE_STRING}.tar )
add_custom_target(dist COMMAND ${DIST_TARCOMMAND} && gzip -f ${PACKAGE_STRING}.tar )
