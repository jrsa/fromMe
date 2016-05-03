cmake_minimum_required(VERSION 3.3)
set(target xformFb)

# hacks (should figure out how to do properly/why it doesnt work)
set(GLFW3_DIR "/usr/local/lib/cmake/glfw/")
set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")
# end hacks

find_package(OpenGL REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(glbinding REQUIRED)

find_library(COREAUDIO_LIB CoreAudio)
find_library(COREFOUNDATION_LIB CoreFoundation)
add_definitions(-D__MACOSX_CORE__)

set(OPENNI_INCLUDE_DIR "/usr/local/include/ni")
set(OPENNI_LIBRARY "/usr/local/lib/libOpenNI.dylib")

set(GLM_INCLUDE_DIR "/usr/local/include/glm")
set(GLOG_LIBRARY "/usr/local/opt/glog/lib/libglog.dylib")

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLFW3_INCLUDE_DIR}
    ${GLBINDING_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
        ${OPENNI_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        "rtaudio"
)

set(libs
    ${OPENGL_LIBRARIES}
    ${GLFW3_LIBRARY}
    ${GLBINDING_LIBRARY}
    ${GLOG_LIBRARY}
    ${OPENNI_LIBRARY}
    ${COREAUDIO_LIB}
    ${COREFOUNDATION_LIB}
    )

set(sources
    simple_file.cpp
    shader.cpp
    main.cpp
    kinect.cpp

    rtaudio/RtAudio.cpp vertex_format.h)

add_executable(${target} ${sources})
target_link_libraries(${target} ${libs})
target_compile_options(${target} PRIVATE ${DEFAULT_COMPILE_FLAGS})
set_target_properties(${target}
    PROPERTIES
    LINKER_LANGUAGE              CXX
    FOLDER                      "${IDE_FOLDER}"
    COMPILE_DEFINITIONS_DEBUG   "${DEFAULT_COMPILE_DEFS_DEBUG}"
    COMPILE_DEFINITIONS_RELEASE "${DEFAULT_COMPILE_DEFS_RELEASE}"
    LINK_FLAGS_DEBUG            "${DEFAULT_LINKER_FLAGS_DEBUG}"
    LINK_FLAGS_RELEASE          "${DEFAULT_LINKER_FLAGS_RELEASE}"
    DEBUG_POSTFIX               "d${DEBUG_POSTFIX}")
