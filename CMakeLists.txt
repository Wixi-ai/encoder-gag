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

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
    httplib::httplib
    nlohmann_json::nlohmann_json
    sobjectizer::StaticLib
    SQLite3::SQLite3
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} ws2_32)
endif()
