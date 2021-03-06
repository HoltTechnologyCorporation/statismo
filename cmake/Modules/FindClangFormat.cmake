#
# - Try to find clang format
# http://ttroy50.github.io/cmake-examples/
#

if(NOT CLANG_FORMAT_BIN_NAME)
    set(CLANG_FORMAT_BIN_NAME clang-format)
endif()

# if custom path check there first
if(CLANG_FORMAT_ROOT_DIR)
    find_program(CLANG_FORMAT_BIN 
        NAMES
        ${CLANG_FORMAT_BIN_NAME}
        PATHS
        "${CLANG_FORMAT_ROOT_DIR}"
        NO_DEFAULT_PATH)
endif()

find_program(CLANG_FORMAT_BIN NAMES ${CLANG_FORMAT_BIN_NAME})

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(
    CLANG_FORMAT
    DEFAULT_MSG 
    CLANG_FORMAT_BIN)

mark_as_advanced(
    CLANG_FORMAT_BIN)

if(CLANG_FORMAT_FOUND)
    # A CMake script to find all source files and setup clang-format targets for them
    include(clang-format)
else()
    message(STATUS "clang-format not found (code formatting disabled)")
endif()
