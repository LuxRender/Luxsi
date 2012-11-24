#######################################################################
#
# This file is part of LuXSI;
# LuXSI is a LuxRender Exporter for Autodesk(C) Softimage(C) ( ex-XSI )
# (http://www.luxrender.org)
#
# Copyright(C) 2007 - 2012  of all Authors:
# Michael Gangolf, 'miga', mailto:miga@migaweb.de
# Pedro Alcaide, 'povmaniaco', mailto:p.alcaide@hotmail.com
#
# LuXSI is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
#
# LuXSI is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty
# of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with LuXSI.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################

MESSAGE(STATUS "CMAKE VERSION DETECTED " ${CMAKE_VERSION})

IF(MSVC)
        cmake_minimum_required(VERSION 2.8)
        cmake_policy(VERSION 2.8)
ENDIF(MSVC)

PROJECT(shaders CXX)
SET(VERSION 1.1)

## include SDK headers ##
INCLUDE_DIRECTORIES("$ENV{XSISDK_ROOT}/include/Core")
INCLUDE_DIRECTORIES("$ENV{XSISDK_ROOT}/include")

## link SDK libs ##
SET(LIBS sicppsdk sicoresdk shader)

## set architecture ##
IF(ARCH_X86_64)
    SET(ARCH nt-x86-64)
ELSE(ARCH_X86_64)
    SET(ARCH nt-x86)
ENDIF(ARCH_X86_64)

LINK_DIRECTORIES("$ENV{XSISDK_ROOT}/lib/${ARCH}")

IF(LIBS)
        LINK_LIBRARIES(${LIBS})
ENDIF(LIBS)

## set project sources and headers ##
file(GLOB luxsi_SHADERS_SOURCES ${CMAKE_SOURCE_DIR}/*.cpp)
file(GLOB luxsi_SHADERS_HEADERS ${CMAKE_SOURCE_DIR}/include/*.h)

SOURCE_GROUP("Source Files" FILES ${luxsi_SHADERS_SOURCES})
SOURCE_GROUP("Header Files" FILES ${luxsi_SHADERS_HEADERS})


## shaders libraries
add_library(lux_car_paint           SHARED lux_car_paint.cpp)
add_library(lux_emitter_mat         SHARED lux_emitter_mat.cpp)
add_library(lux_glass               SHARED lux_glass.cpp)
add_library(lux_glossy              SHARED lux_glossy.cpp)
add_library(lux_matte               SHARED lux_matte.cpp)
add_library(lux_mattetranslucent    SHARED lux_mattetranslucent.cpp)
add_library(lux_metal               SHARED lux_metal.cpp)
add_library(lux_mirror              SHARED lux_mirror.cpp)
add_library(lux_point_light         SHARED lux_point_light.cpp)
add_library(lux_roughglass          SHARED lux_roughglass.cpp)
add_library(lux_scatter             SHARED lux_scatter.cpp)
add_library(lux_shinymetal          SHARED lux_shinymetal.cpp)
add_library(lux_substrate           SHARED lux_substrate.cpp)
add_library(lux_velvet              SHARED lux_velvet.cpp)