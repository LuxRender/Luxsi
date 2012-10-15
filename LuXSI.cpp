/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 - 2012  Michael Gangolf 
Code contributor ; Pedro Alcaide, aka povmaniaco

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


#include "include\luxsi_values.h"
#include "include\LuXSI.h"
//#include "plymesh/rply.h"
//--
using namespace XSI;
using namespace MATH;
using namespace std;


#define PI 3.14159265

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
CString findInGroup(CString s);
//-
void write_ply_object(X3DObject o, CString vFilePLY);

//--
XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
    in_reg.PutAuthor(L"Michael Gangolf");
    in_reg.PutName(L"LuXSI");
    in_reg.PutEmail(L"miga@migaweb.de");
    in_reg.PutURL(L"http://www.migaweb.de");
    in_reg.PutVersion(0,6);
    in_reg.RegisterProperty(L"LuXSI");
    in_reg.RegisterRenderer(L"LuxRender Renderer");
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

    prop.AddParameter( L"use_hidden_obj",   CValue::siBool, sps,L"",L"", vIsHiddenObj,      dft,dft,dft,dft, oParam );
    prop.AddParameter( L"use_hidden_light", CValue::siBool, sps,L"",L"", vIsHiddenLight,    dft,dft,dft,dft, oParam );
    prop.AddParameter( L"use_hidden_surf",  CValue::siBool, sps,L"",L"", vIsHiddenSurface,  dft,dft,dft,dft, oParam );
    prop.AddParameter( L"use_hidden_cam",   CValue::siBool, sps,L"",L"", vIsHiddenCam,      dft,dft,dft,dft, oParam );
    prop.AddParameter( L"smooth_mesh",      CValue::siBool, sps,L"",L"", vSmooth_mesh,      dft,dft,dft,dft, oParam );
    prop.AddParameter( L"sharp_bound",      CValue::siBool, sps,L"",L"", vSharp_bound,      dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bplymesh",         CValue::siBool, sps,L"",L"", vplymesh,          dft,dft,dft,dft, oParam );
    /**/
    prop.AddParameter( L"over_geo",         CValue::siBool, sps,L"",L"", overrGeometry,      dft,dft,dft,dft, oParam );
    
    
    //----/ image /-->
    prop.AddParameter( L"Width",        CValue::siInt4,  sps,L"",L"", vXRes,        0l,2048l,0l,1024l,  oParam);
    prop.AddParameter( L"Height",       CValue::siInt4,  sps,L"",L"", vYRes,        0l,2048l,0l,768l,   oParam);
    prop.AddParameter( L"gamma",        CValue::siFloat, sps,L"",L"", vContrast,    0,10,0,3,           oParam);
    //prop.AddParameter( L"progressive",  CValue::siBool,  sps,L"",L"", vProg,        dft,dft,dft,dft,    oParam);
    
    //-- filter //
    prop.AddParameter( L"bfexpert", CValue::siBool,     sps,L"",L"", vfexpert,  dft,dft,dft,dft, oParam);
    prop.AddParameter( L"bywidth",  CValue::siFloat,    sps,L"",L"", vYwidth,   0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bxwidth",  CValue::siFloat,    sps,L"",L"", vXwidth,   0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bfalpha",  CValue::siFloat,    sps,L"",L"", vFalpha,   0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"ssample",  CValue::siBool,     sps,L"",L"", vSupers,   dft,dft,dft,dft,      oParam);
    prop.AddParameter( L"bF_B",     CValue::siFloat,    sps,L"",L"", vF_B,      0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bF_C",     CValue::siFloat,    sps,L"",L"", vF_C,      0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bTau",     CValue::siFloat,    sps,L"",L"", vTau,      0.0f,10.0f,0.1f,2.0f, oParam);
    prop.AddParameter( L"bfilter",  CValue::siInt4,     sps,L"",L"", vfilter,   0,10,0,10,      oParam ) ;
    
    //-- volume integrator // bvolumeint
    prop.AddParameter( L"bvolumeint",   CValue::siInt4, sps,L"",L"", vvolumeint,   0,10,0,10,      oParam ) ;
    
    //-- Surfaceint : bsurfaceint, eye_depth, blight_depth, blight_str, binc_env, brrstrategy, beyerrthre,
    //-- blightrrthre, bmax_depth
    prop.AddParameter( L"bsurfaceint",  CValue::siInt4, sps,L"",L"",   vSurfaceInt,    0,10,0,10,      oParam );
    prop.AddParameter( L"bsexpert",     CValue::siBool, sps,L"",L"",   vsexpert,      dft,dft,dft,dft, oParam);

    //-- bi-directional
    prop.AddParameter( L"beye_depth",   CValue::siInt4, sps,L"",L"",    vEye_depth,     0,2048,0,48,    oParam );
    prop.AddParameter( L"blight_depth", CValue::siInt4, sps,L"",L"",    vLight_depth,   0,2048,0,48,    oParam );
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
    //--
    prop.AddParameter( L"bdiff_reflect_reject",    CValue::siBool, sps,L"",L"",     vdiff_reflect_reject,   dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdiff_refract_reject",    CValue::siBool, sps,L"",L"",     vdiff_refract_reject,   dft,dft,dft,dft, oParam ); 
    prop.AddParameter( L"bglossy_reflect_reject",  CValue::siBool, sps,L"",L"",     vglossy_reflect_reject, dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bglossy_refract_reject",  CValue::siBool, sps,L"",L"",     vglossy_refract_reject, dft,dft,dft,dft, oParam );

    //--
    prop.AddParameter( L"bdiff_reflect_reject_thr",   CValue::siFloat, sps,L"",L"", vdiff_reflect_reject_thr,   0.0f,10.0f,0.0f,10.0f, oParam ); 
    prop.AddParameter( L"bdiff_refract_reject_thr",   CValue::siFloat, sps,L"",L"", vdiff_refract_reject_thr,   0.0f,10.0f,0.0f,10.0f, oParam ); 
    prop.AddParameter( L"bglossy_reflect_reject_thr", CValue::siFloat, sps,L"",L"", vglossy_reflect_reject_thr, 0.0f,10.0f,0.0f,10.0f, oParam );
    prop.AddParameter( L"bglossy_refract_reject_thr", CValue::siFloat, sps,L"",L"", vglossy_refract_reject_thr, 0.0f,10.0f,0.0f,10.0f, oParam );

    //-- igi
    prop.AddParameter( L"bnsets",    CValue::siInt4,  sps,L"",L"",    vnsets,      0,1024,4,512,  oParam ) ;
    prop.AddParameter( L"bnlights",  CValue::siInt4,  sps,L"",L"",    vnlights,    0,1024,64,512, oParam ) ;
    prop.AddParameter( L"bmindist",  CValue::siFloat, sps,L"",L"",    vmindist,    0.0,1024.0f,0.1f,1024.0,  oParam ) ;
    //--
    //-- exphotonmap
    
    prop.AddParameter( L"brenderingmode",            CValue::siInt4, sps,L"",L"",  vrenderingmode,      0,10,0,10,  oParam ) ;
//  prop.AddParameter( L"bstrategy",                 CValue::siString, sps, L"", L"",
    prop.AddParameter( L"bshadowraycount",           CValue::siInt4, sps,L"",L"",  vshadowraycount,     0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxphotondepth",           CValue::siInt4, sps,L"",L"",  vmaxphotondepth,     0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxeyedepth",              CValue::siInt4, sps,L"",L"",  vmaxeyedepth,        0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxphotondist",            CValue::siFloat, sps,L"",L"", vmaxphotondist,      0.0f,1024.0f,0.0f,1024.0, oParam );
    prop.AddParameter( L"bnphotonsused",             CValue::siInt4, sps,L"",L"",  vnphotonsused,       0,50,0,50,  oParam ) ;
    prop.AddParameter( L"bindirectphotons",          CValue::siInt4, sps,L"",L"",  vindirectphotons,    0,200000,0,200000,      oParam ) ;
    prop.AddParameter( L"bdirectphotons",            CValue::siInt4, sps,L"",L"",  vdirectphotons,      0,10000000,0,10000000,  oParam ) ;
    prop.AddParameter( L"bcausticphotons",           CValue::siInt4, sps,L"",L"",  vcausticphotons,     0,20000,0,20000,        oParam ) ;
    prop.AddParameter( L"bradiancephotons",          CValue::siInt4, sps,L"",L"",  vradiancephotons,    0,200000,0,200000,      oParam ) ;
    prop.AddParameter( L"bfinalgather",              CValue::siBool, sps,L"",L"",  vfinalgather,        dft,dft,dft,dft,        oParam );
    prop.AddParameter( L"bfinalgathersamples",       CValue::siInt4, sps,L"",L"",  vfinalgathersamples, 0,1024,0,1024,          oParam );
    prop.AddParameter( L"bgatherangle",              CValue::siFloat, sps,L"",L"", vgatherangle,        0.0f,360.0f,0.0f,360.0, oParam );
    prop.AddParameter( L"bdistancethreshold",        CValue::siFloat, sps,L"",L"", vdistancethreshold,  0.0f,10.0f,0.0f,10.0,   oParam );
    prop.AddParameter( L"bdbg_enabledirect",         CValue::siBool, sps,L"",L"",  vdbg_direct,         dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableradiancemap",    CValue::siBool, sps,L"",L"",  vdbg_radiancemap,    dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableindircaustic",   CValue::siBool, sps,L"",L"",  vdbg_indircaustic,   dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableindirdiffuse",   CValue::siBool, sps,L"",L"",  vdbg_indirdiffuse,   dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bdbg_enableindirspecular",  CValue::siBool, sps,L"",L"",  vdbg_indirspecular,  dft,dft,dft,dft, oParam );
    
    //-- sppm items
    //-- "bmaxeyedepht", "bmaxphoton", "bpointxpass", "bphotonsxpass", "bstartradius", "balpha", "bdlsampling", "bincenvironment" };
    
    prop.AddParameter( L"bmaxeyedepht",     CValue::siInt4, sps,L"",L"",  vbmaxeyedepht,    0,50,0,50,              oParam ); 
    prop.AddParameter( L"bmaxphoton",       CValue::siInt4, sps,L"",L"",  vbmaxphoton,      0,50,0,50,              oParam ); 
    prop.AddParameter( L"bpointxpass",      CValue::siInt4, sps,L"",L"",  vbpointxpass,     0,50,0,50,              oParam );      
    prop.AddParameter( L"bphotonsxpass",    CValue::siInt4, sps,L"",L"",  vbphotonsxpass,   0,10000000,0,10000000,  oParam );
    prop.AddParameter( L"bstartradius",     CValue::siFloat, sps,L"",L"", vbstartradius,    0.0f,100.0f,0.0f,100.0, oParam );
    prop.AddParameter( L"balpha",           CValue::siFloat, sps,L"",L"", vbalpha,          0.0f,360.0f,0.0f,360.0, oParam );
    prop.AddParameter( L"bdlsampling",      CValue::siBool, sps,L"",L"",  vbdlsampling,     dft,dft,dft,dft,        oParam );
    prop.AddParameter( L"bincenvironment",  CValue::siBool, sps,L"",L"",  vbincenvironment, dft,dft,dft,dft,        oParam );

    //-- Sampler: bsampler, bmutation, bmaxrej, buservarian, bchainlength, bpixsampler, pixelsamples, vbasampler
    prop.AddParameter( L"bsampler",     CValue::siInt4,  sps,L"",L"", vSampler,             0,10,0,10,              oParam );
    prop.AddParameter( L"bmutation",    CValue::siFloat, sps,L"",L"", vlmutationpro,        0.0f,1.0f,0.0f,0.5f,    oParam );
    prop.AddParameter( L"bmaxrej",      CValue::siInt4,  sps,L"",L"", vmaxconsecrejects,    0l,2048l,0l,512l,       oParam );
    prop.AddParameter( L"buservarian",  CValue::siBool,  sps,L"",L"", vusevariance,         dft,dft,dft,dft,        oParam );
    prop.AddParameter( L"bchainlength", CValue::siInt4,  sps,L"",L"", vchainlength,         0,512,16,512,           oParam );
    prop.AddParameter( L"bpixsampler",  CValue::siInt4,  sps,L"",L"", vPixsampler,          0,512,16,512,           oParam );
    prop.AddParameter( L"pixelsamples", CValue::siInt4,  sps,L"",L"", vPixelsamples,        0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bbasampler",   CValue::siInt4,  sps,L"",L"", vbasampler,           0,10,0,10,  oParam ) ; // combo

    prop.AddParameter( L"bpresets",     CValue::siInt4, sps,L"",L"", vpresets,      0,10,0,10,      oParam ) ;

    //-- expert mode menu options
    prop.AddParameter( L"bexpert",      CValue::siBool, sps,L"",L"",  vExpert,      dft,dft,dft,dft, oParam);

    prop.AddParameter( L"savint",   CValue::siInt4, sps,L"",L"", vSave,     0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"disint",   CValue::siInt4, sps,L"",L"", vDis,      0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"hSpp",     CValue::siInt4, sps,L"",L"", vhaltspp,  0l,200l,0l,200l,    oParam);
    prop.AddParameter( L"hTime",    CValue::siInt4, sps,L"",L"", vhalttime, 0l,2000l,0l,2000l,  oParam);
    // unused--prop.AddParameter( L"AmbBack",  CValue::siBool, sps,L"",L"", vAmbBack,  dft,dft,dft,dft,    oParam);

    //- animation
    prop.AddParameter( L"bframestep",   CValue::siInt4, sps,L"",L"", vframestep,    0,100,0,100,  oParam );
    
    //--- save image options // 
    prop.AddParameter( L"save_png_16", CValue::siBool, sps,L"",L"", vWpng_16,   dft,dft,dft,dft, oParam);
    prop.AddParameter( L"png_gamut",   CValue::siBool, sps,L"",L"", vPng_gamut, dft,dft,dft,dft, oParam);
    prop.AddParameter( L"save_png",    CValue::siBool, sps,L"",L"", vPng,       dft,dft,dft,dft, oParam);
    prop.AddParameter( L"mode_rpng",   CValue::siInt4, sps,L"",L"", vRpng,      0,10,0,10,  oParam );
    //test int_values[L"mode_rpng"]=vRpng;
    //-- exr
    prop.AddParameter( L"mode_Znorm",  CValue::siInt4, sps,L"",L"", vExr_Znorm, 0,10,0,10,      oParam );
    prop.AddParameter( L"save_exr",    CValue::siBool, sps,L"",L"", vExr,       dft,dft,dft,dft, oParam);
    //-- tga
    prop.AddParameter( L"mode_rtga",   CValue::siInt4, sps,L"",L"", 1,          0,10,0,10,       oParam ); // vRtga
    prop.AddParameter( L"save_tga",    CValue::siBool, sps,L"",L"", vTga,       dft,dft,dft,dft, oParam);
    prop.AddParameter( L"tga_gamut",   CValue::siBool, sps,L"",L"", vTga_gamut, dft,dft,dft,dft, oParam);
    //--
    prop.AddParameter( L"brmode",   CValue::siInt4, sps,L"",L"", vRmode,        0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bengine",  CValue::siInt4, sps,L"",L"", vEngine,       0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bautode",  CValue::siBool, sps,L"",L"", vAutoTh,       dft,dft,dft,dft,    oParam);
    prop.AddParameter( L"bthreads", CValue::siInt4, sps,L"",L"", vThreads,      0,10,0,10,  oParam ) ;
    //-- Accelerator 
    prop.AddParameter( L"bAccel",              CValue::siInt4, sps,L"",L"",  vAccel,            0,10,0,10,      oParam );
    prop.AddParameter( L"bacexpert",           CValue::siBool, sps,L"",L"",  vacexpert,         dft,dft,dft,dft,    oParam);
    prop.AddParameter( L"bmaxprimsperleaf",    CValue::siInt4, sps,L"",L"",  vmaxprimsperleaf,  0,10,0,10,      oParam );
    prop.AddParameter( L"bfullsweepthreshold", CValue::siInt4, sps,L"",L"",  vfullsweepthr,     0,10,0,10,      oParam );
    prop.AddParameter( L"bskipfactor",         CValue::siInt4, sps,L"",L"",  vskipfactor,       0,10,0,10,      oParam );
    prop.AddParameter( L"btreetype",           CValue::siInt4, sps,L"",L"",  vtreetype,         0,10,0,10,      oParam );
    prop.AddParameter( L"bcostsamples",        CValue::siInt4, sps,L"",L"",  vcostsamples,      0,10,0,10,      oParam );
    prop.AddParameter( L"brefineimmediately",  CValue::siBool, sps,L"",L"",  vrefineinmed,      dft,dft,dft,dft,    oParam);
    prop.AddParameter( L"bmaxprims",           CValue::siInt4, sps,L"",L"",  vmaxprims,         0,10,0,10,      oParam );
    prop.AddParameter( L"bacmaxdepth",         CValue::siInt4, sps,L"",L"",  vacmaxdepth,       0,10,0,10,      oParam );
    prop.AddParameter( L"bemptybonus",         CValue::siFloat, sps,L"",L"", vemptybonus,       0.0f,1.0f,0.0f,1.0f,    oParam);
    prop.AddParameter( L"bintersectcost",      CValue::siInt4, sps,L"",L"",  vintersectcost,	0,100,0,100,    oParam );
    prop.AddParameter( L"btraversalcost",      CValue::siInt4, sps,L"",L"",  vtraversalcost,    0,100,0,100,    oParam );
    //--
    
    prop.AddParameter( L"resume",   CValue::siBool, sps,L"",L"",  vResume,  dft,dft,dft,dft,    oParam);
    prop.AddParameter( L"loglevel", CValue::siBool, sps,L"",L"",  luxdebug, dft,dft,dft,dft,    oParam);
 
    //- set default filename for scene and image out
    vFileExport = app.GetInstallationPath(siProjectPath);

    //- set path for LuxRender binarie file
    vLuXSIPath = app.GetInstallationPath(siUserAddonPath);

    //- for search exported .lxs file
    vblxs_file = app.GetInstallationPath(siProjectPath);
    //--
    #ifdef __unix__
        vFileExport += L"/tmp.lxs";
    #else
        vFileExport += L"/tmp.lxs"; //-- also work in windows systems ?
		
        vLuXSIPath += L"/LuXSI/Application/bin";
    #endif
    
    //-- lxs files for re-render  
    prop.AddParameter( L"blxs_file",    CValue::siString, sps, L"",L"", vblxs_file,    oParam);
    
	prop.AddParameter( L"fObjects",     CValue::siString, sps, L"", L"", vFileExport, oParam );

    prop.AddParameter( L"fLuxPath",     CValue::siString, sps, L"", L"", vLuXSIPath, oParam );

    return CStatus::OK;
}
//-- 
XSIPLUGINCALLBACK CStatus LuXSI_PPGEvent( const CRef& in_ctxt )
{
    //--
    ftime = DBL_MAX;
    //-
    PPGEventContext ctxt( in_ctxt ) ;
    PPGLayout lay = Context(in_ctxt).GetSource() ; // UNUSED

    PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;

    if ( eventID == PPGEventContext::siOnInit )
    {
        //CustomProperty 
        prop = ctxt.GetSource() ;
        //app.LogMessage( L"OnInit called for " + prop.GetFullName() ) ;

        ctxt.PutAttribute(L"Refresh",true);

        params = prop.GetParameters();
        for (int i=0;i<params.GetCount();i++)
        {
            // Update values on init
            Parameter param(params[i]);
            CString pname = param.GetScriptName();
            update_LuXSI_values(param.GetScriptName(), param, ctxt);
        }
    }
    else if ( eventID == PPGEventContext::siButtonClicked )
    {
        CValue buttonPressed = ctxt.GetAttribute( L"Button" );

        //-
        is_preview = false;
        //- bpreview
        if (buttonPressed.GetAsText()==L"blpreview")
        {
            //--
            is_preview = true;
            luxsi_mat_preview();
        }
        if (buttonPressed.GetAsText()==L"bre_render")
        {
            //-- en teoria funciona..
            luxsi_preview(vblxs_file);
        }
        if (buttonPressed.GetAsText()==L"exe_luxsi")
        {
            luxsi_write(ftime);
        }
        if (buttonPressed.GetAsText()==L"render_ani")
        {
            //-- test
            CRefArray projectProps = Application().GetActiveProject().GetProperties();
            Property playControl = projectProps.GetItem( L"Play Control" );
            int time = playControl.GetParameterValue( L"Current" );
            int time_start = playControl.GetParameterValue( L"In" );
            int time_end = playControl.GetParameterValue( L"Out" );

            //- Todo: create option menu for lqueue
            lqueue = true;
            //- reset queue_list
            queue_list.Clear();
            //-
            for ( int i = time_start; i < time_end; i += vframestep)
            {
                //-
                app.LogMessage(L" PLAY FRAME: "+ CString(i));
                
                //- frame
                ftime = i;
                luxsi_write(ftime);
                
                //- test MsgBox
                //long st = 3;
                //kit.MsgBox(L"Wait, render frame: "+ CString(time)+ L"in curse", siMsgOk, L"Warning!!", st);
            }
            /** si deseamos lanzar el render de la animacion inmediatamente
            *   aqui podriamos escribir el .lxq y llamar a luxsi_execute con el
            *   parametro -L "file.lxq"
            */
            int ext = int(vFileExport.ReverseFindString("."));
            //- for 'queue' files
            vFileQueue = vFileExport.GetSubString(0,ext) + L".lxq";
            //-
            f.open(vFileQueue.GetAsciiString());
            //-
            f << queue_list.GetAsciiString();
            f.close();
            //-
            luxsi_execute();
        }
        if (buttonPressed.GetAsText()==L"render_luxsi")
        {
            //-
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Write data to file LuxRender files");
            //-
            luxsi_write(ftime);
            //-
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Execute LuxRender..");
            //-
            luxsi_execute();
        }
        ctxt.PutAttribute(L"Refresh",true);
    }
    else if ( eventID == PPGEventContext::siTabChange )
    {
        CValue tabLabel = ctxt.GetAttribute( L"Tab" ) ;
        //app.LogMessage( L"Tab changed to: " + tabLabel .GetAsText() ) ;
    }
    else if ( eventID == PPGEventContext::siParameterChange )
    {
        Parameter changed = ctxt.GetSource() ;
        //CustomProperty 
        prop = changed.GetParent() ;
        CString paramName = changed.GetScriptName() ;

        app.LogMessage( L"Parameter Changed: " + paramName ) ;

        update_LuXSI_values(paramName, changed, ctxt);         
    }
    return CStatus::OK ;
}

//--
CVector3 convertMatrix(CVector3 v)
{
    CMatrix3 m2(1.0,0.0,0.0,  0.0,0.0,1.0,   0.0,-1.0,0.0);
    return v.MulByMatrix3InPlace (m2);
}

//--
void update_main_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    if (paramName == L"Width")   { vXRes     = changed.GetValue();
    } else if (paramName == L"Height")  { vYRes     = changed.GetValue();
    } else if (paramName == L"gamma")   { vContrast = changed.GetValue();
    
    //----/ hidden objects /------>
    } else if (paramName == L"use_hidden_obj")  { vIsHiddenObj      = changed.GetValue();
    } else if (paramName == L"use_hidden_surf") { vIsHiddenSurface  = changed.GetValue();
    } else if (paramName == L"use_hidden_cloud"){ vIsHiddenClouds   = changed.GetValue();
    } else if (paramName == L"use_hidden_cam")  { vIsHiddenCam      = changed.GetValue();
    } else if (paramName == L"use_hidden_light"){ vIsHiddenLight    = changed.GetValue(); // over_geo
    } else if (paramName == L"over_geo")        { overrGeometry     = changed.GetValue();

    //-- mesh export
    } else if (paramName == L"smooth_mesh")     { vSmooth_mesh  = changed.GetValue();
    } else if (paramName == L"sharp_bound")     { vSharp_bound  = changed.GetValue();
    } else if (paramName == L"bplymesh")        { vplymesh  = changed.GetValue();
   
    //-- save images /----/ tga /--->
    } else if (paramName == L"tga_gamut")   { vTga_gamut    = changed.GetValue();
    } else if (paramName == L"mode_rtga")   { vRtga         = changed.GetValue();
    } else if (paramName == L"save_tga")    
    {
        vTga = changed.GetValue();
        dynamic_luxsi_UI(changed, paramName, ctxt);

    //----/ save images /----/ exr /--->
    } else if (paramName == L"mode_Znorm")  { vExr_Znorm    = changed.GetValue();
    } else if (paramName == L"save_exr")
    {
        vExr = changed.GetValue();
        dynamic_luxsi_UI(changed, paramName, ctxt);

    //----/ save images /----/ png /--->
    } else if (paramName == L"mode_rpng")   { vRpng         = changed.GetValue();
    } else if (paramName == L"save_png_16") { vWpng_16      = changed.GetValue();
    } else if (paramName == L"png_gamut")   { vPng_gamut    = changed.GetValue();
    } else if (paramName == L"save_png")
    {
        vPng = changed.GetValue();
        dynamic_luxsi_UI(changed, paramName, ctxt);

    } else if (paramName == L"fObjects")    { vFileExport   = changed.GetValue();
    
    //- material preview
    } else if (paramName == L"blxs_file")   { vblxs_file    = changed.GetValue();
    }
    else 
    {
        update_general_values(paramName, changed, ctxt);
    }
}
//--
void update_general_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    if (paramName == L"brmode")      { vRmode        = changed.GetValue();
    } else if (paramName == L"bengine")     { vEngine       = changed.GetValue();
    } else if (paramName == L"bautode")     { vAutoTh       = changed.GetValue();
    } else if (paramName == L"bthreads")    { vThreads      = changed.GetValue();
    } else if (paramName == L"disint")      { vDis          = changed.GetValue();
    } else if (paramName == L"savint")      { vSave         = changed.GetValue();
    } else if (paramName == L"fLuxPath")    { vLuXSIPath    = changed.GetValue();
    
    //- animation / particles
    } else if (paramName == L"hSpp")        { vhaltspp      = changed.GetValue();
    } else if (paramName == L"hTime")       { vhalttime     = changed.GetValue();
    } else if (paramName == L"bframestep")  { vframestep    = changed.GetValue();
    } else if (paramName == L"resume")      { vResume       = changed.GetValue();
    } else if (paramName == L"loglevel")    { luxdebug      = changed.GetValue();
    }
    else
    {
        update_surfaceInt_values(paramName, changed, ctxt);
    }
}
//--
void update_surfaceInt_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    if (paramName == L"bsurfaceint") 
    { 
        vSurfaceInt = changed.GetValue();
        //-
        dynamic_surfaceInt_UI(changed, paramName, ctxt);
    } 
    else if (paramName == L"bsexpert")
    { 
        vsexpert = changed.GetValue();
        dynamic_surfaceInt_UI(changed, paramName, ctxt);
    }
    else if (paramName == L"blight_depth")  { vLight_depth  = changed.GetValue();
    } else if (paramName == L"bmaxdepth")   { vmaxdepth     = changed.GetValue();
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
    
    } else if (paramName == L"bdiff_reflect_reject")
    {
        vdiff_reflect_reject = changed.GetValue(); 
        dynamic_surfaceInt_UI(changed, paramName, ctxt);

    } else if (paramName == L"bdiff_refract_reject")
    {
        vdiff_refract_reject = changed.GetValue();
        dynamic_surfaceInt_UI(changed, paramName, ctxt);

    } else if (paramName == L"bglossy_reflect_reject")  
    {
        vglossy_reflect_reject = changed.GetValue();
        dynamic_surfaceInt_UI(changed, paramName, ctxt);

    } else if (paramName == L"bglossy_refract_reject")  
    { 
        vglossy_refract_reject = changed.GetValue();
        dynamic_surfaceInt_UI(changed, paramName, ctxt);

    } else if (paramName == L"bdiff_reflect_reject_thr")   { vdiff_reflect_reject_thr   = changed.GetValue(); 
    } else if (paramName == L"bdiff_refract_reject_thr")   { vdiff_refract_reject_thr   = changed.GetValue(); 
    } else if (paramName == L"bglossy_reflect_reject_thr") { vglossy_reflect_reject_thr = changed.GetValue();
    } else if (paramName == L"bglossy_refract_reject_thr") { vglossy_refract_reject_thr = changed.GetValue();

    //-- igi
    } else if (paramName == L"bnsets")      { vnsets     = changed.GetValue();
    } else if (paramName == L"bnlights")    { vnlights   = changed.GetValue();
    } else if (paramName == L"bmindist")    { vmindist   = changed.GetValue();
    
    //-- exphotonmap
    } else if (paramName == L"brenderingmode")      { vrenderingmode    = changed.GetValue();
//  } else if (paramName == L"bstrategy")           { vstrategy         = changed.GetValue();
    } else if (paramName == L"bshadowraycount")     { vshadowraycount   = changed.GetValue();
    } else if (paramName == L"bmaxphotondepth")     { vmaxphotondepth   = changed.GetValue();
    } else if (paramName == L"bmaxeyedepth")        { vmaxeyedepth      = changed.GetValue();// max eye ?
    } else if (paramName == L"bmaxphotondist")      { vmaxphotondist    = changed.GetValue();
    } else if (paramName == L"bnphotonsused")       { vnphotonsused     = changed.GetValue();
    } else if (paramName == L"bindirectphotons")    { vindirectphotons  = changed.GetValue();
    } else if (paramName == L"bdirectphotons")      { vdirectphotons    = changed.GetValue();
    } else if (paramName == L"bcausticphotons")     { vcausticphotons   = changed.GetValue();
    } else if (paramName == L"bradiancephotons")    { vradiancephotons  = changed.GetValue();
    } else if (paramName == L"bfinalgather")        
    {
        vfinalgather = changed.GetValue();
        dynamic_surfaceInt_UI(changed, paramName, ctxt);

    } else if (paramName == L"bfinalgathersamples") { vfinalgathersamples   = changed.GetValue();
    } else if (paramName == L"bgatherangle")        { vgatherangle          = changed.GetValue();
    } else if (paramName == L"bdistancethreshold")  { vdistancethreshold    = changed.GetValue();
    } else if (paramName == L"bdbg_enabledirect")   { vdbg_direct           = changed.GetValue();
    } else if (paramName == L"bdbg_enableradiancemap")  { vdbg_radiancemap  = changed.GetValue();
    } else if (paramName == L"bdbg_enableindircaustic") { vdbg_indircaustic = changed.GetValue();
    } else if (paramName == L"bdbg_enableindirdiffuse") { vdbg_indirdiffuse = changed.GetValue();
    } else if (paramName == L"bdbg_enableindirspecular"){ vdbg_indirspecular= changed.GetValue();
    //-- sppm
    // "bmaxeyedepht", "bmaxphoton", "bpointxpass", "bphotonsxpass", "bstartradius", "balpha", "bdlsampling", "bincenvironment"
    } else if (paramName == L"bmaxeyedepht")    { vbmaxeyedepht     = changed.GetValue();
    } else if (paramName == L"bmaxphoton")      { vbmaxphoton       = changed.GetValue();
    } else if (paramName == L"bpointxpass")     { vbpointxpass      = changed.GetValue();
    } else if (paramName == L"bphotonsxpass")   { vbphotonsxpass    = changed.GetValue();
    } else if (paramName == L"bstartradius")    { vbstartradius     = changed.GetValue();
    } else if (paramName == L"balpha")          { vbalpha           = changed.GetValue();
    } else if (paramName == L"bdlsampling")     { vbdlsampling      = changed.GetValue();
    } else if (paramName == L"bincenvironment") { vbincenvironment  = changed.GetValue();
    } 
    else
    {
        update_sampler_values(paramName, changed, ctxt);
    }
}
//--
void update_sampler_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    if (paramName == L"bsampler")
    {
        vSampler = changed.GetValue();
        dynamic_sampler_UI(changed, paramName, ctxt);
    } 
    else if (paramName == L"bexpert")
    {
        vExpert = changed.GetValue();
        //-
        dynamic_sampler_UI(changed, paramName, ctxt);
    }
    else if (paramName == L"bbasampler")
    {
        vbasampler = changed.GetValue();
        //-
        dynamic_sampler_UI(changed, paramName, ctxt);
    }
    else if (paramName == L"bmutation")    { vlmutationpro      = changed.GetValue();
    } 
    else if (paramName == L"bmaxrej")      { vmaxconsecrejects  = changed.GetValue();
    } 
    else if (paramName == L"buservarian")  { vusevariance       = changed.GetValue();
    } 
    else if (paramName == L"bchainlength") { vchainlength       = changed.GetValue();
    } 
    else if (paramName == L"bpixsampler")  { vPixsampler        = changed.GetValue();
    } 
    else if (paramName == L"pixelsamples") { vPixelsamples      = changed.GetValue();
    }
    else
    {
        update_filter_values(paramName, changed, ctxt);
    }
}
//--
void update_filter_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    //-- filter
    if (paramName == L"bfilter") 
    {
        vfilter = changed.GetValue();
        dynamic_filter_UI(changed, paramName, ctxt);
    } 
    else if (paramName == L"bfexpert")
    {
        vfexpert = changed.GetValue();
        dynamic_filter_UI(changed, paramName, ctxt);
    } 
    else if (paramName == L"bxwidth")   { vXwidth = changed.GetValue();
    } 
    else if (paramName == L"bywidth")   { vYwidth = changed.GetValue();
    } 
    else if (paramName == L"bfalpha")   { vFalpha = changed.GetValue();
    } 
    else if (paramName == L"ssample")   { vSupers = changed.GetValue();
    } 
    else if (paramName == L"bF_B")      { vF_B = changed.GetValue();
    } 
    else if (paramName == L"bF_C")      { vF_C = changed.GetValue();
    } 
    else if (paramName == L"bTau")      { vTau = changed.GetValue();
    } 
    else
    {
        update_accelerator_values(paramName, changed, ctxt);
    }
}
//--
void update_accelerator_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    //-- Accelerator
    if (paramName == L"bAccel")
    {
        vAccel = changed.GetValue();
        //-
        dynamic_Accel_UI(changed, paramName, ctxt);
    } 
    else if (paramName == L"bacexpert")
    {
        vacexpert = changed.GetValue();
        //-
        dynamic_Accel_UI(changed, paramName, ctxt);
    } 
    else if (paramName == L"bmaxprimsperleaf")    { vmaxprimsperleaf    = changed.GetValue();
    } 
    else if (paramName == L"bfullsweepthreshold") { vfullsweepthr       = changed.GetValue();
    } 
    else if (paramName == L"bskipfactor")         { vskipfactor         = changed.GetValue();
    } 
    else if (paramName == L"btreetype")           { vtreetype           = changed.GetValue();
    } 
    else if (paramName == L"bcostsamples")        { vcostsamples        = changed.GetValue();
    } 
    else if (paramName == L"brefineimmediately")  { vrefineinmed        = changed.GetValue();
    } 
    else if (paramName == L"bintersectcost")      { vintersectcost      = changed.GetValue();
    } 
    else if (paramName == L"btraversalcost")      { vtraversalcost      = changed.GetValue();
    } 
    else if (paramName == L"bmaxprims")           { vmaxprims           = changed.GetValue();
    } 
    else if (paramName == L"bacmaxdepth")         { vacmaxdepth         = changed.GetValue();
    } 
    else if (paramName == L"bemptybonus")         { vemptybonus         = changed.GetValue();
    }
    else 
    {
        dynamic_luxsi_UI(changed, paramName, ctxt);
    }
}
//--
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt )
{   
    //-- first of all, revise presets
    if ( changed.GetName() == L"bpresets" )
    { 
        luxsi_render_presets( ctxt);

        /** for update dynamic UI options
        */
        dynamic_surfaceInt_UI(changed, paramName, ctxt);
    }
    /** Begin the loop for update all values..
    *   TAB MAIN
    */
    update_main_values(paramName, changed, ctxt);

    /** ..and continue
    *   TAB RENDER
    *   Surface Integrators,
    *   Sampler,
    *   Filter,
    *   Accelerator,
    *   and the rest...
    */ 
}
//-- 
void input_presset(CString in_string, CValue in_value)
{
    Parameter(prop.GetParameters().GetItem( in_string )).PutValue( in_value );
}
//-
void luxsi_render_presets( PPGEventContext ctxt)
{
    //----------------------
    // status: need revision
    //----------------------
    int numparams = prop.GetParameters().GetCount();
    vpresets = prop.GetParameterValue( L"bpresets" );

    //-- commons values for all pressets 
    //--
    input_presset(L"hSpp", vhaltspp = 0);
    input_presset(L"hTime", vhalttime = 0 );
       
    //--
    if ( vpresets == 0 ) // Custom parameters
    {
        //-- TODO;
    }
    else if ( vpresets == 1 ) // Preview; Instant Global Illumination
    {
        vSampler = 2 ;
        vPixsampler = 3 ; // low
        vPixelsamples = 1 ;
        //-- distributepath
        vSurfaceInt = 3 ; 
        vLight_str = 2;
        // input_pressets(L"blight_str" )).PutValue( vLight_str = 2 );
        input_presset(L"directsampleall",          vdirectsampleall =  true );
        input_presset(L"bdirectsamples",           vdirectsamples = 1 );
        input_presset(L"bindirectsampleall",       vindirectsampleall = false );
        input_presset(L"bindirectsamples",         vindirectsamples = 1 );
        input_presset(L"bdiffusereflectdepth",     vdiffusereflectdepth = 3 );
        input_presset(L"bdiffusereflectsamples",   vdiffusereflectsamples = 1 );
        input_presset(L"bdiffuserefractdepth",     vdiffuserefractdepth = 5 );
        input_presset(L"bdiffuserefractsamples",   vdiffuserefractsamples = 1 );
        input_presset(L"bdirectdiffuse",           vdirectdiffuse = true );
        input_presset(L"bindirectdiffuse",         vindirectdiffuse = true );
        input_presset(L"bglossyreflectdepth",      vglossyreflectdepth = 2 );
        input_presset(L"bglossyreflectsamples",    vglossyreflectsamples = 1 );
        input_presset(L"bglossyrefractdepth",      vglossyrefractdepth = 5 );
        input_presset(L"bglossyrefractsamples",    vglossyrefractsamples = 1 );
        input_presset(L"bdirectglossy",            vdirectglossy = true );
        input_presset(L"bindirectglossy",          vindirectglossy = true );
        input_presset(L"bspecularreflectdepth",    vspecularreflectdepth = 3 );
        input_presset(L"bspecularrefractdepth",    vspecularrefractdepth = 5 );
        //-- volume int
        //-- filter / commons
    }
    else if ( vpresets == 2 ) // Preview; Directlighting ( No GI )
    {   
        //-- sampler; lowdiscrepance
        vSampler = 2 ;
        vPixsampler = 1 ;
        vPixelsamples = 2 ;
        vSurfaceInt = 2 ; //-- directlighting
        vmaxdepth = 5 ;
        //--
    }
    else if ( vpresets == 3 ) // Preview; Ex-photonmap
    {
        //- Sampler "lowdiscrepancy"
        vPixsampler = 4 ; //- hilbert
        vPixelsamples = 4 ;
        //-- SurfaceIntegrator "exphotonmap"
        vSurfaceInt = 5 ;
        input_presset(L"brenderingmode",    vrenderingmode = 0 ); // directlighting
        input_presset(L"blightstrategy",    vLight_str = 0 ); // auto
        input_presset(L"bshadowraycount",   vshadowraycount = 1 ); // ?
        input_presset(L"bmaxphotondepth",   vmaxphotondepth = 10 );
        input_presset(L"bmaxeyedepth",      vmaxeyedepth = 10 );
        //vmaxdepth = 5 ;
        input_presset(L"bmaxphotondist",    vmaxphotondist = 0.100000f );
        input_presset(L"bnphotonsused",     vnphotonsused = 50 );
        input_presset(L"bindirectphotons",  vindirectphotons = 200000 );
        input_presset(L"bdirectphotons",    vdirectphotons = 1000000 );
        input_presset(L"bcausticphotons",   vcausticphotons = 20000 );
        input_presset(L"bradiancephotons",  vradiancephotons = 20000 );
        input_presset(L"bfinalgather",      vfinalgather = false );
        // VolumeIntegrator "single"
        // Parameter(prop.GetParameters().GetItem( L"stepsize" [1.000000]
    }
    else if ( vpresets == 4 ) // final 1 MLT / Bi-directional PathTracing (int)
    {
        //-- sampler; metropolis 
        vSampler = 0 ;
        vlmutationpro = 0.6f ;
        vmaxconsecrejects = 512 ; 
        vusevariance = false ;
        //-- 
        vSurfaceInt = 0 ; // bidir
        vLight_depth = 32 ; 
        vEye_depth = 32 ;
        //--
    }
    else if ( vpresets == 5 ) // final 2 MLT / PathTracing (ext)
    {
        vSampler = 0 ;// metro
        vlmutationpro = 0.4f ;
        //--
        vSurfaceInt = 1 ; // path
        vmaxdepth = 10 ;
          
        input_presset( L"binc_env", vInc_env = true );
        //--
    }
    else if ( vpresets == 6 ) // progr 1 Bidir Path Tracing (int)
    {
        input_presset(L"bsampler", vSampler = 2);
        input_presset(L"bpixsampler", vPixsampler = 2 ); // 
        vPixelsamples = 1 ;
        //--
        vSurfaceInt = 2 ; //-- 
        vLight_depth = 10 ; 
        vEye_depth = 10 ; 
        //--
    }
    else if ( vpresets == 7 ) //  progr 2 Path Tracing (ext)
    {
        input_presset(L"bsampler", vSampler = 1);
        vchainlength = 512 ;
        vbasampler = 1 ;
        vPixsampler = 2 ; 
        vPixelsamples = 1 ;
        //-- surf
        vSurfaceInt = 1 ; // path
        vmaxdepth = 10 ;
        vInc_env = true ;
        //--
    }
    else if ( vpresets == 8 )// bucket 1 Bidir Path Tracing (int)
    {
        input_presset(L"bsampler", vSampler = 2) ; // low
        vPixsampler = 4 ; 
        vPixelsamples = 64 ;
        //--
        vSurfaceInt = 0 ; // bidir
        vLight_depth = 16 ; 
        vEye_depth = 16 ;
        //--
    }
    else // bucket 2 Path Tracing (ext)
    {
        input_presset(L"bsampler", vSampler = 2 ); // low
        vPixsampler = 4 ; //TODO; revise 
        vPixelsamples = 64 ;
        vSurfaceInt = 1 ; // path
        vsexpert = false ;
        vmaxdepth = 10 ; 
        vInc_env = true ;
        //-- filter - commons
    }
    //-- commons values for all presets
    //-- sampler
    input_presset(L"bsampler", vSampler );

    //-- if ( vsampler == low ) //-- TODO;
    input_presset(L"bpixsampler", vPixsampler );
    input_presset(L"pixelsamples", vPixelsamples );

    //-- if ( vsampler == metro ) //-- TODO;
    input_presset(L"bmutation",     vlmutationpro = 0.6f );
    input_presset(L"bmaxrej",       vmaxconsecrejects = 512 ); 
    input_presset(L"buservarian",   vusevariance = false );
            
    //-- filter
    input_presset(L"bfilter",   vfilter );
    //-- if filter is mitchell...
    input_presset(L"bxwidth",   vXwidth = 1.500000 );
    input_presset(L"bywidth",   vYwidth = 1.500000 );
    input_presset(L"bF_B",      vF_B = 0.3333f );
    input_presset(L"bF_C",      vF_C = 0.3333f );
    input_presset(L"ssample",   vSupers = true );

    //--  ...or  gaussian ? //-- TODO;
    //-- surface integrator
    input_presset(L"bsurfaceint",   vSurfaceInt );
    //-- direct
    input_presset(L"blightrrthre",  vLightRRthre ); 
    //-- path
    input_presset(L"bmaxdepth",     vmaxdepth = 10 );
    //-- bidirect
    input_presset(L"beye_depth",    vEye_depth );
    input_presset(L"blight_depth",  vLight_depth ); 
            
    //-- Accelerator; 'qbvh' 
    input_presset(L"bAccel", vAccel = 0 );
    input_presset(L"bskipfactor", vskipfactor = 1 );
    input_presset(L"bmaxprimsperleaf", vmaxprimsperleaf = 4 );
    input_presset(L"bfullsweepthreshold", vfullsweepthr = 16 );
    
         
    ctxt.PutArrayAttribute(L"Refresh", true);
    //-
    app.LogMessage(L" Parameters for render presets 9; loaded.."+ CString(vpresets));
    
  
//} //-- end cases...
}

