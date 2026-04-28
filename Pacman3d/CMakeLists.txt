# CMakeList.txt : CMake project for Pacman3d, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable(Pacman3d
  Pacman3d.cpp
  Map.cpp
  Map.h
  Player.cpp
  Player.h
  Ghost.cpp
  Ghost.h
)

include(FetchContent)

find_package(OpenGL REQUIRED)

find_package(glfw3 CONFIG QUIET)
if (NOT glfw3_FOUND)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
  )
  FetchContent_MakeAvailable(glfw)
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.9b
)
FetchContent_MakeAvailable(imgui)

find_package(glm CONFIG QUIET)
if (NOT glm_FOUND)
  FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
  )
  FetchContent_MakeAvailable(glm)
endif()

find_package(GLEW QUIET)
if (NOT GLEW_FOUND)
  set(ONLY_LIBS ON CACHE BOOL "" FORCE)
  set(BUILD_UTILS OFF CACHE BOOL "" FORCE)
  set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
  set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)

  FetchContent_Declare(
    glew
    GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
    GIT_TAG glew-cmake-2.2.0
  )

  FetchContent_GetProperties(glew)
  if (NOT glew_POPULATED)
    FetchContent_Populate(glew)
    add_subdirectory(${glew_SOURCE_DIR} ${glew_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
endif()

set(GLFW_TARGET glfw)
if (TARGET glfw3::glfw)
  set(GLFW_TARGET glfw3::glfw)
elseif (TARGET glfw::glfw)
  set(GLFW_TARGET glfw::glfw)
endif()

set(GLM_TARGET glm::glm)
if (TARGET glm)
  set(GLM_TARGET glm)
endif()

set(GLEW_TARGET "")
if (TARGET GLEW::GLEW)
  set(GLEW_TARGET GLEW::GLEW)
elseif (TARGET libglew_static)
  set(GLEW_TARGET libglew_static)
elseif (TARGET glew::glew_s)
  set(GLEW_TARGET glew::glew_s)
elseif (TARGET glew::glew)
  set(GLEW_TARGET glew::glew)
endif()

if (GLEW_TARGET STREQUAL "")
  message(FATAL_ERROR "GLEW target was not found. Install GLEW or provide GLEW_DIR.")
endif()

target_sources(Pacman3d
  PRIVATE
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(Pacman3d
  PRIVATE
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(Pacman3d
  PRIVATE
    OpenGL::GL
    ${GLFW_TARGET}
    ${GLEW_TARGET}
    ${GLM_TARGET}
)

if (GLEW_TARGET STREQUAL "libglew_static" OR GLEW_TARGET STREQUAL "glew::glew_s")
  target_compile_definitions(Pacman3d PRIVATE GLEW_STATIC)
endif()

target_compile_definitions(Pacman3d PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLEW)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.vert" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/shaders/basic.frag" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/shaders")

# TODO: Add tests and install targets if needed.
