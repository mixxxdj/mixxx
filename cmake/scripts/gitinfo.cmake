list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
include(GitInfo)
configure_file("${INPUT_FILE}" "${OUTPUT_FILE}" @ONLY)
