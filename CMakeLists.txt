cmake_minimum_required(VERSION 3.20)
project(encoder_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/generators" ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/generators" ${CMAKE_PREFIX_PATH})

if(EXISTS "${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake")
    include(${CMAKE_BINARY_DIR}/generators/conan_toolchain.cmake)
endif()

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(sobjectizer REQUIRED)
find_package(SQLite3 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/database.cpp
    src/agents/db_agent.cpp
    src/agents/http_agent.cpp
)

target_link_libraries(${PROJECT_NAME}
    httplib::httplib
    nlohmann_json::nlohmann_json
    sobjectizer::StaticLib
    SQLite::SQLite3
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