//-- test OK, use helper
void show_params(CString in_item)
{
    Parameter(params.GetItem(in_item)).PutCapabilityFlag( siNotInspectable, false );
}
//-
void hide_params(CString in_item)
{
    Parameter(params.GetItem(in_item)).PutCapabilityFlag( siNotInspectable, true );
}
//--
void si_read(CString in_read)
{
    Parameter(params.GetItem(in_read)).PutCapabilityFlag( siReadOnly, false );
}
//--
void no_read(CString in_read)
{
    Parameter(params.GetItem(in_read)).PutCapabilityFlag( siReadOnly, true );
}
//-
void dynamic_surfaceInt_UI(Parameter changed, CString paramName, PPGEventContext ctxt)
{
    //-
    vSurfaceInt  = prop.GetParameterValue(L"bsurfaceint");
    vsexpert = prop.GetParameterValue(L"bsexpert");

    //--------------------------------
        
    const char *u_intgrator [66] = {/*bidirectional*/"blight_depth", "beye_depth", "beyerrthre", "blightrrthre",
        /*path*/"blight_str", "binc_env", "brrstrategy", "bmaxdepth", "brrcon_prob",
        /*ditributepath*/"bdirectsampleall", "bdirectsamples", "bindirectsampleall", "bindirectsamples", "bdiffusereflectdepth",
        "bdiffusereflectsamples", "bdiffuserefractdepth", "bdiffuserefractsamples", "bdirectdiffuse", "bindirectdiffuse",
        "bglossyreflectdepth", "bglossyreflectsamples", "bglossyrefractdepth", "bglossyrefractsamples", "bdirectglossy", 
        "bindirectglossy", "bspecularreflectdepth", "bspecularrefractdepth", "bdiff_reflect_reject_thr", "bdiff_refract_reject_thr", 
        "bglossy_reflect_reject_thr", "bglossy_refract_reject_thr", "bdiff_reflect_reject", "bdiff_refract_reject",
        "bglossy_reflect_reject", "bglossy_refract_reject", /*IGI*/"bnsets", "bnlights", "bmindist",
        /*exphotonmap*/"bstrategy", "bshadowraycount", "bmaxphotondepth", "bmaxeyedepth", "bmaxphotondist", "bnphotonsused",
        "bindirectphotons", "bdirectphotons", "bcausticphotons", "bradiancephotons", "bfinalgather", "brenderingmode",
        "bfinalgathersamples", "bgatherangle", "bdistancethreshold", "bdbg_enabledirect", "bdbg_enableradiancemap",
        "bdbg_enableindircaustic", "bdbg_enableindirdiffuse", "bdbg_enableindirspecular",/*sppm 8*/"bmaxeyedepht",
        "bmaxphoton", "bpointxpass", "bphotonsxpass", "bstartradius", "balpha", "bdlsampling", "bincenvironment" };
       
    //--
    for ( long in = 0; in < 66;)
    {
        hide_params(u_intgrator[in]);
        in++;
    }
    //--
    if ( vSurfaceInt == 0 ) //-- bidirectional
    {
        show_params(L"blight_depth");
        show_params(L"beye_depth");
        //--
        if ( vsexpert )
        {
            show_params(L"beyerrthre");
            show_params(L"blightrrthre");
        }
        ctxt.PutAttribute(L"Refresh", true );
    }
    else if ( vSurfaceInt == 1 ) //-- path
    {
        show_params(L"bmaxdepth");
        show_params(L"binc_env");
        show_params(L"brrstrategy");
        show_params(L"brrcon_prob");
        //--
        if ( vsexpert )
        {
            show_params(L"blight_str");
            show_params(L"blight_depth");
            show_params(L"bshadowraycount"); // dude, revised
        }
        ctxt.PutAttribute(L"Refresh", true );
    }
    else if ( vSurfaceInt == 2 ) //-- directlighting
    {
        show_params(L"bmaxdepth");
        //--
        if ( vsexpert )
        {
            show_params(L"bshadowraycount");
            show_params(L"blight_str");
        }
        ctxt.PutAttribute(L"Refresh", true );
    }
    else if ( vSurfaceInt == 3 ) //-- distributepath
    {
        /* 21 params show */
        const char *distributed_params [] = {
            "bdirectsampleall", "bdirectsamples", "bindirectsampleall",
            "bindirectsamples", "bdiffusereflectdepth", "bdiffusereflectsamples", "bdiffuserefractdepth",
            "bdiffuserefractsamples", "bdirectdiffuse", "bindirectdiffuse", "bglossyreflectdepth",
            "bglossyreflectsamples", "bglossyrefractdepth", "bglossyrefractsamples", "bdirectglossy",
            "bindirectglossy", "bspecularreflectdepth", "bspecularrefractdepth", "bdiff_reflect_reject_thr",
            "bdiff_refract_reject_thr", "bglossy_reflect_reject_thr", "bglossy_refract_reject_thr"};
        //-
        for ( unsigned int _distrib = 0; _distrib < 22;)
        {
            show_params(distributed_params[_distrib]);
            _distrib++;
        }
        //-
        show_params(L"bdiff_reflect_reject");
        no_read(L"bdiff_reflect_reject_thr");
        //-
        if ( vdiff_reflect_reject )// 
        {
            si_read(L"bdiff_reflect_reject_thr");
        }
        //--
        show_params(L"bdiff_refract_reject");
        no_read(L"bdiff_refract_reject_thr");
        //-
        if ( vdiff_refract_reject )
        {
            si_read(L"bdiff_refract_reject");
        }
        //--
        show_params(L"bglossy_reflect_reject");
        no_read(L"bglossy_reflect_reject_thr");
        //-
        if ( vglossy_reflect_reject )
        {
            si_read(L"bglossy_reflect_reject_thr");
        }
        //--
        show_params(L"bglossy_refract_reject");
        no_read(L"bglossy_refract_reject_thr");
        //--
        if ( vglossy_refract_reject )
        {
            si_read(L"bglossy_refract_reject_thr");
        }
        //--
        if ( vsexpert )
        {
            show_params(L"blight_str");
        }
        ctxt.PutAttribute(L"Refresh", true );
    }
    else if ( vSurfaceInt == 4 ) 
    {
        show_params(L"bnsets");
        show_params(L"bnlights");
        show_params(L"bmaxdepth");
        show_params(L"bmindist");
        //--
        if ( vsexpert )
        {
            show_params(L"blight_str");
        }
        ctxt.PutAttribute(L"Refresh", true );
    }
    else if ( vSurfaceInt == 5 )// ex-Photonmap
    {
        show_params(L"bmaxeyedepth");
        show_params(L"bmaxphotondepth");
        show_params(L"bdirectphotons");
        show_params(L"bcausticphotons");
        show_params(L"bindirectphotons");
        show_params(L"bradiancephotons");
        show_params(L"bnphotonsused");
        show_params(L"bmaxphotondist");
        show_params(L"bfinalgather");
        //--
        if ( vfinalgather )
        {
            show_params(L"bfinalgathersamples");
            show_params(L"bgatherangle");
        }
        show_params(L"brenderingmode");
        show_params(L"brrstrategy");
        show_params(L"brrcon_prob");
        //--
        if ( vsexpert )
        {
            show_params(L"bshadowraycount");
            show_params(L"blight_str");
            show_params(L"bdistancethreshold");
            show_params(L"bdbg_enabledirect");
            show_params(L"bdbg_enableradiancemap");
            show_params(L"bdbg_enableindircaustic");
            show_params(L"bdbg_enableindirdiffuse");
            show_params(L"bdbg_enableindirspecular");
        }
        ctxt.PutAttribute(L"Refresh", true );
    }
    else //- sppm ( wip )
    { 
        const char *sppm_items [] = {"bmaxeyedepht", "bmaxphoton", "bpointxpass", "bphotonsxpass",
            "bstartradius", "balpha", "bdlsampling", "bincenvironment"};
        //--
        for (int _sp = 0; _sp < 8;)
        {
            show_params(sppm_items[_sp]);
            _sp++;
        }
        //-
        if (vsexpert )
        {
            //--
        }
        //--
        ctxt.PutAttribute(L"Refresh", true );
    }
}
//--
void dynamic_sampler_UI( Parameter changed, CString paramName, PPGEventContext ctxt)
{
    vSampler = prop.GetParameterValue(L"bsampler");
    vExpert = prop.GetParameterValue(L"bexpert");
    vbasampler = prop.GetParameterValue(L"bbasampler");
    //-
    const char *hide_sampler[]={"bmutation", "bmaxrej", "buservarian",
        "bchainlength" , "bpixsampler", "pixelsamples" , "bbasampler"};
    //-
    for( int i = 0; i < 7;)
    {
        hide_params(hide_sampler[i]);
        i++;
    }
    //- create dynamic UI
    if ( vSampler == 0)  //-- metropolis
    {
        show_params(L"bmutation");
        show_params(L"buservarian");
        //-
        if (vExpert)
        {
            show_params(L"bmaxrej");
        }
        ctxt.PutAttribute(L"Refresh", true);
    }
    else if ( vSampler == 1 ) //-- erpt
    {
        show_params(L"bchainlength");
        show_params(L"bbasampler");
        //-
        if ( vbasampler > 0 ) //-- low / random
        {
            show_params(L"bpixsampler");
            show_params(L"pixelsamples"); 
        }
        else //-- metropolis, is unused ?
        {
            show_params(L"bmutation");
            show_params(L"buservarian");

            if ( vExpert )
            {
                show_params(L"bmaxrej");
            }
        }
        ctxt.PutAttribute(L"Refresh", true);
    }
    else  // vSampler is 2 or 3; lowdiscrepance / random
    {
        show_params(L"bpixsampler");
        show_params(L"pixelsamples");
        ctxt.PutAttribute(L"Refresh", true);
    }
}

