cmake_minimum_required (VERSION 3.1)
set (BINANCE_VERSION_MAJOR 0)
set (BINANCE_VERSION_MINOR 1)
project(binance VERSION "${BINANCE_VERSION_MAJOR}.${BINANCE_VERSION_MINOR}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBINANCE_VERSION=${binance_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) # Better for default
endif()
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}") # -DDEBUG")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

set(ENABLE_TESTING OFF CACHE BOOL "Enable testing")
set(BOOST_JSON_BUILD_EXAMPLES OFF CACHE BOOL "TODO")
set(BOOST_JSON_BUILD_TESTS OFF CACHE BOOL "TODO")
set(BOOST_URL_BUILD_TESTS OFF CACHE BOOL "TODO")
set(BOOST_URL_BUILD_EXAMPLES OFF CACHE BOOL "TODO")

add_subdirectory(${PROJECT_SOURCE_DIR}/contrib/boost.url)
add_subdirectory(${PROJECT_SOURCE_DIR}/contrib/boost.json) # should use boost json to send

option(BINANCE_BUILD_EXAMPLES "Builds examples listed on the examples folder." ON)
option(BINANCE_DISABLE_THREADING "Disables the thread library" ON)
option(BINANCE_USE_STRING_VIEW "Use string_view as much as possible" ON)

if(BINANCE_USE_STRING_VIEW)
  add_definitions(-DBINANCE_USE_STRING_VIEW)
  message("Using string_view")
endif()

if(NOT BINANCE_SIMDJSON_DIR)
    set(BINANCE_SIMDJSON_DIR "${PROJECT_SOURCE_DIR}/contrib/simdjson" CACHE STRING "simdjson location")
endif()

if(NOT BINANCE_CUSTOM_SIMDJSON_DIR)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx") # TODO: Detect CPU features
    set(SIMDJSON_JUST_LIBRARY ON CACHE BOOL "SIMDJSON only the lib")
    set(SIMDJSON_BUILD_STATIC ON CACHE BOOL "SIMDJSON static")
    set(SIMDJSON_EXCEPTIONS OFF CACHE BOOL "SIMDJSON exceptions")
    set(SIMDJSON_ENABLE_THREADS OFF CACHE BOOL "SIMDJSON exceptions")
endif()
add_subdirectory(${BINANCE_SIMDJSON_DIR})

add_library(${PROJECT_NAME} INTERFACE)

#Link boost URL and JSON
target_link_libraries(${PROJECT_NAME} INTERFACE boost_url)
target_link_libraries(${PROJECT_NAME} INTERFACE boost_json)
#Include Boost URL, JSON and if needed SIMDJSON
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/contrib/boost.url/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/contrib/boost.url/include/boost)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/contrib/boost.json/include)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/contrib/simdjson/include)

if(BINANCE_DISABLE_THREADING)
  target_compile_definitions(${PROJECT_NAME} INTERFACE BOOST_ASIO_DISABLE_THREADS=1)
endif()

#Find Boost dependencies
if(BINANCE_BUILD_EXAMPLES)
  find_package(Boost COMPONENTS system program_options)
endif()

#Include and link Boost
if(Boost_FOUND)
  target_include_directories(${PROJECT_NAME} INTERFACE ${Boost_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} INTERFACE ${Boost_LIBRARIES})
endif()

#Find OpenSSL
find_package(OpenSSL REQUIRED)

#Include and link OpenSSL
if(OPENSSL_FOUND)
  target_include_directories(${PROJECT_NAME} INTERFACE ${OPENSSL_INCLUDE_DIRS})
  target_link_libraries(${PROJECT_NAME} INTERFACE ${OPENSSL_LIBRARIES})
endif()

if(NOT BINANCE_DISABLE_THREADING)
  find_package(Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME} INTERFACE Threads::Threads)
endif()

target_link_libraries(${PROJECT_NAME} INTERFACE simdjson)

if(BINANCE_BUILD_EXAMPLES)
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
endif()
