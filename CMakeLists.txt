cmake_minimum_required(VERSION 3.31)
project(Sonique)

set(CMAKE_CXX_STANDARD 20)

find_package(raylib 5.0 REQUIRED) # Make sure Raylib is installed
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/fluid-synth;/opt/homebrew")
set(CMAKE_LIBRARY_PATH "/opt/homebrew/lib")
set(CMAKE_INCLUDE_PATH "/opt/homebrew/include")
find_package(PkgConfig REQUIRED)
pkg_check_modules(FLUIDSYNTH REQUIRED fluidsynth)

set(MACOSX_BUNDLE_ICON_FILE appicon.icns)
set_source_files_properties(appicon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

add_executable(Sonique MACOSX_BUNDLE main.cpp appicon.icns)
set_target_properties(Sonique PROPERTIES MACOSX_BUNDLE TRUE)

file(COPY assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Sonique.app/Contents/Resources)
file(COPY Neutraface.otf DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Sonique.app/Contents/Resources)

target_include_directories(Sonique PRIVATE ${FLUIDSYNTH_INCLUDE_DIRS})
target_link_libraries(Sonique PRIVATE raylib ${FLUIDSYNTH_LIBRARIES} ${FLUIDSYNTH_LDFLAGS} "-framework CoreFoundation")