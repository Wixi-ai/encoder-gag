cmake_minimum_required(VERSION 3.15)
project(encoder_project)

# ПРИНУДИТЕЛЬНО выставляем Release, если не указано иное
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)

# Пути к Conan (проверь, что они верные)
set(CONAN_GENS "${CMAKE_CURRENT_SOURCE_DIR}/build/build/Release/generators")
list(APPEND CMAKE_PREFIX_PATH "${CONAN_GENS}")
list(APPEND CMAKE_MODULE_PATH "${CONAN_GENS}")

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(sobjectizer REQUIRED)
find_package(SQLite3 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME}
    httplib::httplib
    nlohmann_json::nlohmann_json
    sobjectizer::StaticLib
    SQLite::SQLite3
    ws2_32
)
if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
