cmake_minimum_required(VERSION 3.17)
set(PROJECT_NAME project_base)
project(${PROJECT_NAME})

set(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_CXX_FLAGS "-Wall -Wextra")

file(GLOB SOURCES "src/*.cpp" "src/*.c")
file(GLOB HEADERS "include/*.h" "include/*.hpp")

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(ASSIMP REQUIRED)

add_subdirectory(libs/glad)
add_definitions(${OPENGL_DEFINITIONS})

set(LIBS glfw glad OpenGL::GL X11 Xrandr Xinerama Xi Xxf86vm Xcursor dl pthread freetype ${ASSIMP_LIBRARY})


include_directories(include/)
add_executable(${PROJECT_NAME}
        ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${PROJECT_NAME}")

