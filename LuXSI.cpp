/*
LuXSI - Softimage XSI Export plug-in for the LuxRender (http://www.luxrender.org) renderer

Copyright (C) 2010  Michael Gangolf

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


#pragma warning (disable : 4245) // signed/unsigned mismatch
#pragma warning (disable : 4996) // strcpy/unsigned mismatch

#include "luxsi_main.h"

using namespace std;
using namespace XSI;
using namespace MATH;


#define PI 3.14159265

void writeLuxsiBasics();
void writeLuxsiCam(X3DObject o);
void writeLuxsiLight(X3DObject o);
int writeLuxsiCloud(X3DObject o);
int writeLuxsiInstance(X3DObject o);
int writeLuxsiObj(X3DObject o);

void writeLuxsiShader();
void luxsi_write();
void luxsi_execute();
CString readIni();
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt, PPGLayout lay);

void luxsi_render_presets( CString paramName, Parameter changed, PPGEventContext ctxt);
void dynamic_luxsi_UI(Parameter changed, PPGEventContext ctxt);


Application app;
Model root( app.GetActiveSceneRoot() );
ofstream f;
stringstream sLight, sObj, sMat;
Null null;
CustomProperty prop ;
UIToolkit kit = app.GetUIToolkit();
ProgressBar pb = kit.GetProgressBar();



//-- distributepath
bool vdirectdiffuse = true, vdirectglossy = true, vindirectsampleall = false, vindirectdiffuse = true, vindirectglossy = true;
bool vdiffuserefractreject= false, vglossyrefractreject= false, vglossyreflectreject = false, vdiffusereflectreject = false;	
bool vdirectsampleall = true;

int vdirectsamples = 1, vindirectsamples = 1, vdiffusereflectdepth = 3, vdiffusereflectsamples = 1;
int vdiffuserefractdepth = 5, vdiffuserefractsamples = 1, vglossyreflectdepth = 2, vglossyreflectsamples = 1;
int vglossyrefractdepth = 5, vglossyrefractsamples = 1,  vspecularreflectdepth = 3, vspecularrefractdepth = 5;

float vdiff_reflect_reject_thr = 10.0f, vdiff_refract_reject_thr = 10.0f, vglossy_reflect_reject_thr = 10.0f;
float vglossy_refract_reject_thr = 10.0f;

//-- photonmap --------
int vmaxdepth = 8, vmaxphotondepth = 10, vshadowraycount, vdirectphotons = 1000000, vcausticphotons = 20000;
int vindirectphotons = 20000, vradiancephotons = 20000, vnphotonsused = 50, vfinalgathersamples = 32, vrenderingmode=0;
float vmaxphotondist = 0.10f, vgatherangle = 10.0f, vdistancethreshold = 0.75f;
bool vfinalgather = false, vdbg_direct = false, vdbg_radiancemap = false, vdbg_indircaustic = false;
bool vdbg_indirdiffuse = false, vdbg_indirspecular = false;

//-- surface integrator
int vSurfaceInt = 0, vRRstrategy = 0, vEye_depth=16, vLight_depth=16, vLight_str = 2;
float vEyeRRthre = 0.0f, vLightRRthre = 0.0f, vrrcon_prob = 0.65f;
bool vInc_env = false, vsexpert = false; 
//--
// Accelerator	
int vmaxprimsperleaf = 4, vfullsweepthr = 16, vskipfactor = 1, vtreetype = 2; // combo
int vcostsamples = 0, vmaxprims = 1, vacmaxdepth = -1;
//-- per kd-tree, floats ?
int vintersectcost = 80, vtraversalcost = 1; 
float vemptybonus = 0.2f;
bool vrefineimmediately = false;


//-- igi
int vnsets = 4, vnlights = 64;
float vmindist = 0.10f; 
//-- Sampler
float vlmutationpro = 0.1f, vmutationrange = 2.5;
int vSampler = 2, vmaxconsecrejects=256, vchainlength= 512, vPixsampler = 3, vPixelsamples=2;
int vbasampler = 0;
bool vusevariance = false;
//-- 
int vBounces=10, vpresets=0 ;

//-- stop render option, display int., save int.
int vEngine = 0, vThreads = 2, vAccel = 2, vRmode = 0;
bool vAutoTh = true, vExpert = false;
int vhaltspp=0, vhalttime=0, vDis=12, vSave=120;

//-- save image options
int vXRes=640, vYRes=480;
int vRpng = 3, vRtga = 1, vExr_Znorm = 2;
bool vPng = true, vWpng_16 = false, vPng_gamut = false;
bool vTga = false, vTga_gamut = false, vExr = false;

//-- hidden options
bool vIsHiddenCam=true, vIsHiddenLight=true, vIsHiddenObj=true, vIsHiddenSurface=false, vIsHiddenClouds=false;
//--
bool vMLT=true, vIsLinux=true;
bool vProg=true; // not used ??
bool vExportDone=false,vResume=false,vUseJitter=true,vExpOne=true;
bool vAmbBack=false;

//-- export
bool vSmooth_mesh = false, vSharp_bound = false;
bool vSupers = false;
float vCSize=0.4f, vGITolerance=0.025f, vSpacingX=0.1f, vSpacingY=0.1f, vContrast=2.2f;
float vrrprob=0.65f;
//-- surface

//-- filter
float vXwidth = 2.0f, vYwidth = 2.0f, vFalpha = 2.0f, vF_B = 0.3f, vF_C = 0.3f, vTau = 0.3f;
int vfilter = 2;
bool vfexpert = false;

//----/ convention names, prefix; Mt /----->
char MtBool[2][6] = { "false", "true" };
char MtBsampler [4] [15] = { "metropolis", "lowdiscrepancy", "random" }; // mode, Asampler [vSampler]
char MtAccel [3] [7] = { "qbvh", "bvh", "kdtree" }; // iter vAccel

//--/ end /-->
CRefArray aGroups;
CStringArray aMatList,aInstanceList;

CString vSun=L"", vHDRI=L"", vLuXSIPath=L"", vFileObjects=L"";

XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
    in_reg.PutAuthor(L"Michael Gangolf");
    in_reg.PutName(L"LuXSI");
    in_reg.PutEmail(L"miga@migaweb.de");
    in_reg.PutURL(L"http://www.migaweb.de");
    in_reg.PutVersion(0,6);
    in_reg.RegisterProperty(L"LuXSI");
    in_reg.RegisterMenu(siMenuTbRenderRenderID,L"LuXSI_Menu",false,false);
    //RegistrationInsertionPoint - do not remove this line // siMenuTbRenderRenderID
    return CStatus::OK;
}

//--
XSIPLUGINCALLBACK CStatus XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
    CString strPluginName;
    strPluginName = in_reg.GetName();
    Application().LogMessage(strPluginName + L" has been unloaded.");
    return CStatus::OK;
}

//--
XSIPLUGINCALLBACK CStatus LuXSI_Define( CRef& in_ctxt )
{
    Context ctxt( in_ctxt );
    // for save space
    int sps = siPersistable;
    CValue dft;  // change for CValue()

    Parameter oParam;
    prop = ctxt.GetSource();

    prop.AddParameter( L"use_hidden_obj",   CValue::siBool, sps,L"",L"", vIsHiddenObj,   dft,dft,dft,dft, oParam);
    prop.AddParameter( L"use_hidden_light", CValue::siBool, sps,L"",L"", vIsHiddenLight, dft,dft,dft,dft, oParam);
    prop.AddParameter( L"use_hidden_cam",   CValue::siBool, sps,L"",L"", vIsHiddenCam,   dft,dft,dft,dft, oParam);
//  prop.AddParameter( L"exp_one",          CValue::siBool, sps,L"",L"", vExpOne,        dft,dft,dft,dft, oParam);
    prop.AddParameter( L"smooth_mesh",      CValue::siBool, sps,L"",L"", vSmooth_mesh,   dft,dft,dft,dft, oParam);
    prop.AddParameter( L"sharp_bound",      CValue::siBool, sps,L"",L"", vSharp_bound,   dft,dft,dft,dft, oParam);
    
    //----/ image /-->
    prop.AddParameter( L"Width",        CValue::siInt4,  sps,L"",L"", vXRes,        0l,2048l,0l,1024l,  oParam);
    prop.AddParameter( L"Height",       CValue::siInt4,  sps,L"",L"", vYRes,        0l,2048l,0l,768l,   oParam);
    prop.AddParameter( L"gamma",        CValue::siFloat, sps,L"",L"", vContrast,    0,10,0,3,           oParam);
    prop.AddParameter( L"progressive",  CValue::siBool,  sps,L"",L"", vProg,        dft,dft,dft,dft,    oParam);
    
    //-- filter //
    prop.AddParameter( L"bfexpert", CValue::siBool,     sps,L"",L"",  vfexpert,      dft,dft,dft,dft, oParam);
    prop.AddParameter( L"bywidth",  CValue::siFloat,    sps,L"",L"", vYwidth,   0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bxwidth",  CValue::siFloat,    sps,L"",L"", vXwidth,   0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bfalpha",  CValue::siFloat,    sps,L"",L"", vFalpha,   0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"ssample",  CValue::siBool,     sps,L"",L"", vSupers,   dft,dft,dft,dft,      oParam);
    prop.AddParameter( L"bF_B",     CValue::siFloat,    sps,L"",L"", vF_B,      0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bF_C",     CValue::siFloat,    sps,L"",L"", vF_C,      0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bTau",     CValue::siFloat,    sps,L"",L"", vTau,      0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bfilter",  CValue::siInt4,     sps,L"",L"", vfilter,   0,10,0,10,      oParam ) ;

    //-- Surfaceint : bsurfaceint, eye_depth, blight_depth, blight_str, binc_env, brrstrategy, beyerrthre,
    //-- blightrrthre, bmax_depth
    prop.AddParameter( L"bsurfaceint",  CValue::siInt4, sps,L"",L"",    vSurfaceInt,    0,10,0,10,      oParam );
    prop.AddParameter( L"bsexpert",      CValue::siBool, sps,L"",L"",   vsexpert,      dft,dft,dft,dft, oParam);

    //-- bi-directional
    prop.AddParameter( L"beye_depth",   CValue::siInt4, sps,L"",L"",    vEye_depth,     0,2048,0,16,    oParam );
    prop.AddParameter( L"blight_depth", CValue::siInt4, sps,L"",L"",    vLight_depth,   0,2048,0,16,    oParam );
    prop.AddParameter( L"beyerrthre",   CValue::siFloat, sps,L"",L"",   vEyeRRthre,     0.0f,2048.0f,0.0,2048.0f,   oParam ); 
    prop.AddParameter( L"blightrrthre", CValue::siFloat, sps,L"",L"",   vLightRRthre,   0.0f,2048.0f,0.0,2048.0f,   oParam ); 
    
    //-- directlighting / path
    prop.AddParameter( L"brrcon_prob",  CValue::siFloat, sps,L"",L"", vrrcon_prob,  0.0f,10.0f,0.1f,2.0f, oParam ); 
    prop.AddParameter( L"bmaxdepth",    CValue::siInt4, sps,L"",L"",  vmaxdepth,    0,4096,0,1024,    oParam );
    prop.AddParameter( L"binc_env",     CValue::siBool, sps,L"",L"",  vInc_env,     dft,dft,dft,dft,  oParam );
    prop.AddParameter( L"brrstrategy",  CValue::siInt1, sps,L"",L"",  vRRstrategy,  0l,4l,0l,4l,      oParam );  
    prop.AddParameter( L"blight_str",   CValue::siInt4, sps,L"",L"",  vLight_str,   0,5,0,5,          oParam ); 
       
    //-- parameters distributepath
    prop.AddParameter( L"bdirectsampleall",         CValue::siBool, sps,L"",L"",    vdirectsampleall,   dft,dft,dft,dft, oParam ) ;
    prop.AddParameter( L"bdirectsamples",           CValue::siInt4, sps,L"",L"",    vdirectsamples,     0l,4l,0l,4l,     oParam ) ;
    prop.AddParameter( L"bindirectsampleall",       CValue::siBool, sps,L"",L"",    vindirectsampleall, dft,dft,dft,dft, oParam ) ;
    prop.AddParameter( L"bindirectsamples",         CValue::siInt4, sps,L"",L"",    vindirectsamples,   0,5,0,5,         oParam ) ;

    prop.AddParameter( L"bdiffusereflectdepth",     CValue::siInt4, sps,L"",L"",    vdiffusereflectdepth,   0,5,0,5,   oParam ) ;
    prop.AddParameter( L"bdiffusereflectsamples",   CValue::siInt4, sps,L"",L"",    vdiffusereflectsamples, 0,5,0,5,   oParam ) ;
    prop.AddParameter( L"bdiffuserefractdepth",     CValue::siInt4, sps,L"",L"",    vdiffuserefractdepth,   0,5,0,5,   oParam ) ;
    prop.AddParameter( L"bdiffuserefractsamples",   CValue::siInt4, sps,L"",L"",    vdiffuserefractsamples, 0,5,0,5,   oParam ) ;
    prop.AddParameter( L"bdirectdiffuse",           CValue::siBool, sps,L"",L"",    vdirectdiffuse,   dft,dft,dft,dft, oParam ) ;
    prop.AddParameter( L"bindirectdiffuse",         CValue::siBool, sps,L"",L"",    vindirectdiffuse, dft,dft,dft,dft, oParam ) ;

    prop.AddParameter( L"bglossyreflectdepth",      CValue::siInt4, sps,L"",L"",    vglossyreflectdepth,   0,10,2,5,   oParam ) ;
    prop.AddParameter( L"bglossyreflectsamples",    CValue::siInt4, sps,L"",L"",    vglossyreflectsamples, 0,10,2,5,   oParam ) ;
    prop.AddParameter( L"bglossyrefractdepth",      CValue::siInt4, sps,L"",L"",    vglossyrefractdepth,   0,10,2,5,   oParam ) ;
    prop.AddParameter( L"bglossyrefractsamples",    CValue::siInt4, sps,L"",L"",    vglossyrefractsamples, 0,10,2,5,   oParam ) ;
    prop.AddParameter( L"bdirectglossy",            CValue::siBool, sps,L"",L"",    vdirectglossy,   dft,dft,dft,dft,  oParam ) ;
    prop.AddParameter( L"bindirectglossy",          CValue::siBool, sps,L"",L"",    vindirectglossy, dft,dft,dft,dft,  oParam ) ;

    prop.AddParameter( L"bspecularreflectdepth",    CValue::siInt4, sps,L"",L"",    vspecularreflectdepth,  0,10,2,5,  oParam ) ;
    prop.AddParameter( L"bspecularrefractdepth",    CValue::siInt4, sps,L"",L"",    vspecularrefractdepth,  0,10,2,5,  oParam ) ;

    prop.AddParameter( L"bdiff_reflect_reject_thr",   CValue::siFloat, sps,L"",L"", vdiff_reflect_reject_thr,   0.0f,10.0f,0.0f,10.0f, oParam ) ; 
    prop.AddParameter( L"bdiff_refract_reject_thr",   CValue::siFloat, sps,L"",L"", vdiff_refract_reject_thr,   0.0f,10.0f,0.0f,10.0f, oParam ) ; 
    prop.AddParameter( L"bglossy_reflect_reject_thr", CValue::siFloat, sps,L"",L"", vglossy_reflect_reject_thr, 0.0f,10.0f,0.0f,10.0f, oParam ) ;
    prop.AddParameter( L"bglossy_refract_reject_thr", CValue::siFloat, sps,L"",L"", vglossy_refract_reject_thr, 0.0f,10.0f,0.0f,10.0f, oParam ) ;

    //-- igi
    prop.AddParameter( L"bnsets",    CValue::siInt4,  sps,L"",L"",    vnsets,      0,1024,4,512,  oParam ) ;
    prop.AddParameter( L"bnlights",  CValue::siInt4,  sps,L"",L"",    vnlights,    0,1024,64,512, oParam ) ;
    prop.AddParameter( L"bmindist",  CValue::siFloat, sps,L"",L"",    vmindist,    0.0,1024.0f,0.1f,1024.0,  oParam ) ;
    //--
    //-- exphotonmap
    
    prop.AddParameter( L"brenderingmode",            CValue::siInt4, sps,L"",L"", vrenderingmode,   0,10,0,10,  oParam ) ;
//  prop.AddParameter( L"bstrategy",                 CValue::siString, sps, L"", L"",
    prop.AddParameter( L"bshadowraycount",           CValue::siInt4, sps,L"",L"",  vshadowraycount,  0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxphotondepth",           CValue::siInt4, sps,L"",L"",  vmaxphotondepth,  0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxphotondist",            CValue::siFloat, sps,L"",L"", vmaxphotondist,   0.0f,1024.0f,0.0f,1024.0, oParam );
    prop.AddParameter( L"bnphotonsused",             CValue::siInt4, sps,L"",L"",  vnphotonsused,       0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bindirectphotons",          CValue::siInt4, sps,L"",L"",  vindirectphotons,    0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bdirectphotons",            CValue::siInt4, sps,L"",L"",  vdirectphotons,      0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bcausticphotons",           CValue::siInt4, sps,L"",L"",  vcausticphotons,     0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bradiancephotons",          CValue::siInt4, sps,L"",L"",  vradiancephotons,    0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bfinalgather",              CValue::siBool, sps,L"",L"",  vfinalgather,        dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bfinalgathersamples",       CValue::siInt4, sps,L"",L"",  vfinalgathersamples, 0,1024,0,1024, oParam );
    prop.AddParameter( L"bgatherangle",              CValue::siFloat, sps,L"",L"", vgatherangle,        0.0f,360.0f,0.0f,360.0, oParam );
    prop.AddParameter( L"bdistancethreshold",        CValue::siFloat, sps,L"",L"", vdistancethreshold,  0.0f,10.0f,0.0f,10.0, oParam );
    prop.AddParameter( L"bdbg_enabledirect",         CValue::siBool, sps,L"",L"",  vdbg_direct,   dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableradiancemap",    CValue::siBool, sps,L"",L"",  vdbg_radiancemap,   dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableindircaustic",   CValue::siBool, sps,L"",L"",  vdbg_indircaustic,  dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableindirdiffuse",   CValue::siBool, sps,L"",L"",  vdbg_indirdiffuse,  dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableindirspecular",  CValue::siBool, sps,L"",L"",  vdbg_indirspecular, dft,dft,dft,dft, oParam );
    
    //-- Sampler: bsampler, bmutation, bmaxrej, buservarian, bchainlength, bpixsampler, pixelsamples, vbasampler
    prop.AddParameter( L"bsampler",     CValue::siInt4,  sps,L"",L"", vSampler,         0,10,0,10,          oParam ) ;
    prop.AddParameter( L"bmutation",    CValue::siFloat, sps,L"",L"", vlmutationpro,        0.0f,1.0f,0.0f,0.5f,oParam);
    prop.AddParameter( L"bmaxrej",      CValue::siInt4,  sps,L"",L"", vmaxconsecrejects,      0l,2048l,0l,512l,   oParam);
    prop.AddParameter( L"buservarian",  CValue::siBool,  sps,L"",L"", vusevariance,      dft,dft,dft,dft,    oParam);
    prop.AddParameter( L"bchainlength", CValue::siInt4,  sps,L"",L"", vchainlength,     0,512,16,512,       oParam ) ;
    prop.AddParameter( L"bpixsampler",  CValue::siInt4,  sps,L"",L"", vPixsampler,      0,512,16,512,       oParam ) ;
    prop.AddParameter( L"pixelsamples", CValue::siInt4,  sps,L"",L"", vPixelsamples,    0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bbasampler",   CValue::siInt4,  sps,L"",L"", vbasampler,       0,10,0,10,  oParam ) ; // combo

    prop.AddParameter( L"bpresets",          CValue::siInt4, sps,L"",L"", vpresets,      0,10,0,10,      oParam ) ;

    //-- expert mode menu options
    prop.AddParameter( L"bexpert",      CValue::siBool, sps,L"",L"",  vExpert,      dft,dft,dft,dft, oParam);

    prop.AddParameter( L"savint",   CValue::siInt4, sps,L"",L"", vSave,     0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"disint",   CValue::siInt4, sps,L"",L"", vDis,      0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"hSpp",     CValue::siInt4, sps,L"",L"", vhaltspp,     0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"hTime",    CValue::siInt4, sps,L"",L"", vhalttime,    0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"AmbBack",  CValue::siBool, sps,L"",L"", vAmbBack,  dft,dft,dft,dft,    oParam);
//--- save image options
    prop.AddParameter( L"save_png_16", CValue::siBool, sps,L"",L"", vWpng_16,   dft,dft,dft,dft, oParam);
    prop.AddParameter( L"png_gamut",   CValue::siBool, sps,L"",L"", vPng_gamut, dft,dft,dft,dft, oParam);
    prop.AddParameter( L"save_png",    CValue::siBool, sps,L"",L"", vPng,       dft,dft,dft,dft, oParam);
    prop.AddParameter( L"mode_rpng",   CValue::siInt4, sps,L"",L"", vRpng,      0,10,0,10,  oParam ) ;
    //-- exr
    prop.AddParameter( L"mode_Znorm",  CValue::siInt4, sps,L"",L"", vExr_Znorm, 0,10,0,10, oParam );
    prop.AddParameter( L"save_exr",    CValue::siBool, sps,L"",L"", vExr,  dft, dft,dft,dft, oParam);
    //-- tga
    prop.AddParameter( L"mode_rtga",   CValue::siInt4, sps,L"",L"", vRtga,      0,10,0,10,       oParam ) ;
    prop.AddParameter( L"save_tga",    CValue::siBool, sps,L"",L"", vTga,       dft,dft,dft,dft, oParam);
    prop.AddParameter( L"tga_gamut",   CValue::siBool, sps,L"",L"", vTga_gamut, dft,dft,dft,dft, oParam);
    //--
    prop.AddParameter( L"brmode",   CValue::siInt4, sps,L"",L"", vRmode,        0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bengine",  CValue::siInt4, sps,L"",L"", vEngine,       0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bautode",  CValue::siBool, sps,L"",L"", vAutoTh,       dft,dft,dft,dft,    oParam);
    prop.AddParameter( L"bthreads", CValue::siInt4, sps,L"",L"", vThreads,      0,10,0,10,  oParam ) ;
    //--
    prop.AddParameter( L"bAccel",              CValue::siInt4, sps,L"",L"",  vAccel,              0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxprimsperleaf",    CValue::siInt4, sps,L"",L"",  vmaxprimsperleaf,    0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bfullsweepthreshold", CValue::siInt4, sps,L"",L"",  vfullsweepthr,       0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bskipfactor",         CValue::siInt4, sps,L"",L"",  vskipfactor,         0,10,0,10,  oParam ) ;
    prop.AddParameter( L"btreetype",           CValue::siInt4, sps,L"",L"",  vtreetype,           0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bcostsamples",        CValue::siInt4, sps,L"",L"",  vcostsamples,        0,10,0,10,  oParam ) ;
    prop.AddParameter( L"brefineimmediately",   CValue::siBool, sps,L"",L"", vrefineimmediately,  dft,dft,dft,dft, oParam);
    prop.AddParameter( L"bmaxprims",           CValue::siInt4, sps,L"",L"",  vmaxprims,           0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bacmaxdepth",         CValue::siInt4, sps,L"",L"",  vacmaxdepth,         0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bemptybonus",         CValue::siFloat, sps,L"",L"", vemptybonus,         0.0f,1.0f,0.0f,1.0f,  oParam);
    prop.AddParameter( L"bintersectcost",      CValue::siInt4, sps,L"",L"",  vintersectcost,           0,100,0,100,  oParam ) ;
    prop.AddParameter( L"btraversalcost",      CValue::siInt4, sps,L"",L"",  vtraversalcost,           0,100,0,100,  oParam ) ;
    //--
    
//  prop.AddParameter( L"mlt",      CValue::siBool, sps,L"",L"",  vMLT,     dft,dft,dft,dft,    oParam); // unused?
    prop.AddParameter( L"resume",   CValue::siBool, sps,L"",L"",  vResume,  dft,dft,dft,dft,    oParam);
 
    // set temp filename
    vFileObjects = app.GetInstallationPath(siUserPath);
    #ifdef __unix__
        vFileObjects += L"/tmp.lxs";
    #else
        vFileObjects += L"/tmp.lxs"; //-- also work in windows systems ?
    #endif

    vLuXSIPath=readIni(); // get luxrender path out of the ini

    prop.AddParameter( L"fObjects", CValue::siString, sps, L"", L"", vFileObjects, oParam ) ;

    prop.AddParameter( L"fLuxPath", CValue::siString, sps, L"", L"", vLuXSIPath, oParam ) ;

    return CStatus::OK;
}
//-- before, define_layout
XSIPLUGINCALLBACK CStatus LuXSI_PPGEvent( const CRef& in_ctxt )
{
    PPGEventContext ctxt( in_ctxt ) ;
    PPGLayout lay = Context(in_ctxt).GetSource() ;

    PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;

    if ( eventID == PPGEventContext::siOnInit )
    {
        CustomProperty prop = ctxt.GetSource() ;
        //app.LogMessage( L"OnInit called for " + prop.GetFullName() ) ;

        ctxt.PutAttribute(L"Refresh",true);

        CRefArray params = prop.GetParameters();;
        for (int i=0;i<params.GetCount();i++){
            // Update values on init
            Parameter param(params[i]);
            update_LuXSI_values(param.GetScriptName(), param, ctxt, lay);
        }
    }
    else if ( eventID == PPGEventContext::siButtonClicked )
    {
        CValue buttonPressed = ctxt.GetAttribute( L"Button" ) ;
        if (buttonPressed.GetAsText()==L"exe_luxsi"){
            luxsi_write();
        }
        if (buttonPressed.GetAsText()==L"render_luxsi"){
            luxsi_write();
            luxsi_execute();
        }
        ctxt.PutAttribute(L"Refresh",true);

        CRefArray params = prop.GetParameters();;
        for (int i=0;i<params.GetCount();i++)
        {
            // Update values on init
            Parameter param(params[i]);
            update_LuXSI_values(param.GetScriptName(),param, ctxt, lay);
        }
        //app.LogMessage( L"Button pressed: " + buttonPressed.GetAsText() ) ;

    }
    else if ( eventID == PPGEventContext::siTabChange )
    {
        CValue tabLabel = ctxt.GetAttribute( L"Tab" ) ;
        //app.LogMessage( L"Tab changed to: " + tabLabel .GetAsText() ) ;
    }
    else if ( eventID == PPGEventContext::siParameterChange )
    {
        Parameter changed = ctxt.GetSource() ;
        CustomProperty prop = changed.GetParent() ;
        CString   paramName = changed.GetScriptName() ;

        //app.LogMessage( L"Parameter Changed: " + paramName ) ;

        update_LuXSI_values(paramName, changed, ctxt, lay);
                
    }

    return CStatus::OK ;
}

//--
string replace(string input) {
    int len = input.length();

    for (int i=0;i<len;i++){
        if (input[i]=='\\') {
            input.replace(i, 1, "\\\\");
            i++;
        }
    }
    return input;
}

//--
CVector3 convertMatrix(CVector3 v){
    CMatrix3 m2(1.0,0.0,0.0,  0.0,0.0,1.0,   0.0,-1.0,0.0);
    return v.MulByMatrix3InPlace (m2);
}

//--
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt, PPGLayout lay){

    // update variables when PPG value changed
    // convention names;
        //-- prefix b; name
        //-- prefix v; variables
    //-- test for presets
    if ( changed.GetName() == L"bpresets" )
    { luxsi_render_presets( paramName, changed, ctxt); }

    //-- end test
/*
    if (paramName==L"bpresets") {
        switch ((int)changed.GetValue()) {
            case 1:
                // Preview direclighting
                //-- sampler
                Parameter(prop.GetParameters().GetItem( L"bsampler" )).PutValue( vSampler = 2 );
                Parameter(prop.GetParameters().GetItem( L"bexpert" )).PutValue( vExpert = false );
                Parameter(prop.GetParameters().GetItem( L"hSpp" )).PutValue( vhaltspp = 0 );
                Parameter(prop.GetParameters().GetItem( L"hTime" )).PutValue( vhalttime = 0 );
                Parameter(prop.GetParameters().GetItem( L"bpixsampler" )).PutValue( vPixsampler = 1 );
                Parameter(prop.GetParameters().GetItem( L"pixelsamples" )).PutValue( vPixelsamples = 2 );
                //-- surfaceintegrator
                Parameter(prop.GetParameters().GetItem( L"bsurfaceint" )).PutValue( vSurfaceInt = 2 );
                Parameter(prop.GetParameters().GetItem( L"bsexpert" )).PutValue( vsexpert = false );
                Parameter(prop.GetParameters().GetItem( L"blight_str" )).PutValue( vLight_str = 2 );
                Parameter(prop.GetParameters().GetItem( L"bmaxdepth" )).PutValue( vmaxdepth = 8 );
             
                //-- volume integrator //-- single: 1.0 // TODO;
                //-- filter
                Parameter(prop.GetParameters().GetItem( L"bfilter" )).PutValue( vfilter = 2 );
                Parameter(prop.GetParameters().GetItem( L"bfexpert" )).PutValue( vfexpert = false );
                Parameter(prop.GetParameters().GetItem( L"bxwidth" )).PutValue( vXwidth = 2.0 );
                Parameter(prop.GetParameters().GetItem( L"bywidth" )).PutValue( vYwidth = 2.0 );
                Parameter(prop.GetParameters().GetItem( L"bF_B" )).PutValue( vF_B = 0.3333f );
                Parameter(prop.GetParameters().GetItem( L"bF_C" )).PutValue( vF_C = 0.3333f );
                Parameter(prop.GetParameters().GetItem( L"ssample" )).PutValue( vSupers = true );
                //-- accelerator = 'qbvh'
                Parameter(prop.GetParameters().GetItem( L"bmaxprimsperleaf" )).PutValue( vmaxprimsperleaf = 4 );
                Parameter(prop.GetParameters().GetItem( L"bfullsweepthreshold" )).PutValue( vfullsweepthr = 16 );
                Parameter(prop.GetParameters().GetItem( L"bskipfactor" )).PutValue( vskipfactor = 1 );

                ctxt.PutArrayAttribute(L"Refresh", true);
                app.LogMessage(L" Parameters for render presets 1, loaded..");
                break;
            case 2:
                // final 1 MLT / Bidi PathT
                vSampler = 0; vmaxconsecrejects = 512; vlmutationpro = 0.6f;
                vSurfaceInt = 0; vLight_depth = 10; vEye_depth = 10; vBounces = 10;
                break;
            case 3:
                // final 2 MLT / PT (ext)
                vSampler = 1; vmaxconsecrejects = 512; vlmutationpro = 0.6f;
                vSurfaceInt = 1; vmaxdepth = 10;    vBounces = 10;
                break;
            case 4:
                // progr 1 BPT (int)
                vSampler = 2; vmaxconsecrejects = 512;
                vSurfaceInt = 2; vLight_depth = 10; vEye_depth = 10; vBounces = 10;
                vPixsampler = 2; vPixelsamples = 1;
                break;
            case 5:
                // progr 2 PT (ext)
                vSampler = 1; vmaxconsecrejects = 512;
                vSurfaceInt = 1; vBounces = 10; vmaxdepth = 10;
                vPixsampler = 2; vPixelsamples = 1;
                break;
            case 6:
                // bucket 1 BPT (int)
                vSampler = 2; vmaxconsecrejects = 512;
                vSurfaceInt =0; vLight_depth = 10; vEye_depth = 8; vBounces = 8;
                vPixsampler = 5; vPixelsamples = 64;
                break;
            case 7:
                // bucket 2 PT (ext)
                vSampler = 2; vmaxconsecrejects = 512;
                vSurfaceInt = 1; vmaxdepth = 8; vBounces = 8;
                vPixsampler = 5; vPixelsamples = 64;
                break;
        }
        ctxt.PutArrayAttribute(L"Refresh", true); // for test
    
    }
    */

    
    
    //-- image

           if (paramName == L"Width")   { vXRes     = changed.GetValue();
    } else if (paramName == L"Height")  { vYRes     = changed.GetValue();
    } else if (paramName == L"gamma")   { vContrast = changed.GetValue();
    } else if (paramName == L"disint")  { vDis      = changed.GetValue();
    } else if (paramName == L"hSpp")    { vhaltspp  = changed.GetValue();
    } else if (paramName == L"hTime")   { vhalttime = changed.GetValue();
    } else if (paramName == L"savint")  { vSave     = changed.GetValue();
    } else if (paramName == L"resume")  { vResume   = changed.GetValue();
    
    //----/ hidden objects /------>
    } else if (paramName == L"use_hidden_obj")  { vIsHiddenObj      = changed.GetValue();
    } else if (paramName == L"use_hidden_surf") { vIsHiddenSurface  = changed.GetValue();
    } else if (paramName == L"use_hidden_cloud"){ vIsHiddenClouds   = changed.GetValue();
    } else if (paramName == L"use_hidden_cam")  { vIsHiddenCam      = changed.GetValue();
    } else if (paramName == L"use_hidden_light"){ vIsHiddenLight    = changed.GetValue();

    //-- mesh export
    } else if (paramName == L"smooth_mesh")     { vSmooth_mesh  = changed.GetValue();
    } else if (paramName == L"sharp_bound")     { vSharp_bound  = changed.GetValue();
   
    //-- save images /----/ tga /--->
    } else if (paramName == L"tga_gamut")   { vTga_gamut    = changed.GetValue();
    } else if (paramName == L"mode_rtga")   { vRtga         = changed.GetValue();
    } else if (paramName == L"save_tga")    { vTga          = changed.GetValue();

    //----/ save images /----/ exr /--->
    } else if (paramName == L"mode_Znorm")  { vExr_Znorm  = changed.GetValue();
    } else if (paramName == L"save_exr")    { vExr        = changed.GetValue();

    //----/ save images /----/ png /--->
    } else if (paramName == L"mode_rpng")       { vRpng         = changed.GetValue();
    } else if (paramName == L"save_png_16")     { vWpng_16      = changed.GetValue();
    } else if (paramName == L"png_gamut")       { vPng_gamut    = changed.GetValue();
    } else if (paramName == L"save_png")        { vPng          = changed.GetValue();

    //-- save file name
    } else if (paramName == L"fObjects")        { vFileObjects  = changed.GetValue();
    
    //--TAB RENDER
    //-- Luxrender engine
    } else if (paramName == L"brmode")   { vRmode      = changed.GetValue();
    } else if (paramName == L"bengine")  { vEngine     = changed.GetValue();
    } else if (paramName == L"bautode")  { vAutoTh     = changed.GetValue();
    } else if (paramName == L"bthreads") { vThreads    = changed.GetValue();
    } else if (paramName == L"fLuxPath") { vLuXSIPath  = changed.GetValue();
        
    //-- Sampler
    } else if (paramName == L"bsampler")     { vSampler      = changed.GetValue();
    } else if (paramName == L"bexpert")      { vExpert       = changed.GetValue();
    } else if (paramName == L"bmutation")    { vlmutationpro     = changed.GetValue();
    } else if (paramName == L"bmaxrej")      { vmaxconsecrejects = changed.GetValue();
    } else if (paramName == L"buservarian")  { vusevariance   = changed.GetValue();
    } else if (paramName == L"bchainlength") { vchainlength  = changed.GetValue();
    } else if (paramName == L"bpixsampler")  { vPixsampler   = changed.GetValue();
    } else if (paramName == L"pixelsamples") { vPixelsamples = changed.GetValue();
    } else if (paramName == L"bbasampler")   { vbasampler    = changed.GetValue();

    //-- Surface integrator
    } else if (paramName == L"bsurfaceint") { vSurfaceInt   = changed.GetValue();
    } else if (paramName == L"bsexpert")    { vsexpert      = changed.GetValue();
    } else if (paramName == L"blight_depth"){ vLight_depth  = changed.GetValue();
    } else if (paramName == L"beye_depth")  { vEye_depth    = changed.GetValue();
    } else if (paramName == L"blight_str")  { vLight_str    = changed.GetValue();
    } else if (paramName == L"binc_env")    { vInc_env      = changed.GetValue();
    } else if (paramName == L"brrstrategy") { vRRstrategy   = changed.GetValue();   
    } else if (paramName == L"beyerrthre")  { vEyeRRthre    = changed.GetValue();
    } else if (paramName == L"blightrrthre"){ vLightRRthre  = changed.GetValue();
    } else if (paramName == L"brrcon_prob") { vrrcon_prob   = changed.GetValue();

    
    //-- distributepath
    } else if (paramName == L"bdirectsampleall")        { vdirectsampleall        = changed.GetValue();
    } else if (paramName == L"bdirectsamples")          { vdirectsamples          = changed.GetValue();
    } else if (paramName == L"bindirectsampleall")      { vindirectsampleall      = changed.GetValue();
    } else if (paramName == L"bindirectsamples")        { vindirectsamples        = changed.GetValue();
    
    } else if (paramName == L"bdiffusereflectdepth")    { vdiffusereflectdepth    = changed.GetValue();
    } else if (paramName == L"bdiffusereflectsamples")  { vdiffusereflectsamples  = changed.GetValue();
    } else if (paramName == L"bdiffuserefractdepth")    { vdiffuserefractdepth    = changed.GetValue();
    } else if (paramName == L"bdiffuserefractsamples")  { vdiffuserefractsamples  = changed.GetValue();
    } else if (paramName == L"bdirectdiffuse")          { vdirectdiffuse          = changed.GetValue();
    } else if (paramName == L"bindirectdiffuse")        { vindirectdiffuse        = changed.GetValue();
    
    } else if (paramName == L"bglossyreflectdepth")     { vglossyreflectdepth     = changed.GetValue();
    } else if (paramName == L"bglossyreflectsamples")   { vglossyreflectsamples   = changed.GetValue();
    } else if (paramName == L"bglossyrefractdepth")     { vglossyrefractdepth     = changed.GetValue();
    } else if (paramName == L"bglossyrefractsamples")   { vglossyrefractsamples   = changed.GetValue();
    } else if (paramName == L"bdirectglossy")           { vdirectglossy           = changed.GetValue();
    } else if (paramName == L"bindirectglossy")         { vindirectglossy         = changed.GetValue();
    
    } else if (paramName == L"bspecularreflectdepth")   { vspecularreflectdepth   = changed.GetValue();
    } else if (paramName == L"bspecularrefractdepth")   { vspecularrefractdepth   = changed.GetValue();
  
    } else if (paramName == L"bdiff_reflect_reject_thr")   { vdiff_reflect_reject_thr   = changed.GetValue(); 
    } else if (paramName == L"bdiff_refract_reject_thr")   { vdiff_refract_reject_thr   = changed.GetValue(); 
    } else if (paramName == L"bglossy_reflect_reject_thr") { vglossy_reflect_reject_thr = changed.GetValue();
    } else if (paramName == L"bglossy_refract_reject_thr") { vglossy_refract_reject_thr = changed.GetValue();


    //-- igi
    } else if (paramName == L"bnsets")      { vnsets     = changed.GetValue();
    } else if (paramName == L"bnlights")    { vnlights   = changed.GetValue();
    } else if (paramName == L"bmindist")    { vmindist   = changed.GetValue();
    
    //-- exphotonmap
    } else if (paramName == L"brenderingmode")      { vrenderingmode       = changed.GetValue();
//  } else if (paramName == L"bstrategy")           { vstrategy            = changed.GetValue();
    } else if (paramName == L"bshadowraycount")     { vshadowraycount      = changed.GetValue();
    } else if (paramName == L"bmaxphotondepth")     { vmaxphotondepth      = changed.GetValue();
    } else if (paramName == L"bmaxdepth")           { vmaxdepth            = changed.GetValue();
    } else if (paramName == L"bmaxphotondist")      { vmaxphotondist       = changed.GetValue();
    } else if (paramName == L"bnphotonsused")       { vnphotonsused        = changed.GetValue();
    } else if (paramName == L"bindirectphotons")    { vindirectphotons     = changed.GetValue();
    } else if (paramName == L"bdirectphotons")      { vdirectphotons       = changed.GetValue();
    } else if (paramName == L"bcausticphotons")     { vcausticphotons      = changed.GetValue();
    } else if (paramName == L"bradiancephotons")    { vradiancephotons     = changed.GetValue();
    } else if (paramName == L"bfinalgather")        { vfinalgather         = changed.GetValue();
    } else if (paramName == L"bfinalgathersamples") { vfinalgathersamples  = changed.GetValue();
    } else if (paramName == L"bgatherangle")        { vgatherangle         = changed.GetValue();
    } else if (paramName == L"bdistancethreshold")  { vdistancethreshold   = changed.GetValue();
    } else if (paramName == L"bdbg_enabledirect")     { vdbg_direct        = changed.GetValue();
    } else if (paramName == L"bdbg_enableradiancemap")  { vdbg_radiancemap    = changed.GetValue();
    } else if (paramName == L"bdbg_enableindircaustic")  { vdbg_indircaustic   = changed.GetValue();
    } else if (paramName == L"bdbg_enableindirdiffuse")  { vdbg_indirdiffuse   = changed.GetValue();
    } else if (paramName == L"bdbg_enableindirspecular") { vdbg_indirspecular  = changed.GetValue();
    
    //-- filter
    } else if (paramName == L"bfexpert"){ vfexpert  = changed.GetValue();
    } else if (paramName == L"bxwidth") { vXwidth   = changed.GetValue();
    } else if (paramName == L"bywidth") { vYwidth   = changed.GetValue();
    } else if (paramName == L"bfalpha") { vFalpha   = changed.GetValue();
    } else if (paramName == L"ssample") { vSupers   = changed.GetValue();
    } else if (paramName == L"bF_B")    { vF_B      = changed.GetValue();
    } else if (paramName == L"bF_C")    { vF_C      = changed.GetValue();
    } else if (paramName == L"bTau")    { vTau      = changed.GetValue();
    
    //-- Accelerator
    } else if (paramName == L"bAccel")              { vAccel              = changed.GetValue();
    } else if (paramName == L"bmaxprimsperleaf")    { vmaxprimsperleaf    = changed.GetValue();
    } else if (paramName == L"bfullsweepthreshold") { vfullsweepthr       = changed.GetValue();
    } else if (paramName == L"bskipfactor")         { vskipfactor         = changed.GetValue();
    } else if (paramName == L"btreetype")           { vtreetype           = changed.GetValue();
    } else if (paramName == L"bcostsamples")        { vcostsamples        = changed.GetValue();
    } else if (paramName == L"brefineimmediately")  { vrefineimmediately  = changed.GetValue();
    } else if (paramName == L"bintersectcost")      { vintersectcost      = changed.GetValue();
    } else if (paramName == L"btraversalcost")      { vtraversalcost      = changed.GetValue();
    } else if (paramName == L"bmaxprims")           { vmaxprims           = changed.GetValue();
    } else if (paramName == L"bacmaxdepth")         { vacmaxdepth         = changed.GetValue();
    } else if (paramName == L"bemptybonus")         { vemptybonus         = changed.GetValue();
    
    
    //--
    } else {
        app.LogMessage(L"End update values.");
    }
       dynamic_luxsi_UI(changed, ctxt);
}
//--
void luxsi_render_presets( CString paramName, Parameter changed, PPGEventContext ctxt)
{
    //-- options
    if ( changed.GetName() == L"bpresets" ) 
    {
        vpresets = prop.GetParameterValue( L"bpresets" );
        //--
        if ( vpresets == 0 ) // Custom parameters
        {
            //-- TODO;
        }
        else if ( vpresets == 1 ) // Preview direclighting
        {   //-- sampler
            Parameter(prop.GetParameters().GetItem( L"bsampler" )).PutValue( vSampler = 2 );
            Parameter(prop.GetParameters().GetItem( L"bexpert" )).PutValue( vExpert = false );
            Parameter(prop.GetParameters().GetItem( L"hSpp" )).PutValue( vhaltspp = 0 );
            Parameter(prop.GetParameters().GetItem( L"hTime" )).PutValue( vhalttime = 0 );
            Parameter(prop.GetParameters().GetItem( L"bpixsampler" )).PutValue( vPixsampler = 1 );
            Parameter(prop.GetParameters().GetItem( L"pixelsamples" )).PutValue( vPixelsamples = 2 );
            //-- surfaceintegrator
            Parameter(prop.GetParameters().GetItem( L"bsurfaceint" )).PutValue( vSurfaceInt = 2 );
            Parameter(prop.GetParameters().GetItem( L"bsexpert" )).PutValue( vsexpert = false );
            Parameter(prop.GetParameters().GetItem( L"blight_str" )).PutValue( vLight_str = 2 );
            Parameter(prop.GetParameters().GetItem( L"bmaxdepth" )).PutValue( vmaxdepth = 8 );
            //-- volume integrator //-- single: 1.0 // TODO;
            //-- filter
            Parameter(prop.GetParameters().GetItem( L"bfilter" )).PutValue( vfilter = 2 );
            Parameter(prop.GetParameters().GetItem( L"bfexpert" )).PutValue( vfexpert = false );
            Parameter(prop.GetParameters().GetItem( L"bxwidth" )).PutValue( vXwidth = 2.0 );
            Parameter(prop.GetParameters().GetItem( L"bywidth" )).PutValue( vYwidth = 2.0 );
            Parameter(prop.GetParameters().GetItem( L"bF_B" )).PutValue( vF_B = 0.3333f );
            Parameter(prop.GetParameters().GetItem( L"bF_C" )).PutValue( vF_C = 0.3333f );
            Parameter(prop.GetParameters().GetItem( L"ssample" )).PutValue( vSupers = true );
            //-- accelerator = 'qbvh'
            Parameter(prop.GetParameters().GetItem( L"bmaxprimsperleaf" )).PutValue( vmaxprimsperleaf = 4 );
            Parameter(prop.GetParameters().GetItem( L"bfullsweepthreshold" )).PutValue( vfullsweepthr = 16 );
            Parameter(prop.GetParameters().GetItem( L"bskipfactor" )).PutValue( vskipfactor = 1 );
            //--
            app.LogMessage(L" Parameters for render presets 1, loaded..");
        }
        else if ( vpresets == 2 ) // final 1 MLT / Bidi PathT
        {
            vSampler = 0; vmaxconsecrejects = 512; vlmutationpro = 0.6f;
            vSurfaceInt = 0; vLight_depth = 10; vEye_depth = 10; vBounces = 10;
            //--
            app.LogMessage(L" Parameters for render presets 2, loaded..");
        }
        else if ( vpresets == 3 ) // final 2 MLT / PT (ext)
        {
            vSampler = 1; vmaxconsecrejects = 512; vlmutationpro = 0.6f;
            vSurfaceInt = 1; vmaxdepth = 10;    vBounces = 10;
            //--
            app.LogMessage(L" Parameters for render presets 3, loaded..");
        }
        else if ( vpresets == 4 ) // progr 1 BPT (int)
        {
            vSampler = 2; vmaxconsecrejects = 512;
            vSurfaceInt = 2; vLight_depth = 10; vEye_depth = 10; vBounces = 10;
            vPixsampler = 2; vPixelsamples = 1;
            //--
            app.LogMessage(L" Parameters for render presets 4; loaded..");
        }
        else if ( vpresets == 5 ) //  progr 2 PT (ext)
        {
            vSampler = 1; vmaxconsecrejects = 512;
            vSurfaceInt = 1; vBounces = 10; vmaxdepth = 10;
            vPixsampler = 2; vPixelsamples = 1;
            //--
            app.LogMessage(L" Parameters for render presets 5; loaded..");
        }
        else if ( vpresets == 6 )// bucket 1 BPT (int
        {
            vSampler = 2; vmaxconsecrejects = 512;
            vSurfaceInt =0; vLight_depth = 10; vEye_depth = 8; vBounces = 8;
            vPixsampler = 5; vPixelsamples = 64;
            //--
            app.LogMessage(L" Parameters for render presets 6; loaded..");
        }
        else // bucket 2 PT (ext)
        {
            vSampler = 2; vmaxconsecrejects = 512;
            vSurfaceInt = 1; vmaxdepth = 8; vBounces = 8;
            vPixsampler = 5; vPixelsamples = 64;
            //--
            app.LogMessage(L" Parameters for render presets 7; loaded..");
        }
        //--
        ctxt.PutArrayAttribute(L"Refresh", true); // for test
    
    } //-- end cases...
}
//--
void dynamic_luxsi_UI( Parameter changed, PPGEventContext ctxt) 
{
    //-- load parameters data
    
    PPGLayout lay = ctxt.GetSource() ;

    //-- convention names;
    //-- prefix l; for Parameter ( logic )

        //------------------------------------//
        if ( changed.GetName() == L"save_tga")
        //------------------------------------//
        {
            Parameter lmode_rtga = prop.GetParameters().GetItem( L"mode_rtga" );
            Parameter ltga_gamut = prop.GetParameters().GetItem( L"tga_gamut" );
            //--
            lmode_rtga.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false );
            ltga_gamut.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false );
                ctxt.PutAttribute(L"Refresh", true);
        }
        //------------------------------------//
        if ( changed.GetName() == L"save_exr")
        //------------------------------------//
        {
            Parameter lmode_Znorm   = prop.GetParameters().GetItem( L"mode_Znorm" );
            //--
            lmode_Znorm.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false );
                ctxt.PutAttribute(L"Refresh", true);
        }
        //------------------------------------//
        if ( changed.GetName() == L"save_png")
        //------------------------------------//
        {
            Parameter lmode_rpng    = prop.GetParameters().GetItem( L"mode_rpng" ) ;
            Parameter lsave_png_16  = prop.GetParameters().GetItem( L"save_png_16" ) ;
            Parameter lpng_gamut    = prop.GetParameters().GetItem( L"png_gamut" ) ;
            //--
            lmode_rpng.PutCapabilityFlag( siNotInspectable,     changed.GetValue() == false ) ;
            lsave_png_16.PutCapabilityFlag( siNotInspectable,   changed.GetValue() == false ) ;
            lpng_gamut.PutCapabilityFlag( siNotInspectable,     changed.GetValue() == false ) ;
                ctxt.PutAttribute(L"Refresh", true);
        }

    //-- for all events -------------------------//
    if (( changed.GetName() == L"bsampler") ||
         ( changed.GetName() == L"bexpert") ||
          ( changed.GetName() == L"bbasampler"))
    //-------------------------------------------//
    {
        vSampler = prop.GetParameterValue(L"bsampler");
        vExpert = prop.GetParameterValue(L"bexpert");
        vbasampler = prop.GetParameterValue(L"bbasampler");

        //-- create list of parameters
        Parameter lmutation     = prop.GetParameters().GetItem( L"bmutation" );
        Parameter lmaxrej       = prop.GetParameters().GetItem( L"bmaxrej" );
        Parameter lbuservarian  = prop.GetParameters().GetItem( L"buservarian" );
        Parameter lchainlength  = prop.GetParameters().GetItem( L"bchainlength" );
        Parameter lbpixsampler  = prop.GetParameters().GetItem( L"bpixsampler" );
        Parameter lpixelsamples = prop.GetParameters().GetItem( L"pixelsamples" );//--
        Parameter lbbasampler   = prop.GetParameters().GetItem( L"bbasampler" );

        //-- assing flags for hidden options
        lmutation.PutCapabilityFlag( siNotInspectable, true );
        lmaxrej.PutCapabilityFlag( siNotInspectable, true );
        lbuservarian.PutCapabilityFlag( siNotInspectable, true );
        lchainlength.PutCapabilityFlag( siNotInspectable, true );
        lbpixsampler.PutCapabilityFlag( siNotInspectable, true );
        lpixelsamples.PutCapabilityFlag( siNotInspectable, true );
        lbbasampler.PutCapabilityFlag( siNotInspectable, true );
        //--

        //-- create dynamic UI
        if ( vSampler == 0)  //-- metropolis
        {
            lmutation.PutCapabilityFlag( siNotInspectable, false );
            if (vExpert)
            {
                lmaxrej.PutCapabilityFlag( siNotInspectable, false );
            }
            lbuservarian.PutCapabilityFlag( siNotInspectable, false );
                ctxt.PutAttribute(L"Refresh", true);
        }
        else if ( vSampler == 1 ) //-- erpt
        {
            lchainlength.PutCapabilityFlag( siNotInspectable, false );
            lbbasampler.PutCapabilityFlag( siNotInspectable, false );
            if ( vbasampler > 0 ) //-- low / random
            {
                lbpixsampler.PutCapabilityFlag( siNotInspectable, false );
                lpixelsamples.PutCapabilityFlag( siNotInspectable, false );
            }
            else //-- metropolis
            {
                lmutation.PutCapabilityFlag( siNotInspectable, false );
                lbuservarian.PutCapabilityFlag( siNotInspectable, false );
                if ( vExpert )
                {
                    lmaxrej.PutCapabilityFlag( siNotInspectable, false );
                }
            }
                ctxt.PutAttribute(L"Refresh", true);
        }
        else  // vSampler is 2 or 3; lowdiscrepance / random
        {
            lbpixsampler.PutCapabilityFlag( siNotInspectable, false );
            lpixelsamples.PutCapabilityFlag( siNotInspectable, false );
                ctxt.PutAttribute(L"Refresh", true);
        }
    }

    //--------------------------------//
    if ( changed.GetName() == L"bfilter")
    //--------------------------------//
    {
        vfilter = prop.GetParameterValue(L"bfilter");
        vExpert = prop.GetParameterValue(L"bexpert");

        //-- create list of parameters
        Parameter lbxwidth  = prop.GetParameters().GetItem( L"bxwidth" );
        Parameter lbywidth  = prop.GetParameters().GetItem( L"bywidth" );
        Parameter lbfalpha  = prop.GetParameters().GetItem( L"bfalpha" );
        Parameter lssample  = prop.GetParameters().GetItem( L"ssample" );
        Parameter lbF_B     = prop.GetParameters().GetItem( L"bF_B" );
        Parameter lbF_C     = prop.GetParameters().GetItem( L"bF_C" );
        Parameter lbTau     = prop.GetParameters().GetItem( L"bTau" );

        //-- assing flags for hidden
        lbfalpha.PutCapabilityFlag( siNotInspectable, true );
        lssample.PutCapabilityFlag( siNotInspectable, true );
        lbF_B.PutCapabilityFlag( siNotInspectable, true );
        lbF_C.PutCapabilityFlag( siNotInspectable, true );
        lbTau.PutCapabilityFlag( siNotInspectable, true );
        lbxwidth.PutCapabilityFlag( siNotInspectable, true );
        lbywidth.PutCapabilityFlag( siNotInspectable, true );

        //-- show with all options, if mode expert is true
        if ( vExpert )
        {
            lbxwidth.PutCapabilityFlag( siNotInspectable, false );
            lbywidth.PutCapabilityFlag( siNotInspectable, false );
            ctxt.PutAttribute(L"Refresh", true);
        }

        if (( vfilter == 1 ) && ( vExpert )) //-- gauss
        {
            lbfalpha.PutCapabilityFlag( siNotInspectable, false );
                ctxt.PutAttribute(L"Refresh", true);
        }
        else if (( vfilter == 2 ) && ( vExpert )) //-- mitchell
        {
            lssample.PutCapabilityFlag( siNotInspectable, false );
            lbF_B.PutCapabilityFlag( siNotInspectable, false );
            lbF_C.PutCapabilityFlag( siNotInspectable, false );
                ctxt.PutAttribute(L"Refresh", true);
        }
        else if (( vfilter == 3 ) && ( vExpert )) //-- sinc
        {
            lbTau.PutCapabilityFlag( siNotInspectable, false );
                ctxt.PutAttribute(L"Refresh", true);
        }
        else //-- box
        {
        lay.PutAttribute(L"Refresh", true); //-- lay, only refresh layout
        }
    }

    //----------------------------------------//
    if (( changed.GetName() == L"bsurfaceint" ) ||
        ( changed.GetName() == L"bfinalgather" ) ||
        ( changed.GetName() == L"bsexpert" ))
    //----------------------------------------//
    {
        vSurfaceInt  = prop.GetParameterValue(L"bsurfaceint");
        vsexpert = prop.GetParameterValue(L"bsexpert");

        //-- bidirectional
        Parameter lblight_depth = prop.GetParameters().GetItem( L"blight_depth" );
        Parameter lbeye_depth   = prop.GetParameters().GetItem( L"beye_depth" );
        Parameter lbeyerrthre   = prop.GetParameters().GetItem( L"beyerrthre" );
        Parameter lblightrrthre = prop.GetParameters().GetItem( L"blightrrthre" );
        //-- path
        Parameter lblight_str   = prop.GetParameters().GetItem( L"blight_str" );
        Parameter lbinc_env     = prop.GetParameters().GetItem( L"binc_env" );
        Parameter lbrrstrategy  = prop.GetParameters().GetItem( L"brrstrategy" );
        Parameter lbmaxdepth    = prop.GetParameters().GetItem( L"bmaxdepth" );
        Parameter lbrrcon_prob  = prop.GetParameters().GetItem( L"brrcon_prob" );
        //-- ditributepath
        Parameter lbdirectsampleall       = prop.GetParameters().GetItem( L"bdirectsampleall");
        Parameter lbdirectsamples         = prop.GetParameters().GetItem( L"bdirectsamples");
        Parameter lbindirectsampleall     = prop.GetParameters().GetItem( L"bindirectsampleall");
        Parameter lbindirectsamples       = prop.GetParameters().GetItem( L"bindirectsamples");
        Parameter lbdiffusereflectdepth   = prop.GetParameters().GetItem( L"bdiffusereflectdepth");
        Parameter lbdiffusereflectsamples = prop.GetParameters().GetItem( L"bdiffusereflectsamples");
        Parameter lbdiffuserefractdepth   = prop.GetParameters().GetItem( L"bdiffuserefractdepth");
        Parameter lbdiffuserefractsamples = prop.GetParameters().GetItem( L"bdiffuserefractsamples");
        Parameter lbdirectdiffuse         = prop.GetParameters().GetItem( L"bdirectdiffuse");
        Parameter lbindirectdiffuse       = prop.GetParameters().GetItem( L"bindirectdiffuse");
        Parameter lbglossyreflectdepth    = prop.GetParameters().GetItem( L"bglossyreflectdepth");
        Parameter lbglossyreflectsamples  = prop.GetParameters().GetItem( L"bglossyreflectsamples");
        Parameter lbglossyrefractdepth    = prop.GetParameters().GetItem( L"bglossyrefractdepth");
        Parameter lbglossyrefractsamples  = prop.GetParameters().GetItem( L"bglossyrefractsamples");
        Parameter lbdirectglossy          = prop.GetParameters().GetItem( L"bdirectglossy");
        Parameter lbindirectglossy        = prop.GetParameters().GetItem( L"bindirectglossy");
        Parameter lbspecularreflectdepth  = prop.GetParameters().GetItem( L"bspecularreflectdepth");
        Parameter lbspecularrefractdepth  = prop.GetParameters().GetItem( L"bspecularrefractdepth");
        Parameter lbdiff_reflect_reject_thr = prop.GetParameters().GetItem( L"bdiff_reflect_reject_thr");
        Parameter lbdiff_refract_reject_thr = prop.GetParameters().GetItem( L"bdiff_refract_reject_thr");
        Parameter lbglossy_reflect_reject_thr = prop.GetParameters().GetItem( L"bglossy_reflect_reject_thr");
        Parameter lbglossy_refract_reject_thr = prop.GetParameters().GetItem( L"bglossy_refract_reject_thr");

        
        //-- IGI
        Parameter lbnsets       = prop.GetParameters().GetItem( L"bnsets" );
        Parameter lbnlights     = prop.GetParameters().GetItem( L"bnlights" );
        Parameter lbmindist     = prop.GetParameters().GetItem( L"bmindist" );
   
        //-- exphotonmap    
        Parameter lbstrategy                = prop.GetParameters().GetItem( L"bstrategy" );
        Parameter lbshadowraycount          = prop.GetParameters().GetItem( L"bshadowraycount" );
        Parameter lbmaxphotondepth          = prop.GetParameters().GetItem( L"bmaxphotondepth" );
        Parameter lbmaxphotondist           = prop.GetParameters().GetItem( L"bmaxphotondist" );
        Parameter lbnphotonsused            = prop.GetParameters().GetItem( L"bnphotonsused" );
        Parameter lbindirectphotons         = prop.GetParameters().GetItem( L"bindirectphotons" );
        Parameter lbdirectphotons           = prop.GetParameters().GetItem( L"bdirectphotons" );
        Parameter lbcausticphotons          = prop.GetParameters().GetItem( L"bcausticphotons" );
        Parameter lbradiancephotons         = prop.GetParameters().GetItem( L"bradiancephotons" );
        Parameter lbfinalgather             = prop.GetParameters().GetItem( L"bfinalgather" );
        Parameter lbrenderingmode           = prop.GetParameters().GetItem( L"brenderingmode" );
        Parameter lbfinalgathersamples      = prop.GetParameters().GetItem( L"bfinalgathersamples" );
        Parameter lbgatherangle             = prop.GetParameters().GetItem( L"bgatherangle" );
        Parameter lbdistancethreshold       = prop.GetParameters().GetItem( L"bdistancethreshold" );
        Parameter lbdbg_enabledirect        = prop.GetParameters().GetItem( L"bdbg_enabledirect" );
        Parameter lbdbg_enableradiancemap   = prop.GetParameters().GetItem( L"bdbg_enableradiancemap" );
        Parameter lbdbg_enableindircaustic  = prop.GetParameters().GetItem( L"bdbg_enableindircaustic" );
        Parameter lbdbg_enableindirdiffuse  = prop.GetParameters().GetItem( L"bdbg_enableindirdiffuse" );
        Parameter lbdbg_enableindirspecular = prop.GetParameters().GetItem( L"bdbg_enableindirspecular" );

    //--flags
   
        //-- bidi
        lblight_depth.PutCapabilityFlag( siNotInspectable, true );
        lbeye_depth.PutCapabilityFlag( siNotInspectable, true );
        lbeyerrthre.PutCapabilityFlag( siNotInspectable, true );
        lblightrrthre.PutCapabilityFlag( siNotInspectable, true );
        
        
        //-- path
        lblight_str.PutCapabilityFlag( siNotInspectable, true );
        lbrrstrategy.PutCapabilityFlag( siNotInspectable, true );
        lbinc_env.PutCapabilityFlag( siNotInspectable, true );
        lbmaxdepth.PutCapabilityFlag( siNotInspectable, true );
        lbrrcon_prob.PutCapabilityFlag( siNotInspectable, true );
        
        //-- distrubute
        lbdirectsampleall.PutCapabilityFlag( siNotInspectable, true );
        lbdirectsamples.PutCapabilityFlag( siNotInspectable, true );
        lbindirectsampleall.PutCapabilityFlag( siNotInspectable, true );
        lbindirectsamples.PutCapabilityFlag( siNotInspectable, true );
        lbdiffusereflectdepth.PutCapabilityFlag( siNotInspectable, true );   
        lbdiffusereflectsamples.PutCapabilityFlag( siNotInspectable, true );
        lbdiffuserefractdepth.PutCapabilityFlag( siNotInspectable, true );
        lbdiffuserefractsamples.PutCapabilityFlag( siNotInspectable, true ); 
        lbdirectdiffuse.PutCapabilityFlag( siNotInspectable, true );         
        lbindirectdiffuse.PutCapabilityFlag( siNotInspectable, true );      
        lbglossyreflectdepth.PutCapabilityFlag( siNotInspectable, true );   
        lbglossyreflectsamples.PutCapabilityFlag( siNotInspectable, true );  
        lbglossyrefractdepth.PutCapabilityFlag( siNotInspectable, true );    
        lbglossyrefractsamples.PutCapabilityFlag( siNotInspectable, true );  
        lbdirectglossy.PutCapabilityFlag( siNotInspectable, true );          
        lbindirectglossy.PutCapabilityFlag( siNotInspectable, true );        
        lbspecularreflectdepth.PutCapabilityFlag( siNotInspectable, true );  
        lbspecularrefractdepth.PutCapabilityFlag( siNotInspectable, true );  
        lbdiff_reflect_reject_thr.PutCapabilityFlag( siNotInspectable, true );
        lbdiff_refract_reject_thr.PutCapabilityFlag( siNotInspectable, true );
        lbglossy_reflect_reject_thr.PutCapabilityFlag( siNotInspectable, true );
        lbglossy_refract_reject_thr.PutCapabilityFlag( siNotInspectable, true );

        //-- IGI 
        lbnsets.PutCapabilityFlag( siNotInspectable, true );
        lbnlights.PutCapabilityFlag( siNotInspectable, true );
        lbmindist.PutCapabilityFlag( siNotInspectable, true );

        //-- exphotonmap
        lbrenderingmode.PutCapabilityFlag( siNotInspectable, true );
        lbstrategy.PutCapabilityFlag( siNotInspectable, true );
        lbshadowraycount.PutCapabilityFlag( siNotInspectable, true ); 
        lbmaxphotondepth.PutCapabilityFlag( siNotInspectable, true );
        lbmaxphotondist.PutCapabilityFlag( siNotInspectable, true );
        lbnphotonsused.PutCapabilityFlag( siNotInspectable, true );
        lbindirectphotons.PutCapabilityFlag( siNotInspectable, true ); 
        lbdirectphotons.PutCapabilityFlag( siNotInspectable, true );
        lbcausticphotons.PutCapabilityFlag( siNotInspectable, true ); 
        lbradiancephotons.PutCapabilityFlag( siNotInspectable, true ); 
        lbfinalgather.PutCapabilityFlag( siNotInspectable, true ); 
        lbdistancethreshold.PutCapabilityFlag( siNotInspectable, true ); 
        lbfinalgathersamples.PutCapabilityFlag( siNotInspectable, true );
        lbgatherangle.PutCapabilityFlag( siNotInspectable, true );
        lbdbg_enabledirect.PutCapabilityFlag( siNotInspectable, true ); 
        lbdbg_enableradiancemap.PutCapabilityFlag( siNotInspectable, true ); 
        lbdbg_enableindircaustic.PutCapabilityFlag( siNotInspectable, true );
        lbdbg_enableindirdiffuse.PutCapabilityFlag( siNotInspectable, true ); 
        lbdbg_enableindirspecular.PutCapabilityFlag( siNotInspectable, true ); 

        if ( vSurfaceInt == 0 ) //-- bidirectional
        {
            lblight_depth.PutCapabilityFlag( siNotInspectable, false );
            lbeye_depth.PutCapabilityFlag( siNotInspectable, false );
            if ( vsexpert )
            {
                lbeyerrthre.PutCapabilityFlag( siNotInspectable, false );
                lblightrrthre.PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 1 ) //-- path
        {
            lbmaxdepth.PutCapabilityFlag( siNotInspectable, false );
            lbinc_env.PutCapabilityFlag( siNotInspectable, false );
            lbrrstrategy.PutCapabilityFlag( siNotInspectable, false );
            lbrrcon_prob.PutCapabilityFlag( siNotInspectable, false );
            if ( vsexpert )
            {
                lblight_str.PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 2 ) //-- directlighting
        {
            lbmaxdepth.PutCapabilityFlag( siNotInspectable, false );
            if ( vsexpert )
            {
                lblight_str.PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 3 ) //-- distributepath
        {
            lbdirectsampleall.PutCapabilityFlag( siNotInspectable, false );
            lbdirectsamples.PutCapabilityFlag( siNotInspectable, false );
            lbindirectsampleall.PutCapabilityFlag( siNotInspectable, false );
            lbindirectsamples.PutCapabilityFlag( siNotInspectable, false );
            lbdiffusereflectdepth.PutCapabilityFlag( siNotInspectable, false );   
            lbdiffusereflectsamples.PutCapabilityFlag( siNotInspectable, false );
            lbdiffuserefractdepth.PutCapabilityFlag( siNotInspectable, false );
            lbdiffuserefractsamples.PutCapabilityFlag( siNotInspectable, false ); 
            lbdirectdiffuse.PutCapabilityFlag( siNotInspectable, false );         
            lbindirectdiffuse.PutCapabilityFlag( siNotInspectable, false );      
            lbglossyreflectdepth.PutCapabilityFlag( siNotInspectable, false );   
            lbglossyreflectsamples.PutCapabilityFlag( siNotInspectable, false );  
            lbglossyrefractdepth.PutCapabilityFlag( siNotInspectable, false );    
            lbglossyrefractsamples.PutCapabilityFlag( siNotInspectable, false );  
            lbdirectglossy.PutCapabilityFlag( siNotInspectable, false );          
            lbindirectglossy.PutCapabilityFlag( siNotInspectable, false );        
            lbspecularreflectdepth.PutCapabilityFlag( siNotInspectable, false );  
            lbspecularrefractdepth.PutCapabilityFlag( siNotInspectable, false );
            lbdiff_reflect_reject_thr.PutCapabilityFlag( siNotInspectable, false );
            lbdiff_refract_reject_thr.PutCapabilityFlag( siNotInspectable, false );
            lbglossy_reflect_reject_thr.PutCapabilityFlag( siNotInspectable, false );
            lbglossy_refract_reject_thr.PutCapabilityFlag( siNotInspectable, false );

            //--
            if ( vsexpert )
            {
                lblight_str.PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 4 ) //-- IGI
        {
            lbnsets.PutCapabilityFlag( siNotInspectable, false );
            lbnlights.PutCapabilityFlag( siNotInspectable, false );
            lbmaxdepth.PutCapabilityFlag( siNotInspectable, false );
            lbmindist.PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                lblight_str.PutCapabilityFlag( siNotInspectable, false );
            }
             ctxt.PutAttribute(L"Refresh", true );
        }
        else
        {
        //    lbstrategy.PutCapabilityFlag( siNotInspectable, true );
        //    lbshadowraycount.PutCapabilityFlag( siNotInspectable, true ); 
            
            lbmaxdepth.PutCapabilityFlag( siNotInspectable, false ); 
            lbmaxphotondepth.PutCapabilityFlag( siNotInspectable, false );
            lbdirectphotons.PutCapabilityFlag( siNotInspectable, false );
            lbcausticphotons.PutCapabilityFlag( siNotInspectable, false ); 
            lbindirectphotons.PutCapabilityFlag( siNotInspectable, false );
            lbradiancephotons.PutCapabilityFlag( siNotInspectable, false ); 
            lbnphotonsused.PutCapabilityFlag( siNotInspectable, false );
            lbmaxphotondist.PutCapabilityFlag( siNotInspectable, false );
            lbfinalgather.PutCapabilityFlag( siNotInspectable, false ); 
            //--
            if ( vfinalgather )
            {
                lbfinalgathersamples.PutCapabilityFlag( siNotInspectable, false );
                lbgatherangle.PutCapabilityFlag( siNotInspectable, false );
             }
            lbrenderingmode.PutCapabilityFlag( siNotInspectable, false ); //-- combo
            lbrrstrategy.PutCapabilityFlag( siNotInspectable, false );
            lbrrcon_prob.PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                lblight_str.PutCapabilityFlag( siNotInspectable, false );
                lbdistancethreshold.PutCapabilityFlag( siNotInspectable, false ); 
                lbdbg_enabledirect.PutCapabilityFlag( siNotInspectable, false ); 
                lbdbg_enableradiancemap.PutCapabilityFlag( siNotInspectable, false ); 
                lbdbg_enableindircaustic.PutCapabilityFlag( siNotInspectable, false );
                lbdbg_enableindirdiffuse.PutCapabilityFlag( siNotInspectable, false ); 
                lbdbg_enableindirspecular.PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        // Parameter lbdbg_enableindirspecular = prop.GetParameters().GetItem( L"bdbg_enableindirspecular" );


    } // end surface int
    //-- Accelerator
    if (( changed.GetName() == L"bAccel" )||
        ( changed.GetName() == L"bsexpert" )) //-- change to dedicated item (bAcexpert)
    {
        vAccel = prop.GetParameterValue(L"bAccel");
        vsexpert = prop.GetParameterValue(L"bsexpert"); //-- provisional
        //--
        Parameter lbmaxprimsperleaf= prop.GetParameters().GetItem( L"bmaxprimsperleaf");
        Parameter lbfullsweepthreshold= prop.GetParameters().GetItem( L"bfullsweepthreshold"); 
        Parameter lbskipfactor= prop.GetParameters().GetItem( L"bskipfactor");
       // Parameter lbtreetype= prop.GetParameters().GetItem( L"btreetype");           
        Parameter lbcostsamples= prop.GetParameters().GetItem( L"bcostsamples");        
       // Parameter lbrefineimmediately= prop.GetParameters().GetItem( L"brefineimmediately");  
        Parameter lbintersectcost= prop.GetParameters().GetItem( L"bintersectcost");      
        Parameter lbtraversalcost= prop.GetParameters().GetItem( L"btraversalcost");      
        Parameter lbmaxprims= prop.GetParameters().GetItem( L"bmaxprims");          
        Parameter lbacmaxdepth= prop.GetParameters().GetItem( L"bacmaxdepth");         
        Parameter lbemptybonus= prop.GetParameters().GetItem( L"bemptybonus");
        //--
        lbmaxprimsperleaf.PutCapabilityFlag( siNotInspectable, true );
        lbfullsweepthreshold.PutCapabilityFlag( siNotInspectable, true );
        lbskipfactor.PutCapabilityFlag( siNotInspectable, true );

       //lbtreetype.PutCapabilityFlag( siNotInspectable, true );           
        lbcostsamples.PutCapabilityFlag( siNotInspectable, true );
        //lbrefineimmediately.PutCapabilityFlag( siNotInspectable, true );
        lbintersectcost.PutCapabilityFlag( siNotInspectable, true );
        lbtraversalcost.PutCapabilityFlag( siNotInspectable, true );
        lbmaxprims.PutCapabilityFlag( siNotInspectable, true );
        lbacmaxdepth.PutCapabilityFlag( siNotInspectable, true );
        lbemptybonus.PutCapabilityFlag( siNotInspectable, true );
        //--
        if (( vAccel == 0 ) && ( vsexpert )) //-- qbvh
        {
            lbmaxprimsperleaf.PutCapabilityFlag( siNotInspectable, false );
            lbfullsweepthreshold.PutCapabilityFlag( siNotInspectable, false );
            lbskipfactor.PutCapabilityFlag( siNotInspectable, false );
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if (( vAccel == 1 )&& ( vsexpert )) //-- bvh
        {
//            lbtreetype.PutCapabilityFlag( siNotInspectable, false ); //-- combo          
            lbcostsamples.PutCapabilityFlag( siNotInspectable, false );
            lbintersectcost.PutCapabilityFlag( siNotInspectable, false );
            lbtraversalcost.PutCapabilityFlag( siNotInspectable, false );
            lbemptybonus.PutCapabilityFlag( siNotInspectable, false );
            ctxt.PutAttribute(L"Refresh", true );
        }
//        else if (( vAccel == 2 )&& ( vsexpert )) //-- grid
//        {
//            lbrefineimmediately.PutCapabilityFlag( siNotInspectable, false );
//            ctxt.PutAttribute(L"Refresh", true );
//        }
        else
        {
            if ( vsexpert )//-- KD Tree
            {
                lbintersectcost.PutCapabilityFlag( siNotInspectable, false );
                lbtraversalcost.PutCapabilityFlag( siNotInspectable, false );
                lbemptybonus.PutCapabilityFlag( siNotInspectable, false );
                lbmaxprims.PutCapabilityFlag( siNotInspectable, false );
                lbacmaxdepth.PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
    }//-- end Accel
}
//--
XSIPLUGINCALLBACK CStatus LuXSI_Menu_Init( CRef& in_ctxt )
{
    Context ctxt( in_ctxt );
    Menu oMenu;
    oMenu = ctxt.GetSource();
    MenuItem oNewItem;
    oMenu.AddCallbackItem(L"LuXSI",L"OnLuXSI_MenuClicked",oNewItem);
    return CStatus::OK;
}

//--
XSIPLUGINCALLBACK CStatus OnLuXSI_MenuClicked( XSI::CRef& )
{
    CValueArray addpropArgs(5) ;
    addpropArgs[0] = L"LuXSI"; // Type of Property
    addpropArgs[3] = L"LuXSI"; // Name for the Property
    addpropArgs[1] = L"Scene_Root";
    bool vAlreadyThere=false;
    CValue retVal ;
    CStatus st;

    // search for existing interface
    CRefArray vItems = app.GetActiveSceneRoot().GetProperties();
    for (int i=0;i<vItems.GetCount();i++){
        if (SIObject(vItems[i]).GetType()==L"LuXSI") {
            vAlreadyThere=true;
            st = true;
            break;
        }
    }

    if (!vAlreadyThere){
        st = app.ExecuteCommand( L"SIAddProp", addpropArgs, retVal ) ;
    }

    if ( st.Succeeded() )
    {
        CValueArray args(5) ;

        args[0] = prop ;
        args[2] = "LuXSI";
        args[3] = siLock;
        args[4] = false;

        app.ExecuteCommand( L"InspectObj", args, retVal ) ;
    }

    return st ;
}

//--
bool find(CStringArray a, CString s){
    //
    // Returns true if String s is in Array a
    //
    for (int i=0;i<a.GetCount();i++){
        if (a[i]==s) {
            return true;
        }
    }
    return false;
}

//--
CString findInGroup(CString s){

    CRefArray grps = root.GetGroups();

    for (int i=0;i<grps.GetCount();i++){
        CRefArray a=Group(grps[i]).GetMembers();
        for (int j=0;j<a.GetCount();j++){
            if (X3DObject(a[j]).GetName()==s) {
                //app.LogMessage(L"Group: " + Group(grps[i]).GetName() + L"Childname: "+X3DObject(a[j]).GetName());
                return Group(grps[i]).GetName();
            }
        }
    }

    return L"";
}
//--
void writeLuxsiBasics(){
    //
    // write basic scene description

    //-- convention names; prefix Mt;
    //----//------>
    char MtRRst [3] [12] = { "none", "probability", "efficiency"};
    char ARmode [2] [8] = { "GUI", "console"};
    char MtlightST [7] [12] = { "one", "all", "auto", "importance", "powerimp", "allpowerimp", "logpowerimp"};
    char Aengine [2] [8] = { "classic", "hybrid" };
    char ArgbT [3] [5] = { "Y", "RGB", "RGBA" }; // iterator  vRtga
    char AZb_norm [3] [22] = { "Camera Start/End clip", "Min/Max", "None" }; //exr
    char ArgbP [4] [5] = { "Y", "YA", "RGB", "RGBA" }; // iterator  vRpng
    char aFilter [5] [9] = { "box", "gaussian", "mitchell", "sinc", "triangle" }; // iter vfilter
    char aSampler [4] [15] = { "metropolis", "erpt", "lowdiscrepancy", "random" }; // mode, Asampler [vSampler]
    char aPxSampler [6] [15] = { "linear", "vegas", "random", "lowdiscrepancy", "tile", "hilbert" };
    char MtSurf [6] [16] = { "bidirectional", "path", "directlighting", "distributedpath", "igi", "exphotonmap",  };
    char MtRendering [2] [15] = { "path", "directlighting" };
    //---
    string fname=vFileObjects.GetAsciiString();
    int loc=(int)fname.rfind(".");

    f << "Film \"fleximage\"\n"; //----
    f << "  \"integer xresolution\" [" << vXRes << "]\n  \"integer yresolution\" [" << vYRes << "]\n";
    f << "  \"string filename\" [\"" << replace(fname.substr(0,loc)) << "\"]\n";

    f << "  \"integer writeinterval\" [" << vSave <<"]\n";
    f << "  \"integer displayinterval\" [" << vDis <<"]\n";

    f << "  \"bool write_exr\" [\"" << MtBool[vExr] <<"\"]\n";
    if (vExr)
    {
        f << "  \"string write_exr_zbuf_normalizationtype\" [\""<< AZb_norm[vExr_Znorm] <<"\"]\n";
    } //----/ TODO: more options for Exr /------>

    f << "  \"bool write_png\" [\"" << MtBool[vPng] <<"\"]\n";
    if (vPng)
    {
        f << "  \"string write_png_channels\" [\""<< ArgbP[vRpng] <<"\"]\n";
        f << "  \"bool write_png_16bit\" [\""<< MtBool[vWpng_16] <<"\"]\n";
        f << "  \"bool write_png_gamutclamp\" [\""<< MtBool[vPng_gamut] <<"\"]\n";
    } //-- options for Png

    f << "  \"bool write_tga\" [\""<< MtBool[vTga] <<"\"]\n";
    if (vTga)
    {
        f << "  \"string write_tga_channels\" [\"" << ArgbT[vRtga] <<"\"]\n";
        f << "  \"bool write_tga_gamutclamp\" [\"" << MtBool[vTga_gamut] <<"\"]\n";
    } //----/ TODO: more options for Tga /------>

    f << "  \"bool write_resume_flm\" [\""<< MtBool[vResume] <<"\"]\n";
    f << "  \"bool premultiplyalpha\" [\""<< MtBool[0] <<"\"]\n"; // TODO;
    f << "  \"integer haltspp\" [" << vhaltspp << "]\n";
    f << "  \"integer halttime\" [" << vhalttime << "]\n";

    f << "  \"float gamma\" ["<< vContrast <<"]\n\n";
    //-------------------------------------------------//
    f << "\nPixelFilter \""<< aFilter[vfilter] <<"\"\n";
    //-------------------------------------------------//
    if ( vfexpert )
    { //-- box -- commons values
        f << "  \"float xwidth\" ["<< vXwidth <<"]\n";
        f << "  \"float ywidth\" ["<< vYwidth <<"]\n";
       
        if ( vfilter == 1 ) //-- gaussian
        {
            f << "  \"float alpha\" ["<< vFalpha <<"]\n";
        }
        else if ( vfilter == 2 ) //-- mitchell
        {
            f << "  \"float B\" ["<< vF_B <<"]\n";
            f << "  \"float C\" ["<< vF_C <<"]\n";
            f << "  \"bool supersample\" [\""<< MtBool[vSupers] <<"\"]\n";
        }
        else //-- sinc
        {
            f << "  \"float tau\" ["<< vTau <<"]\n";
        }
        //-- triangle -- commons values
    }
    //-------------------------------------------------//
    f << "\nSampler \""<< aSampler[vSampler] <<"\"\n";
    //-- for more options, used if else mode ----------//

    if ( vSampler == 0 ) //-- sampler; metropolis
    {
        f << "  \"float largemutationprob\" ["<< vlmutationpro <<"]\n";
        f << "  \"bool usevariance\" [\""<< MtBool[vusevariance] <<"\"]\n"; // TODO;
        if ( vExpert )
        {
            f << "  \"integer maxconsecrejects\" ["<< vmaxconsecrejects <<"]\n";
        }
    }
    else if ( vSampler == 1 ) //-- sampler; erpt
    {
        if ( vbasampler > 0 ) //-- base sampler; low or random
        {
            f << "  \"integer chainlength\" ["<< vchainlength <<"]\n";
            f << "  \"string basesampler\" [\""<< MtBsampler[vbasampler] <<"\"]\n";
            f << "  \"string pixelsampler\" [\""<< aPxSampler[vPixsampler] <<"\"]\n";
            f << "  \"integer pixelsamples\" ["<< vPixelsamples <<"]\n";

        }
        else //-- base sampler; metropolis
        {
            f << "  \"integer chainlength\" ["<< vchainlength <<"]\n";
            f << "  \"string basesampler\" [\""<< MtBsampler[vbasampler] <<"\"]\n";
            if ( vExpert )
            {
                f << "  \"float mutationrange\" [ "<< vmutationrange <<"]\n"; // test
                f << "  \"integer maxconsecrejects\" ["<< vmaxconsecrejects <<"]\n";
            }
        }
    }
    else if ( vSampler > 1 ) //-- sampler; lowdiscrepance
    {
        f << "  \"string pixelsampler\" [\""<< aPxSampler[vPixsampler] <<"\"]\n";
        f << "  \"integer pixelsamples\" ["<< vPixelsamples <<"]\n";
    }
    else //-- error mesage
    {
        app.LogMessage( L"Not Sampler for exporter",siErrorMsg );
    }

    //-----------------------------------------------------------//
    f << "\nSurfaceIntegrator \""<< MtSurf[ vSurfaceInt ] <<"\"\n";
    //-----------------------------------------------------------//
    if ( vSurfaceInt == 0 )  //-- bidi
    {
        f << "  \"integer eyedepth\" ["<< vEye_depth <<"]\n";
        f << "  \"integer lightdepth\" ["<< vLight_depth <<"]\n";
        if ( vsexpert )
        {
            f << "  \"float eyerrthreshold\" ["<< vEyeRRthre <<"]\n"; // TODO;
            f << "  \"float lightrrthreshold\" ["<< vLightRRthre <<"]\n"; // TODO;
        }
    }
    else if ( vSurfaceInt == 1 )// path
    {
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        f << "  \"float rrcontinueprob\" [0.649999976158142]\n"; // TODO:
        f << "  \"string rrstrategy\" [\""<< MtRRst[ vRRstrategy ] <<"\"]\n";
        f << "  \"bool includeenvironment\" [\""<< MtBool[ vInc_env ] <<"\"]\n";
        if ( vsexpert )
        {
            f << "  \"string lightstrategy\" [\""<< MtlightST[ vLight_str ] <<"\"]\n";
        }
    }
    else if ( vSurfaceInt == 2 ) // directlighting
    {
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        if ( vsexpert )
        {
            f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
        }
        //f << "    \"integer shadowraycount\" [" << vShadow_rc << "]\n";
    }
    else if ( vSurfaceInt == 3 ) //-- distributedpath
    {
        f << "  \"bool directsampleall\" [\""<< MtBool[ vdirectsampleall ] <<"\"]\n";
        f << "  \"integer directsamples\" ["<< vdirectsamples <<"]\n";
        f << "  \"bool directdiffuse\" [\""<< MtBool[ vdirectdiffuse ] <<"\"]\n";
        f << "  \"bool directglossy\" [\""<< MtBool[vdirectglossy] <<"\"]\n";
        f << "  \"bool indirectsampleall\" [\""<< MtBool[ vindirectsampleall ] <<"\"]\n";
        f << "  \"integer indirectsamples\" ["<< vindirectsamples <<"]\n";
        f << "  \"bool indirectdiffuse\" [\""<< MtBool[ vindirectdiffuse ] <<"\"]\n";
        f << "  \"bool indirectglossy\" [\""<< MtBool[ vindirectglossy ] <<"\"]\n";
        f << "  \"integer diffusereflectdepth\" ["<< vdiffusereflectdepth <<"]\n";
        f << "  \"integer diffusereflectsamples\" ["<< vdiffusereflectsamples <<"]\n";
        f << "  \"integer diffuserefractdepth\" ["<< vdiffuserefractdepth <<"]\n";
        f << "  \"integer diffuserefractsamples\" ["<< vdiffuserefractsamples <<"]\n";
        f << "  \"integer glossyreflectdepth\" ["<< vglossyreflectdepth <<"]\n";
        f << "  \"integer glossyreflectsamples\" ["<< vglossyreflectsamples <<"]\n";
        f << "  \"integer glossyrefractdepth\" ["<< vglossyrefractdepth <<"]\n";
        f << "  \"integer glossyrefractsamples\" ["<< vglossyrefractsamples <<"]\n";
        f << "  \"integer specularreflectdepth\" ["<< vspecularreflectdepth <<"]\n";
        f << "  \"integer specularrefractdepth\" ["<< vspecularrefractdepth <<"]\n";
        f << "  \"bool diffusereflectreject\" [\""<< MtBool[0] <<"\"]\n"; // TODO
        f << "  \"float diffusereflectreject_threshold\" ["<< vdiff_reflect_reject_thr <<"]\n";
        f << "  \"bool diffuserefractreject\" [\""<< MtBool[0] <<"\"]\n";
        f << "  \"float diffuserefractreject_threshold\" ["<< vdiff_refract_reject_thr <<"]\n";
        f << "  \"bool glossyreflectreject\" [\""<< MtBool[0] <<"\"]\n";
        f << "  \"float glossyreflectreject_threshold\" ["<< vglossy_reflect_reject_thr <<"]\n";
        f << "  \"bool glossyrefractreject\" [\""<< MtBool[0] <<"\"]\n";
        f << "  \"float glossyrefractreject_threshold\" ["<< vglossy_refract_reject_thr <<"]\n";
        if ( vsexpert )
        {
        f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
        }
    }
    else if ( vSurfaceInt == 4 ) //-- igi
    {
        f << "  \"integer nsets\" ["<< vnsets <<"]\n";
        f << "  \"integer nlights\" ["<< vnlights <<"]\n";
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        f << "  \"float mindist\" ["<< vmindist <<"]\n";
        f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
    }
    else //-- exphotonmap
    {
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        f << "  \"integer maxphotondepth\" ["<< vmaxphotondepth <<"]\n";
    //  f << "\"integer shadowraycount\" [5]\n"; //-- not implemented in 2.5 ?
        f << "  \"integer directphotons\" ["<< vdirectphotons <<"] \n";
        f << "  \"integer causticphotons\" ["<< vcausticphotons <<"] \n";
        f << "  \"integer indirectphotons\" ["<< vindirectphotons <<"] \n";
        f << "  \"integer radiancephotons\" ["<< vradiancephotons <<"] \n";
        f << "  \"integer nphotonsused\" ["<< vnphotonsused <<"] \n";
        f << "  \"float maxphotondist\" ["<< vmaxphotondist <<"]\n";
        f << "  \"bool finalgather\" [\""<< MtBool[ vfinalgather ] <<"\"]\n";
        if ( vfinalgather )
        {
            f << "  \"integer finalgathersamples\" ["<< vfinalgathersamples <<"] \n";
            f << "  \"float gatherangle\" ["<< vgatherangle <<"] \n";
        }
        if ( vsexpert )
        {
            f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
            f << "  \"string renderingmode\" [\""<< MtRendering[ vrenderingmode ] <<"\"]\n";
            f << "  \"string rrstrategy\" [\""<< MtRRst[vRRstrategy] <<"\"]\n"; 
            f << "  \"float distancethreshold\" [\""<< vdistancethreshold <<"\"]\n"; 
            f << "  \"bool dbg_enabledirect\" [\""<< MtBool[vdbg_direct] <<"\"]\n";
            f << "  \"bool dbg_enableradiancemap\" [\""<< MtBool[vdbg_radiancemap] <<"\"]\n";
            f << "  \"bool dbg_enableindircaustic\" [\""<< MtBool[vdbg_indircaustic] <<"\"]\n";
            f << "  \"bool dbg_enableindirdiffuse\" [\""<< MtBool[vdbg_indirdiffuse] <<"\"]\n";
            f << "  \"bool dbg_enableindirspecular\" [\""<< MtBool[vdbg_indirspecular] <<"\"]\n";
        }
        
    }

    //--------------------------------------------------//
    f << "\nAccelerator \""<< MtAccel[vAccel] <<"\"\n";
    //--------------------------------------------------//
    if (( vAccel == 0 ) || ( vExpert )) //-- qbvh
    {
        f << "  \"integer maxprimsperleaf\" ["<< vmaxprimsperleaf <<"]\n";
        f << "  \"integer fullsweepthreshold\" ["<< vfullsweepthr <<"]\n";
        f << "  \"integer skipfactor\" ["<< vskipfactor <<"]\n";
    }
    else if (( vAccel == 1 ) || ( vExpert )) //-- bvh
    {
        f << "  \"integer intersectcost\" ["<< vintersectcost <<"]\n";
        f << "  \"integer traversalcost\" ["<< vtraversalcost <<"]\n";
        f << "  \"integer costsamples\" ["<< vcostsamples <<"]\n"; // lbcostsamples
    }
//    else if (( vAccel == 2 ) || ( vExpert )) //-- grid
//   {
//        f << "  \"bool refineimmediately\" [\""<< MtBool[vrefineimmediately] <<"\"]\n";
//    }
    else //-- KD Tree
    {
        f << "  \"integer intersectcost\" ["<< vintersectcost <<"]\n";
        f << "  \"integer traversalcost\" ["<< vtraversalcost <<"]\n";
        f << "  \"float emptybonus\" ["<< vemptybonus <<"]\n";
        f << "  \"integer maxprims\" ["<< vmaxprims <<"]\n";
        f << "  \"integer maxdepth\" ["<< vacmaxdepth <<"]\n\n";
    }
}
//--
void write_environment(){ // test
        // Look if there is an background image for Image Based Lighting (e.g. HDRI)
    CRefArray aEnv = app.GetActiveProject().GetActiveScene().GetActivePass().GetNestedObjects();
    for (int i=0;i<aEnv.GetCount();i++){
        if (SIObject(aEnv[i]).GetName()==L"Environment Shader Stack") {
            CRefArray aImages = SIObject(aEnv[i]).GetNestedObjects();
            for (int j=0;j<aImages.GetCount();j++){
                if (SIObject(aImages[j]).GetType()==L"Shader"){
                    Shader s(aImages[j]);
                    CRefArray aEnvImg=s.GetImageClips();
                    for (int k=0;k<aEnvImg.GetCount();k++){
                        ImageClip2 vImgClip(aEnvImg[k]);
                        Source vImgClipSrc(vImgClip.GetSource());
                        CValue vFileName = vImgClipSrc.GetParameterValue( L"path");
                        if (vFileName.GetAsText()!=L""){
                            vHDRI=vFileName.GetAsText();

                            break;
                        }
                    }
                }
            }
            break;
        }
    }

/*

    else if (vAmbBack) {
            //
            // TODO: check if there is a C++ way to read AmbientLighting color
            //
        CScriptErrorDescriptor status ;
        CValueArray fooArgs(1) ;
        fooArgs[0] = L"" ;
        CValue retVal=false ;
        status = app.ExecuteScriptProcedure(L"function getS(){return GetValue(activesceneroot+\".AmbientLighting.ambience.red\")}",L"JScript",L"getS",fooArgs, retVal  ) ;
        CValue red=retVal;
        status = app.ExecuteScriptProcedure(L"function getS(){return GetValue(activesceneroot+\".AmbientLighting.ambience.green\")}",L"JScript",L"getS",fooArgs, retVal  ) ;
        CValue green=retVal;
        status = app.ExecuteScriptProcedure(L"function getS(){return GetValue(activesceneroot+\".AmbientLighting.ambience.blue\")}",L"JScript",L"getS",fooArgs, retVal  ) ;
        CValue blue=retVal;

        f << "AttributeBegin\n";
        f << "LightSource \"infinite\" \"color L\" [" <<CString(red).GetAsciiString()<<" " <<CString(green).GetAsciiString()<<" " <<CString(blue).GetAsciiString()<< "] \"integer nsamples\" [1]\n";
        f << "AttributeEnd \n";
    } // end enviroment
    */
}
//--
void writeLuxsiCam(X3DObject o){
    //
    // write camera
    //

    X3DObject o2;
    Camera c;
    if (o.GetType()==L"camera") {
        o2=o;
        c=o;
    } else {
        o2=o.GetChildren()[0];
        c=o.GetChildren()[0];
    }

    CVector3 vnegZ(0,0,-1);

    // Operations to calculate look at position.
    vnegZ.MulByMatrix3InPlace(c.GetKinematics().GetGlobal().GetTransform().GetRotationMatrix3());
    vnegZ.NormalizeInPlace();
    vnegZ.ScaleInPlace((double) c.GetParameterValue(L"interestdist"));
    vnegZ.AddInPlace(c.GetKinematics().GetGlobal().GetTransform().GetTranslation());


    CTransformation localTransformation = o2.GetKinematics().GetLocal().GetTransform();
    KinematicState  gs = o2.GetKinematics().GetGlobal();
    CTransformation gt = gs.GetTransform();
    CVector3 translation(localTransformation.GetTranslation());
    bool vDof=false;

    X3DObject ci(o.GetChildren()[1]);
    CValue vCType=L"pinhole";
    CValue vFdist=0.0, vLensr=0.0, vFocal=0;

    CRefArray cShaders = c.GetShaders();
    for (int i=0;i<cShaders.GetCount();i++){
        CString vCSID((Shader(cShaders[i]).GetProgID()).Split(L".")[1]);

        if (vCSID==L"sib_dof") {
            // Depth_of_field shader found
            vLensr=Shader(cShaders[i]).GetParameterValue(L"strenght");
            vFdist=Shader(cShaders[i]).GetParameterValue(L"auto_focal_distance");
        }
    }

    KinematicState  ci_gs = ci.GetKinematics().GetGlobal();
    CTransformation ci_gt = ci_gs.GetTransform();


    CVector3 tranlation(0,1,0);
    CTransformation target=o2.GetKinematics().GetGlobal().GetTransform().AddLocalTranslation(tranlation);
    CVector3 up(target.GetTranslation());
    float vfov;
    if ((int)c.GetParameterValue(CString(L"fovtype"))==1) {
        // calculate the proper FOV (horizontal -> vertical)
        float hfov = (float)c.GetParameterValue(CString(L"fov"));
        vfov=(float) (2* atan(1/(float)c.GetParameterValue(CString(L"aspect")) * tan(hfov/2*PI/180))*180/PI);
    } else {
        // keep vertical FOV
        vfov=(float)c.GetParameterValue(CString(L"fov"));
    }
    // lookat: posX posY posZ targetX targetY targetZ upX upY upZ
    double x,y,z;
    vnegZ.Get( x,y,z );

    //----/ new feature; ortographic camera /---->
    CString vProj=L"";
    if (c.GetParameterValue(CString(L"proj"))==1) {vProj=L"perspective" ;}
    else { vProj=L"orthographic";}
    //----/ end /---->

    CVector3 new_pos = gt.GetTranslation();
    CVector3 new_pos_ci = ci_gt.GetTranslation();
    //--
    f << "LookAt " << new_pos.GetX() << " " << new_pos.GetY() << " " << new_pos.GetZ() << "\n";
    f << "       "<< new_pos_ci.GetX() << " " << new_pos_ci.GetY() << " " << new_pos_ci.GetZ() <<"\n";
    f << "       0 1 0 \n"; //<< CString(up.GetZ()).GetAsciiString() << "\n"; // not working correct
    f << "Camera \"" << vProj.GetAsciiString() <<"\" \n \"float fov\" [" << vfov << "] \n";
    f << "  \"float lensradius\" ["<< CString(vLensr).GetAsciiString()  <<"] \n";
    f << "  \"float focaldistance\" ["<< CString(vFdist).GetAsciiString() <<"]";
    f << "\n\n";
}

//--
void writeLuxsiLight(X3DObject o){
    //
    // write light
    //
    CTransformation localTransformation = o.GetKinematics().GetLocal().GetTransform();
    KinematicState  gs = o.GetKinematics().GetGlobal();
    CTransformation gt = gs.GetTransform();
    CVector3 translation(gt.GetTranslation());
    CValue lType,lPos,lPower;
    float a=0, b=0, c=0, alpha=0;

    Shader s((Light(o).GetShaders())[0]);
    OGLLight myOGLLight(Light(o).GetOGLLight());
    s.GetColorParameterValue(L"color",a,b,c,alpha );
    CString lName = findInGroup(o.GetName());
    //--
    CString group_name;
    //--
    if (lName!=L"") {
        group_name = lName.GetAsciiString();
    } else {
        group_name = o.GetName().GetAsciiString();
    }
    f << "\nLightGroup \""<< group_name.GetAsciiString() <<"\" \n";
    //--
    if (myOGLLight.GetType() == siLightSpot ){
        //-- Spotlight
        X3DObject li;
        CVector3 intPos;
        //-- Get Interest
        li= X3DObject(o.GetParent()).GetChildren()[1];
        CTransformation lt = li.GetKinematics().GetLocal().GetTransform();
        intPos = lt.GetTranslation();
        CRefArray shad(Light(o).GetShaders());
        Shader s(shad[0]);

        //-- values
        f << "\nLightSource \"spot\"\n";
        f << "  \"point from\" [" << (float)translation.GetX() << " " << (float)translation.GetY() << " "  << (float)translation.GetZ()  << "] \n";
        f << "  \"point to\" ["<< (float)intPos.GetX() << " " << (float)intPos.GetY() << " "<< (float)intPos.GetZ() << "]\n";
        f << "  \"float coneangle\" [" << (float)o.GetParameterValue((L"LightCone")) << "]\n";
        f << "  \"float conedeltaangle\" [" << ((float)o.GetParameterValue(L"LightCone")- (float)s.GetParameter(L"spread").GetValue() ) << "]\n";
        f << "  \"color \" [" << a << "  " << b << "  " << c << "] \"float gain\" ["  << (float)s.GetParameterValue(L"intensity") << "]\n";

    } else if  (myOGLLight.GetType()==siLightInfinite ) {

        //-- sunlight
        if (vHDRI!=L""){
            f << "Scale -1 1 1 \nRotate -90 1 0 0 \n"; // for corrected orientation
            f << "LightSource \"infinite\" \"color L\" [1 1 1] \"integer nsamples\" [1]\n"; //-- TODO; removed color
            f << "\"string mapname\" [\"" << replace(vHDRI.GetAsciiString()) << "\"]\n";

        } else {
            CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();
            f << "\nLightSource \"sunsky\"\n";
            f << "  \"integer nsamples\" [4]\n";
            f << "  \"vector sundir\" [ "<< (float)sunTransMat.GetValue(2,0) << " " << (float)sunTransMat.GetValue(2,1) << " " << (float)sunTransMat.GetValue(2,2) << " ]\n";
            f << "  \"float gain\" [" << (float)s.GetParameterValue(L"intensity") << "]\n";
        }

        //---
        //sunlight
        //
    /*  CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();

        f << "\nLightSource \"sunsky\"\n";
        f << "  \"integer nsamples\" [4]\n";
        f << "  \"vector sundir\" [ "<< (float)sunTransMat.GetValue(2,0) << " " << (float)sunTransMat.GetValue(2,1) << " " << (float)sunTransMat.GetValue(2,2) << " ]\n";
        f << "  \"float gain\" [" << (float)s.GetParameterValue(L"intensity") << "]\n";
    */

    } else {
        //
        // Pointlight
        CVector3 intPos;
        CTransformation localTransformation2 = o.GetKinematics().GetLocal().GetTransform();
        KinematicState  gs2 = o.GetKinematics().GetGlobal();
        CTransformation gt2 = gs2.GetTransform();
        intPos=gt2.GetTranslation();
        //--
        f << "\nLightSource \"point\"\n";
        f << "  \"point from\" [" << intPos.GetX() << " " << intPos.GetY() << " " << intPos.GetZ() << "]\n";
        f << "  \"color L\" [" << a << "  " << b << "  " << c << "] \"float gain\" ["<< (float)s.GetParameterValue(L"intensity") << "]\n";

    }
}
//--
void writeLuxsiShader(){
    //
    // Writes shader
    //

    Scene scene = app.GetActiveProject().GetActiveScene();
    Library matlib = scene.GetActiveMaterialLibrary();

    CRefArray materials = matlib.GetItems();
//  char aBool[2][6]={"false","true"};
    CValueArray aShader(11);
    aShader[0] = L"matte" ;
    aShader[1] = L"glass" ;
    aShader[2] = L"glossy";
    aShader[3] = L"shinymetal";
    aShader[4] = L"matte";
    aShader[6] = L"carpaint";
    aShader[7] = L"roughglass";
    aShader[8] = L"mirror";
    aShader[9] = L"metal";
    aShader[10] = L"mattetranslucent";
    int ret=0;

    for ( LONG i=0; i < materials.GetCount(); i++ ) {
        Texture vTexture;
        CString shaderString;
        CString shaderType;
        CString vFileBump;
        CString vChanel=L"", vChanType=L"", shaderTexture=L"";

        float b_red=0.0f,b_green=0.0f,b_blue=0.0f,b_alpha=0.0f,red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f,sp_red=0.0f,sp_green=0.0f,sp_blue=0.0f,sp_alpha=0.0f,vModScale=0.0f,refl_red=0.0f,refl_green=0.0f,refl_blue=0.0f,refl_alpha=0.0f;
        float mRough=0.0f;
        ImageClip2 vBumpFile;
        Shader vBumpTex;
        CString texFact=L"";
        bool vIsSet=false;
        bool vText=false,vNorm=false;
        CValue vDiffType,vMore,vCol,vMore2,vMore3,vTexStr,vMore1,vMore4,vMore5;
        Material m( materials[i] );

        if ( (int)m.GetUsedBy().GetCount()==0) {
            continue;
        }

        CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
        Shader s(shad[0]);
        CString vMatID((s.GetProgID()).Split(L".")[1]);
        char sname[256];
        strcpy(sname,m.GetName().GetAsciiString());

        if ( find(aMatList, m.GetName() ) ) {
            continue;
        } else {
            aMatList.Add(m.GetName());
        }
        //app.LogMessage(L"Shader: " + CString(Parameter(s.GetParameterValue(L"bump")).GetValue()));

        if (vMatID==L"lux_glass") {
            shaderType=L"glass";
            s.GetColorParameterValue(L"kt",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            shaderString += L"  \"float index\" [" + CString((float)s.GetParameter(L"index").GetValue()) + L"]\n";
            shaderString += L"  \"float cauchyb\" [" + CString((float)s.GetParameter(L"cauchyb").GetValue()) + L"]\n";
            /*
            if ((bool)s.GetParameter(L"architectural").GetValue()==true) {
                 addString += L"\"bool architectural\" [\"true\"]";
            }
            */
            vIsSet=true;
        } else if (vMatID==L"lux_roughglass") {
            shaderType=L"roughglass";
            s.GetColorParameterValue(L"kt",red,green,blue,alpha );
            shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString((float)s.GetParameter(L"uroughness").GetValue())+L"] \"float vroughness\" ["+CString((float)s.GetParameter(L"vroughness").GetValue())+L"]\n";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            shaderString += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"index")) + L"]\n";
            shaderString += L"  \"float cauchyb\" [" + CString((float)s.GetParameterValue(L"cauchyb")) + L"]\n";
            vIsSet=true;
        } else if (vMatID==L"lux_matte") {
            shaderType=L"matte";
            s.GetColorParameterValue(L"kd",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [" + CString((float)s.GetParameterValue(L"sigma")) + L"]\n";
            vIsSet=true;
        } else if (vMatID==L"lux_mirror") {
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderType=L"mirror";
            shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            vIsSet=true;
        } else if (vMatID==L"lux_metal") {
            /*
            CValueArray aMetal(5);
            aMetal[0]="amorphous carbon";
            aMetal[1]="silver";
            aMetal[2]="gold";
            aMetal[3]="copper";
            aMetal[4]="aluminium";
            f << "Texture \"name-"<< sname << "\" \"string\" \"constant\" \"string value\" ["<<  aMetal[s.GetParameter(L"string").GetValue()].GetAsText().GetAsciiString()<<"]\n";
            f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"roughness").GetValue() << "]\n";
            f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"roughness").GetValue() << "]\n";
            ret=9;
            vIsSet=true;
            */
        } else if (vMatID==L"lux_shinymetal") {

            shaderType=L"shinymetal";
            s.GetColorParameterValue(L"ks",red,green,blue,alpha );
            shaderString += L"  \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
            shaderString += L"  \"float uroughness\" ["+ CString((float)s.GetParameterValue(L"roughness"))+L"]";
            shaderString += L"  \"float vroughness\" ["+CString((float)s.GetParameterValue(L"roughness"))+L"]\n";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            vIsSet=true;
        } else if (vMatID==L"lux_substrate") {
            shaderType=L"substrate";
            s.GetColorParameterValue(L"kd",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
            shaderString += L"  \"float uroughness\" ["+ CString((float)s.GetParameterValue(L"uroughness"))+L"]";
            shaderString += L"  \"float vroughness\" ["+CString((float)s.GetParameterValue(L"vroughness"))+L"]\n";
            s.GetColorParameterValue(L"ks",red,green,blue,alpha );
            shaderString += L"  \"color Ks\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            vIsSet=true;
        }else if (vMatID==L"lux_mattetranslucent") {
            shaderType=L"mattetranslucent";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            s.GetColorParameterValue(L"kt",red,green,blue,alpha );
            shaderString += L"  \"color Kt\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [" + CString((float)s.GetParameter(L"sigma").GetValue()) + L"]\n";
            vIsSet=true;
        }

        if (!vIsSet) {
            if (s.GetParameterValue(L"refract_inuse")=="-1") {
            //check if material is transparent: phong/lamber/blin/constant/cooktorrance/strauss
            float ior=0.0f;
            s.GetColorParameterValue(L"transparency",red,green,blue,alpha );
            if (red>0 || green>0 || blue>0) {
                shaderType=L"glass";

                shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                s.GetColorParameterValue(L"reflectivity",sp_red,sp_green,sp_blue,sp_alpha );
                shaderString += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
                shaderString += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"index_of_refraction")) + L"]\n";
                shaderString += L"  \"float cauchyb\" [0]\n";

                if ((float)s.GetParameterValue(L"trans_glossy")>0 ) {
                    shaderType=L"roughglass";
                    shaderString += L"  \"float uroughness\" ["+ CString((float)s.GetParameterValue(L"trans_glossy"))+ L"]";
                    shaderString += L"  \"float vroughness\" ["+CString((float)s.GetParameterValue(L"trans_glossy"))+ L"]\n";
                }
                vIsSet=true;
            }
        }
    }

        if (!vIsSet) {
            if ((float)s.GetParameter(L"transparency").GetValue()>0.0f) {
                float ior=0.0f;
                // glass mia-arch shader
                s.GetColorParameterValue(L"refr_color",red,green,blue,alpha );
                s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
                shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderString += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
                shaderString += L"  \"float index\" [" + CString((float)s.GetParameter(L"refr_ior").GetValue()) + L"]\n";
                shaderString += L"  \"float cauchyb\" [0]\n";
                shaderType=L"glass";

                if ((float)s.GetParameter(L"refr_gloss").GetValue()<1 ) {
                    shaderType=L"roughglass";
                    shaderString += L"  \"float uroughness\" ["+ CString(1.0f-(float)s.GetParameterValue(L"refr_gloss"))+ L"]";
                    shaderString += L"  \"float vroughness\" ["+CString(1.0f-(float)s.GetParameterValue(L"refr_gloss"))+ L"]\n";
                }
                vIsSet=true;
            }
        }


        if (!vIsSet) {
            // check if its a reflecting material
            float a,b,c,d;
            s.GetColorParameterValue(L"diffuse",a,b,c,d );
            if (s.GetParameter(L"reflect_inuse").GetValue()=="-1" ){
                if (vMatID==L"mia_material_phen") {

                    s.GetColorParameterValue(L"refl_color",red,green,blue,alpha );
                    mRough=1-(float)s.GetParameter(L"refl_gloss").GetValue();

                    red=red*a;
                    green=green*b;
                    blue=blue*c;

                } else {
                    s.GetColorParameterValue(L"reflectivity",red,green,blue,alpha );
                    mRough=(float)s.GetParameter(L"reflect_glossy").GetValue();
                }
                if (red>0 || green>0 || blue>0) {
                    shaderType=L"shinymetal";
                    shaderString += L"  \"color Kr\" [" + CString(a) + L" " + CString(b) + L" " + CString(c) + L"] ";
                    shaderString += L" \"float uroughness\" ["+ CString(mRough/10)+L"] \"float vroughness\" ["+ CString(mRough/10)+ L"]\n";
                    shaderString += L" \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                    vIsSet=true;
                }
            }
        }
        if (!vIsSet) {
            //app.LogMessage(vMatID);

            // Get Textures
            /*
            JScript Version to find shader in slot:

            s = Selection(0).Material.Shaders(0);
            for (i=0;i<s.parameters.count;i++){
            t=s.parameters(i).source;
            if (t) logmessage(s.parameters(i).name + "=" + t);
            }

            */


            // Material Stuff
            if (vMatID==L"mi_car_paint_phen") {
                // car paint
                /*
                float spr,spg,spb,spa,spr2,spg2,spb2,spa2,r,g,b,a;
                s.GetColorParameterValue(L"spec",spr,spg,spb,spa );
                s.GetColorParameterValue(L"spec_sec",spr2,spg2,spb2,spa2 );
                s.GetColorParameterValue(L"base_color",r,g,b,a );
                f << "Texture \"Kd-" << sname << "\" \"color\" \"constant\" \"color value\" ["<< r << " " << g << " " << b << "]\n";
                f << "Texture \"Ks1-" << sname << "\" \"color\" \"constant\" \"color value\" ["<< spr <<" "<< spg <<" "<<spb<<"]\n";
                f << "Texture \"Ks2-"<< sname <<"\" \"color\" \"constant\" \"color value\" ["<<spr2<<" "<<spg2<<" "<<spb2<<"]\n";
                f << "Texture \"Ks3-"<< sname <<"\" \"color\" \"constant\" \"color value\" ["<<spr2<<" "<<spg2<<" "<<spb2<<"]\n";
                f << "Texture \"R1-" << sname <<"\" \"float\" \"constant\" \"float value\" [1.000]\n";
                f << "Texture \"R2-" << sname << "\" \"float\" \"constant\" \"float value\" [0.094]\n";
                f << "Texture \"R3-" << sname << "\" \"float\" \"constant\" \"float value\" [0.170]\n";
                f << "Texture \"M1-" << sname << "\" \"float\" \"constant\" \"float value\" [0.150]\n";
                f << "Texture \"M2-" << sname << "\" \"float\" \"constant\" \"float value\" [0.043]\n";
                f << "Texture \"M3-" << sname << "\" \"float\" \"constant\" \"float value\" [0.020]\n";
                ret=6; // carpaint
                */
            } else if (vMatID==L"mia_material_phen") {
                // arch vis
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
                mRough=(float)s.GetParameterValue(L"reflect_glossy");
                float refl=(float)s.GetParameterValue(L"reflectivity");
                float brdf=(float)s.GetParameterValue(L"brdf_0_degree_refl");


                if (refl>0) {
                    shaderType=L"shinymetal";
                    shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] ";
                    shaderString += L" \"float uroughness\" ["+ CString(1-mRough)+L"] \"float vroughness\" ["+CString(1-mRough)+L"]\n";
                    shaderString += L" \"color Ks\" [" + CString(refl*sp_red*brdf) + L" " + CString(refl*sp_green*brdf) + L" " + CString(refl*sp_blue*brdf) + L"]\n";
                } else {
                    shaderType=L"matte";
                    s.GetColorParameterValue(L"kd",red,green,blue,alpha );
                    shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                    shaderString += L"  \"float sigma\" [" + CString(mRough) + L"]\n";
                }
            } else if (vMatID==L"material-phong") {
                shaderType=L"glossy";
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
                shaderString += L"  \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]";
                shaderString += L"  \"float vroughness\" ["+CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]\n";
                s.GetColorParameterValue(L"specular",red,green,blue,alpha );
                shaderString += L"  \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            }  else if (vMatID==L"material-lambert"){
                shaderType=L"matte";
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderString += L"  \"float sigma\" [0]\n";
            } else if (vMatID==L"material-ward"){
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                shaderType=L"glossy";
                shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
                shaderString += L"  \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_u"))/10)+ L"]";
                shaderString += L"  \"float vroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_v"))/10)+ L"]\n";
                shaderString += L"  \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            } else if (vMatID==L"material-constant"){
                s.GetColorParameterValue(L"color",red,green,blue,alpha );
                shaderType=L"matte";
                shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderString += L"  \"float sigma\" [0]\n";
            } else if (vMatID==L"material-strauss"){
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                shaderType=L"matte";
                shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderString += L"  \"float sigma\" [0]\n";
            } else {
                // fall back shader
                shaderType=L"matte";
                shaderString += L"  \"color Kd\" [0.7 0.7 0.7]\n";
                shaderString += L"  \"float sigma\" [0]\n";
            }
        }
        //-- search for images
        CRefArray vImags=m.GetShaders();
        for (int i=0;i<vImags.GetCount();i++){
            CRefArray vImags2=Shader(vImags[i]).GetShaders();
            for (int j=0;j<vImags2.GetCount();j++){
                CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
                if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1"){
                    vTexture=vImags2[j];

                    ImageClip2 vImgClip(vTexture.GetImageClip() );
                    Source vImgClipSrc(vImgClip.GetSource());
                    CString vFileName = vImgClipSrc.GetParameterValue( L"path");

                    CString textproj  = vTexture.GetParameterValue(L"tspace_id");
                    app.LogMessage(CString(textproj.GetAsciiString())); // TODO; search "type" projection

                if ((bool)vTexture.GetParameterValue(L"bump_inuse")!=false) {
                    vNorm = true; vChanel = L"bumpmap"; vChanType = L"float";
                        texFact = vTexture.GetParameterValue(L"factor");
                } else {
                        vText=true; vChanel = L"Kd"; vChanType = L"color";
                            s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                                texFact = L" 0.8 0.8 0.8 "; //provisional
                                //texFact = L" "+ CString(red) + L" " + CString(green) + L" " + CString(blue) + L" "; // not work
                                }

                    //----/ rewrite all /-------->
                f << "\nTexture \"" << sname << "::"<< vChanel.GetAsciiString() <<"\" \""<< vChanType.GetAsciiString() <<"\" \"imagemap\" \n";
                f << "  \"string wrap\" [\"repeat\"] \n";//TODO; create option at spdl shader
                //f << "    \"string chanel\" [\"mean\"] \n"; // not work??
                f << "  \"string filename\" [\"" << replace(vFileName.GetAsciiString()) << "\"] \n";
                f << "  \"float gamma\" ["<< vContrast <<"]\n";
                f << "  \"float gain\" [1.000000]\n";
                f << "  \"string filtertype\" [\"bilinear\"] \n";// TODO; create option
                f << "  \"string mapping\" [\"uv\"] \n";// TODO; create option / search data in XSI
                f << "  \"float vscale\" [-1.0]\n";
                f << "  \"float uscale\" [1.0] \n";
                f << "  \"float udelta\" [0.000000] \n";
                f << "  \"float vdelta\" [1.000000] \n";
                f << "Texture \""<< sname << "::"<< vChanel.GetAsciiString() <<".scale""\" \""<< vChanType.GetAsciiString() <<"\" \"scale\" ";
                f << "\"texture tex1\"  [\""<< sname << "::"<< vChanel.GetAsciiString() <<"\"]  \""<< vChanType.GetAsciiString() <<" tex2\" ["<< texFact.GetAsciiString() <<"] \n";
                shaderTexture += L"  \"texture "+ vChanel + L"\" [\""+ m.GetName().GetAsciiString() + L"::"+ vChanel + L".scale\"]\n";
                    vNorm=false;

                    //vText=true;
                }
            }
        }

        //
        // write shader block
        //

        f << "\n MakeNamedMaterial \""<< m.GetName().GetAsciiString() << "\" \n";
        f << "  \"string type\" [\""<< shaderType.GetAsciiString() <<"\"]\n";
        if (shaderTexture !=L""){
        f << shaderTexture.GetAsciiString();
        } else {
        f << shaderString.GetAsciiString();
        }

    }
}


