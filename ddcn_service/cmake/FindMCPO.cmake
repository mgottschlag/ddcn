# Locate the MCPO multicast library
# This module defines
# MCPO_LIBRARY
# MCPO_FOUND, if false, do not try to link to MCPO
# MCPO_INCLUDE_DIR, where to find the headers
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

# If you use this, you shall include MCPO files like
# #include <mcpo/MCPO.h>

FIND_PATH(MCPO_INCLUDE_DIR mcpo/MCPO.h
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

FIND_LIBRARY(MCPO_LIBRARY
  NAMES mcpo
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

# handle the QUIETLY and REQUIRED arguments and set MCPO_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MCPO  DEFAULT_MSG  MCPO_LIBRARY MCPO_INCLUDE_DIR)

MARK_AS_ADVANCED(MCPO_LIBRARY MCPO_INCLUDE_DIR)
