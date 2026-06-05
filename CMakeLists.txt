cmake_minimum_required(VERSION 3.20)
set(PACKAGE_NAMESPACE archive)
set(PACKAGE_NAME encoder_project)
project(${PACKAGE_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Убираем абсолютные пути для установки
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path" FORCE)
endif()

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(sobjectizer REQUIRED)
find_package(SQLite3 REQUIRED)

include_directories(include)
include_directories(http/include)
include_directories(db/include)
include_directories(ffmpeg/include)
include_directories(database/include)
include_directories(utils/include)
include_directories(messages/include)

add_executable(${PACKAGE_NAME}
    encoder/sources/main.cpp
    database/sources/database.cpp
    db/sources/db_agent.cpp
    http/sources/http_agent.cpp
    http/sources/http_handlers.cpp
    http/sources/http_print.cpp
    ffmpeg/sources/ffmpeg_agent.cpp
)

target_link_libraries(${PACKAGE_NAME}
    httplib::httplib
    nlohmann_json::nlohmann_json
    sobjectizer::StaticLib
    SQLite::SQLite3
)

if(WIN32)
    target_link_libraries(${PACKAGE_NAME} ws2_32)
    target_compile_definitions(${PACKAGE_NAME} PRIVATE
        _WIN32_WINNT=0x0601
        WINVER=0x0601
        CPPHTTPLIB_USE_POLL
        CPPHTTPLIB_NO_EXCEPTIONS
    )
endif()

# Установка (опционально)
install(TARGETS ${PACKAGE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