//--
int writeLuxsiObj(X3DObject o, CString vType){
    //
    // Writes objects
    //
    CScriptErrorDescriptor status ;
    CValueArray fooArgs(1) ;
    fooArgs[0] = L"" ;
    CValue retVal=false ;
    CDoubleArray point_count;
    CFloatArray normals;
    CLongArray tri_count,nod,out;
    bool vIsMeshLight=false;
    bool vIsSet=false;
    bool vText=false,vIsSubD=false;
    bool vIsMod=false;

    Geometry g(o.GetActivePrimitive().GetGeometry()) ;
    CTriangleRefArray t( g.GetTriangles() );
    CRefArray mats(o.GetMaterials()); // Array of all materials of the object
    Material m=mats[0];
    CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
    Shader s(shad[0]);
    CGeometryAccessor ga;
    CValue vObjW,vObjType,vObjSh2,vObjSh3,vObjSh;
    CString vUV=L"",vNormals=L"",vTris=L"",vMod=L"",vPoints=L"";
    CFacetRefArray facets( g.GetFacets() );

    //----//----------->
    CGeometryAccessor in_ga;
    CString wFaceIdx;
    CString wVertex;
    CString wNorVect;
    CString wNorIdx;
    CString wUvVect;
    //----//----------->

    //LONG vSubDValue=0;
    LONG subdLevel=0; // my code

    Property geopr=o.GetProperties().GetItem(L"Geometry Approximation");
    if ((int)geopr.GetParameterValue(L"gapproxmordrsl")>0 || (int)geopr.GetParameterValue(L"gapproxmosl")>0) {
        vIsSubD=true;
        if ((int)geopr.GetParameterValue(L"gapproxmordrsl")>0) {//
            subdLevel=(int)geopr.GetParameterValue(L"gapproxmordrsl");// modif..
        } else {
            subdLevel=(int)geopr.GetParameterValue(L"gapproxmosl");//
        }
    } else {
        vIsSubD=false;
    }
    //----/ my modifications /------>
    siConstructionMode constmode = (siConstructionModeModeling);
    siSubdivisionRuleType subdmode = (siCatmullClark);
    //----/ added /------>

    ga = PolygonMesh(g).GetGeometryAccessor();
    ga.GetVertexPositions(point_count);
    ga.GetTriangleVertexIndices(tri_count);
    ga.GetTriangleNodeIndices(nod);
    ga.GetNodeNormals(normals);

    CLongArray pvCount;
    CLongArray vIndices;

    ga.GetTriangleNodeIndices(vIndices);
    ga.GetPolygonVerticesCount(pvCount);

    long nPolyCount = ga.GetPolygonCount();

    CTransformation localTransformation = ga.GetTransform();
    KinematicState  gs = o.GetKinematics().GetLocal();
    CTransformation gt = gs.GetTransform();
    CMatrix4 mat4(gt.GetMatrix4());

    CVector3 axis;
    double rot = gt.GetRotationAxisAngle(axis);
    //----/ my code for subd. meshes /-------------->
// implementado del ejemplo del XSISDK, import-export

//----//write polygon face_indices -gettrianglecount//--------->
    in_ga = PolygonMesh(g).GetGeometryAccessor(constmode, subdmode, subdLevel);
    CLongArray triVtxIdxArray;
    in_ga.GetTriangleVertexIndices(triVtxIdxArray);
        CLongArray triSizeArray(in_ga.GetTriangleCount());
    //----/ triangulate polygons /----->
    CLongArray numVertArray;
    in_ga.GetPolygonVerticesCount(numVertArray);
    //----/ end /-------->
        for (LONG i=0, offset=0; i<triSizeArray.GetCount(); i++)
        {
            wFaceIdx += L" "+ CString(triVtxIdxArray[offset]);
            wFaceIdx += L" "+ CString(triVtxIdxArray[offset+1]);
            wFaceIdx += L" "+ CString(triVtxIdxArray[offset+2]) + L" \n ";
            offset += 3;
        if (numVertArray==4){
            wFaceIdx += L" "+ CString(triVtxIdxArray[offset+0]);
            wFaceIdx += L" "+ CString(triVtxIdxArray[offset+2]);
            wFaceIdx += L" "+ CString(triVtxIdxArray[offset+3]) + L" \n ";
            offset += 3 ;
            }
        }

//----// polygon points_vectors positions -getvertexcount //----------->

    CDoubleArray vtxPosArray;
    in_ga.GetVertexPositions(vtxPosArray);
    LONG nVals = vtxPosArray.GetCount()/3;

    for ( LONG i=0, nCurrent=0; i<nVals; i++ )
    {
        wVertex += L" "+ CString(vtxPosArray[nCurrent]);
        wVertex += L" "+ CString(vtxPosArray[nCurrent+1]);
        wVertex += L" "+ CString(vtxPosArray[nCurrent+2]) + L" \n ";
        nCurrent += 3;
    }
//----//polygon node normal_vectors -getnodecount //------------->.GetNodeIndices
     CFloatArray nodeArray; // original
     in_ga.GetNodeNormals(nodeArray); // original

    LONG norVals = nodeArray.GetCount()/3;

    for ( LONG i=0, normCurrent=0; i<norVals; i++ )
    {
        wNorVect += L" "+ CString(nodeArray[normCurrent]);
        wNorVect += L" "+ CString(nodeArray[normCurrent+1]);
        wNorVect += L" "+ CString(nodeArray[normCurrent+2]) + L" \n ";
        normCurrent += 3;
    }
    //----// normal_indices -gettrianglecount //---------->
    CLongArray triNodeIdxArray;
    in_ga.GetTriangleNodeIndices(triNodeIdxArray);

    for (LONG i=0, offset=0; i<triSizeArray.GetCount(); i++)
    {
        wNorIdx += L" "+ CString(triNodeIdxArray[offset]);
        wNorIdx += L" "+ CString(triNodeIdxArray[offset+1]);
        wNorIdx += L" "+ CString(triNodeIdxArray[offset+2]) + L"> \n ";
        offset += 3;
    }
    //----// uv_vectors -getnodecount//---------->
    CRefArray uvs = in_ga.GetUVs();
    LONG nUVs = uvs.GetCount();
     // iterate over the uv data
        for ( LONG i=0; i<nUVs; i++ )
        {
            ClusterProperty uv(uvs[i]);
         // get the values
            CFloatArray uvValues;
            uv.GetValues( uvValues );

            // log the values
            LONG nValues = uvValues.GetCount();
            for ( LONG idxElem=0; idxElem<nValues; idxElem += 3)
            {
              wUvVect += L"   "+  CString(uvValues[idxElem]);
              wUvVect += L" "+ CString(uvValues[idxElem+1]) + L"\n";
             }
        }
//----/end to modif. /---->
    /////////////-------------->
    if (point_count.GetCount() > 0 || tri_count.GetCount() > 0 ) {
        f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
        if ((float)s.GetParameterValue(L"inc_inten")> 0 ) {
            // check for Meshlight
            vIsMeshLight=true;
            float red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f;
            s.GetColorParameterValue(L"incandescence",red,green,blue,alpha );
            CString lName = findInGroup(o.GetName());
            f << " LightGroup \"";
            if (lName!=L"") {
                f << lName.GetAsciiString();
            } else {
                f << (o.GetName()).GetAsciiString();
            }
            f << "\"\n";
            f << "\nAreaLightSource \"area\" \"integer nsamples\" [1] \"color L\" ["<<(red*(float)s.GetParameterValue(L"inc_inten")) <<" "<<(green*(float)s.GetParameterValue(L"inc_inten")) <<" "<<(blue*(float)s.GetParameterValue(L"inc_inten"))<<"] \"float gain\" [" << (float)s.GetParameterValue(L"inc_inten") << "] \n";

        } else {
            //shader = writeLuxsiShader(o);
            f << "\nNamedMaterial \""<< m.GetName().GetAsciiString() <<"\"\n";
        }
            // write triangles

            if (vType==L"instance") {
                f << "Identity\n";
            }
                f << "Translate " << gt.GetPosX() << " " << gt.GetPosY() << " "<< gt.GetPosZ() << "\n";
                if (rot!=0){
                f << "Rotate " << (rot*180/PI) << " "<< axis[0] << " " << axis[1] << " "<< axis[2] << "\n";
                }
                if (gt.GetSclX()==1 && gt.GetSclY()==1 && gt.GetSclZ()==1) {} else {
                    f << "Scale " << gt.GetSclX() << " " << gt.GetSclY() << " "<< gt.GetSclZ() << "\n";
                }
        //  }

            CTriangleRefArray triangles(g.GetTriangles()); // miga

            CLongArray indices( triangles.GetIndexArray() );

            CVector3Array allPoints(triangles.GetCount()*3);
            CVector3Array allUV(triangles.GetCount()*3);
            CVector3Array allNormals(triangles.GetCount()*3);

            long index=0;
            for (int i=0; i<triangles.GetCount();i++){
                Triangle triangle(triangles.GetItem(i));
                for (int j=0;j<triangle.GetPoints().GetCount();j++){
                    TriangleVertex vertex0(triangle.GetPoints().GetItem(j));
                    CVector3 pos(vertex0.GetPosition());
                    CVector3 normal(vertex0.GetNormal());
                    CUV uvs(vertex0.GetUV());


                    long arrayPos=index++;

                    allPoints[arrayPos] = pos;
                    allNormals[arrayPos] = normal;
                    allUV[arrayPos] = CVector3(uvs.u, uvs.v,0);
                    vTris += CValue(arrayPos).GetAsText()+L" ";

                    }
                vTris += L"\n";
            }

        //  app.LogMessage(L"count: "+CValue(allPoints.GetCount()).GetAsText());
            for (LONG j=0;j<allPoints.GetCount();j++){
                    vPoints +=  L" "+ CString(allPoints[j][0]) + L" "+  CString(allPoints[j][1]) + L" "+ CString(allPoints[j][2])+L"\n";
                    vUV +=  L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]) + L"\n";
                    vNormals +=  L" "+ CString(allNormals[j][0]) + L" "+  CString(allNormals[j][1]) + L" "+ CString(allNormals[j][2])+L"\n";
            }

            CRefArray vImags=m.GetShaders();
            for (int i=0;i<vImags.GetCount();i++){
                CRefArray vImags2=Shader(vImags[i]).GetShaders();
                for (int j=0;j<vImags2.GetCount();j++){
                    CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
                    if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1"){
                        vText=true;
                    }
                }
            }

            // write

            string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
            if (loc != string::npos) {
                sLight << " PortalShape ";
                sLight << " \"trianglemesh\"\n \"integer indices\" [\n";
                sLight << vTris.GetAsciiString();
                sLight << " ] \"point P\" [\n" ;
                sLight << vPoints.GetAsciiString();
                sLight << "] \"normal N\" [\n";
                sLight << vNormals.GetAsciiString();
                sLight << "]\n";
            } else {
                f << " Shape ";

                if (vIsSubD) {
                    f << "\"loopsubdiv\" \"integer nlevels\" ["<< subdLevel-1 <<"]\n";
                    f << "\"bool dmnormalsmooth\" [\""<< MtBool[vSmooth_mesh] <<"\"]\n";
                    f << "\"bool dmsharpboundary\" [\""<< MtBool[vSharp_bound] <<"\"]\n";
                    f << "\"integer indices\" [\n ";// integer indices
                } else {
                    f << " \"mesh\"\n";
                    f << "  \"integer nsubdivlevels\" [" << subdLevel <<"] \"string subdivscheme\" [\"loop\"] \n";// Not CatmullClark..??
                    f << "  \"bool dmnormalsmooth\" [\""<< MtBool[vSmooth_mesh] <<"\"]";
                    f << "  \"bool dmsharpboundary\" [\""<< MtBool[vSharp_bound] <<"\"] \n";// preserve edges [ or (
                    f << "  \"string acceltype\" [\""<< MtAccel[vAccel] <<"\"] \"string tritype\" [\"wald\"] \n";//TODO; create menu option type... "
                    f << "  \"integer triindices\" [\n ";// integer indices, integer quadindices
                 }
                if (vIsSubD) { // new code
                    f << wFaceIdx.GetAsciiString(); // vtris index, wquad index
                } else {
                    f << vTris.GetAsciiString(); //old code
                       }
                    f << " ] \"point P\" [\n " ;

                if (vIsSubD) {
                    f << wVertex.GetAsciiString(); // my code adapted from "SDK import-export demo"
                } else {
                    f << vPoints.GetAsciiString();
                       }
                f << "] ";
            if (vIsSubD) { //----//--------->
                    f << " \"normal N\" [\n " ;
                    f << wNorVect.GetAsciiString();
                    f << "] ";
                } else {
                    f << " \"normal N\" [\n " ;
                    f << vNormals.GetAsciiString();
                    f << "] ";
                }
                if(vText){
                    f << " \"float uv\" [\n";
                    f << vUV.GetAsciiString();
                    //f <<  wUvVect.GetAsciiString(); // not working correct
                    f << " ]\n";
                }

            }

        f << "\nAttributeEnd #" << o.GetName().GetAsciiString() << "\n";

    }

    return 0;
}

