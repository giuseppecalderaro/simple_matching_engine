find_package(Catch2 3 QUIET)

if (NOT Catch2_FOUND)
  include(FetchContent)
  FetchContent_Declare(Catch2
                       GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                       GIT_TAG        v3.7.1
                       GIT_SHALLOW    TRUE)
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

include(Catch)

include_directories(${PROJECT_SOURCE_DIR}/include)

if (APPLE)
  include_directories(/opt/homebrew/include)
endif()

# Main Executable
add_executable(tests
               ${PROJECT_SOURCE_DIR}/src/engine.cpp
               ${PROJECT_SOURCE_DIR}/src/order.cpp
               ${PROJECT_SOURCE_DIR}/src/orderbook.cpp
               tests.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

catch_discover_tests(tests)
