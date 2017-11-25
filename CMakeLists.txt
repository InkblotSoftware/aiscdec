cmake_minimum_required (VERSION 2.8)

find_package (PythonLibs)


##  --------------------------------------------------------------------------
##  Compiler config - all targets
##    NB libais doesn't compile under -Wall -Werror

set (CMAKE_CXX_STANDARD 11)


##  --------------------------------------------------------------------------
##  Find libais - includes library target 'ais'

add_subdirectory (foreign/libais)


##  --------------------------------------------------------------------------
##  Compiler config - just for this project

if (CMAKE_COMPILER_IS_GNUCC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif (CMAKE_COMPILER_IS_GNUCC)
if (MSVC)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif (MSVC)


##  --------------------------------------------------------------------------
##  Target: aiscdec library (the project)

add_library (aiscdec
  src/aiscdec.cpp
  foreign/libais/src/libais/ais_py.cpp
)
target_link_libraries (aiscdec
  PUBLIC  json-c
  PRIVATE ais
          python2.7
)
target_include_directories (aiscdec
  PUBLIC  include
  PRIVATE ${PYTHON_INCLUDE_DIRS}
)
set_target_properties (aiscdec
  PROPERTIES PUBLIC_HEADER include/aiscdec.h
)


##  --------------------------------------------------------------------------
##  Target: test runner

add_executable (run_tests
  src/run_tests.cpp
)
target_link_libraries (run_tests
  aiscdec
  ais
  
  json-c
  python2.7
)