//--
int writeLuxsiCloud(X3DObject obj){
    //
    // Writes pointclouds
    //

    ICEAttribute attr;
    CICEAttributeDataArrayVector3f aPointPosition;
    CICEAttributeDataArrayLong aID;
    CICEAttributeDataArrayFloat aSize;
    CICEAttributeDataArrayVector3f aVel;

    CRefArray attrs = obj.GetActivePrimitive().GetGeometry().GetICEAttributes();

    for( int i = 0; i<attrs.GetCount(); i++ ) {
            ICEAttribute attr = attrs[i];
            /*
            xsi.LogMessage( L"*******************************************************************" );
            xsi.LogMessage( L"Name: " + attr.GetName() );
            xsi.LogMessage( L"DataType: " + CString(attr.GetDataType()) );
            xsi.LogMessage( L"StructType: " + CString(attr.GetStructureType()) );
            xsi.LogMessage( L"ContextType: " + CString(attr.GetContextType()) );
            xsi.LogMessage( L"IsConstant: " + CString(attr.IsConstant()) );
            xsi.LogMessage( L"Readonly: " + CString(attr.IsReadonly()) );
            xsi.LogMessage( L"AttributeCategory: " + CString(attr.GetAttributeCategory()) );
            xsi.LogMessage( L"Element count: " + CString(attr.GetElementCount()) );
            */
            if (attr.GetName() == L"PointPosition"){
                attr.GetDataArray(aPointPosition);
            }
            if (attr.GetName() == L"ID"){
                attr.GetDataArray(aID);
            }
            if (attr.GetName() == L"Size"){
                attr.GetDataArray(aSize);
            }
            if (attr.GetName() == L"PointVelocity"){
                attr.GetDataArray(aVel);
            }
        }



    for (unsigned int i=0;i<aPointPosition.GetCount();i++){
        // get all points

        f << "\nAttributeBegin #\"" << obj.GetName().GetAsciiString() << (int)aID[i] << "\" \n";
        f << "  Translate " << (float)aPointPosition[i][0] << " " << (float)aPointPosition[i][1] << " "<< (float)aPointPosition[i][2] << "\n";
        f << "  Material \"matte\" \"color Kd\" [0 0.8 0.8 ]\n";
        f << "  Shape \"sphere\" \"float radius\" [" << (float)aSize[i] << "]\n";
        f << "AttributeEnd #"<< obj.GetName().GetAsciiString()<<"\n";

    }

    return 0;
}


