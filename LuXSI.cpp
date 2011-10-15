/*
LuXSI - Softimage XSI Export plug-in for the LuxRender (http://www.luxrender.org) renderer

Copyright (C) 2010 2011  Michael Gangolf and Pedro Alcaide

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


#include "luxsi_values.h"
#include "plymesh/rply.h"
//--
using namespace XSI;
using namespace MATH;
using namespace std;


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
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt);

void luxsi_render_presets( CString paramName, Parameter changed, PPGEventContext ctxt);
void dynamic_luxsi_UI(Parameter changed, CString paramName, PPGEventContext ctxt);

//--
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

    prop.AddParameter( L"use_hidden_obj",   CValue::siBool, sps,L"",L"", vIsHiddenObj,     dft,dft,dft,dft, oParam );
    prop.AddParameter( L"use_hidden_light", CValue::siBool, sps,L"",L"", vIsHiddenLight,   dft,dft,dft,dft, oParam );
    prop.AddParameter( L"use_hidden_surf",  CValue::siBool, sps,L"",L"", vIsHiddenSurface, dft,dft,dft,dft, oParam );
    prop.AddParameter( L"use_hidden_cam",   CValue::siBool, sps,L"",L"", vIsHiddenCam,     dft,dft,dft,dft, oParam );
//  prop.AddParameter( L"exp_one",          CValue::siBool, sps,L"",L"", vExpOne,          dft,dft,dft,dft, oParam );
    prop.AddParameter( L"smooth_mesh",      CValue::siBool, sps,L"",L"", vSmooth_mesh,     dft,dft,dft,dft, oParam );
    prop.AddParameter( L"sharp_bound",      CValue::siBool, sps,L"",L"", vSharp_bound,     dft,dft,dft,dft, oParam );
    prop.AddParameter( L"bplymesh",         CValue::siBool, sps,L"",L"", vplymesh,         dft,dft,dft,dft, oParam );
    
    //-- lights  
    //prop.AddParameter( L"blights",      CValue::siInt4,  sps,L"",L"", vlights,        0l,4l,0l,4l,  oParam );
    prop.AddParameter( L"bIES_file",    CValue::siString, sps, L"",L"", ies_file,    oParam);
    prop.AddParameter( L"bUse_IES",     CValue::siBool,   sps, L"",L"", vUse_IES,    dft,dft,dft,dft,    oParam);
    
    //----/ image /-->
    prop.AddParameter( L"Width",        CValue::siInt4,  sps,L"",L"", vXRes,        0l,2048l,0l,1024l,  oParam);
    prop.AddParameter( L"Height",       CValue::siInt4,  sps,L"",L"", vYRes,        0l,2048l,0l,768l,   oParam);
    prop.AddParameter( L"gamma",        CValue::siFloat, sps,L"",L"", vContrast,    0,10,0,3,           oParam);
    prop.AddParameter( L"progressive",  CValue::siBool,  sps,L"",L"", vProg,        dft,dft,dft,dft,    oParam);
    
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
    prop.AddParameter( L"bvolumeint",  CValue::siInt4,     sps,L"",L"", vvolumeint,   0,10,0,10,      oParam ) ;
    
    //-- Surfaceint : bsurfaceint, eye_depth, blight_depth, blight_str, binc_env, brrstrategy, beyerrthre,
    //-- blightrrthre, bmax_depth
    prop.AddParameter( L"bsurfaceint",  CValue::siInt4, sps,L"",L"",    vSurfaceInt,    0,10,0,10,      oParam );
    prop.AddParameter( L"bsexpert",      CValue::siBool, sps,L"",L"",   vsexpert,      dft,dft,dft,dft, oParam);

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
    prop.AddParameter( L"bdiff_reflect_reject",    CValue::siBool, sps,L"",L"",     vdiff_reflect_reject,   dft,dft,dft,dft, oParam ) ;
    prop.AddParameter( L"bdiff_refract_reject",    CValue::siBool, sps,L"",L"",     vdiff_refract_reject,   dft,dft,dft,dft, oParam ) ; 
    prop.AddParameter( L"bglossy_reflect_reject",  CValue::siBool, sps,L"",L"",     vglossy_reflect_reject,   dft,dft,dft,dft, oParam ) ;
    prop.AddParameter( L"bglossy_refract_reject",  CValue::siBool, sps,L"",L"",     vglossy_refract_reject,   dft,dft,dft,dft, oParam ) ;

    //--
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
    prop.AddParameter( L"bmaxeyedepth",              CValue::siInt4, sps,L"",L"",  vmaxeyedepth,  0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bmaxphotondist",            CValue::siFloat, sps,L"",L"", vmaxphotondist,   0.0f,1024.0f,0.0f,1024.0, oParam );
    prop.AddParameter( L"bnphotonsused",             CValue::siInt4, sps,L"",L"",  vnphotonsused,       0,50,0,50,  oParam ) ;
    prop.AddParameter( L"bindirectphotons",          CValue::siInt4, sps,L"",L"",  vindirectphotons,    0,200000,0,200000,  oParam ) ;
    prop.AddParameter( L"bdirectphotons",            CValue::siInt4, sps,L"",L"",  vdirectphotons,      0,10000000,0,10000000,  oParam ) ;
    prop.AddParameter( L"bcausticphotons",           CValue::siInt4, sps,L"",L"",  vcausticphotons,     0,20000,0,20000,  oParam ) ;
    prop.AddParameter( L"bradiancephotons",          CValue::siInt4, sps,L"",L"",  vradiancephotons,    0,200000,0,200000,  oParam ) ;
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
    //-- Accelerator 
    prop.AddParameter( L"bAccel",              CValue::siInt4, sps,L"",L"",  vAccel,              0,10,0,10,  oParam ) ;
    prop.AddParameter( L"bacexpert",           CValue::siBool, sps,L"",L"",  vacexpert,       dft,dft,dft,dft, oParam) ;
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
 
    // set default filename
    vFileObjects = app.GetInstallationPath(siProjectPath);
    vLuXSIPath = app.GetInstallationPath(siUserAddonPath);
    vLux_console = vLuXSIPath;
    //--
    #ifdef __unix__
        vFileObjects += L"/tmp.lxs";
    #else
        vFileObjects += L"/tmp.lxs"; //-- also work in windows systems ?
        vLuXSIPath += L"/LuXSI/Application/bin/luxrender.exe";
        vLux_console += L"/LuXSI/Application/bin/luxconsole.exe";
    #endif
    //-- /Addons/LuXSI/Application/bin
    //-- vLuXSIPath=readIni(); // get luxrender path out of the ini

    prop.AddParameter( L"fObjects", CValue::siString, sps, L"", L"", vFileObjects, oParam ) ;

    prop.AddParameter( L"fLuxPath", CValue::siString, sps, L"", L"", vLuXSIPath, oParam ) ;

    return CStatus::OK;
}
//-- 
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

        params = prop.GetParameters();;
        for (int i=0;i<params.GetCount();i++)
        {
            // Update values on init
            Parameter param(params[i]);
            update_LuXSI_values(param.GetScriptName(), param, ctxt);
        }
    }
    else if ( eventID == PPGEventContext::siButtonClicked )
    {
        CValue buttonPressed = ctxt.GetAttribute( L"Button" ) ;
        if (buttonPressed.GetAsText()==L"exe_luxsi")
        {
            luxsi_write();
        }
        if (buttonPressed.GetAsText()==L"render_luxsi")
        {
            luxsi_write();
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
        CustomProperty prop = changed.GetParent() ;
        CString paramName = changed.GetScriptName() ;

        app.LogMessage( L"Parameter Changed: " + paramName ) ;

        update_LuXSI_values(paramName, changed, ctxt);
                
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
void update_LuXSI_values(CString paramName, Parameter changed, PPGEventContext ctxt )
{   
    // update variables when PPG value changed
    // convention names;
        //-- prefix b; name
        //-- prefix v; variables
    //-- test for presets
    if ( changed.GetName() == L"bpresets" )
    { 
        luxsi_render_presets( paramName, changed, ctxt);
    }

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

    //-- mesh export /bplymesh
    } else if (paramName == L"smooth_mesh")     { vSmooth_mesh  = changed.GetValue();
    } else if (paramName == L"sharp_bound")     { vSharp_bound  = changed.GetValue();
     } else if (paramName == L"bplymesh")       { vplymesh  = changed.GetValue();
   
    //-- save images /----/ tga /--->
    } else if (paramName == L"tga_gamut")   { vTga_gamut    = changed.GetValue();
    } else if (paramName == L"mode_rtga")   { vRtga         = changed.GetValue();
    } else if (paramName == L"save_tga")    { vTga          = changed.GetValue();

    //----/ save images /----/ exr /--->
    } else if (paramName == L"mode_Znorm")  { vExr_Znorm  = changed.GetValue();
    } else if (paramName == L"save_exr")    { vExr        = changed.GetValue();

    //----/ save images /----/ png /--->
    } else if (paramName == L"mode_rpng")    { vRpng         = changed.GetValue();
    } else if (paramName == L"save_png_16")  { vWpng_16      = changed.GetValue();
    } else if (paramName == L"png_gamut")    { vPng_gamut    = changed.GetValue();
    } else if (paramName == L"save_png")     { vPng          = changed.GetValue();

    //-- save file name
    } else if (paramName == L"fObjects")     { vFileObjects  = changed.GetValue();
    } else {
        //app.LogMessage(L"Tab 'Main' update values..");
    }
    //-- TAB SPECIAL LIGHTS
    //-- lights / 
    //} else if (paramName == L"blights")     { vlights  =   changed.GetValue();
           if (paramName == L"bIES_file")   { ies_file	=  changed.GetValue();
    } else if (paramName == L"bUse_IES")    { vUse_IES	=  changed.GetValue();
    
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
    
    } else if (paramName == L"bdiff_reflect_reject")    { vdiff_reflect_reject   = changed.GetValue(); 
    } else if (paramName == L"bdiff_refract_reject")    { vdiff_refract_reject   = changed.GetValue(); 
    } else if (paramName == L"bglossy_reflect_reject")  { vglossy_reflect_reject = changed.GetValue();
    } else if (paramName == L"bglossy_refract_reject")  { vglossy_refract_reject = changed.GetValue();

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
    } else if (paramName == L"bmaxeyedepth")        { vmaxeyedepth         = changed.GetValue();// max eye ?
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
    } else if (paramName == L"bfilter"){ vfilter    = changed.GetValue();
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
    } else if (paramName == L"bacexpert")           { vacexpert           = changed.GetValue();
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
       // app.LogMessage(L"Tab 'Render' update values.");
    }
       dynamic_luxsi_UI(changed, paramName, ctxt);
}
//--
void luxsi_render_presets( CString paramName, Parameter changed, PPGEventContext ctxt)
{
    //--
    int numparams = prop.GetParameters().GetCount();
    //-- options
    if ( changed.GetName() == L"bpresets" ) 
    {
        vpresets = prop.GetParameterValue( L"bpresets" );
        //-- commons values for all presets 
        //--
        Parameter(prop.GetParameters().GetItem( L"hSpp" )).PutValue( vhaltspp = 0 );
        Parameter(prop.GetParameters().GetItem( L"hTime" )).PutValue( vhalttime = 0 );
         
        //--
        if ( vpresets == 0 ) // Custom parameters
        {
            //-- TODO;
        }
        else if ( vpresets == 1 ) // Preview; Instant Global Illumination
        {
           /* //--
            MtBool[ vdirectsampleall ] 
            vdirectsamples 
            MtBool[ vdirectdiffuse ]
            MtBool[vdirectglossy]
            MtBool[ vindirectsampleall ] 
            vindirectsamples 
            MtBool[ vindirectdiffuse ] 
            MtBool[ vindirectglossy ] 
            vdiffusereflectdepth 
            vdiffusereflectsamples 
            vdiffuserefractdepth 
            vdiffuserefractsamples 
            vglossyreflectdepth 
            vglossyreflectsamples 
            vglossyrefractdepth 
            vglossyrefractsamples 
            vspecularreflectdepth 
            vspecularrefractdepth 
            MtBool[0] 
            vdiff_reflect_reject_thr 
            MtBool[0] 
            vdiff_refract_reject_thr 
            MtBool[0] 
            vglossy_reflect_reject_thr 
            MtBool[0] 
            vglossy_refract_reject_thr 
        if ( vsexpert )
           MtlightST[vLight_str]
*/ 
            //-- sampler
            vSampler = 2 ;
            vPixsampler = 3 ; // low
            vPixelsamples = 1 ;
            //-- distributepath
            vSurfaceInt = 3 ; 
            vLight_str = 2;
           // Parameter(prop.GetParameters().GetItem( L"blight_str" )).PutValue( vLight_str = 2 );
            Parameter(prop.GetParameters().GetItem( L"directsampleall" )).PutValue( vdirectsampleall =  true );
            Parameter(prop.GetParameters().GetItem( L"bdirectsamples" )).PutValue( vdirectsamples = 1 );
            Parameter(prop.GetParameters().GetItem( L"bindirectsampleall" )).PutValue( vindirectsampleall = false );
            Parameter(prop.GetParameters().GetItem( L"bindirectsamples" )).PutValue( vindirectsamples = 1 );
            Parameter(prop.GetParameters().GetItem( L"bdiffusereflectdepth" )).PutValue( vdiffusereflectdepth = 3 );
            Parameter(prop.GetParameters().GetItem( L"bdiffusereflectsamples" )).PutValue( vdiffusereflectsamples = 1 );
            Parameter(prop.GetParameters().GetItem( L"bdiffuserefractdepth" )).PutValue( vdiffuserefractdepth = 5 );
            Parameter(prop.GetParameters().GetItem( L"bdiffuserefractsamples" )).PutValue( vdiffuserefractsamples = 1 );
            Parameter(prop.GetParameters().GetItem( L"bdirectdiffuse" )).PutValue( vdirectdiffuse = true );
            Parameter(prop.GetParameters().GetItem( L"bindirectdiffuse" )).PutValue( vindirectdiffuse = true );
            Parameter(prop.GetParameters().GetItem( L"bglossyreflectdepth" )).PutValue( vglossyreflectdepth = 2 );
            Parameter(prop.GetParameters().GetItem( L"bglossyreflectsamples" )).PutValue( vglossyreflectsamples = 1 );
            Parameter(prop.GetParameters().GetItem( L"bglossyrefractdepth" )).PutValue( vglossyrefractdepth = 5 );
            Parameter(prop.GetParameters().GetItem( L"bglossyrefractsamples" )).PutValue( vglossyrefractsamples = 1 );
            Parameter(prop.GetParameters().GetItem( L"bdirectglossy" )).PutValue( vdirectglossy = true );
            Parameter(prop.GetParameters().GetItem( L"bindirectglossy" )).PutValue( vindirectglossy = true );
            Parameter(prop.GetParameters().GetItem( L"bspecularreflectdepth" )).PutValue( vspecularreflectdepth = 3 );
            Parameter(prop.GetParameters().GetItem( L"bspecularrefractdepth" )).PutValue( vspecularrefractdepth = 5 );
            //-- volume int
            //-- filter / commons
            app.LogMessage(L" Parameters for render presets 1, loaded..");
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
            app.LogMessage(L" Parameters for render presets 2, loaded..");
        }
        else if ( vpresets == 3 ) // Preview; Ex-photonmap
        {
            //- Sampler "lowdiscrepancy"
            vPixsampler = 5 ; //- hilbert
            vPixelsamples = 4 ;
            //-- SurfaceIntegrator "exphotonmap"
            vSurfaceInt = 5 ;
            Parameter(prop.GetParameters().GetItem( L"brenderingmode" )).PutValue( vrenderingmode = 0 ); // directlighting
            Parameter(prop.GetParameters().GetItem( L"blightstrategy" )).PutValue( vLight_str = 0 ); // auto
            Parameter(prop.GetParameters().GetItem( L"bshadowraycount" )).PutValue( vshadowraycount = 1 ); // ?
            Parameter(prop.GetParameters().GetItem( L"bmaxphotondepth" )).PutValue( vmaxphotondepth = 10 );
            Parameter(prop.GetParameters().GetItem( L"bmaxeyedepth" )).PutValue( vmaxeyedepth = 10 );
            //vmaxdepth = 5 ;
            Parameter(prop.GetParameters().GetItem( L"bmaxphotondist" )).PutValue( vmaxphotondist = 0.100000f );
            Parameter(prop.GetParameters().GetItem( L"bnphotonsused" )).PutValue( vnphotonsused = 50 );
            Parameter(prop.GetParameters().GetItem( L"bindirectphotons" )).PutValue( vindirectphotons = 200000 );
            Parameter(prop.GetParameters().GetItem( L"bdirectphotons" )).PutValue( vdirectphotons = 1000000 );
            Parameter(prop.GetParameters().GetItem( L"bcausticphotons" )).PutValue( vcausticphotons = 20000 );
            Parameter(prop.GetParameters().GetItem( L"bradiancephotons" )).PutValue( vradiancephotons = 20000 );
            Parameter(prop.GetParameters().GetItem( L"bfinalgather" )).PutValue( vfinalgather = false );

            // VolumeIntegrator "single"
            // Parameter(prop.GetParameters().GetItem( L"stepsize" [1.000000]
            app.LogMessage(L" Parameters for render presets 3; loaded..");
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
            app.LogMessage(L"Parameters for render presets 4; loaded..");
        }
        else if ( vpresets == 5 ) // final 2 MLT / PathTracing (ext)
        {
            vSampler = 0 ;// metro
            vlmutationpro = 0.4f ;
            //--
            vSurfaceInt = 1 ; // path
            vmaxdepth = 10 ;
            
            Parameter(prop.GetParameters().GetItem( L"binc_env" )).PutValue( vInc_env = true );
            //--
            app.LogMessage(L"Parameters for render presets 5; loaded..");
        }
        else if ( vpresets == 6 ) // progr 1 Bidir Path Tracing (int)
        {
            vSampler = 2 ;
            vPixsampler = 2 ; 
            vPixelsamples = 1 ;
            //--
            vSurfaceInt = 2 ; //-- 
            vLight_depth = 10 ; 
            vEye_depth = 10 ; 
            //--
            app.LogMessage(L"Parameters for render presets 6; loaded..");
        }
        else if ( vpresets == 7 ) //  progr 2 Path Tracing (ext)
        {
            vSampler = 1 ;
            vchainlength = 512 ;
            vbasampler = 1 ;
            vPixsampler = 2 ; 
            vPixelsamples = 1 ;
            //-- surf
            vSurfaceInt = 1 ; // path
            vmaxdepth = 10 ;
            vInc_env = true ;
            //--
            app.LogMessage(L" Parameters for render presets 7; loaded..");
        }
        else if ( vpresets == 8 )// bucket 1 Bidir Path Tracing (int)
        {
            vSampler = 2 ; // low
            vPixsampler = 5 ; 
            vPixelsamples = 64 ;
            //--
            vSurfaceInt = 0 ; // bidir
            vLight_depth = 16 ; 
            vEye_depth = 16 ;
            //--
            app.LogMessage(L" Parameters for render presets 8; loaded..");
        }
        else // bucket 2 Path Tracing (ext)
        {
            vSampler = 2 ; // low
            vPixsampler = 5 ; // hilbert
            vPixelsamples = 64 ;

            vSurfaceInt = 1 ; // path
            vsexpert = false ;
            vmaxdepth = 10 ; 
            vInc_env = true ;
            //-- filter - commons
            app.LogMessage(L" Parameters for render presets 9; loaded..");
        }
        //-- commons values for all presets
            //-- sampler
            Parameter(prop.GetParameters().GetItem( L"bsampler" )).PutValue( vSampler );
            //-- if ( vsampler == low ) //-- TODO;
            Parameter(prop.GetParameters().GetItem( L"bpixsampler" )).PutValue( vPixsampler );
            Parameter(prop.GetParameters().GetItem( L"pixelsamples" )).PutValue( vPixelsamples );
            //-- if ( vsampler == metro ) //-- TODO;
            Parameter(prop.GetParameters().GetItem( L"bmutation" )).PutValue( vlmutationpro = 0.6f );
            Parameter(prop.GetParameters().GetItem( L"bmaxrej" )).PutValue( vmaxconsecrejects = 512 ); 
            Parameter(prop.GetParameters().GetItem( L"buservarian" )).PutValue( vusevariance = false );
            
            //-- filter
            Parameter(prop.GetParameters().GetItem( L"bfilter" )).PutValue( vfilter );
            //-- if filter is mitchell...
            Parameter(prop.GetParameters().GetItem( L"bxwidth" )).PutValue( vXwidth = 1.500000 );
            Parameter(prop.GetParameters().GetItem( L"bywidth" )).PutValue( vYwidth = 1.500000 );
            Parameter(prop.GetParameters().GetItem( L"bF_B" )).PutValue( vF_B = 0.3333f );
            Parameter(prop.GetParameters().GetItem( L"bF_C" )).PutValue( vF_C = 0.3333f );
            Parameter(prop.GetParameters().GetItem( L"ssample" )).PutValue( vSupers = true );
            //--  ...or  gaussian ? //-- TODO;

            //-- surface integrator
            Parameter(prop.GetParameters().GetItem( L"bsurfaceint" )).PutValue( vSurfaceInt );
            //-- direct
            Parameter(prop.GetParameters().GetItem( L"blightrrthre" )).PutValue( vLightRRthre ); 
            //-- path
            Parameter(prop.GetParameters().GetItem( L"bmaxdepth" )).PutValue( vmaxdepth = 10 );
            //-- bi-direct
            Parameter(prop.GetParameters().GetItem( L"blight_depth" )).PutValue( vLight_depth ); 
            Parameter(prop.GetParameters().GetItem( L"beye_depth" )).PutValue( vEye_depth );
            
            //-- Accelerator; 'qbvh' 
            Parameter(prop.GetParameters().GetItem( L"bAccel" )).PutValue( vAccel = 0 );
            Parameter(prop.GetParameters().GetItem( L"bmaxprimsperleaf" )).PutValue( vmaxprimsperleaf = 4 );
            Parameter(prop.GetParameters().GetItem( L"bfullsweepthreshold" )).PutValue( vfullsweepthr = 16 );
            Parameter(prop.GetParameters().GetItem( L"bskipfactor" )).PutValue( vskipfactor = 1 );
           
        ctxt.PutArrayAttribute(L"Refresh", true); 
    
    } //-- end cases...
}
//--
void dynamic_luxsi_UI( Parameter changed, CString paramName, PPGEventContext ctxt) 
{
    //-- load parameters data
    
    PPGLayout lay = ctxt.GetSource() ; 
    int numparams = prop.GetParameters().GetCount();
    

    //-- convention names;
    //-- prefix l; for Parameter ( logic )

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

    //-- for all events -------------------------
    if (( changed.GetName() == L"bsampler") ||
         ( changed.GetName() == L"bexpert") ||
          ( changed.GetName() == L"bbasampler"))
    //-------------------------------------------
    {
        vSampler = prop.GetParameterValue(L"bsampler");
        vExpert = prop.GetParameterValue(L"bexpert");
        vbasampler = prop.GetParameterValue(L"bbasampler");

        //-- create list of parameters
        Parameter lmutation     = prop.GetParameters().GetItem( L"bmutation" );
        Parameter lbmaxrej      = prop.GetParameters().GetItem( L"bmaxrej" );
        Parameter lbuservarian  = prop.GetParameters().GetItem( L"buservarian" );
        Parameter lchainlength  = prop.GetParameters().GetItem( L"bchainlength" );
        Parameter lbpixsampler  = prop.GetParameters().GetItem( L"bpixsampler" );
        Parameter lpixelsamples = prop.GetParameters().GetItem( L"pixelsamples" );//--
        Parameter lbbasampler   = prop.GetParameters().GetItem( L"bbasampler" );

        //-- assing flags for hidden options
        lmutation.PutCapabilityFlag( siNotInspectable, true );
        lbmaxrej.PutCapabilityFlag( siNotInspectable, true );
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
            lbuservarian.PutCapabilityFlag( siNotInspectable, false );
            if (vExpert)
            {
                lbmaxrej.PutCapabilityFlag( siNotInspectable, false );
            }
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
                    lbmaxrej.PutCapabilityFlag( siNotInspectable, false );
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

    //--------------------------------------//
    if (( changed.GetName() == L"bfilter")||
        ( changed.GetName() == L"bfexpert"))
    //--------------------------------------//
    {
        vfilter = prop.GetParameterValue(L"bfilter");
        vfexpert = prop.GetParameterValue(L"bfexpert");
    
        //--- change form
        const char *ui_filter [7] = {"bxwidth", "bywidth", "bfalpha", "ssample", "bF_B", "bF_C", "bTau" };
        //--
        for ( long f = 0; f < 7;)
        {
            Parameter(params.GetItem(ui_filter[f])).PutCapabilityFlag(siNotInspectable, true);
            f++;
        }
        
        //-- show with all options, if mode expert is true
        if ( vfexpert )
        {
            Parameter(params.GetItem(L"bxwidth")).PutCapabilityFlag( siNotInspectable, false ) ;
            Parameter(params.GetItem(L"bywidth")).PutCapabilityFlag( siNotInspectable, false ) ;
            ctxt.PutAttribute(L"Refresh", true);
        }

        if (( vfilter == 1 ) && ( vfexpert )) //-- gauss
        {
            Parameter(params.GetItem(L"bfalpha")).PutCapabilityFlag( siNotInspectable, false ) ;
            ctxt.PutAttribute(L"Refresh", true);
        }
        else if (( vfilter == 2 ) && ( vfexpert )) //-- mitchell
        {
            Parameter(params.GetItem(L"ssample")).PutCapabilityFlag( siNotInspectable, false ) ;
            Parameter(params.GetItem(L"bF_B")).PutCapabilityFlag( siNotInspectable, false ) ;
            Parameter(params.GetItem(L"bF_C")).PutCapabilityFlag( siNotInspectable, false ) ;
            ctxt.PutAttribute(L"Refresh", true);
        }
        else if (( vfilter == 3 ) && ( vfexpert )) //-- sinc
        {
            Parameter(params.GetItem(L"bTau")).PutCapabilityFlag( siNotInspectable, false ) ;
            ctxt.PutAttribute(L"Refresh", true);
        }
        else //-- box
        {
        ctxt.PutAttribute(L"Refresh", true); 
        }
    }

    //---------------------------------------------//
    if (( changed.GetName() == L"bsurfaceint" ) ||
        ( changed.GetName() == L"bfinalgather" ) ||
        ( changed.GetName() == L"bpresets" ) ||
        ( changed.GetName() == L"bsexpert" ))
    //--------------------------------------------//
    {
        vSurfaceInt  = prop.GetParameterValue(L"bsurfaceint");
        vsexpert = prop.GetParameterValue(L"bsexpert");

        //--------------------------------
        const char *u_intgrator [58] = {/*bidirectional*/"blight_depth", "beye_depth", "beyerrthre", "blightrrthre",
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
            "bdbg_enableindircaustic", "bdbg_enableindirdiffuse", "bdbg_enableindirspecular" };
       
        //--
        for ( long in = 0; in < 58;)
        {
            Parameter(params.GetItem(u_intgrator[in])).PutCapabilityFlag(siNotInspectable, true);
            in++;
        }
        //--
        if ( vSurfaceInt == 0 ) //-- bidirectional
        {
            Parameter(params.GetItem(L"blight_depth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"beye_depth")).PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                Parameter(params.GetItem(L"beyerrthre")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"blightrrthre")).PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 1 ) //-- path
        {
            Parameter(params.GetItem(L"bmaxdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"binc_env")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"brrstrategy")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"brrcon_prob")).PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                Parameter(params.GetItem(L"blight_str")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"blight_depth")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bshadowraycount")).PutCapabilityFlag( siNotInspectable, false ); // dude, revised
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 2 ) //-- directlighting
        {
            Parameter(params.GetItem(L"bmaxdepth")).PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                Parameter(params.GetItem(L"bshadowraycount")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"blight_str")).PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 3 ) //-- distributepath
        {
            Parameter(params.GetItem(L"bdirectsampleall")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdirectsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bindirectsampleall")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bindirectsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdiffusereflectdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdiffusereflectsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdiffuserefractdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdiffuserefractsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdirectdiffuse")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bindirectdiffuse")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bglossyreflectdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bglossyreflectsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bglossyrefractdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bglossyrefractsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdirectglossy")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bindirectglossy")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bspecularreflectdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bspecularrefractdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdiff_reflect_reject_thr")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdiff_refract_reject_thr")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bglossy_reflect_reject_thr")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bglossy_refract_reject_thr")).PutCapabilityFlag( siNotInspectable, false );
            //--
            Parameter(params.GetItem(L"bdiff_reflect_reject")).PutCapabilityFlag( siNotInspectable, false );
            if ( vdiff_reflect_reject == false )// 
            {
            //    lbdiff_reflect_reject_thr.PutCapabilityFlag( siReadOnly, true );
            }
            else
            {
            //    lbdiff_reflect_reject_thr.PutCapabilityFlag( siReadOnly, false );
            }

            //--
            Parameter(params.GetItem(L"bdiff_refract_reject")).PutCapabilityFlag( siNotInspectable, false );
            if ( vdiff_refract_reject == false )
            {
            //    lbdiff_refract_reject_thr.PutCapabilityFlag( siReadOnly, true );
            }
            //--
            Parameter(params.GetItem(L"bglossy_reflect_reject")).PutCapabilityFlag( siNotInspectable, false );
            if ( vglossy_reflect_reject == false )
            {
            //    lbglossy_reflect_reject_thr.PutCapabilityFlag( siReadOnly, true );
            }
            //--
            Parameter(params.GetItem(L"bglossy_refract_reject")).PutCapabilityFlag( siNotInspectable, false );
            //--
            Parameter(params.GetItem(L"bglossy_refract_reject_thr")).PutCapabilityFlag( siReadOnly, false );
            if ( vglossy_refract_reject == false )
            {
                Parameter(params.GetItem(L"bglossy_refract_reject_thr")).PutCapabilityFlag( siReadOnly, true );
            }

            //--
            if ( vsexpert )
            {
                Parameter(params.GetItem(L"blight_str")).PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if ( vSurfaceInt == 4 ) //-- IGI
        {
            Parameter(params.GetItem(L"bnsets")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bnlights")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bmaxdepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bmindist")).PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                Parameter(params.GetItem(L"blight_str")).PutCapabilityFlag( siNotInspectable, false );
            }
             ctxt.PutAttribute(L"Refresh", true );
        }
        else
        {
            Parameter(params.GetItem(L"bmaxeyedepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bmaxphotondepth")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bdirectphotons")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bcausticphotons")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bindirectphotons")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bradiancephotons")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bnphotonsused")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bmaxphotondist")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bfinalgather")).PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vfinalgather )
            {
                Parameter(params.GetItem(L"bfinalgathersamples")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bgatherangle")).PutCapabilityFlag( siNotInspectable, false );
             }
            Parameter(params.GetItem(L"brenderingmode")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"brrstrategy")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"brrcon_prob")).PutCapabilityFlag( siNotInspectable, false );
            //--
            if ( vsexpert )
            {
                Parameter(params.GetItem(L"bshadowraycount")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"blight_str")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bdistancethreshold")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bdbg_enabledirect")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bdbg_enableradiancemap")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bdbg_enableindircaustic")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bdbg_enableindirdiffuse")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bdbg_enableindirspecular")).PutCapabilityFlag( siNotInspectable, false );
            }
            ctxt.PutAttribute(L"Refresh", true );
        }
        // Parameter lbdbg_enableindirspecular = prop.GetParameters().GetItem( L"bdbg_enableindirspecular" );


    } // end surface int
    //-- Accelerator
    if (( changed.GetName() == L"bAccel" )||
        ( changed.GetName() == L"bacexpert" )) //-- change to dedicated item (bAcexpert)
    {
        vAccel = prop.GetParameterValue(L"bAccel");
        vacexpert = prop.GetParameterValue(L"bacexpert"); //-- provisional
        //--
        const char *ui_accel [9] = {/*qbvh*/ "bmaxprimsperleaf", "bfullsweepthreshold", "bskipfactor",
            /*bvh*/"bcostsamples", "bintersectcost", "btraversalcost", "bmaxprims", "bacmaxdepth", "bemptybonus" };
        //--
        for ( long ac = 0; ac < 9;)
        {
            Parameter(params.GetItem(ui_accel[ac])).PutCapabilityFlag(siNotInspectable, true);
            ac++;
        }

        // Parameter lbrefineimmediately= prop.GetParameters().GetItem( L"brefineimmediately");
        // Parameter lbtreetype= prop.GetParameters().GetItem( L"btreetype"); 
        
        //--
        if (( vAccel == 0 ) && ( vacexpert )) //-- qbvh
        {
            Parameter(params.GetItem(L"bmaxprimsperleaf")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bfullsweepthreshold")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bskipfactor")).PutCapabilityFlag( siNotInspectable, false );
            ctxt.PutAttribute(L"Refresh", true );
        }
        else if (( vAccel == 1 )&& ( vacexpert )) //-- bvh
        {
//            lbtreetype.PutCapabilityFlag( siNotInspectable, false ); //-- combo          
            Parameter(params.GetItem(L"bcostsamples")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bintersectcost")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"btraversalcost")).PutCapabilityFlag( siNotInspectable, false );
            Parameter(params.GetItem(L"bemptybonus")).PutCapabilityFlag( siNotInspectable, false );
            ctxt.PutAttribute(L"Refresh", true );
        }
//        else if (( vAccel == 2 )&& ( vsexpert )) //-- grid
//        {
//            lbrefineimmediately.PutCapabilityFlag( siNotInspectable, false );
//            ctxt.PutAttribute(L"Refresh", true );
//        }
        else
        {
            if ( vacexpert )//-- KD Tree
            {
                Parameter(params.GetItem(L"bintersectcost")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"btraversalcost")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bemptybonus")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bmaxprims")).PutCapabilityFlag( siNotInspectable, false );
                Parameter(params.GetItem(L"bacmaxdepth")).PutCapabilityFlag( siNotInspectable, false );
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
CString findInGroup(CString s)
{
    CRefArray grps = root.GetGroups();
    //--
    for (int i=0;i<grps.GetCount();i++)
    {
        CRefArray a=Group(grps[i]).GetMembers();
        for (int j=0;j<a.GetCount();j++)
        {
            if (X3DObject(a[j]).GetName()==s) 
            {
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
    const char *MtRRst [3] = { "none", "probability", "efficiency"};
    const char *ARmode [2] = { "GUI", "console"};
    const char *MtlightST [7] = { "one", "all", "auto", "importance", "powerimp", "allpowerimp", "logpowerimp"};
    const char *Aengine [2] = { "classic", "hybrid" };
    const char *ArgbT [3] = { "Y", "RGB", "RGBA" }; // iterator  vRtga
    const char *AZb_norm [3] = { "Camera Start/End clip", "Min/Max", "None" }; //exr
    const char *ArgbP [4] = { "Y", "YA", "RGB", "RGBA" }; // iterator  vRpng
    const char *aFilter [5] = { "box", "gaussian", "mitchell", "sinc", "triangle" }; // iter vfilter
    const char *aSampler [4] = { "metropolis", "erpt", "lowdiscrepancy", "random" }; // mode, Asampler [vSampler]
    const char *aPxSampler [6] = { "linear", "vegas", "random", "lowdiscrepancy", "tile", "hilbert" };
    const char *MtSurf [6] = { "bidirectional", "path", "directlighting", "distributedpath", "igi", "exphotonmap" };
    const char *MtRendering [2] = { "path", "directlighting" };
    //---
    std::string fname = vFileObjects.GetAsciiString();
    int loc=(int)fname.rfind(".");

    f << "\nFilm \"fleximage\"\n"; //----
    f << "  \"integer xresolution\" ["<< vXRes << "]\n  \"integer yresolution\" ["<< vYRes <<"]\n";
    f << "  \"string filename\" [\""<< replace(fname.substr(0,loc)) << "\"]\n";

    f << "  \"integer writeinterval\" [" << vSave <<"]\n";
    f << "  \"integer displayinterval\" [" << vDis <<"]\n";

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
    else //-- exphotonmap
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
        if ( vfinalgather )
        {
            f << "  \"integer finalgathersamples\" ["<< vfinalgathersamples <<"] \n";
            f << "  \"float gatherangle\" ["<< vgatherangle <<"] \n";
        }
        if ( vsexpert )
        {
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
void write_environment()
{
/*    // Look if there is an background image for Image Based Lighting (e.g. HDRI)
    CRefArray aEnv = app.GetActiveProject().GetActiveScene().GetActivePass().GetNestedObjects();
    for (int i=0;i<aEnv.GetCount();i++)
    {
        if (SIObject(aEnv[i]).GetName()==L"Environment Shader Stack")
        {
            CRefArray aImages = SIObject(aEnv[i]).GetNestedObjects();
            for (int j=0;j<aImages.GetCount();j++)
            {
                if (SIObject(aImages[j]).GetType()==L"Shader")
                {
                    Shader s(aImages[j]);
                    CRefArray aEnvImg=s.GetImageClips();
                    for (int k=0;k<aEnvImg.GetCount();k++)
                    {
                        ImageClip2 vImgClip(aEnvImg[k]);
                        Source vImgClipSrc(vImgClip.GetSource());
                        CString vFileName = vImgClipSrc.GetParameterValue( L"path");
                        if (vFileName !=L"")
                        {
                            string::size_type hdr = string(CString(vFileName).GetAsciiString()).find( ".hdr", 0 );
                            string::size_type exr = string(CString(vFileName).GetAsciiString()).find( ".exr", 0 );
	                        if ( hdr != string::npos || exr != string::npos ) vHDRI = vFileName; vEnviron = true;

                          //  break;
                        }
                    }
                }
            }
         //   break;
        }
    }



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
    vnegZ.MulByMatrix3InPlace(c.GetKinematics().GetGlobal().GetTransform().GetRotationMatrix3());
    vnegZ.NormalizeInPlace();
    vnegZ.ScaleInPlace((double) c.GetParameterValue(L"interestdist"));
    vnegZ.AddInPlace(c.GetKinematics().GetGlobal().GetTransform().GetTranslation());

    CTransformation local_transf = o2.GetKinematics().GetLocal().GetTransform();
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
        app.LogMessage(L" lens shader: "+ CString(vCSID)); //---------------------
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
    CTransformation ci_gt = ci_gs.GetTransform();

    CVector3 tranlation(0,1,0);
    CTransformation target=o2.GetKinematics().GetGlobal().GetTransform().AddLocalTranslation(tranlation);
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
    f << "LookAt "<< new_pos.GetX() <<" "<< new_pos.GetY() <<" "<< new_pos.GetZ() <<"\n";
    f << "       "<< new_pos_ci.GetX() <<" "<< new_pos_ci.GetY() <<" "<< new_pos_ci.GetZ() <<"\n";
    f << "       0 1 0 \n"; //<< CString(up.GetZ()).GetAsciiString() << "\n"; // not working correct
        
    if ( camera_proj == 1 )
    {
        f << "Camera \"perspective\" \n";
        f << "  \"float fov\" ["<< vfov <<"] \n";
        f << "  \"float lensradius\" ["<< vLensr/10 <<"] \n";
        f << "  \"float focaldistance\" ["<< vFdist <<"] \n";
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

//--
void writeLuxsiLight(X3DObject o)
{
    //-- search image based lighting
    CString vFile_env = L"";
    bool vEnviron = false;
    int env_mode;
   
    CRefArray aEnv = app.GetActiveProject().GetActiveScene().GetActivePass().GetNestedObjects();
    for (int i=0;i<aEnv.GetCount();i++)
    {
        // app.LogMessage(L"object render pass: "+ CString(SIObject(aEnv[i]).GetName()));
        if (SIObject(aEnv[i]).GetName()==L"Environment Shader Stack")
        {
            CRefArray aImages = SIObject(aEnv[i]).GetNestedObjects();
            for (int j=0;j<aImages.GetCount();j++)
            {
                //app.LogMessage(L"Nested object render pass: "+ CString(SIObject(aImages[j]).GetType()));
                if (SIObject(aImages[j]).GetType()==L"Shader")
                {
                    Shader s(aImages[j]);
                    CRefArray aEnvImg = s.GetImageClips();
                    for (int k=0; k < aEnvImg.GetCount(); k++)
                    {
                        ImageClip2 vImgClip(aEnvImg[k]);
                        Source vImgClipSrc(vImgClip.GetSource());
                        //--
                        env_mode = s.GetParameterValue(L"mode");
                        app.LogMessage(L"mod env: "+ CString(env_mode));
                        //--
                        vFile_env = vImgClipSrc.GetParameterValue( L"path");
                        if (vFile_env !=L"")
                        {
                            string::size_type hdr = string(CString(vFile_env).GetAsciiString()).find( ".hdr", 0 );
                            string::size_type exr = string(CString(vFile_env).GetAsciiString()).find( ".exr", 0 );
	                        if ( hdr != string::npos || exr != string::npos ) vEnviron = true;
                        }
                    }
                }
            }
         //   break;
        }
    }
    
    CTransformation localTransformation = o.GetKinematics().GetLocal().GetTransform();
    //-- point from
    KinematicState  global_kinex_state = o.GetKinematics().GetGlobal();
    CTransformation global_transf = global_kinex_state.GetTransform();
    CVector3 light_from(global_transf.GetTranslation());
    //
    float a=0, b=0, c=0, alpha=0;

    //-- point to 
    X3DObject li;
    li= X3DObject(o.GetParent()).GetChildren()[1];
    CTransformation lt = li.GetKinematics().GetLocal().GetTransform();
    CVector3 light_to(lt.GetTranslation());
    
    //--
    KinematicState  local_state = o.GetKinematics().GetLocal(); 
    CTransformation local_transf = local_state.GetTransform();
   
    //--
    Shader s((Light(o).GetShaders())[0]);
    s.GetColorParameterValue(L"color",a,b,c,alpha );
    //--
    CString lName = findInGroup(o.GetName());
    CString group_name; 
    if (lName != L"") 
    {
        group_name = lName.GetAsciiString(); 
    } 
    else 
    {
        group_name = o.GetName().GetAsciiString();
    }
    //--
    int lType = Light(o).GetParameterValue(L"Type");
    app.LogMessage(L" light type: "+ CString(lType) );
    //--
    bool vSiArealight = o.GetParameterValue(L"LightArea");
    bool vSiArea_vis = o.GetParameterValue(L"LightAreaVisible");
    int vlight_geo = o.GetParameterValue(L"LightAreaGeom");
    float vLightCone = o.GetParameterValue(L"LightCone"); 
    float vIntensity = s.GetParameterValue(L"intensity");
    float vSpotblend = s.GetParameterValue(L"spread"); //
    //--
    f << "\nLightGroup \""<< group_name.GetAsciiString() <<"\" \n";
    
    if (lType == 2) //-- spot
    {
        //-- values
        f << "\nLightSource \"spot\"\n";
        f << "  \"float gain\" [" << vIntensity << "]\n";
        f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
        if ( vUse_IES )
        {
            f << "  \"string iesname\" [\"" << replace(ies_file.GetAsciiString()) << "\"]\n";
        }
        f << "  \"point from\" [" << light_from.GetX() << " " << light_from.GetY() << " "  << light_from.GetZ()  << "] \n";
        f << "  \"point to\" ["<< light_to.GetX() << " " << light_to.GetY() << " "<< light_to.GetZ() << "]\n";
        f << "  \"float coneangle\" [" << vLightCone << "]\n";
        f << "  \"float conedeltaangle\" [" << vLightCone - vSpotblend << "]\n";
    } 
    else if  (lType == 1) //-- infinite
    {
        if ( vEnviron && vFile_env != L"")
        {
            f << "\nTransformBegin \n";
            f << "Scale -1 1 1 \nRotate -90 1 0 0 \n"; // for corrected orientation ??
            f << "LightSource \"infinite\"\n";
            f << "  \"float gain\" [" << vIntensity << "]\n";
            f << "  \"float importance\" [1.0]\n"; // TODO
            f << "  \"string mapname\" [\"" << replace(vFile_env.GetAsciiString()) << "\"]\n";
            f << "  \"string mapping\" [\"latlong\"]\n"; // TODO
            f << "  \"float gamma\" [1.0]\n";
            f << "  \"integer nsamples\" [1]\n"; // TODO
            f << "\nTransformEnd \n";
        } 
        else
        {
            f << "LightSource \"infinite\"\n";
            f << "  \"float gain\" [" << vIntensity << "]\n";
            f << "  \"float importance\" [1.0]\n"; // TODO
            f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
        }
         /*
            CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();
            f << "\nLightSource \"sunsky\"\n";
            f << "  \"integer nsamples\" [4]\n";
            f << "  \"vector sundir\" [ "<< sunTransMat.GetValue(2,0) << " " << sunTransMat.GetValue(2,1) << " " << sunTransMat.GetValue(2,2) << " ]\n";
            f << "  \"float gain\" [" << vIntensity << "]\n";
            */
    } 
    else 
    {
        // Pointlight
        //-- in Softimage, light area, only work into 'point' or 'spot' lights
        if ( vSiArealight )
        {
            //--
            float size_X(o.GetParameterValue(L"LightAreaXformSX"));
            float size_Y(o.GetParameterValue(L"LightAreaXformSY"));
            float size_Z(o.GetParameterValue(L"LightAreaXformSZ"));
            //-- rotate; use int, float crash
            int R_areaX(o.GetParameterValue(L"LightAreaXformRX"));
            int R_areaY(o.GetParameterValue(L"LightAreaXformRY"));
            int R_areaZ(o.GetParameterValue(L"LightAreaXformRZ"));
            //--
            CString A_rotation = L"";// for test, replace by Matriz
            if ( R_areaX != 0 )  A_rotation += "Rotate "+ CString(R_areaX) + L" 1 0 0 \n";
            if ( R_areaY != 0 )  A_rotation += "Rotate "+ CString(R_areaY) + L" 0 1 0 \n";
            if ( R_areaZ != 0 )  A_rotation += "Rotate "+ CString(R_areaZ) + L" 0 0 1 \n";
            //--
            //CVector3 axis;
            //double rot = global_transf.GetRotationAxisAngle(axis);
                        
            //-- samples U + V / 2
            int U_samples = o.GetParameterValue(L"LightAreaSampU");
            int V_samples = o.GetParameterValue(L"LightAreaSampV");
            
            //--
            CString aPoints = L""; // like luxblend
            aPoints += L""+ CString( -size_X/2.0 ) + L" "+ CString( size_Y/2.0 ) + L" "+ CString( 0.0 );
            aPoints += L" "+ CString( size_X/2.0 ) + L" "+ CString( size_Y/2.0 ) + L" "+ CString( 0.0 ); 
            aPoints += L" "+ CString( size_X/2.0 ) + L" "+ CString( -size_Y/2.0 ) + L" "+ CString( 0.0 );
            aPoints += L" "+ CString( -size_X/2.0 ) + L" "+ CString( -size_Y/2.0 ) + L" "+ CString( 0.0 );
            //--
            f << "\nTransformBegin \n";
            f << "Translate "<< light_from.GetX() <<" "<< light_from.GetY() <<" "<< light_from.GetZ() <<"\n";
            if (A_rotation != L"")
            {
                f << A_rotation.GetAsciiString() <<"\n";;
            }
            //--
            f << "\nAreaLightSource \"area\"\n";
	        f << "  \"float gain\" [" << vIntensity << "]\n";
            f << "  \"float importance\" [1.0]\n"; // TODO
	        f << "  \"float power\" ["<< float(o.GetParameterValue(L"LightEnergyIntens"))/100 <<"]\n"; //-- ;
	        f << "  \"float efficacy\" [17.0]\n";
	        f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
	        f << "  \"integer nsamples\" ["<< (U_samples + V_samples)/2 <<"]\n"; 
	        if ( vUse_IES )
            {
                f << "  \"string iesname\" [\"" << replace(ies_file.GetAsciiString()) << "\"]\n";
            }           
            //--
            f << "\nShape \"trianglemesh\"\n";
	        f << "  \"integer indices\" [0 1 2 0 2 3]\n";
	        f << "  \"point P\" ["<< aPoints.GetAsciiString() <<"]\n";
            //--
            f << "\nTransformEnd \n";         
        }
        else
        {
            f << "\nLightSource \"point\"\n";
            f << "  \"float gain\" [" << vIntensity << "]\n";
            f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
            if ( vUse_IES )
            {
                f << "  \"string iesname\" [\"" << replace(ies_file.GetAsciiString()) << "\"]\n";
                f << "  \"bool flipz\" [\"true\"]\n"; // TODO; option into 'special' tab
            }
            f << "  \"point from\" [" << light_from.GetX() << " " << light_from.GetY() << " " << light_from.GetZ() << "]\n";
        }
    }
}
//--
void writeLuxsiShader()
{
    // Writes shader
 
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

    for ( LONG i=0; i < materials.GetCount(); i++ )
    {
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
        bool vText=false, vNorm=false;
        //CValue vDiffType,vMore,vCol,vMore2,vMore3,vTexStr,vMore1,vMore4,vMore5;
        Material m( materials[i] );

        if ( int(m.GetUsedBy().GetCount())== 0 ) continue;
        
        
        CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
        Shader s(shad[0]);
        CString vMatID((s.GetProgID()).Split(L".")[1]);
        char sname[256];
        strcpy(sname,m.GetName().GetAsciiString());

        if ( !find(aMatList, m.GetName() ) ) aMatList.Add(m.GetName()); 
        else continue;
       
        //app.LogMessage(L"Shader: " + CString(Parameter(s.GetParameterValue(L"bump")).GetValue()));

        if (vMatID==L"lux_glass") 
        {
            shaderType=L"glass";
            s.GetColorParameterValue(L"kt",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
            s.GetColorParameterValue(L"kr", red, green, blue, alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            shaderString += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"index")) + L"]\n";
            shaderString += L"  \"float cauchyb\" [" + CString((float)s.GetParameterValue(L"cauchyb")) + L"]\n";
            shaderString += L"  \"bool architectural\" [\""+ CString((bool)s.GetParameterValue(L"architectural")) + L"\"]\n";
            //---------          
            vIsSet=true;
        }
        else if (vMatID==L"lux_roughglass") 
        {
            shaderType=L"roughglass";
            s.GetColorParameterValue(L"kt",red,green,blue,alpha );
            shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString((float)s.GetParameter(L"uroughness").GetValue())+L"] \"float vroughness\" ["+CString((float)s.GetParameter(L"vroughness").GetValue())+L"]\n";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            shaderString += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"index")) + L"]\n";
            shaderString += L"  \"float cauchyb\" [" + CString((float)s.GetParameterValue(L"cauchyb")) + L"]\n";
            //---------
            vIsSet=true;
        } 
        else if (vMatID==L"lux_matte") 
        {
            shaderType=L"matte";
            s.GetColorParameterValue(L"kd",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [" + CString((float)s.GetParameterValue(L"sigma")) + L"]\n";
            //----------
            vIsSet=true;
        } 
        else if (vMatID==L"lux_mirror") 
        {
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderType=L"mirror";
            shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            //----------
            vIsSet=true;
        } 
        else if (vMatID==L"lux_metal") 
        {
            const char *ametal [5] = {"amorphous carbon", "silver", "gold", "copper", "aluminium"}; 
            int nmetal = s.GetParameterValue(L"mname");
            //--
            shaderString += L"  \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
            shaderString += L"  \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
            shaderString += L"  \"string name\" [\""+ CString(ametal[nmetal]) + L"\"]\n";
            shaderString += L"  \"string type\" [\"metal\"] \n";
            ret=9;
            vIsSet=true;
            
        }
        else if (vMatID==L"lux_shinymetal") 
        {
            shaderType=L"shinymetal";
            s.GetColorParameterValue(L"ks",red,green,blue,alpha );
            shaderString += L"  \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]";
            shaderString += L"  \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]";
            shaderString += L"  \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
            vIsSet=true;
        }
        else if (vMatID==L"lux_substrate") 
        {
            shaderType=L"substrate";
            s.GetColorParameterValue(L"kd",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]";
            shaderString += L"  \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"uroughness"))) + L"]";
            shaderString += L"  \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"vroughness"))) + L"]\n";
            s.GetColorParameterValue(L"ks",red,green,blue,alpha );
            shaderString += L"  \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
            vIsSet=true;
        }
        else if (vMatID==L"lux_mattetranslucent") 
        {
            shaderType=L"mattetranslucent";
            s.GetColorParameterValue(L"kr",red,green,blue,alpha );
            shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            s.GetColorParameterValue(L"kt",red,green,blue,alpha );
            shaderString += L"  \"color Kt\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [" + CString((float)s.GetParameterValue(L"sigma")) + L"]\n";
            vIsSet=true;
        }
        // Material Stuff
        else if (vMatID==L"lux_car_paint") 
        {
            //-- car paint
            const char *A_carpaint [8] = {"2k acrylack", "blue", "blue matte", "bmw339",
                "ford f8", "opel titan", "polaris silber", "white"};
            int presets = s.GetParameterValue(L"presets");
               
            float spr,spg,spb,spa,spr2,spg2,spb2,spa2,r,g,b,a;
            s.GetColorParameterValue(L"spec",spr,spg,spb,spa ); //spec primary
            s.GetColorParameterValue(L"spec_sec",spr2,spg2,spb2,spa2 ); // spec secondary
            s.GetColorParameterValue(L"kd",r,g,b,a );
            s.GetColorParameterValue(L"reflectivity",r,g,b,a );
            //--
            if ( presets == 0 )
            {
                shaderString += L"  \"color Kd\" ["+ CString(r) + L" "+ CString(g) + L" "+ CString(b) + L"] \n";
	            shaderString += L"  \"color Ks1\" ["+ CString(spr) + L" "+ CString(spg) + L" "+ CString(spb) + L"] \n";
	            shaderString += L"  \"color Ks2\" ["+ CString(spr2) + L" "+ CString(spg2) + L" "+ CString(spb2) + L"] \n";
	            shaderString += L"  \"color Ks3\" ["+ CString(spr2) + L" "+ CString(spg2) + L" "+ CString(spb2) + L"] \n";
	            shaderString += L"  \"float M1\" ["+ CString(float(s.GetParameterValue(L"m1"))) + L"] \n";
	            shaderString += L"  \"float M2\" ["+ CString(float(s.GetParameterValue(L"m2"))) + L"] \n";
	            shaderString += L"  \"float M3\" ["+ CString(float(s.GetParameterValue(L"m3"))) + L"] \n";
	            shaderString += L"  \"float R1\" ["+ CString(float(s.GetParameterValue(L"r1"))) + L"] \n";
	            shaderString += L"  \"float R2\" ["+ CString(float(s.GetParameterValue(L"r2"))) + L"] \n";
	            shaderString += L"  \"float R3\" ["+ CString(float(s.GetParameterValue(L"r3"))) + L"] \n";
            }
            else
            {
                shaderString += L"  \"string name\" [\""+ CString(A_carpaint[presets]) + L"\"]\n"; // presets
            }
	        shaderType=L"carpaint";
            //--
            vIsSet=true;
        } 
        else if (vMatID==L"mia_material_phen") 
        {
            // arch vis
            s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
            s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
            mRough=(float)s.GetParameterValue(L"reflect_glossy");
            float refl=(float)s.GetParameterValue(L"reflectivity");
            float brdf=(float)s.GetParameterValue(L"brdf_0_degree_refl");
            //--
            if (refl > 0) 
            {
                shaderType = L"shinymetal";
                shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] ";
                shaderString += L" \"float uroughness\" ["+ CString(1-mRough)+L"] \"float vroughness\" ["+CString(1-mRough)+L"]\n";
                shaderString += L" \"color Ks\" [" + CString(refl*sp_red*brdf) + L" " + CString(refl*sp_green*brdf) + L" " + CString(refl*sp_blue*brdf) + L"]\n";
            } 
            else 
            {
                shaderType=L"matte";
                s.GetColorParameterValue(L"kd",red,green,blue,alpha );
                shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderString += L"  \"float sigma\" [" + CString(mRough) + L"]\n";
            }
            //--
            vIsSet=true;
        }
        else if (vMatID==L"material-phong") 
        {
            shaderType=L"glossy";
            s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
            shaderString += L"  \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]";
            shaderString += L"  \"float vroughness\" ["+CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]\n";
            s.GetColorParameterValue(L"specular",red,green,blue,alpha );
            shaderString += L"  \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            //--
            vIsSet=true;
        } 
        else if (vMatID==L"material-lambert")
        {
            shaderType=L"matte";
            s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [0]\n";
            //--
            vIsSet=true;
        } 
        else if (vMatID==L"material-ward")
        {
            s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
            shaderType=L"glossy";
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
            shaderString += L"  \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_u"))/10)+ L"]";
            shaderString += L"  \"float vroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_v"))/10)+ L"]\n";
            shaderString += L"  \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            //--
            vIsSet=true;
        } 
        else if (vMatID==L"material-constant")
        {
            s.GetColorParameterValue(L"color",red,green,blue,alpha );
            shaderType=L"matte";
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [0]\n";
            //--
            vIsSet=true;
        } 
        else if (vMatID==L"material-strauss")
        {
            s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
            shaderType=L"matte";
            shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            shaderString += L"  \"float sigma\" [0]\n";
            //--
            vIsSet=true;
        }
        //-- if yet vMatID is defined......
        if (!vIsSet) 
        {
            if (s.GetParameterValue(L"refract_inuse")=="-1")
            {
                //check if material is transparent: phong/lamber/blin/constant/cooktorrance/strauss
                float ior=0.0f;
                s.GetColorParameterValue(L"transparency",red,green,blue,alpha );
                if (red>0 || green>0 || blue>0)
                {
                    shaderType=L"glass";
                    shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                    s.GetColorParameterValue(L"reflectivity",sp_red,sp_green,sp_blue,sp_alpha );
                    shaderString += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
                    shaderString += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"index_of_refraction")) + L"]\n";
                    shaderString += L"  \"float cauchyb\" [0]\n";
                    //--
                    if ((float)s.GetParameterValue(L"trans_glossy")>0 )
                    {
                        shaderType=L"roughglass";
                        shaderString += L"  \"float uroughness\" ["+ CString((float)s.GetParameterValue(L"trans_glossy"))+ L"]";
                        shaderString += L"  \"float vroughness\" ["+ CString((float)s.GetParameterValue(L"trans_glossy"))+ L"]\n";
                    }
                }
            }
        }
        if (!vIsSet)
        {
            if ( (float)s.GetParameterValue(L"transparency") > 0.0f )
            {
                float ior=0.0f;
                // glass mia-arch shader
                s.GetColorParameterValue(L"refr_color",red,green,blue,alpha );
                s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
                shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderString += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
                shaderString += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"refr_ior")) + L"]\n";
                shaderString += L"  \"float cauchyb\" [0]\n";
                shaderType=L"glass";
                if ((float)s.GetParameter(L"refr_gloss").GetValue()<1 )
                {
                    shaderType=L"roughglass";
                    shaderString += L"  \"float uroughness\" ["+ CString(1.0f - float(s.GetParameterValue(L"refr_gloss"))) + L"]";
                    shaderString += L"  \"float vroughness\" ["+ CString(1.0f - float(s.GetParameterValue(L"refr_gloss"))) + L"]\n";
                }
            }
            vIsSet=true;
        }
        if (!vIsSet) 
        {
            // check if its a reflecting material
            float a,b,c,d;
            s.GetColorParameterValue(L"diffuse",a,b,c,d );
            if (s.GetParameterValue(L"reflect_inuse")== "-1" )
            {
                if (vMatID==L"mia_material_phen") 
                {
                    s.GetColorParameterValue(L"refl_color",red,green,blue,alpha );
                    mRough = 1 - float(s.GetParameterValue(L"refl_gloss"));
                    red = red*a;
                    green = green*b;
                    blue = blue*c;
                }
                else
                {
                    s.GetColorParameterValue(L"reflectivity",red,green,blue,alpha );
                    mRough = (float)s.GetParameterValue(L"reflect_glossy");
                }
                if (red>0 || green>0 || blue>0)
                {
                    shaderType=L"shinymetal";
                    shaderString += L"  \"color Kr\" ["+ CString(a) + L" "+ CString(b) + L" "+ CString(c) + L"] ";
                    shaderString += L" \"float uroughness\" ["+ CString(mRough/10) + L"] \"float vroughness\" ["+ CString(mRough/10)+ L"]\n";
                    shaderString += L" \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                    vIsSet=true;
                }
            }
        }
        if (!vIsSet)
        {
            //app.LogMessage(vMatID);

                // Get Textures
                
                //JScript Version to find shader in slot:

                //s = Selection(0).Material.Shaders(0);
                //for (i=0;i<s.parameters.count;i++){
                //t=s.parameters(i).source;
                //if (t) logmessage(s.parameters(i).name + "=" + t);
               
        }
        else 
        {
            // fall back shader
            shaderType=L"matte";
            shaderString += L"  \"color Kd\" [0.7 0.7 0.7]\n";
            shaderString += L"  \"float sigma\" [0]\n";
        }
        
        //-- search for texture image
        CRefArray vImags=m.GetShaders();
        for (int i=0; i < vImags.GetCount(); i++)
        {
            CRefArray vImags2=Shader(vImags[i]).GetShaders();
            for (int j=0;j<vImags2.GetCount();j++)
            {
                CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
                if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1")
                {
                    vTexture=vImags2[j];

                    ImageClip2 vImgClip(vTexture.GetImageClip() );
                    Source vImgClipSrc(vImgClip.GetSource());
                    CString vFileName = vImgClipSrc.GetParameterValue( L"path");

                    CString textproj  = vTexture.GetParameterValue(L"tspace_id");
                    app.LogMessage(CString(textproj.GetAsciiString())); // TODO; search "type" projection

                    if ((bool)vTexture.GetParameterValue(L"bump_inuse")!=false) 
                    {
                        vNorm = true; vChanel = L"bumpmap"; vChanType = L"float";
                        texFact = vTexture.GetParameterValue(L"factor");
                    }
                    else
                    {
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
        f << shaderString.GetAsciiString();
        if (shaderTexture !=L"")
        {
            f << shaderTexture.GetAsciiString();
        }
    }
}
//--
int writeLuxsiSurface(X3DObject o, CString vType)
{
    //--
    Geometry g(o.GetActivePrimitive().GetGeometry()) ;
    CRefArray mats(o.GetMaterials()); // Array of all materials of the object
    Material m = mats[0];
    CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
    Shader s(shad[0]);
    //--
    KinematicState  gs = o.GetKinematics().GetLocal(); 
    CTransformation gt = gs.GetTransform();
    //--
    f << "\nAttributeBegin\n";
    CVector3 axis;
        double rot = gt.GetRotationAxisAngle(axis);
        //-- TODO; changed for matrix
        f << "\nTransformBegin\n";
        f << "\nTranslate "<< gt.GetPosX() <<" "<< gt.GetPosY() <<" "<< gt.GetPosZ() <<"\n";
        if (rot!= 0)
        {
            f << "Rotate " << (rot*180/PI) << " "<< axis[0] << " " << axis[1] << " "<< axis[2] << "\n";
        }
        if (gt.GetSclX()!=1 || gt.GetSclY()!=1 || gt.GetSclZ()!=1) 
        {
            f << "Scale " << gt.GetSclX() << " " << gt.GetSclY() << " "<< gt.GetSclZ() << "\n";
        }
        //--
        float vradius = o.GetParameterValue(L"radius");// 
        float vzmin = o.GetParameterValue(L"startuangle");
        float vzmax = o.GetParameterValue(L"enduangle");
        float vphimax = o.GetParameterValue(L"endvangle");
        //--
        f << " Shape  \"sphere\" \n";
            f << "  \"float radius\" [" << vradius << "]\n";
            f << "  \"float zmin\" [" << -vzmax /2  << "]\n";
            f << "  \"float zmax\" [" << vzmax /2  << "]\n";
            f << "  \"float phimax\" [" << vphimax * 2 << "]\n";
            //--
        f << "TransformEnd\n";
    f << "\nAttributeEnd\n";
    return 0;
}
//--
int writeLuxsiObj(X3DObject o, CString vType)
{
    // Writes objects
    //
    CScriptErrorDescriptor status ;
    CValueArray fooArgs(1) ;
    fooArgs[0] = L"" ;
    CValue retVal=false ;
    bool vIsMeshLight=false;
    bool vIsSet=false;
    bool vText = false, vIsSubD = false;
    bool vIsMod=false;

    Geometry g(o.GetActivePrimitive().GetGeometry()) ;
    CRefArray mats(o.GetMaterials()); // Array of all materials of the object
    Material m = mats[0];
    CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
    Shader s(shad[0]);
    CGeometryAccessor ga;
    CString vUV=L"",vNormals=L"",vTris=L"",vMod=L"",vPoints=L"";
    
    LONG subdLevel = 0;
    Property geopr = o.GetProperties().GetItem(L"Geometry Approximation");
    if ((int)geopr.GetParameterValue(L"gapproxmordrsl") > 0 ) 
    {
        vIsSubD = true;
        subdLevel = (int)geopr.GetParameterValue(L"gapproxmordrsl"); //-- only render
    }
    //--
    CRefArray vImags=m.GetShaders();
    for (int i=0;i<vImags.GetCount();i++)
    {
        CRefArray vImags2 = Shader(vImags[i]).GetShaders();
        for (int j=0;j<vImags2.GetCount();j++)
        {
            CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
            if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1")
            {
                vText = true;
            }
        }
    }
    //--
    //CTransformation localTransformation = ga.GetTransform();
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal(); 
    CTransformation global_trans = global_kinec_state.GetTransform();
    //CMatrix4 mat4(gt.GetMatrix4()); 
   //--
    if (int(g.GetTriangles().GetCount()) > 0 )
    {
        //--
        CTriangleRefArray triangles(g.GetTriangles()); // miga
        CLongArray indices( triangles.GetIndexArray() ); 

        CVector3Array allPoints(triangles.GetCount()*3);
        CVector3Array allUV(triangles.GetCount()*3);
        CVector3Array allNormals(triangles.GetCount()*3);

        long index=0;
        for (int i=0; i<triangles.GetCount();i++)
        {
            Triangle triangle(triangles.GetItem(i));
            for (int j=0;j<triangle.GetPoints().GetCount();j++)
            {
                TriangleVertex vertex0(triangle.GetPoints().GetItem(j));
                CVector3 pos(vertex0.GetPosition());
                CVector3 normal(vertex0.GetNormal());
                CUV uvs(vertex0.GetUV());
                //--
                long arrayPos = index++;
                allPoints[arrayPos] = pos;
                allNormals[arrayPos] = normal;
                allUV[arrayPos] = CVector3(uvs.u, uvs.v,0);
                vTris += L" "+ CString(arrayPos) + L" ";
            }
            vTris += L"\n";
        }
                             
        //-- test to optimize processes
        if ( !vplymesh )
        {
            for (LONG j=0; j < allPoints.GetCount(); j++)
            {
                //-- test
                CVector3 new_pos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
                new_pos.MulByTransformationInPlace(global_trans);
                vPoints += L" "+ CString(new_pos[0]) + L" "+  CString(new_pos[1]) + L" "+ CString(new_pos[2]) + L"\n";
                //--
                CVector3 tr_normals(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
                tr_normals.MulByTransformationInPlace(global_trans);
                vNormals +=  L" "+ CString(tr_normals[0]) + L" "+  CString(tr_normals[1]) + L" "+ CString(tr_normals[2]) + L"\n";
                //-- UV need transpose?                
                vUV +=  L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]) + L"\n";     
            } 
        }
        //--
        f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
        //--
        f << "\nNamedMaterial \""<< m.GetName().GetAsciiString() <<"\"\n";

        //-- Geometry associated to lights
        bool vIsPortal = false;
        string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
        if (loc != string::npos) vIsPortal = true;
        //-- meshlight
        if (float(s.GetParameterValue(L"inc_inten"))> 0 ) vIsMeshLight = true;
        CString type_mesh = L"mesh";
        if ( vplymesh ) type_mesh = L"plymesh";
        CString type_shape = L"Shape";
        if ( vIsPortal ) type_shape = L"PortalShape";
        
        //-----------------
        if ( vIsMeshLight )
        //-----------------
        {
            //--
            float red = 0.0f, green = 0.0f, blue = 0.0f, alpha = 0.0f;
            s.GetColorParameterValue(L"incandescence",red,green,blue,alpha );
            float inc_intensity(s.GetParameterValue(L"inc_inten"));
            //--
            CString lName = findInGroup(o.GetName());
            if (lName == L"") lName = o.GetName().GetAsciiString();
            //--
            f << " LightGroup \"" << lName.GetAsciiString() << "\"\n";
            f << "\nAreaLightSource \"area\" \n";
            f << "  \"float importance\" [1.00] \n";
            f << "  \"float gain\" ["<< inc_intensity <<"] \n";
            //f << "  \"float power\" [100.0]  \"float efficacy\" [17.0] \n";
            f << "  \"color L\" ["<< (red * inc_intensity) <<" "<< (green * inc_intensity) <<" "<<(blue * inc_intensity ) <<"]\n";
        }
        f << type_shape.GetAsciiString() <<" \""<< type_mesh.GetAsciiString() <<"\" \n";
        //-------------
        if ( vplymesh )
        //-------------
        {
            //--
            CString vInput_FileName = vFileObjects.GetAsciiString();
            int Loc = (int)vInput_FileName.ReverseFindString(".");
            vFilePLY = vInput_FileName.GetSubString(0,Loc) + L"_"+ o.GetName() + L".ply";
            f << "  \"string filename\" [\"" << replace(vFilePLY.GetAsciiString()) << "\"] \n";
        }
        //-- share
        f << "  \"integer nsubdivlevels\" [" << subdLevel  <<"] \"string subdivscheme\" [\"loop\"] \n";
        f << "  \"bool dmnormalsmooth\" [\""<< MtBool[vSmooth_mesh] <<"\"]";
        f << "  \"bool dmsharpboundary\" [\""<< MtBool[vSharp_bound] <<"\"] \n";         
        //f << "  \"string displacementmap\" [\"none\"]\n"; 
        //f << "  \"float dmscale\" [\"0.0\"] \"float dmoffset\" [\" 0.0\"]\n"; 

        //--------------
        if ( !vplymesh )
        //--------------
        {
            f << "  \"string acceltype\" [\""<< MtAccel[vAccel] <<"\"] \"string tritype\" [\"wald\"] \n";//TODO
            f << "  \"integer triindices\" [\n" << vTris.GetAsciiString() << "\n ]";
            f << "  \"point P\" [\n" << vPoints.GetAsciiString() << "\n ]";
            if ( vSmooth_mesh && vNormals != L"" )
            {
                f << " \"normal N\" [\n" << vNormals.GetAsciiString() << "\n ]";
            }
            //--
            if ( vText && vUV != L"" )
            {
                f << " \"float uv\" [\n" << vUV.GetAsciiString() << "\n ]";   
            }
        }
        //--
        f << "\nAttributeEnd #" << o.GetName().GetAsciiString() << "\n";
    } 
    return 0;
}
//--
void write_ply_object(X3DObject o, CString vFilePLY)
{
    Geometry g(o.GetActivePrimitive().GetGeometry());
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal(); 
    CTransformation global_trans = global_kinec_state.GetTransform();
    
    //--
    CTriangleRefArray triangles(g.GetTriangles());
    CLongArray indices(triangles.GetIndexArray());

    CVector3Array allPoints(triangles.GetCount()*3);
    CVector3Array allUV(triangles.GetCount()*3);
    CVector3Array allNormals(triangles.GetCount()*3);

    long index=0; 
    for (int i=0; i<triangles.GetCount();i++)
    {
        Triangle triangle(triangles.GetItem(i));
        for (int j=0;j<triangle.GetPoints().GetCount();j++)
        {
             TriangleVertex vertex0(triangle.GetPoints().GetItem(j));
             CVector3 pos(vertex0.GetPosition());
             CVector3 normal(vertex0.GetNormal());
             CUV uvs(vertex0.GetUV());
             //--
             long arrayPos = index++;
             allPoints[arrayPos] = pos;
             allNormals[arrayPos] = normal;
             allUV[arrayPos] = CVector3(uvs.u, uvs.v,0);
        }
    }
    //----------------------------------------------------------
    //-- vertex
    long vCount(triangles.GetCount()*3);
    long pCount(g.GetTriangles().GetCount());
       
    //-- vertex
    CString sPos;
    for (LONG j=0; j < allPoints.GetCount(); j++)
    {
        //-- vertex
        CVector3 point_pos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
        point_pos.MulByTransformationInPlace(global_trans);
        //-- normals
        CVector3 norm_idx(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
        norm_idx.MulByTransformationInPlace(global_trans);
        //-- write all
        sPos += CString(point_pos[0]) + L" "+ CString(point_pos[1]) + L" "+ CString(point_pos[2]);
        if ( vSmooth_mesh ){
        sPos += L" "+ CString(norm_idx[0]) + L" "+ CString(norm_idx[1]) + L" "+ CString(norm_idx[2]);
        }
        sPos += L"\n";
    }
    //-- triangles
    CString vFaces;
    LONG nIndices = indices.GetCount();
    for ( LONG k=0; k < nIndices; k += 3 )
    {
        //--
        vFaces += L"3 "+ CString(int(k)) + L" "+ CString(int(k+1)) + L" "+ CString(int(k+2));
        vFaces += L"\n";   
    }
    //--
    
    vFilePLY += L"_"+ o.GetName() + L".ply";
    //FILE * file=fopen(vFilePLY.GetAsciiString(), "wb"); // from kies project
   // f.open(vFilePLY.GetAsciiString(), ios::out | ios::binary);
    f << "ply\n";
    f << "format ascii 1.0\n";
	f << "comment LuXSI; LuxRender Exporter for Autodesk Softimage\n";
	f << "element vertex "<< CString(vCount).GetAsciiString() <<"\n";
	f << "property float x\n";
	f << "property float y\n";
	f << "property float z\n";
   /*
    f << "property uchar red\n";
	f << "property uchar green\n";
	f << "property uchar blue\n";
    */
    if ( vSmooth_mesh ) {
        f << "property float nx\n";
	    f << "property float ny\n";
	    f << "property float nz\n";
    }
	f << "element face "<< CString(pCount).GetAsciiString() <<"\n";
	f << "property list uchar uint vertex_indices\n";
	f << "end_header\n";
   
    //-- vertex
    f << sPos.GetAsciiString();
    //-- faces
    f << vFaces.GetAsciiString();
	
	f.close();
    //--

}
        
//--
int writeLuxsiCloud(X3DObject obj)
{
    //-----------------------------------------------------------------
    ICEAttribute attr = obj.GetActivePrimitive().GetGeometry().GetICEAttributeFromName( L"PointPosition" );

		CICEAttributeDataArrayVector3f points;
		attr.GetDataArray( points );

		Application xsi;
		for( ULONG i=0; i<points.GetCount( ); i++ )
		{
			xsi.LogMessage( CString( points[ i ] ) );
		}

    //-------------------------------------------------------------
       
    //
    // Writes pointclouds
    //
/*
    ICEAttribute attr;
    CICEAttributeDataArrayVector3f aPointPosition; 
    CICEAttributeDataArrayLong aID;
    CICEAttributeDataArrayFloat aSize;
    CICEAttributeDataArrayVector3f aVel;

    CRefArray attrs = obj.GetActivePrimitive().GetGeometry().GetICEAttributes();

    for( int i = 0; i<attrs.GetCount(); i++ ) {
            ICEAttribute attr = attrs[i];
            
            app.LogMessage( L"*******************************************************************" );
            app.LogMessage( L"Name: " + attr.GetName() );
            app.LogMessage( L"DataType: " + CString(attr.GetDataType()) );
            app.LogMessage( L"StructType: " + CString(attr.GetStructureType()) );
            app.LogMessage( L"ContextType: " + CString(attr.GetContextType()) );
            app.LogMessage( L"IsConstant: " + CString(attr.IsConstant()) );
            app.LogMessage( L"Readonly: " + CString(attr.IsReadonly()) );
            app.LogMessage( L"AttributeCategory: " + CString(attr.GetAttributeCategory()) );
            app.LogMessage( L"Element count: " + CString(attr.GetElementCount()) );
           //--
            if (attr.GetName() == L"PointPosition"){
                attr.GetDataArray(aPointPosition);
            }
            if (attr.GetName() == L"State_ID"){
                attr.GetDataArray(aID);
            }
            if (attr.GetName() == L"Size"){
                attr.GetDataArray(aSize);
            }
            if (attr.GetName() == L"PointVelocity"){
                attr.GetDataArray(aVel);
            }
        }

*/

    //for (unsigned int i=0;i<aPointPosition.GetCount();i++)
        for (unsigned int i=0;i<points.GetCount();i++)
    {
        // get all points

        f << "\nAttributeBegin #\n";//" << obj.GetName().GetAsciiString() << (int)aID[i] << "\" \n";
        f << "  Translate " << (float)points[i][0] << " " << (float)points[i][1] << " "<< (float)points[i][2] << "\n";
        f << "  Material \"matte\" \"color Kd\" [0 0.8 0.8 ]\n";
        f << "  Shape \"sphere\" \"float radius\" [0.5]\n";
        // f << "  Shape \"sphere\" \"float radius\" [" << (float)aSize[i] << "]\n";
        f << "AttributeEnd #"<< obj.GetName().GetAsciiString()<<"\n";
        //--------------f << " Shape  \"sphere\" \n";
/*           f << "  \"float radius\" [" << vradius << "]\n";
            f << "  \"float zmin\" [" << -vzmax /2  << "]\n";
            f << "  \"float zmax\" [" << vzmax /2  << "]\n";
            f << "  \"float phimax\" [" << vphimax * 2 << "]\n";
            //--
*/
    }

    return 0;
   
}
//--
int writeLuxsiInstance(X3DObject o){
    // instance
    //write source object [won't be displayed]
    Model vModel = Model(o).GetInstanceMaster();
    CRefArray vGroup = X3DObject(vModel).GetChildren();

    if ( find(aInstanceList, vModel.GetName() ) ) 
    {
        //return 0;
    }
    else
    {
        f << "\nObjectBegin \""<< vModel.GetName().GetAsciiString()<<"\"";
        for (int i=0; i < vGroup.GetCount(); i++)
        {
            writeLuxsiObj(X3DObject(vGroup[i]),L"instance");
        }
        f << "\nObjectEnd #"<< o.GetName().GetAsciiString()<<"\n";
        aInstanceList.Add(vModel.GetName());
    }

    f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
    KinematicState  gs = o.GetKinematics().GetGlobal();
    CTransformation gt = gs.GetTransform();
    CMatrix4 mat4(gt.GetMatrix4()); 

    CVector3 axis;
    double rot = gt.GetRotationAxisAngle(axis);
    //
    f << "\nTranslate "<< gt.GetPosX() <<" "<< gt.GetPosY() <<" "<< gt.GetPosZ() <<"\n";
        //--
    if (rot!= 0)
        {
            f << "Rotate " << (rot*180/PI) << " "<< axis[0] << " " << axis[1] << " "<< axis[2] << "\n";
        }
    if (gt.GetSclX()!=1 || gt.GetSclY()!=1 || gt.GetSclZ()!=1) //-- changed && to ||, add support for not uniform scale
        {
            f << "Scale " << gt.GetSclX() << " " << gt.GetSclY() << " "<< gt.GetSclZ() << "\n";
        }
        
    f << "ObjectInstance \"" << Model(o).GetInstanceMaster().GetName().GetAsciiString() <<"\"\n";
    f << "AttributeEnd #" << o.GetName().GetAsciiString() << "\n\n";
    return 0;
}
//--
/*
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
      data += x;
   }
   //app.LogMessage(L""+CString(data));
   load.close();
   return data;
}
*/
//--
void write_header_files()
{
    //-- commons header for files .lxm and .lxo
    f <<"\n# File created by Luxrender Exporter for Softimage; Luxsi. \n";
    f <<"# Copyright (C) 2010  Michael Gangolf \n";
    f <<"# Developed by Michael Gangolf, aka Miga, \n";
    f <<"# continued by P. Alcaide, aka povmaniaco. \n \n";
}
//--
void luxsi_mat_preview()
{
    //--
    writeLuxsiShader();
    if ( find(aMatList, L"Preview" ) ){
       
    
    //------------------------------------
        f << " Shape  \"sphere\" \n";
        f << "  \"float radius\" [1.0]\n";
        f << "  \"float zmin\" [-0.5]\n";
        f << "  \"float zmax\" [0.5]\n";
        f << "  \"float phimax\" [1.0]\n";
            //--
    }
}
void luxsi_write()
{
    // write objects, materials, lights, cameras
    root= app.GetActiveSceneRoot();
    vIsLinux = CUtils::IsLinuxOS(); // linux check
    CScriptErrorDescriptor status ;
    CValueArray fooArgs(1) ;
    fooArgs.Clear();
    CValue retVal2="";

    // only write settings
    if ( vFileObjects == L"" )
    {
        CString def_lxs_file = app.GetInstallationPath(siProjectPath);
        def_lxs_file += L"/def.lxs";
        app.LogMessage( L"File path export is empty, used default path: "+ def_lxs_file );
        vFileObjects = def_lxs_file; 
    }

    if (vFileObjects != L""){

        CRefArray array,aObj,aLight,aCam,aSurfaces,aClouds,aInstance;
        //--
        CRefArray aPlymesh;
        aPlymesh.Clear();
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
            app.LogMessage( L"\tObject name: " + o.GetName() + L":" + o.GetType() + L" parent:"+ X3DObject(o.GetParent()).GetType());
            //--
            Property visi = o.GetProperties().GetItem(L"Visibility");
            bool view_visbl = (bool)visi.GetParameterValue(L"viewvis");
            bool rend_visbl = (bool)visi.GetParameterValue(L"rendvis");

            //-- Collection objects / visibilty check
            if (o.GetType()==L"polymsh")
            {
                if (vIsHiddenObj || (vIsHiddenObj == false && (view_visbl == true && rend_visbl == true)))
                {
                    aObj.Add(o); // for create link into _geo.lxo file
                    if ( vplymesh ) aPlymesh.Add(o); // for write .ply file
                }
            }
            if (o.GetType()==L"CameraRoot")
            {
                if (vIsHiddenCam || (vIsHiddenCam==false && (view_visbl == true && rend_visbl == true)))
                {
                    aCam.Add(o);    
                }
            }
            if (o.GetType()==L"camera" && X3DObject(o.GetParent()).GetType()!=L"CameraRoot")
            {
                if (vIsHiddenCam || (vIsHiddenCam == false && (view_visbl == true && rend_visbl == true)))
                {
                    aCam.Add(o); 
                }
            }
            if (o.GetType()==L"light")
            {
                if (vIsHiddenLight || (vIsHiddenLight == false && (view_visbl == true && rend_visbl == true)))
                {
                    aLight.Add(o);  
                }
            }
            if (o.GetType()==L"surfmsh")
            {
                if (vIsHiddenSurface || (vIsHiddenSurface == false && (view_visbl == true && rend_visbl == true)))
                {
                    aSurfaces.Add(o);
                   // if ( vplymesh ) aPlymesh.Add(o);
                }
            }
            if (o.GetType()==L"pointcloud")
            {
                if (vIsHiddenClouds || (vIsHiddenClouds == false && (view_visbl == true && rend_visbl == true)))
                {
                    aClouds.Add(o); 
                }
            }
            if (o.GetType()==L"#model")
            {   // model
                if (Model(o).GetModelKind()==2)
                {   // instances
                    if (vIsHiddenObj || (vIsHiddenObj == false && (view_visbl == true && rend_visbl == true)))
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
            app.LogMessage(L"Any objects/surfaces to export! Check if objects are visible in XSI or switch on 'export hidden objects / surfaces'",siFatalMsg );
        }
        else if (aCam.GetCount()==0)
        {
            app.LogMessage(L"Any camera to export! Check if a camera is visible in XSI or switch on 'export hidden cameras'",siFatalMsg );
        }
        else
        {

            //-- write files
            CString vFileLXM = L"", vFileLXO = L"";
            CString vInput_FileName = vFileObjects.GetAsciiString();
            int Loc = (int)vInput_FileName.ReverseFindString(".");
            vFileLXM = vInput_FileName.GetSubString(0,Loc) + (L"_mat.lxm");
            vFileLXO = vInput_FileName.GetSubString(0,Loc) + (L"_geo.lxo");
            // vFileVOL = vInput_FileName.GetSubString(0,Loc) + (L"_vol.lxm");
            vFilePLY = vInput_FileName.GetSubString(0,Loc);


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
            f << "\nInclude \""<< vFileLXM.GetAsciiString() <<"\" \n";
            f << "Include \""<< vFileLXO.GetAsciiString() <<"\" \n";

            f << "\nAttributeBegin \n";

            //-- lights
            for (int i=0;i<aLight.GetCount();i++) writeLuxsiLight(aLight[i]);

            f << "\nAttributeEnd \n \n";

            f << "WorldEnd";

            f.close(); //-- end lxs

            //-- open file _mat.lxm 
            f.open(vFileLXM.GetAsciiString()); 

            //-- insert header
                write_header_files();

            //-- write materials
                writeLuxsiShader();

            f.close(); //--< end lxm

            //-- test ply
            for (int i=0;i<aPlymesh.GetCount();i++)
            {
                // if (write_ply_object(aPlymesh[i],vFilePLY)==-1) break;
                write_ply_object(aPlymesh[i],vFilePLY);
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }

            //-- open file  _geom.lxo 
            f.open(vFileLXO.GetAsciiString()); 

            //-- insert header
            write_header_files();

            //-- objects
            for (int i=0;i<aObj.GetCount();i++) 
            {
                if (writeLuxsiObj(aObj[i],L"obj")==-1) break;
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //-- surfaces
            for (int i=0;i<aSurfaces.GetCount();i++) {
                //if (writeLuxsiObj(aSurfaces[i],L"surface")==-1) break;
                if (writeLuxsiSurface(aSurfaces[i],L"surface")==-1) break;
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
void luxsi_execute()
{
    //-- make default path
    if ( vLuXSIPath == L"" )
    {
        CString def_exe_path = app.GetInstallationPath(siUserAddonPath);
        def_exe_path += L"/LuXSI/Application/bin/luxrender.exe";
        app.LogMessage(L"Path empty, used default path: "+ CString(def_exe_path));
        vLuXSIPath = def_exe_path;
    }
    //--
    if (vExportDone)
    {
        #ifdef __unix__
                pid_t pid = fork();
				if( 0 == pid ) 
                {
                     system ( ( vLuXSIPath +" \""+ vFileObjects).GetAsciiString());
                     exit(0);
                }
        #else
            // windows
            if (vRmode == 1) //-- console
            {
                //vLux_console += // added parameters
                vLuXSIPath = vLux_console;
            }
                CString exec = vLuXSIPath +" \""+ vFileObjects + "\"";
                app.LogMessage(exec);
                loader(exec.GetAsciiString()); //-- for execute in Windows systems
        #endif
    }
    else
    {
        app.LogMessage(L" Not data file exported, retry exporting scene, before render ",siErrorMsg );
    }
}