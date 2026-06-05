add_library(ffmpeg_lib STATIC
    sources/ffmpeg_agent.cpp
)

target_include_directories(ffmpeg_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(ffmpeg_lib
    database_lib
    sobjectizer::StaticLib
    messages_lib
    utils_lib
    nlohmann_json::nlohmann_json
)
