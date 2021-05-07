execute_process(
  COMMAND git describe --tags --always --dirty=-modified
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR_}
  OUTPUT_VARIABLE GIT_DESCRIBE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
configure_file(${CMAKE_SOURCE_DIR_}/cmake/gitdescribecache.in ${CMAKE_BINARY_DIR_}/gitdescribecache @ONLY)
