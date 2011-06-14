# Locate the QCA2 cryptography library
# This module defines
# QCA2_LIBRARY
# QCA2_FOUND, if false, do not try to link to QCA2
# QCA2_INCLUDE_DIR, where to find the headers
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

# If you use this, you shall include QCA2 files like
# #include <QtCrypto>

FIND_PATH(QCA2_INCLUDE_DIR qca.h
  PATH_SUFFIXES include/QtCrypto include
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

FIND_LIBRARY(QCA2_LIBRARY
  NAMES qca
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

# handle the QUIETLY and REQUIRED arguments and set QCA2_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(QCA2 DEFAULT_MSG QCA2_LIBRARY QCA2_INCLUDE_DIR)

MARK_AS_ADVANCED(QCA2_LIBRARY QCA2_INCLUDE_DIR)
