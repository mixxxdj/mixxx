cmake_minimum_required(VERSION 3.21)

if(NOT DEFINED PATCH_FILE)
  message(FATAL_ERROR "PATCH_FILE not defined")
endif()
if(NOT DEFINED WORKING_DIR)
  message(FATAL_ERROR "WORKING_DIR not defined")
endif()

find_program(GIT_EXECUTABLE git)
find_program(PATCH_EXECUTABLE patch)

if(GIT_EXECUTABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} apply --check -p1 "${PATCH_FILE}"
    WORKING_DIRECTORY "${WORKING_DIR}"
    OUTPUT_VARIABLE apply_output
    ERROR_VARIABLE apply_error
    RESULT_VARIABLE check_result
  )

  if(check_result EQUAL 0)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} apply -p1 "${PATCH_FILE}"
      WORKING_DIRECTORY "${WORKING_DIR}"
      OUTPUT_VARIABLE apply_output
      ERROR_VARIABLE apply_error
      RESULT_VARIABLE result
    )
  else()
    execute_process(
      COMMAND ${GIT_EXECUTABLE} apply --reverse --check -p1 "${PATCH_FILE}"
      WORKING_DIRECTORY "${WORKING_DIR}"
      OUTPUT_VARIABLE apply_output
      ERROR_VARIABLE apply_error
      RESULT_VARIABLE reverse_result
    )

    if(reverse_result EQUAL 0)
      set(result 0)
    else()
      set(result ${check_result})
    endif()
  endif()
elseif(PATCH_EXECUTABLE)
  execute_process(
    COMMAND ${PATCH_EXECUTABLE} -p1 --forward --silent --input=${PATCH_FILE}
    WORKING_DIRECTORY "${WORKING_DIR}"
    OUTPUT_VARIABLE apply_output
    ERROR_VARIABLE apply_error
    RESULT_VARIABLE result
  )

  if(result GREATER 1)
    message(FATAL_ERROR "Patch failed with exit code ${result}")
  endif()
else()
  message(FATAL_ERROR "Neither git nor patch found. Cannot apply ${PATCH_FILE}")
endif()

if(NOT result EQUAL 0)
  message(FATAL_ERROR "Patch failed with exit code ${result}:\n${apply_error}")
endif()
