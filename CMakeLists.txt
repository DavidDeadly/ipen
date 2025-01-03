cmake_minimum_required (VERSION 3.10)

project (
  ipen
  VERSION 0.0.0
  LANGUAGES CXX
)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++20 -Wno-attributes")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# executable
file (GLOB_RECURSE source_files
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
  "${PROJECT_SOURCE_DIR}/lib/imgui/*.cpp"
)
add_executable (ipen ${source_files})

# headers
# include_directories ("${PROJECT_SOURCE_DIR}/lib/glfw/include")
include_directories(${GLFW3_INCLUDE_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/lib/skia")
include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui")

set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# deps
# set (glfw_library "${PROJECT_SOURCE_DIR}/lib/glfw/build/src/libglfw3.a")
set (skia_library "${PROJECT_SOURCE_DIR}/lib/skia/out/Release/libskia.a")
target_link_libraries (ipen
  # ${glfw_library}
  glfw 
  wayland-client
  ${skia_library}
  GL
  m
  png
  freetype
  fontconfig
  input
  udev
)
