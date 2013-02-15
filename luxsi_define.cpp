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

#include "include\luxsi_main.h"

using namespace std;
using namespace XSI;

/**/
extern CString vFileExport;

/**/
extern CString vLuXSIPath;

/**/
extern CString vblxs_file;

/**/
extern CustomProperty prop;

/**/
extern Application app;

/**/
extern CString vmatPreview;

//-
XSIPLUGINCALLBACK CStatus LuXSI_Define( CRef& in_ctxt )
{
    Context ctxt( in_ctxt ); 
    //-
    int sps = siPersistable;
    CValue dft;  //- default

    Parameter oParam;
    prop = ctxt.GetSource();

    prop.AddParameter( L"use_hidden_obj",   CValue::siBool, sps,L"",L"", true,  oParam );
    prop.AddParameter( L"use_hidden_light", CValue::siBool, sps,L"",L"", true,  oParam );
    prop.AddParameter( L"use_hidden_surf",  CValue::siBool, sps,L"",L"", false, oParam );
    prop.AddParameter( L"use_hidden_cam",   CValue::siBool, sps,L"",L"", true,  oParam );
    prop.AddParameter( L"smooth_mesh",      CValue::siBool, sps,L"",L"", false, oParam );
    prop.AddParameter( L"sharp_bound",      CValue::siBool, sps,L"",L"", false, oParam );
    /**/
    prop.AddParameter( L"bplymesh", CValue::siBool, sps,L"",L"",    false,  oParam );
    prop.AddParameter( L"over_geo", CValue::siBool, sps,L"",L"",    false,  oParam );    
    
    //----/ image /-->
    prop.AddParameter( L"Width",    CValue::siInt4,  sps,L"",L"",   640,    oParam );
    prop.AddParameter( L"Height",   CValue::siInt4,  sps,L"",L"",   480,    oParam );
    prop.AddParameter( L"gamma",    CValue::siFloat, sps,L"",L"",   2.2,    oParam );
    
    //-- filter //
    prop.AddParameter( L"bfexpert", CValue::siBool,     sps,L"",L"", false, oParam );
    prop.AddParameter( L"ssample",  CValue::siBool,     sps,L"",L"", false, oParam );
    prop.AddParameter( L"bywidth",  CValue::siFloat,    sps,L"",L"", 2.0,   0.0f,10.0f,0.1f,2.0f,   oParam );
    prop.AddParameter( L"bxwidth",  CValue::siFloat,    sps,L"",L"", 2.0,   0.0f,10.0f,0.1f,2.0f,   oParam );
    prop.AddParameter( L"bfalpha",  CValue::siFloat,    sps,L"",L"", 2.0,   0.0f,10.0f,0.1f,2.0f,   oParam );
    prop.AddParameter( L"bF_B",     CValue::siFloat,    sps,L"",L"", 0.3,   0.0f,10.0f,0.1f,2.0f,   oParam );
    prop.AddParameter( L"bF_C",     CValue::siFloat,    sps,L"",L"", 0.3,   0.0f,10.0f,0.1f,2.0f,   oParam );
    prop.AddParameter( L"bTau",     CValue::siFloat,    sps,L"",L"", 0.3,   0.0f,10.0f,0.1f,2.0f,   oParam );
    prop.AddParameter( L"bfilter",  CValue::siInt4,     sps,L"",L"", 2,     0,10,0,10,              oParam );
    
    //-- volume integrator
    prop.AddParameter( L"bvolumeint",   CValue::siInt4, sps,L"",L"", 0, 0,10,0,10,  oParam ) ;
    
    //-- Surfaceint 
    prop.AddParameter( L"bsurfaceint",  CValue::siInt4, sps,L"",L"",   0,   0,10,0,10,  oParam );
    prop.AddParameter( L"bsexpert",     CValue::siBool, sps,L"",L"",   false,   oParam );

    //-- bi-directional
    prop.AddParameter( L"beye_depth",   CValue::siInt4,  sps,L"",L"", 16,   0,2048,0,48,    oParam );
    prop.AddParameter( L"blight_depth", CValue::siInt4,  sps,L"",L"", 16,   0,2048,0,48,    oParam );
    prop.AddParameter( L"beyerrthre",   CValue::siFloat, sps,L"",L"", 0.0,  0.0f,2048.0f,0.0,2048.0f,   oParam ); 
    prop.AddParameter( L"blightrrthre", CValue::siFloat, sps,L"",L"", 0.0,  0.0f,2048.0f,0.0,2048.0f,   oParam ); 
    
    //-- directlighting / path
    prop.AddParameter( L"binc_env",     CValue::siBool,     sps,L"",L"", false,    oParam );
    prop.AddParameter( L"brrcon_prob",  CValue::siFloat,    sps,L"",L"", 0.65,     0.0,10.0,0.1,2.0, oParam ); 
    prop.AddParameter( L"bmaxdepth",    CValue::siInt4,     sps,L"",L"", 10,       0,4096,0,1024,    oParam );
    prop.AddParameter( L"brrstrategy",  CValue::siInt1,     sps,L"",L"", 0,        0l,4l,0l,4l,      oParam );  
    prop.AddParameter( L"blight_str",   CValue::siInt4,     sps,L"",L"", 2,        0,5,0,5,          oParam ); 
       
    //-- parameters distributepath
    prop.AddParameter( L"bdirectsampleall",         CValue::siBool, sps,L"",L"",    false,  oParam );
    prop.AddParameter( L"bindirectsampleall",       CValue::siBool, sps,L"",L"",    true,   oParam );
    prop.AddParameter( L"bdirectsamples",           CValue::siInt4, sps,L"",L"",    1,      0l,4l,0l,4l,    oParam );
    prop.AddParameter( L"bindirectsamples",         CValue::siInt4, sps,L"",L"",    1,      0,5,0,5,        oParam );

    prop.AddParameter( L"bdiffusereflectdepth",     CValue::siInt4, sps,L"",L"",    3,      0,5,0,5,    oParam ) ;
    prop.AddParameter( L"bdiffusereflectsamples",   CValue::siInt4, sps,L"",L"",    1,      0,5,0,5,    oParam ) ;
    prop.AddParameter( L"bdiffuserefractdepth",     CValue::siInt4, sps,L"",L"",    5,      0,5,0,5,    oParam ) ;
    prop.AddParameter( L"bdiffuserefractsamples",   CValue::siInt4, sps,L"",L"",    1,      0,5,0,5,    oParam ) ;
    prop.AddParameter( L"bdirectdiffuse",           CValue::siBool, sps,L"",L"",    true,   oParam ) ;
    prop.AddParameter( L"bindirectdiffuse",         CValue::siBool, sps,L"",L"",    true,   oParam ) ;

    prop.AddParameter( L"bglossyreflectdepth",      CValue::siInt4, sps,L"",L"",    2,      0,10,2,5,   oParam );
    prop.AddParameter( L"bglossyreflectsamples",    CValue::siInt4, sps,L"",L"",    1,      0,10,2,5,   oParam );
    prop.AddParameter( L"bglossyrefractdepth",      CValue::siInt4, sps,L"",L"",    5,      0,10,2,5,   oParam );
    prop.AddParameter( L"bglossyrefractsamples",    CValue::siInt4, sps,L"",L"",    1,      0,10,2,5,   oParam );
    prop.AddParameter( L"bdirectglossy",            CValue::siBool, sps,L"",L"",    true,   oParam );
    prop.AddParameter( L"bindirectglossy",          CValue::siBool, sps,L"",L"",    true,   oParam );

    prop.AddParameter( L"bspecularreflectdepth",    CValue::siInt4, sps,L"",L"",    3,      0,10,2,5,  oParam );
    prop.AddParameter( L"bspecularrefractdepth",    CValue::siInt4, sps,L"",L"",    5,      0,10,2,5,  oParam );
    //--
    prop.AddParameter( L"bdiff_reflect_reject",     CValue::siBool, sps,L"",L"",    false,  oParam );
    prop.AddParameter( L"bdiff_refract_reject",     CValue::siBool, sps,L"",L"",    false,  oParam ); 
    prop.AddParameter( L"bglossy_reflect_reject",   CValue::siBool, sps,L"",L"",    false,  oParam );
    prop.AddParameter( L"bglossy_refract_reject",   CValue::siBool, sps,L"",L"",    false,  oParam );
    //--
    prop.AddParameter( L"bdiff_reflect_reject_thr",   CValue::siFloat, sps,L"",L"", 10.0,   0.0f,10.0f,0.0f,10.0f, oParam ); 
    prop.AddParameter( L"bdiff_refract_reject_thr",   CValue::siFloat, sps,L"",L"", 10.0,   0.0f,10.0f,0.0f,10.0f, oParam ); 
    prop.AddParameter( L"bglossy_reflect_reject_thr", CValue::siFloat, sps,L"",L"", 10.0,   0.0f,10.0f,0.0f,10.0f, oParam );
    prop.AddParameter( L"bglossy_refract_reject_thr", CValue::siFloat, sps,L"",L"", 10.0,   0.0f,10.0f,0.0f,10.0f, oParam );

    //-- igi
    prop.AddParameter( L"bnsets",    CValue::siInt4,  sps,L"",L"",    4,    0,1024,4,512,   oParam );
    prop.AddParameter( L"bnlights",  CValue::siInt4,  sps,L"",L"",    32,   0,1024,64,512,  oParam );
    prop.AddParameter( L"bmindist",  CValue::siFloat, sps,L"",L"",    0.1,  0.0,1024.0,0.1f,1024.0,  oParam );
    
    //-- exphotonmap    
    prop.AddParameter( L"brenderingmode",            CValue::siInt4,    sps,L"",L"", 0,         0,10,0,10,              oParam );
    prop.AddParameter( L"bshadowraycount",           CValue::siInt4,    sps,L"",L"", 0,         0,10,0,10,              oParam );
    prop.AddParameter( L"bmaxphotondepth",           CValue::siInt4,    sps,L"",L"", 10,        0,10,0,10,              oParam );
    prop.AddParameter( L"bmaxeyedepth",              CValue::siInt4,    sps,L"",L"", 8,         0,10,0,10,              oParam );
    prop.AddParameter( L"bnphotonsused",             CValue::siInt4,    sps,L"",L"", 50,        0,50,0,50,              oParam );
    prop.AddParameter( L"bindirectphotons",          CValue::siInt4,    sps,L"",L"", 200000,    0,200000,0,200000,      oParam );
    prop.AddParameter( L"bdirectphotons",            CValue::siInt4,    sps,L"",L"", 1000000,   0,10000000,0,10000000,  oParam );
    prop.AddParameter( L"bcausticphotons",           CValue::siInt4,    sps,L"",L"", 200000,    0,20000,0,20000,        oParam );
    prop.AddParameter( L"bradiancephotons",          CValue::siInt4,    sps,L"",L"", 20000,     0,200000,0,200000,      oParam );
    prop.AddParameter( L"bfinalgathersamples",       CValue::siInt4,    sps,L"",L"", 32,        0,1024,0,1024,          oParam );

    prop.AddParameter( L"bgatherangle",              CValue::siFloat,   sps,L"",L"", 10.0,  0.0f,360.0f,0.0f,360.0,     oParam );
    prop.AddParameter( L"bdistancethreshold",        CValue::siFloat,   sps,L"",L"", 0.75,  0.0f,10.0f,0.0f,10.0,       oParam );
    prop.AddParameter( L"bmaxphotondist",            CValue::siFloat,   sps,L"",L"", 0.10,  0.0f,1024.0f,0.0f,1024.0,   oParam );

    prop.AddParameter( L"bdbg_enabledirect",         CValue::siBool,    sps,L"",L"", false,     oParam );
    prop.AddParameter( L"bdbg_enableradiancemap",    CValue::siBool,    sps,L"",L"", false,     oParam );
    prop.AddParameter( L"bdbg_enableindircaustic",   CValue::siBool,    sps,L"",L"", false,     oParam );
    prop.AddParameter( L"bdbg_enableindirdiffuse",   CValue::siBool,    sps,L"",L"", false,     oParam );
    prop.AddParameter( L"bfinalgather",              CValue::siBool,    sps,L"",L"", false,     oParam );
    prop.AddParameter( L"bdbg_enableindirspecular",  CValue::siBool,    sps,L"",L"", false,     oParam );
    
    //-- sppm
    prop.AddParameter( L"bmaxeyedepht",     CValue::siInt4,     sps,L"",L"",  48,       0,50,0,50,              oParam ); 
    prop.AddParameter( L"bmaxphoton",       CValue::siInt4,     sps,L"",L"",  48,       0,50,0,50,              oParam ); 
    prop.AddParameter( L"bpointxpass",      CValue::siInt4,     sps,L"",L"",  0,        0,50,0,50,              oParam );      
    prop.AddParameter( L"bphotonsxpass",    CValue::siInt4,     sps,L"",L"",  1000000,  0,10000000,0,10000000,  oParam );
    prop.AddParameter( L"bstartradius",     CValue::siFloat,    sps,L"",L"",  2.0,      0.0f,100.0f,0.0f,100.0, oParam );
    prop.AddParameter( L"balpha",           CValue::siFloat,    sps,L"",L"",  0.70,     0.0f,360.0f,0.0f,360.0, oParam );
    prop.AddParameter( L"bdlsampling",      CValue::siBool,     sps,L"",L"",  false,    oParam );
    prop.AddParameter( L"bincenvironment",  CValue::siBool,     sps,L"",L"",  false,    oParam );
    
    //--- expert
    prop.AddParameter( L"bstoreglossy", CValue::siBool,     sps,L"",L"",  false,    dft,dft,dft,dft,        oParam );
	prop.AddParameter( L"buseproba",    CValue::siBool,     sps,L"",L"",  true,     dft,dft,dft,dft,        oParam );
	prop.AddParameter( L"bwavelengthstratificationpasses",  CValue::siInt4, sps,L"",L"",    0, 0,50,0,50,   oParam );  
	//    "string lookupaccel" ["kdtree"]
	prop.AddParameter( L"bparallelhashgridspare",   CValue::siFloat,    sps,L"",L"",  1.0,  0.0f,10.0f,0.0f,10.0, oParam );
	//    "string pixelsampler" ["hilbert"]
	//    "string photonsampler" ["halton"]
    
    //-- Sampler
    prop.AddParameter( L"bsampler",     CValue::siInt4,     sps,L"",L"",   2,       0,10,0,10,            oParam );
    prop.AddParameter( L"bmutation",    CValue::siFloat,    sps,L"",L"",   0.1,     0.0f,1.0f,0.0f,0.5f,  oParam );
    prop.AddParameter( L"bmaxrej",      CValue::siInt4,     sps,L"",L"",   256,     0l,2048l,0l,512l,     oParam );
    prop.AddParameter( L"buservarian",  CValue::siBool,     sps,L"",L"",   false,   dft,dft,dft,dft,      oParam );
    prop.AddParameter( L"bchainlength", CValue::siInt4,     sps,L"",L"",   512,     0,512,16,512,         oParam );
    prop.AddParameter( L"bpixsampler",  CValue::siInt4,     sps,L"",L"",   3,       0,512,16,512,         oParam );
    prop.AddParameter( L"pixelsamples", CValue::siInt4,     sps,L"",L"",   2,       0,64,0,64,            oParam );
    prop.AddParameter( L"bbasampler",   CValue::siInt4,     sps,L"",L"",   0,       0,10,0,10,            oParam );

    prop.AddParameter( L"bpresets",     CValue::siInt4,     sps,L"",L"",   2,       0,10,0,10,      oParam );

    //-- expert mode menu options
    prop.AddParameter( L"bexpert",      CValue::siBool, sps,L"",L"",    false,  oParam );

    prop.AddParameter( L"savint",       CValue::siInt4, sps,L"",L"", 120,   0l,200l,0l,200l,    oParam );
    prop.AddParameter( L"disint",       CValue::siInt4, sps,L"",L"", 12,    0l,200l,0l,200l,    oParam );
    prop.AddParameter( L"hSpp",         CValue::siInt4, sps,L"",L"", 128,   0l,200l,0l,200l,    oParam );
    prop.AddParameter( L"hTime",        CValue::siInt4, sps,L"",L"", 0,     0l,2000l,0l,2000l,  oParam );
    
    //- animation
    prop.AddParameter( L"bframestep",   CValue::siInt4, sps,L"",L"", 1,     1,100,1,100, oParam );
    
    //--- save image options // 
    prop.AddParameter( L"mode_rpng",    CValue::siInt4, sps,L"",L"", 3,      0,10,0,10, oParam );
    prop.AddParameter( L"save_png_16",  CValue::siBool, sps,L"",L"", false,  oParam );
    prop.AddParameter( L"png_gamut",    CValue::siBool, sps,L"",L"", false,  oParam );
    prop.AddParameter( L"save_png",     CValue::siBool, sps,L"",L"", true,   oParam );    
    //-- exr
    prop.AddParameter( L"mode_Znorm",   CValue::siInt4, sps,L"",L"", 2,     0,10,0,10,  oParam );
    prop.AddParameter( L"save_exr",     CValue::siBool, sps,L"",L"", false, oParam );
    //-- tga
    prop.AddParameter( L"mode_rtga",    CValue::siInt4, sps,L"",L"", 1,     0,10,0,10,  oParam );
    prop.AddParameter( L"save_tga",     CValue::siBool, sps,L"",L"", false, oParam );
    prop.AddParameter( L"tga_gamut",    CValue::siBool, sps,L"",L"", false, oParam );
    //--
    prop.AddParameter( L"brmode",       CValue::siInt4, sps,L"",L"", 0,     0,10,0,10,  oParam );
    prop.AddParameter( L"bengine",      CValue::siInt4, sps,L"",L"", 0,     0,10,0,10,  oParam );
    prop.AddParameter( L"bthreads",     CValue::siInt4, sps,L"",L"", 2,     0,10,0,10,  oParam );
    prop.AddParameter( L"bautode",      CValue::siBool, sps,L"",L"", true,  oParam );
    
    //-- Accelerator 
    prop.AddParameter( L"bAccel",              CValue::siInt4, sps,L"",L"",  2,     0,10,0,10,      oParam );
    prop.AddParameter( L"bmaxprimsperleaf",    CValue::siInt4, sps,L"",L"",  4,     0,10,0,10,      oParam );
    prop.AddParameter( L"bfullsweepthreshold", CValue::siInt4, sps,L"",L"",  16,    0,16,0,16,      oParam );
    prop.AddParameter( L"bskipfactor",         CValue::siInt4, sps,L"",L"",  1,     0,10,0,10,      oParam );
    prop.AddParameter( L"btreetype",           CValue::siInt4, sps,L"",L"",  2,     0,10,0,10,      oParam );
    prop.AddParameter( L"bcostsamples",        CValue::siInt4, sps,L"",L"",  0,     0,10,0,10,      oParam );
    prop.AddParameter( L"bmaxprims",           CValue::siInt4, sps,L"",L"",  1,     0,10,0,10,      oParam );
    prop.AddParameter( L"bacmaxdepth",         CValue::siInt4, sps,L"",L"",  -1,    0,10,0,10,      oParam );
    prop.AddParameter( L"bintersectcost",      CValue::siInt4, sps,L"",L"",  80,	0,100,0,100,    oParam );
    prop.AddParameter( L"btraversalcost",      CValue::siInt4, sps,L"",L"",  1,     0,100,0,100,    oParam );
    prop.AddParameter( L"bacexpert",           CValue::siBool, sps,L"",L"",  false, oParam );
    prop.AddParameter( L"brefineimmediately",  CValue::siBool, sps,L"",L"",  false, oParam );
    prop.AddParameter( L"bemptybonus",         CValue::siFloat, sps,L"",L"", 0.2,   0.0,1.0,0.0,1.0,    oParam );
    //--
    
    prop.AddParameter( L"resume",   CValue::siBool, sps,L"",L"",  false,    oParam );
    prop.AddParameter( L"loglevel", CValue::siBool, sps,L"",L"",  false,    oParam );
 
    //- set default filename for scene and image out
    vFileExport = app.GetInstallationPath(siProjectPath);

    //- set path for LuxRender binarie file
    vLuXSIPath = app.GetInstallationPath(siUserAddonPath);

    //- for search exported .lxs file
    vblxs_file = app.GetInstallationPath(siProjectPath);
    
    //--
    vFileExport += L"/tmp.lxs";
	vLuXSIPath += L"/LuXSI/Application/bin";    
    
    //-- lxs files for re-render
    prop.AddParameter( L"blxs_file",    CValue::siString, sps,L"",L"",  vblxs_file,     oParam );
    
    //- name for Material preview 
    prop.AddParameter( L"bmatPreview",  CValue::siString, sps,L"",L"",  vmatPreview,    oParam );
    
	prop.AddParameter( L"fileExport",   CValue::siString, sps,L"",L"",  vFileExport,    oParam );

    prop.AddParameter( L"fLuxPath",     CValue::siString, sps,L"",L"",  vLuXSIPath,     oParam );

    return CStatus::OK;
}
//-- 