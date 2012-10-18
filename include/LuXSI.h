/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender Renderer
(http://www.luxrender.org)

Copyright (C) 2010 - 2012  Michael Gangolf, 'miga'
Code contributor ; Pedro Alcaide, 'povmaniaco'

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef LUXSI_H
#define LUXSI_H
//--
#pragma warning (disable : 4244) 
#pragma warning (disable : 4996) 

#include "luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/** CString containers 
*   vFileLxs        //- lxs file name
*   vFileQueue      //- lxq file name
*   luxsiShaderData //- shader data
*   queue_list      //- queue list data
*   vblxs_file      //- name of reload .lxs file
*/
CString vFileLxs, vFileQueue, luxsiShaderData, queue_list, vblxs_file;

//-- new test for values
//-- save image options
int vXRes = 640, vYRes = 480, vRpng = 3, vExr_Znorm = 2, vRtga = 1;
bool vPng = true, vWpng_16 = false, vPng_gamut = false, vTga = false, vTga_gamut = false, vExr = false;

/* for launch LuxRender with -L "queue.lxq" param 
*/
bool lqueue = false;

/* override geometry ( wip ) 
*/
bool overrGeometry = false;

/* time 
*/
double ftime = DBL_MAX;

/*  define frame step */
int vframestep = 1;

/* Update UI values
*/
void update_general_values(CString paramName, Parameter changed, PPGEventContext ctxt);

/**/
void update_surfaceInt_values(CString paramName, Parameter changed, PPGEventContext ctxt);

/**/
void update_sampler_values(CString paramName, Parameter changed, PPGEventContext ctxt);

/**/
void update_filter_values(CString paramName, Parameter changed, PPGEventContext ctxt);

/**/
void update_accelerator_values(CString paramName, Parameter changed, PPGEventContext ctxt);

/**/
void dynamic_surfaceInt_UI(Parameter changed, CString paramName, PPGEventContext ctxt);

/**/
void dynamic_filter_UI(Parameter changed, CString paramName, PPGEventContext ctxt);

/**/
void dynamic_Accel_UI(Parameter changed, CString paramName, PPGEventContext ctxt);

/**/
void writeLuxsiBasics();

void writeLuxsiCam(X3DObject o);

//-
int writeLuxsiLight();

CString writeLuxsiCloud(X3DObject o);

CString writeLuxsiInstance(X3DObject o);

CString writeLuxsiObj(X3DObject o);

CString writeLuxsiShader();

void luxsi_write(double ftime);

void luxsi_preview(CString in_mat);

//-
void luxsi_execute();

void luxsi_mat_preview();

std::string luxsi_replace(string input);
//--
bool luxsi_find(CStringArray a, CString s);
//--
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt);
//--
void luxsi_render_presets( PPGEventContext ctxt);
//--
void dynamic_luxsi_UI(Parameter changed, CString paramName, PPGEventContext ctxt);
//-
void dynamic_sampler_UI( Parameter changed, CString paramName, PPGEventContext ctxt);
//-
extern CString findInGroup(CString s);

/**/
std::map<CString, int> int_values;

#endif //LUXSI_H