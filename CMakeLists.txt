project(ibus-rime)
cmake_minimum_required(VERSION 2.8)

option(BUILD_STATIC "Build Rime using static libraries" OFF)

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

set(RIME_DEPS "dl")

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
