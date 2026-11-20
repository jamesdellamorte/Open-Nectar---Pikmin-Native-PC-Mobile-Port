# Aurora is a git submodule at games/pikmin1/pc_port/aurora
# Aurora must not inherit the game's include paths: include/Dolphin shadows
# Aurora's <dolphin/gx.h> on case-insensitive filesystems and the system <float.h>.
set_directory_properties(PROPERTIES
    INCLUDE_DIRECTORIES ""
    COMPILE_DEFINITIONS ""
    COMPILE_OPTIONS "")

set(AURORA_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(AURORA_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../aurora" "${CMAKE_BINARY_DIR}/aurora")
