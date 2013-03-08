/***********************************************************************
This file is part of LuXSI;
LuXSI is a LuxRender Exporter for Autodesk(C) Softimage(C) ( ex-XSI )
http://www.luxrender.net

Copyright(C) 2007 - 2013  of all Authors:
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

***********************************************************************/

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

/* CString containers */
/*  Scene file name */
CString vFileLxs;
/*
* Name for Queue file used in animation */
CString vFileQueue;          
/**/
/* 
*  for list of .lxs filenames writed inside Queue file.
*/
CString queue_list;
/**/
CString vblxs_file;
/**/
CString vmatPreview;

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

//-- volume integrator
int vvolumeint = 2;

/*  Define frame step */
int vframestep = 1;

/* Real-time update UI values */
//void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt);
void update_main_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/**/
void update_general_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/**/
void update_surfaceInt_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/**/
void update_sampler_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/**/
void update_filter_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/**/
void update_accelerator_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/* test */
void update_slgSurfIntegrator_values(CString paramName, Parameter changed, PPGEventContext ctxt);
/* 
* Show or hide UI elements
*/
void dynamic_luxsi_UI(Parameter changed, CString paramName, PPGEventContext ctxt);
/**/
void dynamic_sampler_UI( Parameter changed, CString paramName, PPGEventContext ctxt);
/**/
void dynamic_surfaceInt_UI(Parameter changed, CString paramName, PPGEventContext ctxt);
/**/
void dynamic_filter_UI(Parameter changed, CString paramName, PPGEventContext ctxt);
/**/
void dynamic_Accel_UI(Parameter changed, CString paramName, PPGEventContext ctxt);
/* test */
void dynamic_slgSurfIntegrator_UI(Parameter changed, CString paramName, PPGEventContext ctxt);

/**/
CString writeLuxsiBasics();

/**/
CString writeLuxsiCam();

//-
CString writeLuxsiLight();

CString writeLuxsiCloud(X3DObject o);

CString writeLuxsiInstance(CRefArray in_array);

CString writeLuxsiObj(X3DObject o);

CString writeLuxsiShader();

/* functions group scene */
void createLuxsiScene(double ftime);
//
int writeLxmFile(CString in_string);
//
int writeLxsFile(CString in_string); //, CRefArray in_array);
//
int writeLxoFile(CString in_file, CRefArray in_mesh, CRefArray in_models, 
                 CRefArray in_clouds, CRefArray in_surfaces, CRefArray in_hairs);

/**/
void luxsiRender(CString in_file);

//-
CString SLGFileconfig(CString in_filename);
//-
void luxsi_execute();
//-
void luxsi_mat_preview(bool onlyExport);

std::string luxsi_replace(string input);
//--
bool luxsi_find(CStringArray a, CString s);

/**/
void loader(const char szArgs[]);

// externs functions...---------------->
/**/
extern CRefArray sceneCollectionsCameras();
/**/
extern CString findInGroup(CString s);

 /**/
extern CString luxsi_normalize_path(CString in_Filepath);

/**/
extern bool is_visible(X3DObject o, CString objType);

/**/
extern CString writeLuxsiHair(X3DObject o);

/**/
extern CString writeLuxsiSurface(X3DObject o);

/**
* Array references for scene elements..     \n
* sceneItemsArray   array for all scene items   \n
* aObj              for objects( polygon mesh ) \n
* aCam              for camera                  \n
* aSurfaces         for 'surface' primitives    \n
* aClouds           for 'pointclouds' objects   \n
* aModels           for model objects           \n
* aHair             for hair 'emitter' object.
*/
CRefArray sceneItemsArray, aObj, aCam, aSurfaces, aClouds, aModels, aHair;

/**/
extern CRefArray sceneCollectionMaterials();
/* Pressets */
extern void luxsi_render_presets( PPGEventContext ctxt);

// test
//-- distributepath
bool vdirectdiffuse = true, vdirectglossy = true, vindirectsampleall = false, vindirectdiffuse = true;
bool vindirectglossy = true, vdirectsampleall = true, vdiff_reflect_reject = false;
bool vdiff_refract_reject = false, vglossy_reflect_reject = false, vglossy_refract_reject = false;

int vdirectsamples = 1, vindirectsamples = 1, vdiffusereflectdepth = 3, vdiffusereflectsamples = 1;
int vdiffuserefractdepth = 5, vdiffuserefractsamples = 1, vglossyreflectdepth = 2, vglossyreflectsamples = 1;
int vglossyrefractdepth = 5, vglossyrefractsamples = 1,  vspecularreflectdepth = 3, vspecularrefractdepth = 5;

float vdiff_reflect_reject_thr = 10.0f, vdiff_refract_reject_thr = 10.0f, vglossy_reflect_reject_thr = 10.0f;
float vglossy_refract_reject_thr = 10.0f;

//-- end
//- slg modes
float vrusianrouletdepth = 0.5;

#endif //LUXSI_H