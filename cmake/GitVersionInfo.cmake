find_program(GIT git)
if(NOT GIT)
  message(FATAL_ERROR "Git not found")
endif()

if(NOT EXISTS "${GIT_REPO_ROOT}/.git")
  message(FATAL_ERROR "Must be run from a Git repository")
endif()

execute_process(
  COMMAND ${GIT} describe --tags --abbrev=0
  WORKING_DIRECTORY ${GIT_REPO_ROOT}
  OUTPUT_VARIABLE GIT_TAG
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

execute_process(
  COMMAND ${GIT} describe --tags --always --dirty=-modified
  WORKING_DIRECTORY ${GIT_REPO_ROOT}
  OUTPUT_VARIABLE GIT_COMMIT_DESCRIPTION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

set(ENV{TZ} UTC)
execute_process(
  COMMAND ${GIT} show --quiet --format=%cd --date=short
  WORKING_DIRECTORY ${GIT_REPO_ROOT}
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
