cmake_minimum_required(VERSION 3.20)
project(caro LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Link the static runtime -> the exe runs on machines without the VC++ Redistributable
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ---- third party: raylib (vendored, builds offline) ----
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/raylib EXCLUDE_FROM_ALL)

# Warning options apply only to our own code (not raylib)
function(caro_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 /utf-8 /permissive-)
    target_compile_definitions(${target} PRIVATE _CRT_SECURE_NO_WARNINGS)  # fopen/snprintf of the standard library
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra)
  endif()
endfunction()

# ---- core library: board, move generation, evaluator, agents ----
file(GLOB CORE_SRCS CONFIGURE_DEPENDS src/core/*.cpp)
add_library(caro_core STATIC ${CORE_SRCS})
target_include_directories(caro_core PUBLIC src/core)
caro_warnings(caro_core)

# ---- GUI ----
file(GLOB GUI_SRCS CONFIGURE_DEPENDS src/gui/*.cpp)
add_executable(caro_gui ${GUI_SRCS})
target_include_directories(caro_gui PRIVATE third_party/raygui)
target_link_libraries(caro_gui PRIVATE caro_core raylib)
caro_warnings(caro_gui)
add_custom_command(TARGET caro_gui POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:caro_gui>/assets)

# ---- Benchmark CLI ----
file(GLOB BENCH_SRCS CONFIGURE_DEPENDS src/bench/*.cpp)
add_executable(caro_bench ${BENCH_SRCS})
target_link_libraries(caro_bench PRIVATE caro_core)
caro_warnings(caro_bench)

# ---- Tests ----
file(GLOB TEST_SRCS CONFIGURE_DEPENDS tests/*.cpp)
add_executable(caro_tests ${TEST_SRCS})
target_link_libraries(caro_tests PRIVATE caro_core)
caro_warnings(caro_tests)
enable_testing()
add_test(NAME caro_tests COMMAND caro_tests)