//--
void dynamic_filter_UI( Parameter changed, CString paramName, PPGEventContext ctxt)
{
    vfilter = prop.GetParameterValue(L"bfilter");
    vfexpert = prop.GetParameterValue(L"bfexpert");
    
    //--- change form
    const char *ui_filter [7] = {"bxwidth", "bywidth", "bfalpha", "ssample", "bF_B", "bF_C", "bTau" };
    //--
    for ( long f = 0; f < 7;)
    {
        hide_params(ui_filter[f]);
        f++;
    }    
    //-- show with all options, if mode expert is true
    if ( vfexpert )
    {
        show_params(L"bxwidth");
        show_params(L"bywidth");
        //--
        ctxt.PutAttribute(L"Refresh", true);
    }

    if ( vfilter == 1 && vfexpert ) //-- gauss
    {
        show_params(L"bfalpha");
        //--
        ctxt.PutAttribute(L"Refresh", true);
    }
    else if ( vfilter == 2 && vfexpert ) //-- mitchell
    {
        show_params(L"ssample");
        show_params(L"bF_B");
        show_params(L"bF_C");
        //--
        ctxt.PutAttribute(L"Refresh", true);
    }
    else if ( vfilter == 3 && vfexpert ) //-- sinc
    {
        show_params(L"bTau");
        //--
        ctxt.PutAttribute(L"Refresh", true);
    }
    else //-- box. Use 'Refresh' for update UI from other options
    {
        ctxt.PutAttribute(L"Refresh", true); 
    }
}
//-
void dynamic_Accel_UI(Parameter changed, CString paramName, PPGEventContext ctxt)
{
    //-- Accelerator
    vAccel = prop.GetParameterValue(L"bAccel");
    vacexpert = prop.GetParameterValue(L"bacexpert");
    //--
    const char *ui_accel [9] = {/*qbvh*/ "bmaxprimsperleaf", "bfullsweepthreshold", "bskipfactor",
        /*bvh*/"bcostsamples", "bintersectcost", "btraversalcost", "bmaxprims", "bacmaxdepth", "bemptybonus" };
    //--
    for ( long ac = 0; ac < 9;)
    {
        hide_params(ui_accel[ac]);
        ac++;
    }
    //--
    if ( vAccel == 0 && vacexpert ) //-- qbvh
    {
        show_params(L"bmaxprimsperleaf");
        show_params(L"bfullsweepthreshold");
        show_params(L"bskipfactor");
        //-
        ctxt.PutAttribute(L"Refresh", true);
    }
    else if ( vAccel == 1 && vacexpert ) //-- bvh
    {
        //--        
        show_params(L"bcostsamples");
        show_params(L"bintersectcost");
        show_params(L"btraversalcost");
        show_params(L"bemptybonus");
        //-
        ctxt.PutAttribute(L"Refresh", true);
    }
    //else if (( vAccel == 2 )&& ( vsexpert )) //-- grid
    //{
    //    lbrefineimmediately.PutCapabilityFlag( siNotInspectable, false );
    //    ctxt.PutAttribute(L"Refresh", true );
    //}
    else
    {
        if ( vacexpert )//-- KD Tree
        {
            show_params(L"bintersectcost");
            show_params(L"btraversalcost");
            show_params(L"bemptybonus");
            show_params(L"bmaxprims");
            show_params(L"bacmaxdepth");
        }
        ctxt.PutAttribute(L"Refresh", true);
    }
}
//-
void dynamic_luxsi_UI( Parameter changed, CString paramName, PPGEventContext ctxt) 
{
    //--------------------
    // Status: for revised
    //--------------------
    
    //------------------------------------
    if ( changed.GetName() == L"save_tga")
    //------------------------------------
    {
        Parameter lmode_rtga = prop.GetParameters().GetItem( L"mode_rtga" );
        Parameter ltga_gamut = prop.GetParameters().GetItem( L"tga_gamut" );
        //--
        lmode_rtga.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false );
        ltga_gamut.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false );
        ctxt.PutAttribute(L"Refresh", true);
    }
    //------------------------------------
    if ( changed.GetName() == L"save_exr")
    //------------------------------------
    {
        Parameter lmode_Znorm = prop.GetParameters().GetItem( L"mode_Znorm" );
        //--
        lmode_Znorm.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false );
        ctxt.PutAttribute(L"Refresh", true);
    }
    //------------------------------------
    if ( changed.GetName() == L"save_png")
    //------------------------------------
    {
        Parameter lmode_rpng    = prop.GetParameters().GetItem( L"mode_rpng" ) ;
        Parameter lsave_png_16  = prop.GetParameters().GetItem( L"save_png_16" ) ;
        Parameter lpng_gamut    = prop.GetParameters().GetItem( L"png_gamut" ) ;
        //--
        lmode_rpng.PutCapabilityFlag( siNotInspectable,   changed.GetValue() == false ) ;
        lsave_png_16.PutCapabilityFlag( siNotInspectable, changed.GetValue() == false ) ;
        lpng_gamut.PutCapabilityFlag( siNotInspectable,   changed.GetValue() == false ) ;
        ctxt.PutAttribute(L"Refresh", true);
    }
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
    for (int i=0;i<vItems.GetCount();i++)
    {
        if (SIObject(vItems[i]).GetType()==L"LuXSI") 
        {
            vAlreadyThere=true;
            st = true;
            break;
        }
    }

    if (!vAlreadyThere)
    {
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
void writeLuxsiBasics()
{
    //
    // write basic scene description

    //-- convention names; prefix Mt;
    //----//------>
    const char *MtRRst [3] = { "none", "probability", "efficiency"};
    const char *ARmode [2] = { "GUI", "console"};
    const char *MtlightST [7] = { "one", "all", "auto", "importance", "powerimp", "allpowerimp", "logpowerimp"};
    const char *Aengine [2] = { "classic", "hybrid" };
    const char *ArgbT [3] = { "Y", "RGB", "RGBA" }; // iterator  vRtga
    const char *AZb_norm [3] = { "Camera Start/End clip", "Min/Max", "None" }; //exr
    const char *ArgbP [4] = { "Y", "YA", "RGB", "RGBA" }; // iterator  vRpng
    const char *aFilter [5] = { "box", "gaussian", "mitchell", "sinc", "triangle" }; // iter vfilter
    const char *aSampler [4] = { "metropolis", "erpt", "lowdiscrepancy", "random" }; // mode, Asampler [vSampler]
    const char *aPxSampler [5] = { "linear", "vegas", "lowdiscrepancy", "tile", "hilbert" };
    const char *MtSurf [7] = { "bidirectional", "path", "directlighting", "distributedpath", "igi", "exphotonmap", "sppm" };
    // moved // const char *MtRendering [2] = { "path", "directlighting" };
    //---
    std::string fname = vFileLxs.GetAsciiString();
    int loc=(int)fname.rfind(".");

    f << "\nFilm \"fleximage\"\n"; //----
    f << "  \"integer xresolution\" ["<< vXRes <<"]\n  \"integer yresolution\" ["<< vYRes <<"]\n";
    f << "  \"string filename\" [\""<< luxsi_replace(fname.substr(0,loc)) <<"\"]\n";

    f << "  \"integer writeinterval\" ["<< vSave <<"]\n";
    f << "  \"integer displayinterval\" ["<< vDis <<"]\n";

    f << "  \"bool write_exr\" [\""<< MtBool[vExr] <<"\"]\n";
    if (vExr)
    {
        f << "  \"string write_exr_zbuf_normalizationtype\" [\""<< AZb_norm[vExr_Znorm] <<"\"]\n";
    } //----/ TODO: more options for Exr /------>

    f << "  \"bool write_png\" [\""<< MtBool[vPng] <<"\"]\n";
    if (vPng)
    {
        f << "  \"string write_png_channels\" [\""<< ArgbP[vRpng] <<"\"]\n";
        f << "  \"bool write_png_16bit\" [\""<< MtBool[vWpng_16] <<"\"]\n";
        f << "  \"bool write_png_gamutclamp\" [\""<< MtBool[vPng_gamut] <<"\"]\n";
    } //-- options for Png

    f << "  \"bool write_tga\" [\""<< MtBool[vTga] <<"\"]\n";
    if (vTga)
    {
        f << "  \"string write_tga_channels\" [\""<< ArgbT[vRtga] <<"\"]\n";
        f << "  \"bool write_tga_gamutclamp\" [\""<< MtBool[vTga_gamut] <<"\"]\n";
    } //----/ TODO: more options for Tga /------>

    f << "  \"bool write_resume_flm\" [\""<< MtBool[vResume] <<"\"]\n";
    f << "  \"bool premultiplyalpha\" [\""<< MtBool[0] <<"\"]\n"; // TODO;
    f << "  \"integer haltspp\" ["<< vhaltspp <<"]\n";
    f << "  \"integer halttime\" ["<< vhalttime <<"]\n";

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
    //-------------------------------------------------
    f << "\nSampler \""<< aSampler[vSampler] <<"\"\n";
    //-------------------------------------------------

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
    

    //-------------------------------------------------------------
    f << "\nSurfaceIntegrator \""<< MtSurf[ vSurfaceInt ] <<"\"\n";
    //-------------------------------------------------------------
    if ( vSurfaceInt == 0 )  //-- bidi
    {
        f << "  \"integer eyedepth\" ["<< vEye_depth <<"]\n";
        f << "  \"integer lightdepth\" ["<< vLight_depth <<"]\n";
        if ( vsexpert )
        {
            f << "  \"float eyerrthreshold\" ["<< vEyeRRthre <<"]\n"; 
            f << "  \"float lightrrthreshold\" ["<< vLightRRthre <<"]\n"; 
        }
    }
    else if ( vSurfaceInt == 1 )//-- path
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
    else if ( vSurfaceInt == 2 ) //-- directlighting
    {
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        if ( vsexpert )
        {
            f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
            f << "  \"integer shadowraycount\" [" << vshadowraycount << "]\n";
        }
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
        //--
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
    else if ( vSurfaceInt == 5 ) //-- exphotonmap
    {
        f << "  \"integer maxdepth\" ["<< vmaxeyedepth <<"]\n"; // vmax eye depth ?
        f << "  \"integer maxphotondepth\" ["<< vmaxphotondepth <<"]\n";
        f << "  \"integer directphotons\" ["<< vdirectphotons <<"] \n";
        f << "  \"integer causticphotons\" ["<< vcausticphotons <<"] \n";
        f << "  \"integer indirectphotons\" ["<< vindirectphotons <<"] \n";
        f << "  \"integer radiancephotons\" ["<< vradiancephotons <<"] \n";
        f << "  \"integer nphotonsused\" ["<< vnphotonsused <<"] \n";
        f << "  \"float maxphotondist\" ["<< vmaxphotondist <<"]\n";
        f << "  \"bool finalgather\" [\""<< MtBool[ vfinalgather ] <<"\"]\n";
        //-
        if ( vfinalgather )
        {
            f << "  \"integer finalgathersamples\" ["<< vfinalgathersamples <<"] \n";
            f << "  \"float gatherangle\" ["<< vgatherangle <<"] \n";
        }
        if ( vsexpert )
        {
            //-- test
            const char *MtRendering [] = { "path", "directlighting" };
            //--
            f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
            f << "  \"integer shadowraycount\" [\""<< vshadowraycount <<"\"]\n"; 
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
    else // sppm
    {
        //- use default values.. atm!
        f << "  \"integer maxeyedepth\" ["<< vbmaxeyedepht <<"]\n"; 
	    f << "  \"integer maxphotondepth\" ["<< vbmaxphoton <<"]\n";
	    f << "  \"integer photonperpass\" ["<< vbphotonsxpass <<"]\n";
	    f << "  \"integer hitpointperpass\" ["<< vbpointxpass <<"]\n"; 
	    f << "  \"float startradius\" ["<< vbstartradius <<"]\n";
	    f << "  \"float alpha\" ["<< vbalpha <<"]\n";
	    f << "  \"bool includeenvironment\" [\""<< MtBool[vbincenvironment] <<"\"]\n";
	    f << "  \"bool directlightsampling\" [\""<< MtBool[vbdlsampling] <<"\"]\n"; 
        //-
        if ( vsexpert )
        {
	        f << "  \"bool storeglossy\" [\"false\"]\n";
	        f << "  \"bool useproba\" [\"true\"]\n";
	        f << "  \"integer wavelengthstratificationpasses\" [8]\n";
	        f << "  \"string lookupaccel\" [\"hybridhashgrid\"]\n";
	        f << "  \"float parallelhashgridspare\" [1.000000000000000]\n";
	        f << "  \"string pixelsampler\" [\"hilbert\"]\n";
	        f << "  \"string photonsampler\" [\"halton\"]\n";
        }
    }

    //--------------------------------------------------
    f << "\nAccelerator \""<< MtAccel[vAccel] <<"\"\n";
    //--------------------------------------------------
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
//        f << "  \"bool refineimmediately\" [\""<< MtBool[vrefineinmed] <<"\"]\n";
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
void writeLuxsiCam(X3DObject o){
    //
    // write camera
    //

    X3DObject o2;
    Camera c;
    if (o.GetType()==L"camera") 
    {
        o2=o;  c=o;
    }
    else 
    {
        o2=o.GetChildren()[0];  c=o.GetChildren()[0];
    }

    // Operations to calculate look at position.
    CVector3 vnegZ(0,0,-1);
    //--
    vnegZ.MulByMatrix3InPlace(c.GetKinematics().GetGlobal().GetTransform(ftime).GetRotationMatrix3());
    vnegZ.NormalizeInPlace();
    vnegZ.ScaleInPlace((double) c.GetParameterValue(L"interestdist"));
    vnegZ.AddInPlace(c.GetKinematics().GetGlobal().GetTransform(ftime).GetTranslation());

    CTransformation local_transf = o2.GetKinematics().GetLocal().GetTransform(ftime);
    KinematicState  gs = o2.GetKinematics().GetGlobal();
    CTransformation gt = gs.GetTransform();
    CVector3 translation(local_transf.GetTranslation());
    
    X3DObject ci(o.GetChildren()[1]);
    CValue vCType=L"pinhole";
    float vFdist = 0.0, vLensr = 0.0, vFocal = 0;
    int vdof_mode = 0;

    CRefArray cShaders = c.GetShaders();
    for (int i=0;i<cShaders.GetCount();i++)
    {
        CString vCSID((Shader(cShaders[i]).GetProgID()).Split(L".")[1]);
        if ( luxdebug ) app.LogMessage(L" Lens shader in use: "+ CString(vCSID));
        //-
        if (vCSID==L"sib_dof") 
        {
            //-- Depth_of_field shader found
            vdof_mode = Shader(cShaders[i]).GetParameterValue(L"mode");
            //--
            if ( vdof_mode = 0 ) //- custom
            {
                /* custom_near_focus, custom_far_focus, custom_coc */
            }
            if ( vdof_mode = 1 ) //- auto
            {
                vFdist = Shader(cShaders[i]).GetParameterValue(L"auto_focal_distance");
            }
            if ( vdof_mode = 2 ) //- lens
            {
                /* len_focal_distance, len_focal_lenght, len_fstop, len_coc */
                vFdist = Shader(cShaders[i]).GetParameterValue(L"len_focal_distance");
            }
            //-- commons
            vLensr = Shader(cShaders[i]).GetParameterValue(L"strenght");        
        }
    }

    KinematicState  ci_gs = ci.GetKinematics().GetGlobal();
    CTransformation ci_gt = ci_gs.GetTransform(ftime);

    CVector3 tranlation(0,1,0);                                     //- time
    CTransformation target=o2.GetKinematics().GetGlobal().GetTransform(ftime).AddLocalTranslation(tranlation);
    CVector3 up(target.GetTranslation());
    float vfov;
    if ((int)c.GetParameterValue(L"fovtype")==1) 
    {
        // calculate the proper FOV (horizontal -> vertical)
        float hfov = (float)c.GetParameterValue(L"fov");
        vfov = float(2* atan(1/(float)c.GetParameterValue(L"aspect") * tan(hfov/2*PI/180))*180/PI);
    } 
    else
    {
        // keep vertical FOV
        vfov = (float)c.GetParameterValue(L"fov");
    }
    //--
    double x,y,z;
    vnegZ.Get( x,y,z );
    CVector3 new_pos = gt.GetTranslation();
    CVector3 new_pos_ci = ci_gt.GetTranslation();

    //--
    int camera_proj = c.GetParameterValue(L"proj");
    //--
    f << "LookAt "<< new_pos[0] <<" "<< -new_pos[2] <<" "<< new_pos[1] <<"\n";
    f << "       "<< new_pos_ci[0] <<" "<< -new_pos_ci[2] <<" "<< new_pos_ci[1] <<"\n";
    f << "       0 0 1 \n"; //<< CString(up.GetZ()).GetAsciiString() << "\n"; // not working correct
        
    if ( camera_proj == 1 )
    {
        f << "Camera \"perspective\" \n";
        f << "  \"float fov\" ["<< vfov <<"] \n";
        f << "  \"float lensradius\" ["<< vLensr/10 <<"] \n";
        f << "  \"float focaldistance\" ["<< vFdist <<"] \n";

        /* by dof

        1 "float fov" [49.134342077604479]
	    2 "float screenwindow" [-1.000000000000000 1.000000000000000 -0.562500000000000 0.562500000000000]
	    3 "bool autofocus" ["false"]
	    4 "float shutteropen" [0.000000000000000]
	    5 "float shutterclose" [0.041666666666667]

	    "float lensradius" [0.006250000106437]
	    "integer blades" [0]
	    "integer power" [0]
	    "string distribution" ["uniform"]

	    6 "float focaldistance" [7.000000000000000]
        
        //-- no dof
        1 "float fov" [49.134342077604479]
	    2 "float screenwindow" [-1.000000000000000 1.000000000000000 -0.562500000000000 0.562500000000000]
	    3 "bool autofocus" ["false"]
	    4 "float shutteropen" [0.000000000000000]
	    5 "float shutterclose" [0.041666666666667]
	    6 "float focaldistance" [7.000000000000000]
        */
    }
    else
    {
        //-- orthographic
       /*
        f << "Camera \"orthographic\" \n";
        "float screenwindow" [-3.657142877578735 3.657142877578735 -2.057142868638039 2.057142868638039]
	    "bool autofocus" ["false"]
	    "float shutteropen" [0.00000]
	    "float shutterclose" [0.0416]
	    f << "  \"float lensradius\" [" << vLensr << "] \n";
        */
    }
}

//-- work in progress
int writeLuxsiSurface(X3DObject o, CString vType)
{
    //-- WIP: lack a lots of updates..
    //- or use other methode (like Yafxsi :)

    Geometry g(o.GetActivePrimitive().GetGeometry(ftime)) ; // add time
    CRefArray mats(o.GetMaterials()); // Array of all materials of the object
    Material m = mats[0];
    CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
    Shader s(shad[0]);
    //--
    KinematicState  gs = o.GetKinematics().GetLocal(); 
    CTransformation gt = gs.GetTransform(ftime); // add time
    
	//--
    float vradius = o.GetParameterValue(L"radius");
	//-	 
    float start_u = o.GetParameterValue(L"startuangle");
    float end_u = o.GetParameterValue(L"enduangle");
	//
	CVector3 axis(0.0f, 0.0f, 0.0f);
	float vphimax = 360;
	float rotar = 0.0;
	//-
	// el objeto a dibujar es la diferencia entre el mayor y el menor valor
	if (start_u > end_u)
	{
		if (start_u == 180)
		{
			vphimax = start_u + end_u;
		}
		else //if (start_u != 180)
		{
			vphimax = (360 - start_u) + end_u;
			rotar = end_u - start_u; // result negative
			axis.Set(0,0,1);
		}
	}
	else
	{
		vphimax = end_u - start_u;
		rotar = -vphimax;
		axis.Set(0,0,1);
	}
    f << "\nAttributeBegin\n";
    //CVector3 axis;
    //double rot = gt.GetRotationAxisAngle(axis);
    //-- TODO; changed for matrix
    f << "\nTransformBegin\n";
    f << "\nTranslate "<< gt.GetPosX() <<" "<< gt.GetPosY() <<" "<< gt.GetPosZ() <<"\n";
    //-
    if (rotar != 0)
    {
        f << "Rotate "<< rotar /*(rot*180/PI)*/ <<" "<< axis[0] <<" "<< axis[1] <<" "<< axis[2] <<"\n";
    }
    if (gt.GetSclX()!=1 || gt.GetSclY()!=1 || gt.GetSclZ()!=1) 
    {
        f << "Scale " << gt.GetSclX() << " " << gt.GetSclY() << " "<< gt.GetSclZ() << "\n";
    }    
	//-
    float end_v = o.GetParameterValue(L"endvangle");
	float start_v = o.GetParameterValue(L"startvangle");	
    //--
    f << " Shape  \"sphere\" \n";
    f << "  \"float radius\" [" << vradius << "]\n";
	f << "  \"float zmin\" [ -90 ]\n";
    f << "  \"float zmax\" [ 90 ]\n";
    f << "  \"float phimax\" ["<< vphimax <<"]\n";
    //--
    f << "TransformEnd\n";
    f << "\nAttributeEnd\n";
    return 0;
}
//-
CString writeLuxsiInstance(X3DObject o)
{
    //--------------------
    // status: in progress
    //--------------------
    //-- instance
    CString instanceData = L"";
    //- write source object [won't be displayed]
    Model vModel = Model(o).GetInstanceMaster();
    CRefArray vGroup = X3DObject(vModel).GetChildren();
    //-
    if ( luxsi_find(aInstanceList, vModel.GetName() ) ) 
    {
        if ( luxdebug ) app.LogMessage(L"The instance already exists");
    }
    else
    {
        CString obj_instance = L"";
        //-
        instanceData = L"\nObjectBegin \""+ vModel.GetName() + L"\"";
        //-
        for (int i=0; i < vGroup.GetCount(); i++)
        {
            obj_instance = writeLuxsiObj(X3DObject(vGroup[i]));
        }
        //- test
        instanceData += obj_instance;
        //- end
        instanceData += L"\nObjectEnd #"+ o.GetName() + L"\n";
        aInstanceList.Add(vModel.GetName());// is correct?
    }
    //-
    instanceData += L"\nAttributeBegin #"+ o.GetName();
    //--
    KinematicState global_state = o.GetKinematics().GetGlobal();
    CTransformation global_transf = global_state.GetTransform(ftime);
    CVector3 axis;
    double rot = global_transf.GetRotationAxisAngle(axis); 
    
    //-
    instanceData += L"\nTranslate "
        + CString( global_transf.GetPosX() )  + L" "
        + CString( -global_transf.GetPosZ())  + L" "
        + CString( global_transf.GetPosY() )  + L"\n";
    //--
    if (rot!= 0)
    {
        instanceData += L"Rotate "
            + CString( rot*180/PI) + L" "
            + CString( axis[0] )   + L" "
            + CString( -axis[2])   + L" "
            + CString( axis[1] )   + L"\n";
    }
    /** changed && to ||, add support for not uniform scale 
    */
    if (global_transf.GetSclX()!=1 || global_transf.GetSclY()!=1 || global_transf.GetSclZ()!=1)
    {
        //- !WARNING! change 'Y' for 'Z', but not negative ( -z).
        instanceData += L"Scale "
            + CString(global_transf.GetSclX()) + L" "
            + CString(global_transf.GetSclZ()) + L" "
            + CString(global_transf.GetSclY()) + L"\n";
    }

    instanceData += L"ObjectInstance \""+ Model(o).GetInstanceMaster().GetName() + L"\"\n";
    instanceData += L"AttributeEnd #"+ o.GetName() + L"\n\n";
    //-
    return instanceData;
}
//--
CString write_header_files()
{
    //-- commons header for files .lxm and .lxo
    CString _header = L"";
    _header += L"\n# Created by LuXSI; Luxrender Exporter for Autodesk Softimage \n";
    _header += L"# Copyright (C) 2010 - 2012 by Michael Gangolf aka Miga \n";
    _header += L"# Code contributor;    P. Alcaide, aka povmaniaco. \n \n";
    //-
    return _header;
}
//--
void luxsi_mat_preview()
{
    //--
    CString 
        vFile_scene_folder = L"",   //- this file is create a './resources' folder
        vfile_mat_preview = L"",    //- file name for exporter material data 
        mat_data_preview = L"";     //- container string for material data
    
    //--  base folder
    vFile_scene_folder = app.GetInstallationPath(siUserAddonPath);
    //--
    vfile_mat_preview = vFile_scene_folder + L"/LuXSI/resources/scene_preview_mat.lxm";
    //--
    if ( luxdebug ) app.LogMessage(L"File for material preview: "+ vfile_mat_preview);
   
    mat_data_preview = writeLuxsiShader();
    //-
    if ( luxdebug ) app.LogMessage(L"Data for Preview mat: "+ mat_data_preview);
    //--
    if ( luxsi_find(aMatList, L"Preview" ) )
    {
        std::ofstream fmat;
		fmat.open(vfile_mat_preview.GetAsciiString());
        fmat << "# Material preview for LuXSI";
        fmat << mat_data_preview.GetAsciiString();
        fmat.close();

        //- scene file
        vFile_scene_folder += L"/LuXSI/resources/scene.lxs";

        //- start rendering..
        luxsi_preview( vFile_scene_folder );
    }
    else
    {
        app.LogMessage(L"Not material named 'Preview'", siWarningMsg);
    }
}
//--
CString luxsi_normalize_path(CString vFile)
{
    //-- normalize path name
    CString normalized_path = luxsi_replace(vFile.GetAsciiString()).c_str();

    //- extract folder base for use 'relative path'
    int base = int(normalized_path.ReverseFindString("\\\\")); // .ext
    CString folder_base = normalized_path.GetSubString(0, base+2);

    //- extract filename
    CString file_path = normalized_path.GetSubString(base+2, normalized_path.Length());
    // file path is ==  file.ext
    //-
    if ( luxdebug )
    {
        app.LogMessage(L"Path normalized is: "+ folder_base + L" File name is: "+ file_path);
    }
    //- extract extension
    int ext = int(file_path.ReverseFindString("."));

    //- return only filename, without extension
    return file_path.GetSubString(0, ext);

}
//--
void luxsi_write(double ftime)
{
    // write objects, materials, lights, cameras
    if (vFileExport != L"")
    {
        CRefArray 
            itemsArray, //- array for all scene items
            aObj,       //- for objects( polygon mesh )
            aCam,       //- for cams ( only export active camera ? )
            aSurfaces,  //- for 'surface' primitives
            aClouds,    //- for 'pointclouds' objects
            aInstance,  //- for instance objects
            aPlymesh;   //- for exporter object in .ply format
            
        //--
        CStringArray emptyArray;
        //-
        emptyArray.Clear();
        //-
        itemsArray.Clear();
        aObj.Clear();
        aCam.Clear();
        aSurfaces.Clear();
        aClouds.Clear();
        aInstance.Clear();
        aPlymesh.Clear();

        //- cleaned global CStringArray  for instance list
        aInstanceList.Clear();
        //--
        if ( luxdebug ) app.LogMessage(L"[DEBUG]: Created and cleaned arrays..");

        root = app.GetActiveSceneRoot();
        //--
        itemsArray += root.FindChildren( L"", L"", emptyArray, true );
        for ( int i=0; i < itemsArray.GetCount(); i++ )
        {
            X3DObject o(itemsArray[i]);
            //app.LogMessage( L"\tObject Name: " + o.GetName() + L" Type: " + o.GetType() 
            //    + L" parent: "+ X3DObject(o.GetParent()).GetType());
            //--
            Property visi = o.GetProperties().GetItem(L"Visibility");
            bool view_visbl = (bool)visi.GetParameterValue(L"viewvis");
            bool rend_visbl = (bool)visi.GetParameterValue(L"rendvis");

            //-- Collection objects / visibilty check
            if (o.GetType()==L"polymsh")
            {
                if (vIsHiddenObj || (!vIsHiddenObj && ( view_visbl && rend_visbl )))
                {
                    aObj.Add(o); // for create link into _geo.lxo file
                    if ( vplymesh ) aPlymesh.Add(o); // for write .ply file
                }
            }
            if (o.GetType()==L"CameraRoot")
            {
                if (vIsHiddenCam || ( !vIsHiddenCam && (view_visbl && rend_visbl )))
                {
                    aCam.Add(o);    
                }
            }
            if (o.GetType()==L"camera" && X3DObject(o.GetParent()).GetType()!=L"CameraRoot")
            {
                if (vIsHiddenCam || (!vIsHiddenCam && (view_visbl && rend_visbl )))
                {
                    aCam.Add(o); 
                }
            }
            if (o.GetType()==L"surfmsh")
            {
                if (vIsHiddenSurface || (!vIsHiddenSurface && (view_visbl && rend_visbl )))
                {
                    aSurfaces.Add(o);
                   // if ( vplymesh ) aPlymesh.Add(o);
                }
            }
            if (o.GetType()==L"pointcloud")
            {
                if (vIsHiddenClouds || (!vIsHiddenClouds && (view_visbl && rend_visbl )))
                {
                    aClouds.Add(o); 
                }
            }
            if (o.GetType()==L"#model")
            {
                //- test for include object instance master
                if (Model(o).GetModelKind()==2 ) //|| Model(o).GetModelKind()==0)
                {   // instances
                    if (vIsHiddenObj || (!vIsHiddenObj && (view_visbl && rend_visbl )))
                    {
                        aInstance.Add(o);   
                    }
                }
            }
        } 
        //-- end for visibility check
        int vNumObj = aObj.GetCount() + aSurfaces.GetCount();

        if ( vNumObj == 0)
        {
            app.LogMessage(L"Any objects/surfaces to export! Check a list of 'Export hidden items..' in GUI", siFatalMsg );
        }
        else if (aCam.GetCount()==0)
        {
            app.LogMessage(L"Any camera to export! Check a visible cameras in scene or switch ON 'export hidden cameras'", siFatalMsg );
        }
        else
        {
            /** vFileExport is a base name path
            *   set by user from GUI.
            */
            vFileLxs.Clear();
            vFileLxs = vFileExport;
            //- default extension..
            int ext = 0;
            ext = int(vFileLxs.ReverseFindString("."));
            
            //- use only for exporter animation -----------------------------------------//
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: ftime is: "+ CString(ftime));
            CString vtime = L"";
            //-
            if ( ftime != DBL_MAX )
            {
                //- add frame number to outfile name
                vtime = CString(ftime);
                vFileLxs = vFileLxs.GetSubString(0, ext) + (L"_"+ vtime + L".lxs");
                //-
                if ( lqueue )
                {
                    queue_list += vFileLxs + L"\n";
                }
                if ( luxdebug ) app.LogMessage(L"OUT Filename: "+ vFileLxs );
            }//--------------------------------------------------------------------------//

            /** Setup name and extension for include files into .lxs file.
            *   luxsi_normalize_path() return filename + framenumber if exist,
            *   but not include the extension.
            */
            CString path_base = luxsi_normalize_path(vFileLxs);
            CString inc_LXM = path_base + L"_mat.lxm";  // material definitions
            CString inc_LXO = path_base + L"_geo.lxo";  // geometry definitions
            CString inc_LXV = path_base + L"_geo.lxv";  // volume definitions

            /** For animation, reset ext value to new filename.
            *   The lenght of new filename as change (name + frame).
            *   Not use normalized path.
            */
            ext = int(vFileLxs.ReverseFindString("."));
            //- add extension for material file..
            CString vFileLXM = vFileLxs.GetSubString(0, ext) + (L"_mat.lxm");
            //- add extension for geometry file..
            CString vFileLXO = vFileLxs.GetSubString(0, ext) + (L"_geo.lxo");
            //- add extension for volume file..
            CString vFileVOL = vFileLxs.GetSubString(0, ext) + L"_vol.lxv";

            /** For include link to PLY files into LXO file, setup global vFilePLY
            *   with vFileLxs value, for include frame number.
            *   Name of object and extension, is added later.
            */
            vFilePLY = vFileLxs;
            
            //- init progress bar
            pb.PutValue(0);
            pb.PutMaximum( aObj.GetCount()+ aInstance.GetCount() );
            pb.PutStep(1);
            pb.PutVisible( true );
            pb.PutCaption( L"Processing data for exporter.." );
            pb.PutCancelEnabled(true);

            //--
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Open files for write data "+ vFileLxs);
            //--

            CString _header = write_header_files();
            //-
            f.open(vFileLxs.GetAsciiString());

            //- insert header
            f << _header.GetAsciiString();

            //--
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Write Camera..");
            //--
            for (int i = 0; i < aCam.GetCount(); i++) writeLuxsiCam(aCam[i]);

            //-- basics values
            writeLuxsiBasics();

            f << "\nWorldBegin \n";

            //-- create includes for geometry and materials
            f << "\nInclude \""<< inc_LXM.GetAsciiString() <<"\" \n";
            f << "Include \""<< inc_LXO.GetAsciiString() <<"\" \n";

            f << "\nAttributeBegin \n";

            //-- lights
            writeLuxsiLight();

            f << "\nAttributeEnd \n \n";

            f << "WorldEnd";

            f.close(); //------------------------------------------------------- end lxs

            /** TEST NEW METHOD
            *   1) gathering data, 2) open file, 3)write data, 4)closed file
            *   This method reduces the time that the file is open for writing.
            */
            CString luxsi_Shader_Data = L"";
            luxsi_Shader_Data = writeLuxsiShader();
            
            //-- open file .lxm for write data
            f.open(vFileLXM.GetAsciiString()); 

            //-- insert header
            f << _header.GetAsciiString();
            
            //- insert shader data
            f << luxsi_Shader_Data.GetAsciiString();
            
            //->
            f.close(); //--< end lxm
            //-
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Writed materials file: "+ vFileLXM);

           
            //- Ghatering geometry data -------------------------------#
            //--
            CString geometryData = L"";
            for (int i = 0; i < aObj.GetCount(); i++) 
            {
                geometryData += writeLuxsiObj(aObj[i]);
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- instance data
            CString instanceData = L"";
            for (int i=0; i < aInstance.GetCount(); i++) 
            {
                instanceData += writeLuxsiInstance(aInstance[i]);
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //--
            CString pointCloudData = L"";
            for (int i = 0; i < aClouds.GetCount(); i++) 
            {
                pointCloudData += writeLuxsiCloud(aClouds[i]);
                if (pb.IsCancelPressed() ) break; 
                pb.Increment();
            }
            //-
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Open file for geometry: "+ vFileLXO);
            //-
            f.open(vFileLXO.GetAsciiString()); 

            //-- insert header
            f << _header.GetAsciiString();

            //-- polymesh
            f << geometryData.GetAsciiString();

            //-- surfaces
            for (int i=0;i<aSurfaces.GetCount();i++) 
            {
                if (writeLuxsiSurface(aSurfaces[i], L"surface")==-1) break;
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- pointclouds
            f << pointCloudData.GetAsciiString();

            //-- instances
            f << instanceData.GetAsciiString();
            
            //-- close _geom file
            f.close();

            //-- close pogress bar
            pb.PutVisible( false );
            //- done..
            vExportDone = true;
        }
    }
    else
    {
        app.LogMessage(L"Filename is empty", siFatalMsg );
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
void luxsi_execute()
{
    //-- make default path
    if ( vLuXSIPath == L"" )
    {
        // To do; use option for validate path
        CString def_exe_path = app.GetInstallationPath(siUserAddonPath);
        def_exe_path += L"/LuXSI/Application/bin";
        app.LogMessage(L"Path empty, used default path: "+ CString(def_exe_path), siErrorMsg);
        vLuXSIPath = def_exe_path;
    }
    //--
    if (vExportDone)
    {
        #ifdef __unix__
                pid_t pid = fork();
				if( 0 == pid ) 
                {
                     system ( ( vLuXSIPath +" \""+ vFileExport.GetAsciiString()));
                     exit(0);
                }
        #else
            // windows
            CString Lux_Binarie = L"", exec = L"";

            //- by default use LuxRender GUI..
            Lux_Binarie = vLuXSIPath + L"/luxrender.exe";

            //- or use console..
            if (vRmode == 1) Lux_Binarie = vLuXSIPath + L"/luxconsole.exe";

            //- by default load  unique .lxs file..
            exec = Lux_Binarie +" \""+ vFileLxs + "\"";

            //- or use queue list for load a list files..
            if (lqueue) exec = Lux_Binarie + L" -L \""+ vFileQueue + "\"";
            
            //- reset queue
            lqueue = false;
            //-
            app.LogMessage(exec);
            loader(exec.GetAsciiString());
        #endif
    }
    else
    {
        app.LogMessage(L" Not data file exported, retry exporting scene, before render ", siErrorMsg );
    }
}
//--
void luxsi_preview(CString vFile_scene_preview)
{
    //--
    CString lux_bin = L"";

    /** Use only GUI mode, obviusly..
    */
    lux_bin = vLuXSIPath + L"/luxrender.exe";
    //-
    CString exec = lux_bin +" \""+ vFile_scene_preview + "\"";

    //- show commandline for debug...
    if ( luxdebug ) app.LogMessage(exec);
    //-
    loader(exec.GetAsciiString());
}