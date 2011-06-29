# Locate the log4cxx logging library
# This module defines
# LOG4CXX_LIBRARY
# LOG4CXX_FOUND, if false, do not try to link to Ariba
# LOG4CXX_INCLUDE_DIR, where to find the headers
#
# Created by Mathias Gottschlag. This was influenced by the FindOpenAL.cmake
# module by Eric Wing.

#=============================================================================
# Copyright 2005-2009 Kitware, Inc.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

# If you use this, you shall include log4cxx files like
# #include <log4cxx/log4cxx.h>

FIND_PATH(LOG4CXX_INCLUDE_DIR log4cxx/log4cxx.h
  PATH_SUFFIXES include
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw # Fink
  /opt/local # DarwinPorts
  /opt/csw # Blastwave
  /opt
)

FIND_LIBRARY(LOG4CXX_LIBRARY
  NAMES log4cxx
  PATH_SUFFIXES lib64 lib libs64 libs libs/Win32 libs/Win64
  PATHS
  ~/Library/Frameworks
  /Library/Frameworks
  /usr/local
  /usr
  /sw
  /opt/local
  /opt/csw
  /opt
)

# handle the QUIETLY and REQUIRED arguments and set LOG4CXX_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4cxx DEFAULT_MSG LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)

MARK_AS_ADVANCED(LOG4CXX_LIBRARY LOG4CXX_INCLUDE_DIR)
