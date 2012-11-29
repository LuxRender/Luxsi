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

#ifndef LUXSI_H
#define LUXSI_H
//--
#pragma warning (disable : 4244) 
#pragma warning (disable : 4996) 

#include "luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;

#define PI 3.14159265

/** CString containers 
*   vFileLxs        //- lxs file name
*   vFileQueue      //- lxq file name
*   luxsiShaderData //- shader data
*   queue_list      //- queue list data
*   vblxs_file      //- name of reload .lxs file
*/
CString vFileLxs, vFileQueue, luxsiShaderData, queue_list, vblxs_file, vmatPreview;

/* for launch LuxRender with -L "queue.lxq" parameter 
*/
bool lqueue = false;

/* override geometry ( wip ) 
*/
bool overrGeometry = false;

/* Export Material Preview definition.*/
CString vluxMatExport;
/* Time 
*/
double ftime = DBL_MAX;

/*  Define frame step */
int vframestep = 1;

/*  Update UI values */
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

/**/
void writeLuxsiCam(X3DObject o);

//-
int writeLuxsiLight();

CString writeLuxsiCloud(X3DObject o);

CString writeLuxsiInstance(X3DObject o);

CString writeLuxsiObj(X3DObject o);

CString writeLuxsiShader();

void writeLuxsiScene(double ftime);

void luxsiRender(CString in_file);

//-
void luxsi_execute();

void luxsi_mat_preview(bool onlyExport);

std::string luxsi_replace(string input);
//--
bool luxsi_find(CStringArray a, CString s);
//--
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt);

/* Pressets */
void luxsi_render_presets( PPGEventContext ctxt);
//--
void dynamic_luxsi_UI(Parameter changed, CString paramName, PPGEventContext ctxt);
//-
void dynamic_sampler_UI( Parameter changed, CString paramName, PPGEventContext ctxt);

/**/
void loader(const char szArgs[]);

/**/
extern CString findInGroup(CString s);

 /**/
extern CString luxsi_normalize_path(CString in_Filepath);

/**/
std::map<CString, int> int_values;

#endif //LUXSI_H