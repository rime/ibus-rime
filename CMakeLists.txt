project(ibus-rime)
cmake_minimum_required(VERSION 2.8)

set(LIBRIME_VERSION 0.1)
set(LIBRIME_SOVERSION 0)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

include(${CMAKE_ROOT}/Modules/FindPkgConfig.cmake)
pkg_check_modules(IBus REQUIRED ibus-1.0)
if(IBus_FOUND)
  include_directories(${IBus_INCLUDE_DIRS})
  link_directories(${IBus_LIBRARY_DIRS})
endif(IBus_FOUND)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.40.0 REQUIRED COMPONENTS filesystem signals system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
endif(Boost_FOUND)

find_package(YamlCpp REQUIRED)
if(YamlCpp_FOUND)
  include_directories(${YamlCpp_INCLUDE_DIR})
endif(YamlCpp_FOUND)

find_package(Threads)
find_package(ZLIB)
find_package(KyotoCabinet REQUIRED)
if(KyotoCabinet_FOUND)
  include_directories(${KyotoCabinet_INCLUDE_DIR})
endif(KyotoCabinet_FOUND)

find_package(Opencc REQUIRED)
if(Opencc_FOUND)
  include_directories(${Opencc_INCLUDE_DIR})
endif(Opencc_FOUND)

include_directories(${PROJECT_SOURCE_DIR}/../librime/include)
link_directories(${PROJECT_SOURCE_DIR}/../librime/build/lib)

aux_source_directory(. IBUS_RIME_SRC)
add_executable(ibus-engine-rime ${IBUS_RIME_SRC})
target_link_libraries(ibus-engine-rime rime stdc++
  ${IBus_LIBRARIES} ${Boost_LIBRARIES} ${YamlCpp_LIBRARY} ${KyotoCabinet_LIBRARY} ${Opencc_LIBRARY})
