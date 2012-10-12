##################################################
MESSAGE(STATUS "CMAKE VERSION DETECTED " ${CMAKE_VERSION})
IF(MSVC)
	cmake_minimum_required(VERSION 2.8)
ENDIF(MSVC)

PROJECT(luxsi)
SET(VERSION 1.0)    


SET(XSISDK_ROOT h:/XSISDK75)
SET(XSI_ROOT H:/Softimage/Softimage_Mod_Tool_7.5/Application/bin)

include_directories("${XSISDK_ROOT}/include/Core")
include_directories("${XSISDK_ROOT}/include")
set(XSILIB "${XSISDK_ROOT}/lib/nt-x86")


#set(CMAKE_CXX_FLAGS "-L${XSISDK_ROOT}/lib/nt-x86/sicppsdk.lib -L${XSISDK_ROOT}/lib/nt-x86/sicoresdk.lib")
add_library(luxsi SHARED LuXSI.cpp luxsi_geometry.cpp luxsi_gui.cpp luxsi_integrated.cpp luxsi_light.cpp luxsi_material.cpp luxsi_pointclouds.cpp luxsi_texture.cpp luxsi_utils.cpp)

IF(MSVC90)
		MESSAGE(STATUS "Version 9")
		
		SET(MSVC_RELEASE_COMPILER_FLAGS "/O2 /Ob1 /Oi /Oy- /GT /GL /EHsc /MD /GS /fp:precise /Zc:forScope /GR /Gy /Gd /TP /Ot /favor:blend /Qfast_transcendentals /MP")
		SET(MSVC_RELEASE_WITH_DEBUG_COMPILER_FLAGS "/Zi")
		IF(NOT ARCH_X86_64)
			SET(MSVC_RELEASE_COMPILER_FLAGS ${MSVC_RELEASE_COMPILER_FLAGS} " /arch:SSE2")
		ENDIF(NOT ARCH_X86_64)
ENDIF(MSVC90)
