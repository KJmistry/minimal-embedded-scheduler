cmake_minimum_required(VERSION 3.10)
project(minimal_embedded_scheduler C)

# Set source and build directories
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(BUILD_DIR "${CMAKE_SOURCE_DIR}/build")

# Collect all source files from src and src/common
file(GLOB SRC_FILES "${SRC_DIR}/*.c")
file(GLOB COMMON_FILES "${SRC_DIR}/common/*.c")
set(SRCS ${SRC_FILES} ${COMMON_FILES})

# Set output directory for library
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_DIR})

# Include headers
include_directories(${SRC_DIR})
include_directories(${SRC_DIR}/common)

# Create static library
add_library(scheduler STATIC ${SRCS})

# Set library output name to libscheduler.a
set_target_properties(scheduler PROPERTIES OUTPUT_NAME "scheduler")

# Install rule for the static library to local install directory
install(TARGETS scheduler ARCHIVE DESTINATION ${CMAKE_SOURCE_DIR}/install/lib)

# Build the sample example executable
set(EXAMPLE_SRC "${CMAKE_SOURCE_DIR}/example/sample_main.c")
add_executable(sample_example.bin ${EXAMPLE_SRC})

# Link the static library to the example
target_link_libraries(sample_example.bin scheduler)

# Set output directory for the example binary
set_target_properties(sample_example.bin PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Install the example binary to bin folder
install(TARGETS sample_example.bin RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/bin)

# Custom clean target to remove build, bin, and install directories
add_custom_target(extra_clean
    COMMAND rm -rf "${CMAKE_SOURCE_DIR}/build" "${CMAKE_SOURCE_DIR}/bin" "${CMAKE_SOURCE_DIR}/install"
    COMMENT "Removing build/, bin/, and install/ directories"
)
