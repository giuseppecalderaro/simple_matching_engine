find_package(Catch2 REQUIRED)

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
target_link_libraries(tests PRIVATE Catch2::Catch2)
