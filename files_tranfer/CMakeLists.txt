cmake_minimum_required(VERSION 3.10)  # Adjust as needed for your CMake version

# Set the project name
project(SAR_SIFT)

# Find OpenCV package
# set(CMAKE_PREFIX_PATH "C:/Environment/opencv/build")
find_package(OpenCV REQUIRED)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(./)

# Add the executable target
add_executable(jpeg main.cpp match.cpp sar_sift.cpp)

# Link the OpenCV libraries
target_link_libraries(jpeg ${OpenCV_LIBS})

