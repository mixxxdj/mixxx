#!/bin/bash

function build {
    # Check dependencies
    command -v cmake >/dev/null 2>&1 || { echo >&2 "I require CMake but it's not installed. Install with 'sudo apt-get install cmake' and try again."; exit 1; }
    command -v gcc >/dev/null 2>&1 || { echo >&2 "I require Gcc but it's not installed. Install with 'sudo apt-get install gcc' and try again."; exit 1; }

    # Set directory variables
    DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
    TEMP_DIR=${DIR}/.cmake_temp

    # Create temp dir (after removing old one, if necessary)
    $(rm -rf ${TEMP_DIR})
    $(mkdir ${TEMP_DIR})

    echo "Creating CMake files..."
    eval "cmake -B${TEMP_DIR} -H${DIR}"

    echo "Building"
    eval "cmake --build ${TEMP_DIR} --target install"

    echo "Cleaning up the mess I made... :)"
    eval "rm -rf ${TEMP_DIR}"

    echo "Done! Run the tool from bin/layoutstool"
}

build
