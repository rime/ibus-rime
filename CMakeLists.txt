project(ibus-rime)
cmake_minimum_required(VERSION 3.10)

set(ibus_rime_version 1.6.0)

option(BUILD_STATIC "Build Rime using static libraries" OFF)

include(GNUInstallDirs)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)
pkg_check_modules(IBus REQUIRED ibus-1.0)
if(IBus_FOUND)
  include_directories(${IBus_INCLUDE_DIRS})
  link_directories(${IBus_LIBRARY_DIRS})
endif(IBus_FOUND)

pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
if (LIBNOTIFY_FOUND)
  include_directories(${LIBNOTIFY_INCLUDE_DIRS})
  link_directories(${LIBNOTIFY_LIBRARY_DIRS})
endif(LIBNOTIFY_FOUND)

find_package(Rime REQUIRED)
if(Rime_FOUND)
  include_directories(${Rime_INCLUDE_DIR})
endif(Rime_FOUND)

if(NOT DEFINED RIME_DATA_DIR)
  find_package(RimeData REQUIRED)
endif(NOT DEFINED RIME_DATA_DIR)
message(STATUS "Precompiler macro RIME_DATA_DIR is set to \"${RIME_DATA_DIR}\"")
add_definitions(-DRIME_DATA_DIR="${RIME_DATA_DIR}")

set(ibus_rime_icons_dir "${CMAKE_INSTALL_FULL_DATADIR}/ibus-rime/icons")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/rime_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/rime_config.h"
  @ONLY)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

if(BUILD_STATIC)
  find_package(ICU REQUIRED uc)
  if(ICU_FOUND)
    set(RIME_DEPS ${RIME_DEPS} ${ICU_LIBRARIES})
  endif()

  link_directories(${PROJECT_SOURCE_DIR}/lib)
  set(RIME_DEPS ${RIME_DEPS} m stdc++ pthread
      libboost_filesystem.a libboost_locale.a libboost_regex.a libboost_signals.a libboost_system.a libboost_thread.a
      libglog.a libleveldb.a libmarisa.a libopencc.a libyaml-cpp.a)
endif(BUILD_STATIC)

aux_source_directory(. IBUS_RIME_SRC)
add_executable(ibus-engine-rime ${IBUS_RIME_SRC})
target_link_libraries(ibus-engine-rime ${IBus_LIBRARIES} ${LIBNOTIFY_LIBRARIES} ${Rime_LIBRARIES} ${RIME_DEPS})

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/rime.xml.in"
  "${CMAKE_CURRENT_BINARY_DIR}/rime.xml"
  @ONLY)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/rime.xml" DESTINATION "${CMAKE_INSTALL_DATADIR}/ibus/component")
install(TARGETS ibus-engine-rime DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}/ibus-rime")
install(DIRECTORY icons DESTINATION "${CMAKE_INSTALL_DATADIR}/ibus-rime" FILES_MATCHING PATTERN "*.png")
install(FILES ibus_rime.yaml DESTINATION "${RIME_DATA_DIR}")
