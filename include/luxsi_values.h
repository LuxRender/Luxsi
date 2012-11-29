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

#ifndef LUXSI_VALUES_H
#define LUXSI_VALUES_H
//--
#pragma warning (disable : 4244) 
#pragma warning (disable : 4996) 

#include "luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;


Application app;
Model root;
ofstream f;
Null null;
CustomProperty prop ;
UIToolkit kit = app.GetUIToolkit();
ProgressBar pb = kit.GetProgressBar();
//--
CRefArray params;

//-
float red = 0.0f, green = 0.0f, blue = 0.0f, alpha = 0.0f;

//-- distributepath
bool vdirectdiffuse = true, vdirectglossy = true, vindirectsampleall = false, vindirectdiffuse = true, vindirectglossy = true;
bool vdirectsampleall = true;
bool vdiff_reflect_reject = false, vdiff_refract_reject = false, vglossy_reflect_reject = false, vglossy_refract_reject = false;

int vdirectsamples = 1, vindirectsamples = 1, vdiffusereflectdepth = 3, vdiffusereflectsamples = 1;
int vdiffuserefractdepth = 5, vdiffuserefractsamples = 1, vglossyreflectdepth = 2, vglossyreflectsamples = 1;
int vglossyrefractdepth = 5, vglossyrefractsamples = 1,  vspecularreflectdepth = 3, vspecularrefractdepth = 5;

float vdiff_reflect_reject_thr = 10.0f, vdiff_refract_reject_thr = 10.0f, vglossy_reflect_reject_thr = 10.0f;
float vglossy_refract_reject_thr = 10.0f;


//-- photonmap --------
int vmaxeyedepth = 8, vmaxphotondepth = 10, vshadowraycount = 0, vdirectphotons = 1000000, vcausticphotons = 20000;
int vindirectphotons = 200000, vradiancephotons = 20000, vnphotonsused = 50, vfinalgathersamples = 32, vrenderingmode=0;
float vmaxphotondist = 0.10f, vgatherangle = 10.0f, vdistancethreshold = 0.75f;
bool vfinalgather = false, vdbg_direct = false, vdbg_radiancemap = false, vdbg_indircaustic = false;
bool vdbg_indirdiffuse = false, vdbg_indirspecular = false;
CString bphotonmaps = L"";

//-- sppm 
int vbmaxeyedepht = 48, vbmaxphoton = 48, vbpointxpass = 0, vbphotonsxpass = 1000000;
float vbstartradius= 2.0f, vbalpha = 0.70f;
bool vbdlsampling = false, vbincenvironment = false;


//-- surface integrator
int vSurfaceInt = 0, vRRstrategy = 0, vEye_depth=16, vLight_depth=16, vLight_str = 2, vmaxdepth = 10;
float vEyeRRthre = 0.0f, vLightRRthre = 0.0f, vrrcon_prob = 0.65f;
bool vInc_env = false, vsexpert = false;

//-- volume integrator
int vvolumeint = 0;

// Accelerator	
int vmaxprimsperleaf = 4, vfullsweepthr = 16, vskipfactor = 1, vtreetype = 2; // combo
int vcostsamples = 0, vmaxprims = 1, vacmaxdepth = -1;

//-- kd-tree
int vintersectcost = 80, vtraversalcost = 1; 
float vemptybonus = 0.2f;
bool vrefineinmed = false, vacexpert = false;


//-- igi
int vnsets = 4, vnlights = 64;
float vmindist = 0.10f; 
//-- Sampler
float vlmutationpro = 0.1f, vmutationrange = 2.5;
int vSampler = 2, vmaxconsecrejects = 256, vchainlength = 512, vPixsampler = 3, vPixelsamples = 2;
int vbasampler = 0;
bool vusevariance = false;
//-- 
int vBounces=10, vpresets=2 ;

//-- stop render option, display int., save int.
int vEngine = 0, vThreads = 2, vAccel = 2, vRmode = 0;
bool vAutoTh = true, vExpert = false, luxdebug = false;
int vhaltspp = 0, vhalttime = 0, vDis = 12, vSave = 120;



//-- hidden options
/** Booleans for export hidden scene items:
*   vIsHiddenCam,       />! cameras. Really, atm, luxsi only expor an active camera
*   vIsHiddenLight,     />! lights
*   vIsHiddenObj,       />! Geometry..
*   vIsHiddenSurface,   />! Surface primitives
*   vIsHiddenClouds,    />! Pointclouds
*/
bool vIsHiddenCam = true, vIsHiddenLight = true, vIsHiddenObj = true, vIsHiddenSurface = false, vIsHiddenClouds = false;

//--
bool vIsLinux = false;
//bool vProg = true; // not used ??
bool vExportDone = false, vResume = false;

//-- export
bool vSmooth_mesh = false, vSharp_bound = false, vplymesh = false;;

// float vCSize = 0.4f, vGITolerance = 0.025f, vSpacingX = 0.1f, vSpacingY = 0.1f; 
float vContrast = 2.2f;
float vrrprob = 0.65f;
//-- surface

//-- filter
float vXwidth = 2.0f, vYwidth = 2.0f, vFalpha = 2.0f, vF_B = 0.3f, vF_C = 0.3f, vTau = 0.3f;
int vfilter = 2;
bool vfexpert = false, vSupers = false;;

//-- material preview

int vlights = 0; //  UNUSED ??

/* is 'preview' or normal scene
*/
bool is_preview;

//----/ convention names, prefix; Mt /----->
const char *MtBool[] = { "false", "true" };
const char *MtBsampler[]= { "lowdiscrepancy", "random" }; // mode, Asampler [vSampler]
const char *MtAccel[]= { "qbvh", "bvh", "kdtree" }; // mode; iter vAccel

//--
CRefArray aGroups;
/** CStringArrays:
*   aMatList,       />! for Material
*   aInstanceList,  />! for Instances
*/
CStringArray aMatList, aInstanceList;

CString vSun = L"", vHDRI = L"", vFileExport = L"", vFileGeo = L"", vLuXSIPath = L"";

//-- new test for values
//-- save image options
int vXRes = 640, vYRes = 480, vRpng = 3, vExr_Znorm = 2, vRtga = 1;
bool vPng = true, vWpng_16 = false, vPng_gamut = false, vTga = false, vTga_gamut = false, vExr = false;

#endif //LUXSI_VALUES_H