add_library(database_lib STATIC
    sources/database.cpp
    sources/database_impl.cpp
)

target_include_directories(database_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(database_lib
    SQLite::SQLite3
    messages_lib
    utils_lib
    sobjectizer::StaticLib
)