//--
int writeLuxsiInstance(X3DObject o){
    // instance

    //write source object [won't be displayed]

    Model m = Model(o).GetInstanceMaster();


    CRefArray vGroup = X3DObject(m).GetChildren();

    if ( find(aInstanceList, m.GetName() ) ) {
    } else {
        f << "\nObjectBegin \""<< m.GetName().GetAsciiString()<<"\"";
        for (int i=0;i<vGroup.GetCount();i++){
            writeLuxsiObj(X3DObject(vGroup[i]),L"instance");
        }
        f << "\nObjectEnd #"<< o.GetName().GetAsciiString()<<"\n";
        aInstanceList.Add(m.GetName());
    }

    f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
    KinematicState  gs = o.GetKinematics().GetGlobal();
    CTransformation gt = gs.GetTransform();
    CMatrix4 mat4(gt.GetMatrix4());

    CVector3 axis;
    double rot = gt.GetRotationAxisAngle(axis);
    f << "\nTranslate " << CString(gt.GetPosX()).GetAsciiString() << " " << CString(-gt.GetPosZ()).GetAsciiString() << " "<< CString(gt.GetPosY()).GetAsciiString() << "\n";
    if (rot!=0){
    f << "Rotate " << (rot*180/PI) << " "<< CString(axis[0]).GetAsciiString() << " " << CString(-axis[2]).GetAsciiString() << " "<< CString(axis[1]).GetAsciiString() << "\n";
    }
    if (gt.GetSclX()==1 && gt.GetSclY()==1 && gt.GetSclZ()==1) {} else {
        f << "Scale " << CString(gt.GetSclX()).GetAsciiString() << " " << CString(gt.GetSclZ()).GetAsciiString() << " "<< CString(gt.GetSclY()).GetAsciiString() << "\n";
    }
    f << "ObjectInstance \"" << Model(o).GetInstanceMaster().GetName().GetAsciiString() <<"\"\n";
    f << "AttributeEnd #" << o.GetName().GetAsciiString() << "\n\n";
    return 0;
}

