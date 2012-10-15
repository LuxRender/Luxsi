############################################################################
# LuXSI - Autodesk(c) Softimage(c) XSI Exporter addon for LuxRender  Renderer
# (http://www.luxrender.org)
#
# Copyright (C) 2010 - 2012  Michael Gangolf 'miga'
# Code contributor ; Pedro Alcaide,  'povmaniaco'
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
########################################################################

MESSAGE(STATUS "CMAKE VERSION DETECTED " ${CMAKE_VERSION})

IF(MSVC)
	cmake_minimum_required(VERSION 2.8)
	cmake_policy(VERSION 2.8)
ENDIF(MSVC)

PROJECT(luxsi CXX)
SET(VERSION 1.1)    

## include SDK headers ##
INCLUDE_DIRECTORIES("$ENV{XSISDK_ROOT}/include/Core")
INCLUDE_DIRECTORIES("$ENV{XSISDK_ROOT}/include")

## link SDK libs ##
SET(LIBS sicppsdk sicoresdk)
LINK_DIRECTORIES("$ENV{XSISDK_ROOT}/lib/nt-x86")

IF(LIBS)
	LINK_LIBRARIES(${LIBS})
ENDIF(LIBS)

## set project sources and headers ##
file(GLOB luxsi_SOURCES ${CMAKE_SOURCE_DIR}/*.cpp)
file(GLOB luxsi_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)

SOURCE_GROUP("Source Files" FILES ${luxsi_SOURCES})
SOURCE_GROUP("Header Files" FILES ${luxsi_HEADERS})

## add all files ##
ADD_LIBRARY(luxsi SHARED ${luxsi_SOURCES} ${luxsi_HEADERS})


## configure flags (experimental, copy from LuxRender project)##
IF(MSVC90)
	MESSAGE(STATUS "Version 9")
	SET(MSVC_RELEASE_COMPILER_FLAGS "/O2 /Oi /Oy- /GF /EHsc /MD /fp:precise /GR /Gy /Gd /TP")

	SET(MSVC_RELEASE_WITH_DEBUG_COMPILER_FLAGS "/Zi")

	IF(NOT ARCH_X86_64)
		SET(MSVC_RELEASE_COMPILER_FLAGS "/O2 /Oi /Oy- /GF /EHsc /MD /fp:precise /GR /Gy /Gd /TP /arch:SSE2")
		MESSAGE(STATUS "Arch x86")
	ENDIF(NOT ARCH_X86_64)

	SET(CMAKE_CXX_FLAGS_RELEASE "${MSVC_RELEASE_COMPILER_FLAGS} ${MSVC_RELEASE_WITH_DEBUG_COMPILER_FLAGS}")
ENDIF(MSVC90)












