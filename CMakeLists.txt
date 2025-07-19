cmake_minimum_required(VERSION 3.31)
project(TankeryClient)
include(FetchContent)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_EXECUTABLE_SUFFIX ".html")

if(NOT CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "Emscripten toolchain not loaded!")
endif()

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp" "src/*.hpp")
file(GLOB_RECURSE LIB_FILES "lib/*.cpp" "lib/*.hpp" "lib/*.h")
add_executable(TankeryClient ${SOURCE_FILES} ${LIB_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

target_link_options(TankeryClient PRIVATE
        -pthread
        "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/src/payload.html"
        "SHELL:-s SHARED_MEMORY=0"
        "SHELL:-s USE_WEBGL2=1"
        "SHELL:-s WASM=1"
        "SHELL:-s MODULARIZE=1"
        "SHELL:-s ALLOW_MEMORY_GROWTH=1"
        "SHELL:-s EXPORT_ES6=1"
        "SHELL:-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        "SHELL:-s NO_EXIT_RUNTIME=1"
        "SHELL:-s ASSERTIONS=1"
        "SHELL:-s USE_GLFW=3"
        "SHELL:-s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1"
        "SHELL:-s FORCE_FILESYSTEM=1"
        "SHELL:-s ASYNCIFY"
        "SHELL:-s USE_PTHREADS=1"
        "SHELL:-s ENVIRONMENT=web,worker"
        "SHELL:-s PTHREAD_POOL_SIZE=16"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets"
        "SHELL:-s STACK_SIZE=16777216"
        "SHELL:-s INITIAL_MEMORY=134217728"
        "SHELL:-O2"
)

target_include_directories(TankeryClient PRIVATE
        ${EMSCRIPTEN_ROOT_PATH}/cache/sysroot/include
)

target_link_libraries(TankeryClient PRIVATE
        "-s LEGACY_GL_EMULATION=0"
        "-lGL"
        "-lhtml5"
        "-lwebsocket.js"
        "-lraylib"
)

target_link_libraries(TankeryClient PRIVATE Jolt)

add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR)
add_compile_definitions(JPH_PROFILE_ENABLED)
add_compile_definitions(JPH_DEBUG_RENDERER)
add_compile_definitions(JPH_OBJECT_LAYER_BITS=32)
