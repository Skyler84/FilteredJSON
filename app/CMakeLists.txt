add_executable(filteredjson_app)

target_sources(filteredjson_app
    PRIVATE
        main.cpp
)

target_link_libraries(filteredjson_app
    PRIVATE
        filteredjson)