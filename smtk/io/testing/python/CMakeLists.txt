set(smtkIOPythonTests
  unitOperatorLog
)

# Additional tests that require SMTK_DATA_DIR
set(smtkIOPythonDataTests
)

foreach (test ${smtkIOPythonTests})
  add_test(${test}Py ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
  set_tests_properties(${test}Py
    PROPERTIES
      ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
  )
endforeach()

if (SMTK_DATA_DIR AND EXISTS ${SMTK_DATA_DIR}/ReadMe.mkd)
  foreach (test ${smtkIOPythonDataTests})
    add_test(${test}Py
             ${PYTHON_EXECUTABLE}
             ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
             --data-dir=${SMTK_DATA_DIR})
    set_tests_properties(${test}Py
      PROPERTIES
        ENVIRONMENT "PYTHONPATH=${VTKPY_DIR}${VTKPY_DIR}${SHIBOKEN_SMTK_PYTHON};${LIB_ENV_VAR}"
    )
  endforeach()
endif()
