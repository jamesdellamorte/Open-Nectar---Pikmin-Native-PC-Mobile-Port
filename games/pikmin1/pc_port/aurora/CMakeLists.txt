set(AURORA_DIR "$ENV{HOME}/aurora" CACHE PATH "Path to an Aurora checkout")
if(NOT EXISTS "${AURORA_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "Aurora not found at ${AURORA_DIR}; pass -DAURORA_DIR=<checkout>")
endif()

# Aurora must not inherit the game's include paths: include/Dolphin shadows
# Aurora's <dolphin/gx.h> on case-insensitive filesystems and the system <float.h>.
set_directory_properties(PROPERTIES
    INCLUDE_DIRECTORIES ""
    COMPILE_DEFINITIONS ""
    COMPILE_OPTIONS "")

set(AURORA_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(AURORA_ENABLE_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${AURORA_DIR}" "${CMAKE_BINARY_DIR}/aurora")