//--
CString readIni(){

    char x;
    CString data;
    ifstream load;

    CString iniPath;

    iniPath = app.GetInstallationPath(siUserPath);
    //app.LogMessage(L"userdir:"+ iniPath);
    #ifdef __unix__
        iniPath += L"/LuXSI.ini";
    #else
        iniPath += L"\\LuXSI.ini";
    #endif
    //app.LogMessage(L""+iniPath);
    load.open( iniPath.GetAsciiString() );

    while(load.get(x)) {
      data+=x;
   }
   //app.LogMessage(L""+CString(data));
   load.close();
   return data;
}

//--
void write_header_files(){
    //-- commons header for files .lxm and .lxo
    f <<"\n# File created by Luxrender Exporter for Softimage; Luxsi. \n";
    f <<"# Copyright (C) 2010  Michael Gangolf \n";
    f <<"# Developed by Michael Gangolf, aka Miga, \n";
    f <<"# continued by P. Alcaide, aka povmaniaco. \n \n";
}
//--
void luxsi_write(){
    // write objects, materials, lights, cameras
    root= app.GetActiveSceneRoot();
    vIsLinux = CUtils::IsLinuxOS(); // linux check
    CScriptErrorDescriptor status ;
    CValueArray fooArgs(1) ;
    fooArgs.Clear();
    CValue retVal2="";

    // only write settings

    if (vFileObjects!=L""){

        CRefArray array,aObj,aLight,aCam,aSurfaces,aClouds,aInstance;
        sLight.str("");
        //--
        CStringArray emptyArray;
        emptyArray.Clear();
        array.Clear();
        aMatList.Clear();
        aInstanceList.Clear();

        aObj.Clear();
        aSurfaces.Clear();
        aLight.Clear();
        aCam.Clear();
        aClouds.Clear();
        aInstance.Clear();

        array += root.FindChildren( L"", L"", emptyArray, true );
        for ( int i=0; i<array.GetCount();i++ )
        {
            X3DObject o(array[i]);
            //app.LogMessage( L"\tObject name: " + o.GetName() + L":" +o.GetType() + L" parent:"+X3DObject(o.GetParent()).GetType());
            Property visi=o.GetProperties().GetItem(L"Visibility");
            // Collection objects
            if (o.GetType()==L"polymsh")
            {
                if (vIsHiddenObj || (vIsHiddenObj==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                {
                    aObj.Add(o);
                }
            }
            if (o.GetType()==L"CameraRoot")
            {
                if (vIsHiddenCam || (vIsHiddenCam==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                {
                    aCam.Add(o);    // visibilty check
                }
            }
            if (o.GetType()==L"camera" && X3DObject(o.GetParent()).GetType()!=L"CameraRoot")
            {
                if (vIsHiddenCam || (vIsHiddenCam==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                {
                    aCam.Add(o);    // visibilty check
                }
            }
            if (o.GetType()==L"light")
            {
                if (vIsHiddenLight || (vIsHiddenLight==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                {
                    aLight.Add(o);  // visibilty check
                }
            }
            if (o.GetType()==L"surfmsh")
            {
                if (vIsHiddenSurface || (vIsHiddenSurface==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                {
                    aSurfaces.Add(o);   // visibilty check
                }
            }
            if (o.GetType()==L"pointcloud")
            {
                if (vIsHiddenClouds || (vIsHiddenClouds==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                {
                    aClouds.Add(o); // visibilty check
                }
            }
            if (o.GetType()==L"#model")
            {   // model
                if (Model(o).GetModelKind()==2)
                {   // instances
                    if (vIsHiddenObj || (vIsHiddenObj==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))
                    {
                        aInstance.Add(o);   // visibilty check
                    }
                }
            }
        } //-- end for visibility check

        if (aObj.GetCount()==0)
        {
            app.LogMessage(L"No objects are exported! Check if objects are visible in XSI or switch on 'export hidden objects'",siFatalMsg );
        }
        else if (aCam.GetCount()==0)
        {
            app.LogMessage(L"No cameras are exported! Check if a camera is visible in XSI or switch on 'export hidden cameras'",siFatalMsg );
        }
        else
        {

//----/ create files /------------->

            CString vFileLXM = L"", vFileLXO = L"";
            CString vInput_FileName = vFileObjects.GetAsciiString();
            int Loc = (int)vInput_FileName.ReverseFindString(".");
            vFileLXM = vInput_FileName.GetSubString(0,Loc) + (L"_mat.lxm");
            vFileLXO = vInput_FileName.GetSubString(0,Loc) + (L"_geo.lxo");
            // vFileVOL = vInput_FileName.GetSubString(0,Loc) + (L"_vol.lxm");

            //-- init progress bar
            pb.PutValue(0);
            pb.PutMaximum( aObj.GetCount()+aInstance.GetCount() );
            pb.PutStep(1);
            pb.PutVisible( true );
            pb.PutCaption( L"Processing data for exporter.." );
            pb.PutCancelEnabled(true);

            //-- open lxs file
            f.open (vFileObjects.GetAsciiString()); //--

            // insert header for files
            write_header_files();

            //-- cam
            for (int i=0;i<aCam.GetCount();i++) writeLuxsiCam(aCam[i]);

            //-- basics values
            writeLuxsiBasics();

            f << "\nWorldBegin \n";

            //-- includes
            f << "\nInclude ""\""<< vFileLXM.GetAsciiString() <<"\" \n";
            f << "Include ""\""<< vFileLXO.GetAsciiString() <<"\" \n";

            f << "\nAttributeBegin \n";

            //-- lights
            for (int i=0;i<aLight.GetCount();i++) writeLuxsiLight(aLight[i]);

            //-- environment, test
            write_environment();

            f << "\nAttributeEnd \n \n";

            f << "WorldEnd";

            f.close(); //-- end lxs

            // open file _mat.lxm --->
            f.open (vFileLXM.GetAsciiString()); //--->

            //-- insert header for files
                write_header_files();

            //-- write materials
                writeLuxsiShader();

            f.close(); //--< end lxm

            // open file  _geom.lxo --->
            f.open(vFileLXO.GetAsciiString()); //--->

            //-- insert header for files
                write_header_files();

            //-- objects
            for (int i=0;i<aObj.GetCount();i++) {
                if (writeLuxsiObj(aObj[i],L"obj")==-1) break;
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- surfaces
            for (int i=0;i<aSurfaces.GetCount();i++) {
                if (writeLuxsiObj(aSurfaces[i],L"surface")==-1) break;
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- clouds
            for (int i=0;i<aClouds.GetCount();i++) {
                if (writeLuxsiCloud(aClouds[i])==-1) break;
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- instances
            for (int i=0;i<aInstance.GetCount();i++) {
                if (writeLuxsiInstance(aInstance[i])==-1) break;
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- close pogress bar
                pb.PutVisible( false );

            //-- close _geom file
                f.close();
            vExportDone=true;
        }
    }
    else
    {
     app.LogMessage(L"Filename is empty",siErrorMsg );
    }
}
//--
#if defined(_WIN32) || defined(_WIN64)
void loader(const char szArgs[]){
        //HANDLE hFile ;
        PROCESS_INFORMATION  pi;
        // start a program in windows
        STARTUPINFO  si = { sizeof(si) };
        CreateProcess(NULL, (LPSTR)szArgs, 0, 0, FALSE, 0, 0, 0, LPSTARTUPINFOA(&si), &pi);
    }
#endif
//--
void luxsi_execute(){

    if (vLuXSIPath!=L"")
    {
        if (vExportDone)
        {
            app.LogMessage(vLuXSIPath +L" " + vFileObjects );

            #ifdef __unix__
                 //if(!fork()) {
                     app.LogMessage(L"hier");
                     system("kedit");
                 //}

                //  exit(0);
                //}

            #else
                // win
                CString exec = vLuXSIPath +" \""+ vFileObjects + "\"";
                app.LogMessage(exec);
                loader(exec.GetAsciiString()); // mod. for execute into Windows
            #endif
        }
    }
    else
    {
        app.LogMessage(L"Select the Luxrender path",siErrorMsg );
    }
}