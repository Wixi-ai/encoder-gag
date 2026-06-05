add_library(db_lib STATIC
    sources/db_agent.cpp
)

target_include_directories(db_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(db_lib
    database_lib
    sobjectizer::StaticLib
    messages_lib
    nlohmann_json::nlohmann_json
)
