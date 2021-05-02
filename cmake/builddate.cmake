# Check if the worktree is dirty
execute_process(
  COMMAND git diff --quiet
  RESULT_VARIABLE DIRTY
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR_}
  ERROR_QUIET
)
if(DIRTY EQUAL "0")
  message(STATUS "Git worktree is clean")
else()
  if(DIRTY EQUAL "1")
     message(STATUS "Git worktree is dirty")
  else()
     message(STATUS "Not in a Git worktree")
  endif()
  # This depends on mixxx-lib so it will pick a new date
  # Whenever mixxx-lib has changed.
  string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%SZ" UTC)
endif()

configure_file(${CMAKE_SOURCE_DIR_}/src/builddate.cpp.in ${CMAKE_BINARY_DIR_}/src/builddate.cpp @ONLY)
