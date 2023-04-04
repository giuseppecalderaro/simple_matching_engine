cmake_minimum_required(VERSION 3.25.2)

project(main
        VERSION 1.0.0
        LANGUAGES CXX)

# C++20 (12/3/2023)
set(CMAKE_CXX_STANDARD 20)

# Set binaries directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable tests
enable_testing()

# Sub directories
add_subdirectory(${PROJECT_SOURCE_DIR}/src)
# add_subdirectory(${PROJECT_SOURCE_DIR}/test)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

if (APPLE)
  include_directories(/opt/homebrew/include)
endif()

# External packages
find_package(Boost REQUIRED)

# Enable debugging
set(CMAKE_CXX_FLAGS "-ggdb")

# Main Executable
add_executable(main
               ${SRCS}
               main.cpp)
