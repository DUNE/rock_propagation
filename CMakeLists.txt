cmake_minimum_required(VERSION 3.00)
project (RockPropGENIE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(LIBRARY_OUTPUT_PATH "${CMAKE_BINARY_DIR}/lib")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
set (ROCKPROP_SRC_DIR "${PROJECT_SOURCE_DIR}/src")

find_package(ROOT REQUIRED COMPONENTS Core RIO Hist Tree Rint XMLIO Geom Physics EGPythia6 GenVector)
include(${ROOT_USE_FILE})

if(GENIE STREQUAL "")
  cmessage(FATAL_ERROR "Variable GENIE is not defined. ")
endif()

execute_process (COMMAND genie-config --topsrcdir OUTPUT_VARIABLE GENIE_INC_DIR)
execute_process (COMMAND genie-config --libs OUTPUT_VARIABLE GENIE_LIBS_DIR_WITH_LFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process (COMMAND genie-config --libdir OUTPUT_VARIABLE GENIE_LIBS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)
include_directories(${GENIE_INC_DIR})

message(STATUS "GENIE inc : ${GENIE_INC_DIR}")
#message(STATUS "GENIE libdir  : ${GENIE_LIBS_DIR}")
#message(STATUS "GENIE libs    : ${GENIE_LIBS_DIR_WITH_LFLAGS}")

find_library(log4cpp NAMES log4cpp PATHS $ENV{LOG4CPP_LIB} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
include_directories("$ENV{LOG4CPP_INC}")

find_library(pythia6 NAMES Pythia6 pythia6 PATHS $ENV{PYTHIA6_LIBRARY} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
include_directories("$ENV{PYTHIA_INC}")

find_library(libxml2 NAMES libxml2 xml2 PATHS $ENV{LIBXML2_FQ_DIR} PATH_SUFFIXES lib lib64 NO_DEFAULT_PATH)
include_directories("$ENV{LIBXML2_INC}")

find_package(Geant4 REQUIRED)
include(${Geant4_USE_FILE})

include_directories(${ROCKPROP_SRC_DIR})
add_subdirectory(src)
