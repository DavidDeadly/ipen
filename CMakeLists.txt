cmake_minimum_required (VERSION 3.10)

project (
  ipen
  VERSION 1.0.0
  LANGUAGES CXX
)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++20 -Wno-attributes")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)


# headers
# include_directories ("${PROJECT_SOURCE_DIR}/lib/glfw/include")
include_directories(${GLFW3_INCLUDE_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/lib/stb")
include_directories ("${PROJECT_SOURCE_DIR}/lib/skia")
include_directories ("${PROJECT_SOURCE_DIR}/lib/imgui")

set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# executable
file (GLOB_RECURSE source_files
  "${PROJECT_SOURCE_DIR}/src/*.cpp"
  "${PROJECT_SOURCE_DIR}/lib/imgui/*.cpp"
)
add_executable (${PROJECT_NAME} ${source_files})

# deps
# set (glfw_library "${PROJECT_SOURCE_DIR}/lib/glfw/build/src/libglfw3.a")
set (skia_library "${PROJECT_SOURCE_DIR}/lib/skia/out/Release/libskia.a")
target_link_libraries (${PROJECT_NAME}
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

# installation
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  install(
    TARGETS ${PROJECT_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
  )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}.desktop.in
    ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop
    @ONLY
  )

  install(
    FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.desktop
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications
  )

  install(
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}.png
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons/hicolor/128x128/apps
  )
endif()
