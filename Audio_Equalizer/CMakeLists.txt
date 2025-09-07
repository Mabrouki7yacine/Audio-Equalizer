cmake_minimum_required(VERSION 3.10)
project(Audio_EQ)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Include)

# Source files
set(SOURCES
    main.cpp
    Include/Audio_eq.cpp
    Include/Signals.cpp
    Include/wav_handler.cpp
)
# Create executable
add_executable(Audio_EQ ${SOURCES})