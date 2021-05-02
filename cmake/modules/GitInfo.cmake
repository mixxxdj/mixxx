# Get the current commit ref
execute_process(
  COMMAND git describe --tags --always --dirty=-modified
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DESCRIBE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT GIT_DESCRIBE)
  message(NOTICE "Git describe: unknown")
else()
  message(NOTICE "Git describe: ${GIT_DESCRIBE}")
  string(REGEX MATCH "-modified$" GIT_DIRTY "${GIT_DESCRIBE}")
  if (GIT_DIRTY)
    message("Git worktree modified: yes")
    set(GIT_DIRTY 1)
  else()
    message(NOTICE "Git worktree modified: no")
    set(GIT_DIRTY 0)
  endif()
endif()

# Get the current working branch
execute_process(
  COMMAND git rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT GIT_BRANCH)
  message(NOTICE "Git branch: unknown")
else()
  message(NOTICE "Git branch: ${GIT_BRANCH}")
endif()

# Get the current commit date
execute_process(
  COMMAND git show --quiet --format=%cI --date=short
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT GIT_COMMIT_DATE)
  message(NOTICE "Git commit date: unknown")
  # use current date in case of tar ball builds
  string(TIMESTAMP GIT_COMMIT_DATE "%Y-%m-%dT%H:%M:%SZ" UTC)
else()
  message(NOTICE "Git commit date: ${GIT_COMMIT_DATE}")
  string(REGEX MATCH "^[0-9][0-9][0-9][0-9]" GIT_COMMIT_YEAR "${GIT_COMMIT_DATE}")
  message(NOTICE "Git commit year: ${GIT_COMMIT_YEAR}")
endif()

# Get the number of commits on the working branch
execute_process(
  COMMAND git rev-list --count --first-parent HEAD
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_COUNT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT GIT_COMMIT_COUNT)
  message(NOTICE "Git commit count: unknown")
else()
  message(NOTICE "Git commit count: ${GIT_COMMIT_COUNT}")
endif()
