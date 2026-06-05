cmake_minimum_required(VERSION 3.20)
set(PACKAGE_NAMESPACE archive)
set(PACKAGE_NAME encoder_project)
project(${PACKAGE_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path" FORCE)
endif()

find_package(httplib REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(sobjectizer REQUIRED)
find_package(SQLite3 REQUIRED)

# Подключаем компоненты
add_subdirectory(database)
add_subdirectory(db)
add_subdirectory(http)
add_subdirectory(ffmpeg)
add_subdirectory(utils)
add_subdirectory(messages)

add_executable(${PACKAGE_NAME}
    encoder/sources/main.cpp
)

target_link_libraries(${PACKAGE_NAME}
    database_lib
    db_lib
    http_lib
    ffmpeg_lib
    utils_lib
    messages_lib
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

# Тесты
enable_testing()
find_package(GTest REQUIRED)

function(add_test_case NAME SOURCE)
    add_executable(${NAME} ${SOURCE})
    target_link_libraries(${NAME}
        database_lib
        db_lib
        http_lib
        ffmpeg_lib
        utils_lib
        messages_lib
        SQLite::SQLite3
        GTest::gtest_main
        sobjectizer::StaticLib
    )
    add_test(NAME ${NAME} COMMAND ${NAME})
endfunction()

add_test_case(test_vaa_blocks tests/test_vaa_blocks.cpp)
add_test_case(test_single_vaa_block tests/test_single_vaa_block.cpp)
add_test_case(test_database tests/test_database.cpp)

install(TARGETS ${PACKAGE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
