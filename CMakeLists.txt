cmake_minimum_required(VERSION 3.21)

project(vscode-git-wrap-for-msys2 VERSION 0.1 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(vscode-git-wrap-for-msys2)
set_target_properties(vscode-git-wrap-for-msys2 PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WIN32_EXECUTABLE OFF
    OUTPUT_NAME vscode-git-wrap-for-msys2
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)

execute_process(
    COMMAND git rev-parse --short --verify HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
execute_process(
    COMMAND date --iso-8601=seconds
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE COMPILE_TIME
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if (GIT_COMMIT_HASH STREQUAL "")
    message(WARNING "Could not determine git commit hash")
    set(GIT_COMMIT_HASH "Unknown")
endif ()

target_compile_definitions(vscode-git-wrap-for-msys2 PRIVATE
    GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
    COMPILE_TIME="${COMPILE_TIME}"
    APPLICATION_VERSION="${PROJECT_VERSION}"
)

target_compile_options(vscode-git-wrap-for-msys2 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_sources(vscode-git-wrap-for-msys2 PRIVATE main.c)
