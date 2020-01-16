# - Basic CMake setup
# Check version meets ou requirements
# Declare project, which will configure compiler for us
cmake_minimum_required(VERSION 3.3)
project(fltranslate)

# Modules use FalaiseBayeux, so we need to locate this or fail
# Locating Falaise will automatically locate all of its
# dependencies such as Bayeux, ROOT and Boost.
find_package(Falaise REQUIRED)

# (Re)Find ROOT, need TreePlayer lib
find_package(ROOT REQUIRED TreePlayer)

# Build an executable from our sources
add_executable(fltranslate
  fltranslate.cc
  FLSimulateCommandLine.h
  FLSimulateErrors.h
  FLSimulateArgs.h
  FLSimulateArgs.cc
  getopt_pp.h
  getopt_pp.cpp)

# Link it to the FalaiseModule library
# This ensures the correct compiler flags, include paths
# and linker flags are applied to our dynamic library.
target_link_libraries(fltranslate PUBLIC
  Falaise::Falaise
  Bayeux::Bayeux_mctools_geant4
  ${ROOT_TreePlayer_LIBRARY}
  )
