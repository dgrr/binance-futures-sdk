cmake_minimum_required (VERSION 3.1)
set (BINANCE_VERSION_MAJOR 0)
set (BINANCE_VERSION_MINOR 1)
project(binance_futures VERSION "${BINANCE_VERSION_MAJOR}.${BINANCE_VERSION_MINOR}")

configure_file("${PROJECT_SOURCE_DIR}/definitions.hpp.in" "${PROJECT_SOURCE_DIR}/include/binance/definitions.hpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin) 

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
set(BOOST_JSON_BUILD_FUZZERS oFF CACHE BOOL "TODO")
set(BOOST_URL_BUILD_TESTS OFF CACHE BOOL "TODO")
set(BOOST_URL_BUILD_EXAMPLES OFF CACHE BOOL "TODO")

add_subdirectory(${PROJECT_SOURCE_DIR}/contrib/boost.url)
add_subdirectory(${PROJECT_SOURCE_DIR}/contrib/boost.json) # should use boost json to send

option(BINANCE_BUILD_EXAMPLES "Builds examples listed on the examples folder." ON)
option(BINANCE_DISABLE_THREADING "Disables the thread library" ON)
option(BINANCE_USE_STRING_VIEW "Use string_view as much as possible" ON)
option(BINANCE_WEBSOCKET_SHARED_PTR "Enables `enabled_shared_from_this` in binance::websocket::stream" OFF)
option(BINANCE_WEBSOCKET_ASYNC_CLOSE "Enables async_close function in binance::websocket::stream" OFF)

if(NOT BINANCE_SIMDJSON_DIR)
    set(BINANCE_SIMDJSON_DIR "${PROJECT_SOURCE_DIR}/contrib/simdjson" CACHE STRING "simdjson location")
endif()

if(NOT BINANCE_CUSTOM_SIMDJSON_DIR)
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx") # TODO: Detect CPU features
    set(SIMDJSON_JUST_LIBRARY ON CACHE BOOL "SIMDJSON only the lib")
    set(SIMDJSON_BUILD_STATIC ON CACHE BOOL "SIMDJSON static")
    set(SIMDJSON_EXCEPTIONS ON CACHE BOOL "SIMDJSON exceptions")
    set(SIMDJSON_ENABLE_THREADS OFF CACHE BOOL "SIMDJSON threading")
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

if(BINANCE_WEBSOCKET_SHARED_PTR)
    target_compile_definitions(${PROJECT_NAME} INTERFACE BINANCE_WEBSOCKET_SHARED_PTR=1)
endif()

if(BINANCE_WEBSOCKET_ASYNC_CLOSE)
    target_compile_definitions(${PROJECT_NAME} INTERFACE BINANCE_WEBSOCKET_ASYNC_CLOSE=1)
endif()

if(BINANCE_USE_STRING_VIEW)
  target_compile_definitions(${PROJECT_NAME} INTERFACE BINANCE_USE_STRING_VIEW=1)
  message("Using string_view")
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

if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(OPENSSL_USE_STATIC_LIBS true)
  set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
endif()

#Find OpenSSL
find_package(OpenSSL REQUIRED)

#Include and link OpenSSL
if(OPENSSL_FOUND)
  if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    target_include_directories(${PROJECT_NAME} INTERFACE "/usr/local/opt/openssl/include/")
  endif()
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