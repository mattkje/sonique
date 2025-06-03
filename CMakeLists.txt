cmake_minimum_required(VERSION 3.31)
project(untitled)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib 5.0 REQUIRED) # Make sure Raylib is installed
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/fluid-synth;/opt/homebrew")
set(CMAKE_LIBRARY_PATH "/opt/homebrew/lib")
set(CMAKE_INCLUDE_PATH "/opt/homebrew/include")
find_package(PkgConfig REQUIRED)
pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth)

add_executable(untitled main.cpp)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY Neutraface.otf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY SoundFonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(untitled PRIVATE ${FLUIDSYNTH_INCLUDE_DIRS})
target_link_libraries(untitled PRIVATE raylib ${FLUIDSYNTH_LIBRARIES} ${FLUIDSYNTH_LDFLAGS})