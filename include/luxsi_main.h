/*
This file is part of LuXSI;
LuXSI is a LuxRender Exporter for Autodesk(C) Softimage(C) ( ex-XSI )
http://www.luxrender.net

Copyright(C) 2007 - 2012  of all Authors:
Michael Gangolf, 'miga', mailto:miga@migaweb.de                                               
Pedro Alcaide, 'povmaniaco', mailto:p.alcaide@hotmail.com
 
LuXSI is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation, either version 3 of the License,
or (at your option) any later version.

LuXSI is distributed in the hope that it will be useful,              
but WITHOUT ANY WARRANTY; without even the implied warranty of        
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         
GNU General Public License for more details.                          
                                                                           
You should have received a copy of the GNU General Public License     
along with LuXSI.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LUXSI_MAIN_H
#define LUXSI_MAIN_H

//#define __forceinline inline
#pragma once // it is need?
//-- test from S. Caron
#include <stdio.h>
#include <assert.h>
//#include <string.h>
//-- end test

#include <math.h>
#include <string.h>
#include <sstream>
#include <fstream>
#include <map> // for map

//--
#include <xsi_segment.h>
#include <xsi_materiallibrary.h>
#include <xsi_particlecloud.h>
#include <xsi_particlecloudprimitive.h>
#include <xsi_application.h>
#include <xsi_pluginregistrar.h>
#include <xsi_geometryaccessor.h>
//- for ICE ?
#include <xsi_iceattribute.h>
#include <xsi_iceattributedataarray.h>

#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_source.h>
#include <xsi_triangle.h>
#include <xsi_material.h>
#include <xsi_kinematics.h>
#include <xsi_project.h>
#include <xsi_camera.h>
#include <xsi_group.h>
#include <xsi_null.h>
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_light.h>
#include <xsi_polygonmesh.h>
#include <xsi_texture.h>
#include <xsi_imageclip2.h>
#include <xsi_progressbar.h>
#include <xsi_uitoolkit.h>
#include <xsi_pass.h>
#include <xsi_scene.h>
#include <xsi_nurbssurfacemesh.h>
#include <xsi_nurbssurface.h>
#include <xsi_controlpoint.h>
#include <xsi_ogllight.h>
#include <xsi_trianglevertex.h>
#include <xsi_polygonface.h>
#include <xsi_vertex.h>
#include <xsi_polygonnode.h>
#include <xsi_utils.h>

//
#include <xsi_scene.h>
#include <xsi_library.h>
#include <xsi_source.h>

#include <xsi_projectitem.h>
#include <xsi_siobject.h>
#include <xsi_base.h>

//- for render integrated
#include <xsi_renderer.h>
#include <xsi_renderercontext.h>
#include <xsi_math.h> // problems ?

//- for hair
#include <xsi_hairprimitive.h>
#include <xsi_renderhairaccessor.h>


#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#include <winioctl.h>
#endif

#define WIN32_LEAN_AND_MEAN

#endif // LUXSI_MAIN_H