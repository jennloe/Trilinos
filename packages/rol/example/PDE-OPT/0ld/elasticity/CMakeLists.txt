
IF(${PROJECT_NAME}_ENABLE_Intrepid AND 
   ${PROJECT_NAME}_ENABLE_Epetra   AND
   ${PROJECT_NAME}_ENABLE_Amesos   AND
   ${PROJECT_NAME}_ENABLE_Amesos2  AND
   ${PROJECT_NAME}_ENABLE_Tpetra )

  # Need ROL_TpetraMultiVector.hpp
  TRIBITS_INCLUDE_DIRECTORIES(${${PACKAGE_NAME}_SOURCE_DIR}/adapters/tpetra/src/vector)

  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    example_01
    SOURCES example_01.cpp
    ARGS PrintItAll
    NUM_MPI_PROCS 1
    NUM_TOTAL_CORES_USED 1
    PASS_REGULAR_EXPRESSION "TEST PASSED"
    ADD_DIR_TO_NAME
  )
  
  TRIBITS_ADD_EXECUTABLE_AND_TEST(
    example_02
    SOURCES example_02.cpp
    ARGS PrintItAll
    NUM_MPI_PROCS 1
    NUM_TOTAL_CORES_USED 1
    PASS_REGULAR_EXPRESSION "TEST PASSED"
    ADD_DIR_TO_NAME
  )

  TRIBITS_COPY_FILES_TO_BINARY_DIR(
    OldElasticityDataCopy
    SOURCE_FILES
      input.xml
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}"
  )


ENDIF()
