cmake_minimum_required (VERSION 3.1)
project(binance-depth-example)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_DIR}/main.cc)

target_link_libraries(${PROJECT_NAME} PUBLIC binance_futures)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/../../include)