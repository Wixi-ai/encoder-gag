add_library(http_lib STATIC
    sources/http_agent.cpp
    sources/http_handlers.cpp
    sources/http_print.cpp
)

target_include_directories(http_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(http_lib
    httplib::httplib
    nlohmann_json::nlohmann_json
    sobjectizer::StaticLib
    messages_lib
    utils_lib
)
