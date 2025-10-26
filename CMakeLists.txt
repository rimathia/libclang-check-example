cmake_minimum_required(VERSION 3.15)

# Option to use explicit system includes (useful for cross-compilation and tooling)
option(USE_EXPLICIT_SYSTEM_INCLUDES "Explicitly add system include paths to compile commands" ON)

project(eigen_auto_checker CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include toolchain settings if requested
if(USE_EXPLICIT_SYSTEM_INCLUDES)
    include(cmake/toolchain.cmake)
endif()

# Find Eigen3
find_package(Eigen3 3.3 REQUIRED NO_MODULE)

# Example executable
add_executable(examples examples.cpp)
target_link_libraries(examples Eigen3::Eigen)
