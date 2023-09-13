cmake_minimum_required(VERSION "3.22.1")
project("FilteredJSON" CXX)

set(PROJECT_NAME )

set(CMAKE_CXX_STANDARD 20)

add_library(filteredjson ${LIB_TYPE})

target_sources(filteredjson
PRIVATE
  src/parser.cpp
  src/filter.cpp
  src/json.cpp
)

target_include_directories(filteredjson
PUBLIC
  include/
)

add_subdirectory(tests)
add_subdirectory(app)