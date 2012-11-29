/*
This file is part of LuXSI.
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

#pragma warning (disable : 4245) // signed/unsigned mismatch
#pragma warning (disable : 4996) // strcpy/unsigned mismatch


#include "include\luxsi_values.h"
#include "include\LuXSI.h"
//#include "plymesh/rply.h"

//--
XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
    in_reg.PutAuthor(L"Michael Gangolf");
    in_reg.PutName(L"LuXSI");
    in_reg.PutEmail(L"miga@migaweb.de");
    in_reg.PutURL(L"http://www.migaweb.de");
    in_reg.PutVersion(1,1);
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

//-
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
        //-
        prop = ctxt.GetSource() ;
        //app.LogMessage( L"OnInit called for " + prop.GetFullName() ) ;

        ctxt.PutAttribute(L"Refresh",true);

        params = prop.GetParameters();
        /** Update values on init
        */
        for (int i=0; i < params.GetCount(); i++)
        {
            Parameter param(params[i]);
            CString pname = param.GetScriptName();
            //-
            update_LuXSI_values(pname, param, ctxt);
        }
    }
    else if ( eventID == PPGEventContext::siButtonClicked )
    {
        CValue buttonPressed = ctxt.GetAttribute( L"Button" );
        //-
        CString buttonOption = buttonPressed.GetAsText();
        is_preview = false;
        //- 
        if ( buttonOption == L"bre_render" || buttonOption == L"blpreview" )
        {
            //--
            if( buttonOption == L"blpreview" )
            {
                is_preview = true;
                luxsi_mat_preview(false);
            }
            else
            {
                //-- its work...
                luxsiRender(vblxs_file);
            }
        }
        if ( buttonOption == L"export_luxsi" || buttonOption == L"render_luxsi")
        {
            //- always..
            writeLuxsiScene(ftime);

            //- depend..
            if( buttonOption == L"render_luxsi" ) luxsi_execute();
        }
        if ( buttonOption == L"export_ani" || buttonOption == L"render_ani" )
        {
            //-- test
            CRefArray projectProps = Application().GetActiveProject().GetProperties();
            Property playControl = projectProps.GetItem( L"Play Control" );
            int time = playControl.GetParameterValue( L"Current" );
            int time_start = playControl.GetParameterValue( L"In" );
            int time_end = playControl.GetParameterValue( L"Out" );
            //- 
            lqueue = true;
            //- reset queue_list
            queue_list.Clear();
            //-
            for ( int i = time_start; i < time_end; i += vframestep)
            {
                if( luxdebug ) app.LogMessage(L" PLAY FRAME: "+ CString(i));
                
                //- frame
                ftime = i;
                writeLuxsiScene(ftime);
                
                //- test MsgBox
                //long st = 3;
                //kit.MsgBox(L"Message text..", siMsgOk, L"Warning!!", st);
            }
            int ext = int(vFileExport.ReverseFindString("."));
            //- for 'queue' files
            vFileQueue = vFileExport.GetSubString(0,ext) + L".lxq";
            //-
            f.open(vFileQueue.GetAsciiString());
            //-
            f << queue_list.GetAsciiString();
            f.close();
            //-
            if( buttonOption == L"render_ani") luxsi_execute();
        }
    }
    else if ( eventID == PPGEventContext::siTabChange )
    {
        CValue tabLabel = ctxt.GetAttribute( L"Tab" );
        //app.LogMessage( L"Tab changed to: " + tabLabel .GetAsText() ) ;
    }
    else if ( eventID == PPGEventContext::siParameterChange )
    {
        Parameter changed = ctxt.GetSource();
        //CustomProperty 
        prop = changed.GetParent() ;
        CString paramName = changed.GetScriptName();

        app.LogMessage( L"Parameter Changed: " + paramName );

        update_LuXSI_values(paramName, changed, ctxt);         
    }
    return CStatus::OK ;
}

//--
CVector3 convertMatrix(CVector3 v)
{
    CMatrix3 m2(1.0,0.0,0.0,  0.0,0.0,1.0,   0.0,-1.0,0.0);
    return v.MulByMatrix3InPlace(m2);
}

//--
void update_main_values(CString paramName, Parameter changed, PPGEventContext ctxt)
{
    if (paramName == L"Width")  { vXRes = changed.GetValue();
    } else if (paramName == L"Height")  { vYRes     = changed.GetValue();
    } else if (paramName == L"gamma")   { vContrast = changed.GetValue();
    
    //----/ hidden objects /------>
    } else if (paramName == L"use_hidden_obj")  { vIsHiddenObj      = changed.GetValue();
    } else if (paramName == L"use_hidden_surf") { vIsHiddenSurface  = changed.GetValue();
    } else if (paramName == L"use_hidden_cloud"){ vIsHiddenClouds   = changed.GetValue();
    } else if (paramName == L"use_hidden_cam")  { vIsHiddenCam      = changed.GetValue();
    } else if (paramName == L"use_hidden_light"){ vIsHiddenLight    = changed.GetValue();
    
    //-- mesh options
    } else if (paramName == L"smooth_mesh")     { vSmooth_mesh  = changed.GetValue();
    } else if (paramName == L"sharp_bound")     { vSharp_bound  = changed.GetValue();
    //- geometry
    } else if (paramName == L"over_geo")        { overrGeometry = changed.GetValue();
    } else if (paramName == L"bplymesh")
    {
        vplymesh = changed.GetValue();
        dynamic_luxsi_UI(changed, paramName, ctxt);
    
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

    } else if (paramName == L"fileExport")  { vFileExport   = changed.GetValue();
    //-
    } else if (paramName == L"luxMatExport"){ vluxMatExport = changed.GetValue();
    
    //- material preview // 
    } else if (paramName == L"bmatPreview") { vmatPreview   = changed.GetValue();
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
    // "bmaxeyedepht", "bmaxphoton", "bpointxpass", "bphotonsxpass", "bstartradius", "balpha",
    // "bdlsampling", "bincenvironment"
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
    input_presset(L"hSpp",  vhaltspp = 0);
    input_presset(L"hTime", vhalttime = 0);
       
    //--
    if ( vpresets == 0 ) // Custom parameters
    {
        //-- TODO;
    }
    else if ( vpresets == 1 ) // Preview; Instant Global Illumination
    {
        input_presset( L"bsampler",      vSampler = 2);
        input_presset( L"bpixsampler",   vPixsampler = 3); // low
        input_presset( L"pixelsamples",  vPixelsamples = 1);
        //-- distributepath
        input_presset( L"bsurfaceint",              vSurfaceInt = 3); 
        input_presset( L"blight_str",               vLight_str = 2 );
        input_presset( L"directsampleall",          vdirectsampleall =  true );
        input_presset( L"bdirectsamples",           vdirectsamples = 1 );
        input_presset( L"bindirectsampleall",       vindirectsampleall = false );
        input_presset( L"bindirectsamples",         vindirectsamples = 1 );
        input_presset( L"bdiffusereflectdepth",     vdiffusereflectdepth = 3 );
        input_presset( L"bdiffusereflectsamples",   vdiffusereflectsamples = 1 );
        input_presset( L"bdiffuserefractdepth",     vdiffuserefractdepth = 5 );
        input_presset( L"bdiffuserefractsamples",   vdiffuserefractsamples = 1 );
        input_presset( L"bdirectdiffuse",           vdirectdiffuse = true );
        input_presset( L"bindirectdiffuse",         vindirectdiffuse = true );
        input_presset( L"bglossyreflectdepth",      vglossyreflectdepth = 2 );
        input_presset( L"bglossyreflectsamples",    vglossyreflectsamples = 1 );
        input_presset( L"bglossyrefractdepth",      vglossyrefractdepth = 5 );
        input_presset( L"bglossyrefractsamples",    vglossyrefractsamples = 1 );
        input_presset( L"bdirectglossy",            vdirectglossy = true );
        input_presset( L"bindirectglossy",          vindirectglossy = true );
        input_presset( L"bspecularreflectdepth",    vspecularreflectdepth = 3 );
        input_presset( L"bspecularrefractdepth",    vspecularrefractdepth = 5 );
        //-- volume int
        //-- filter / commons
    }
    else if ( vpresets == 2 ) // Preview; Directlighting ( No GI )
    {
        input_presset( L"bsurfaceint",  vSurfaceInt = 2); //-- directlighting
        input_presset( L"blight_str",   vLight_str = 0 ); // auto
        input_presset( L"bsampler",     vSampler = 3); 
        input_presset( L"bpixsampler",  vPixsampler = 1);
        input_presset( L"pixelsamples", vPixelsamples = 1);
        input_presset( L"bmaxdepth",    vmaxdepth = 5);

        input_presset( L"bvolumeint",   vvolumeint = 1);
        //- filter
        input_presset( L"bywidth",      vYwidth = 2.0);
        input_presset( L"bxwidth",      vXwidth = 2.0);
        
        //--
    }
    else if ( vpresets == 3 ) // Preview; Ex-photonmap
    {
        //- Sampler "lowdiscrepancy"
        input_presset( L"bpixsampler",      vPixsampler = 4); //- hilbert
        input_presset( L"pixelsamples",     vPixelsamples = 4);
        //-- SurfaceIntegrator "exphotonmap"
        input_presset( L"bsurfaceint",      vSurfaceInt = 5);
        input_presset( L"brenderingmode",   vrenderingmode = 0 ); // directlighting
        input_presset( L"blight_str",       vLight_str = 0 ); // auto
        input_presset( L"bshadowraycount",  vshadowraycount = 1 ); // ?
        input_presset( L"bmaxphotondepth",  vmaxphotondepth = 10 );
        input_presset( L"bmaxeyedepth",     vmaxeyedepth = 10 );
        //vmaxdepth = 5 ;
        input_presset( L"bmaxphotondist",   vmaxphotondist = 0.100000f );
        input_presset( L"bnphotonsused",    vnphotonsused = 50 );
        input_presset( L"bindirectphotons", vindirectphotons = 200000 );
        input_presset( L"bdirectphotons",   vdirectphotons = 1000000 );
        input_presset( L"bcausticphotons",  vcausticphotons = 20000 );
        input_presset( L"bradiancephotons", vradiancephotons = 20000 );
        input_presset( L"bfinalgather",     vfinalgather = false );
        // VolumeIntegrator "single"
        // Parameter(prop.GetParameters().GetItem( L"stepsize" [1.000000]
    }
    else if ( vpresets == 4 ) // final 1 MLT / Bi-directional PathTracing (int)
    {
        //-- sampler; metropolis 
        input_presset( L"bsampler", vSampler = 0);
        input_presset( L"bmutation", vlmutationpro = 0.6f);
        vmaxconsecrejects = 512 ; 
        vusevariance = false ;
        //-- 
        input_presset( L"bsurfaceint",   vSurfaceInt = 0); // bidir
        input_presset( L"blight_depth",  vLight_depth = 32); 
        input_presset( L"beye_depth",    vEye_depth = 32);
        //--
    }
    else if ( vpresets == 5 ) // final 2 MLT / PathTracing (ext)
    {
        input_presset( L"bsampler",     vSampler = 0);// metro
        input_presset( L"bmutation",    vlmutationpro = 0.4f);
        //--
        input_presset( L"bsurfaceint",  vSurfaceInt = 1); // path
        input_presset( L"bmaxdepth",    vmaxdepth = 10);
        input_presset( L"binc_env",     vInc_env = true );
        //--
    }
    else if ( vpresets == 6 ) // progr 1 Bidir Path Tracing (int)
    {
        input_presset( L"bsampler",      vSampler = 2);
        input_presset( L"bpixsampler",   vPixsampler = 2 );
        input_presset( L"pixelsamples",  vPixelsamples = 1);
        //--
        input_presset( L"bsurfaceint",   vSurfaceInt = 2);
        input_presset( L"blight_depth",  vLight_depth = 10); 
        input_presset( L"beye_depth",    vEye_depth = 10); 
        //--
    }
    else if ( vpresets == 7 ) //  progr 2 Path Tracing (ext)
    {
        input_presset( L"bsampler", vSampler = 1);
        vchainlength = 512;
        vbasampler = 1;
        input_presset( L"bpixsampler",   vPixsampler = 2); 
        input_presset( L"pixelsamples",  vPixelsamples = 1);
        //-- surf
        input_presset( L"bsurfaceint",   vSurfaceInt = 1); // path
        input_presset( L"bmaxdepth",     vmaxdepth = 10);
        input_presset( L"binc_env",     vInc_env = true);
        //--
    }
    else if ( vpresets == 8 )// bucket 1 Bidir Path Tracing (int)
    {
        //- surf - bidir
        input_presset( L"bsurfaceint",   vSurfaceInt = 0);
        //-
        input_presset( L"bsampler",      vSampler = 2) ; // low
        input_presset( L"bpixsampler",   vPixsampler = 4) ; // hilbert
        input_presset( L"pixelsamples",  vPixelsamples = 64);
        //--
        input_presset( L"blight_depth",  vLight_depth = 16); 
        input_presset( L"beye_depth",    vEye_depth = 16);
        //--
    }
    else // bucket 2 Path Tracing (ext)
    {
        input_presset( L"bsampler",     vSampler = 2 ); // low
        input_presset( L"bpixsampler",  vPixsampler = 4 ); //TODO; revise 
        input_presset( L"pixelsamples", vPixelsamples = 64);
        input_presset( L"bsurfaceint",  vSurfaceInt = 1); // path
        vsexpert = false ;
        input_presset( L"bmaxdepth",    vmaxdepth = 10); 
        input_presset( L"binc_env",     vInc_env = true);
        //-- filter - commons
    }
    //-- commons values for all presets
    //-- sampler
    input_presset( L"bsampler", vSampler );

    //-- if ( vsampler == low ) //-- TODO;
    input_presset( L"bpixsampler", vPixsampler );
    input_presset( L"pixelsamples", vPixelsamples );

    //-- if ( vsampler == metro ) //-- TODO;
    input_presset( L"bmutation",     vlmutationpro = 0.6f );
    input_presset( L"bmaxrej",       vmaxconsecrejects = 512 ); 
    input_presset( L"buservarian",   vusevariance = false );
            
    //-- filter
    input_presset( L"bfilter",   vfilter );
    //-- if filter is mitchell...
    input_presset( L"bxwidth",   vXwidth = 1.500000 );
    input_presset( L"bywidth",   vYwidth = 1.500000 );
    input_presset( L"bF_B",      vF_B = 0.3333f );
    input_presset( L"bF_C",      vF_C = 0.3333f );
    input_presset( L"ssample",   vSupers = true );

    //--  ...or  gaussian ? //-- TODO;
    //-- surface integrator
    input_presset( L"bsurfaceint",   vSurfaceInt );
    //-- direct
    input_presset( L"blightrrthre",  vLightRRthre ); 
    //-- path
    input_presset( L"bmaxdepth",     vmaxdepth = 10 );
    //-- bidirect
    input_presset( L"beye_depth",    vEye_depth );
    input_presset( L"blight_depth",  vLight_depth ); 
            
    //-- Accelerator; 'qbvh' 
    input_presset( L"bAccel", vAccel = 0 );
    input_presset( L"bskipfactor", vskipfactor = 1 );
    input_presset( L"bmaxprimsperleaf", vmaxprimsperleaf = 4 );
    input_presset( L"bfullsweepthreshold", vfullsweepthr = 16 );
    
         
    ctxt.PutArrayAttribute(L"Refresh", true);
    //-
    app.LogMessage(L" Parameters for render presets loaded: "+ CString(vpresets));
    
  
//} //-- end cases...
}


//-
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
void luxsi_SurfaceIntegrator()
{
    //-
    const char *MtSurf [7] = { "bidirectional", "path", "directlighting",
        "distributedpath", "igi", "exphotonmap", "sppm" };
    //-
    const char *MtRRst [3] = { "none", "probability", "efficiency"};
    //-
    const char *MtlightST [7] = { "one", "all", "auto", "importance", 
        "powerimp", "allpowerimp", "logpowerimp"};
    
    //-------------------------------------------------------------
    f << "\nSurfaceIntegrator \""<< MtSurf[ vSurfaceInt ] <<"\"\n";
    //-------------------------------------------------------------
    if ( vSurfaceInt == 0 )  //-- bidi
    {
        /*
        "integer eyedepth" [48]
	    "integer lightdepth" [48]
	    "float eyerrthreshold" [0.000000000000000]
	    "float lightrrthreshold" [0.000000000000000]
	    "string lightpathstrategy" ["auto"]
	    "string lightstrategy" ["auto"]
        */
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
        //-
        if ( vsexpert )
        {
            f << "  \"integer shadowraycount\" [" << vshadowraycount << "]\n";
        }
    }
    else if ( vSurfaceInt == 2 ) //-- directlighting
    {
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        //-
        if ( vsexpert )
        {
            //f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
            f << "  \"integer shadowraycount\" [" << vshadowraycount << "]\n";
        }
    }
    else if ( vSurfaceInt == 3 ) //-- distributedpath
    {
        f << "  \"integer directsamples\" ["<< vdirectsamples <<"]\n";
        f << "  \"integer indirectsamples\" ["<< vindirectsamples <<"]\n";
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
        // 
        if ( vsexpert )
        {
            f << "  \"bool directsampleall\" [\""<< MtBool[ vdirectsampleall ] <<"\"]\n";
            f << "  \"bool directdiffuse\" [\""<< MtBool[ vdirectdiffuse ] <<"\"]\n";
            f << "  \"bool directglossy\" [\""<< MtBool[vdirectglossy] <<"\"]\n";
        
            f << "  \"bool indirectsampleall\" [\""<< MtBool[ vindirectsampleall ] <<"\"]\n";
            f << "  \"bool indirectdiffuse\" [\""<< MtBool[ vindirectdiffuse ] <<"\"]\n";
            f << "  \"bool indirectglossy\" [\""<< MtBool[ vindirectglossy ] <<"\"]\n";
            f << "  \"bool diffusereflectreject\" [\""<< MtBool[0] <<"\"]\n"; // TODO
            f << "  \"float diffusereflectreject_threshold\" ["<< vdiff_reflect_reject_thr <<"]\n";
            f << "  \"bool diffuserefractreject\" [\""<< MtBool[0] <<"\"]\n";
            f << "  \"float diffuserefractreject_threshold\" ["<< vdiff_refract_reject_thr <<"]\n";
            f << "  \"bool glossyreflectreject\" [\""<< MtBool[0] <<"\"]\n";
            f << "  \"float glossyreflectreject_threshold\" ["<< vglossy_reflect_reject_thr <<"]\n";
            f << "  \"bool glossyrefractreject\" [\""<< MtBool[0] <<"\"]\n";
            f << "  \"float glossyrefractreject_threshold\" ["<< vglossy_refract_reject_thr <<"]\n";
            //--
            //f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
        }
    }
    else if ( vSurfaceInt == 4 ) //-- igi
    {
        f << "  \"integer nsets\" ["<< vnsets <<"]\n";
        f << "  \"integer nlights\" ["<< vnlights <<"]\n";
        f << "  \"integer maxdepth\" ["<< vmaxdepth <<"]\n";
        f << "  \"float mindist\" ["<< vmindist <<"]\n";
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
        /*
        SurfaceIntegrator "sppm"
	    "integer maxeyedepth" [48]
	    "integer maxphotondepth" [48]
	    "integer photonperpass" [1000000]
	    "integer hitpointperpass" [0]
	    "float startradius" [2.000000000000000]
	    "float alpha" [0.699999988079071]
	    "bool includeenvironment" ["true"]
	    "bool directlightsampling" ["true"]
        //--- expert
        "bool storeglossy" ["false"]
	    "bool useproba" ["true"]
	    "integer wavelengthstratificationpasses" [8]
	    "string lookupaccel" ["kdtree"]
	    "float parallelhashgridspare" [1.000000000000000]
	    "string pixelsampler" ["hilbert"]
	    "string photonsampler" ["halton"]
        */
        if ( vsexpert )
        {
	        f << "  \"bool storeglossy\" [\"false\"]\n";
	        f << "  \"bool useproba\" [\"true\"]\n";
	        f << "  \"integer wavelengthstratificationpasses\" [8]\n";
	        f << "  \"string lookupaccel\" [\"kdtree\"]\n";
	        f << "  \"float parallelhashgridspare\" [1.000000000000000]\n";
	        f << "  \"string pixelsampler\" [\"hilbert\"]\n";
	        f << "  \"string photonsampler\" [\"halton\"]\n";
        }
    }
    //- light strategy for all, except sppm
    if ( vSurfaceInt < 6 )
    {
        f << "  \"string lightstrategy\" [\""<< MtlightST[vLight_str] <<"\"]\n";
    }
}

//--
CString luxsi_Sampler()
{
    CString samplerStr;
    //-
    const char *aPxSampler [5] = { "linear", "vegas", "lowdiscrepancy", "tile", "hilbert" };
    //-
    const char *aSampler [4] = { "metropolis", "erpt", "lowdiscrepancy", "random" }; // mode, Asampler [vSampler]
    
    //------------------------------------------------------------------
    samplerStr = L"\nSampler \""+ CString( aSampler[vSampler] ) + L"\"\n";
    //------------------------------------------------------------------

    if ( vSampler == 0 ) //-- sampler; metropolis
    {
        samplerStr += L"  \"float largemutationprob\" ["+ CString( vlmutationpro ) + L"]\n";
        samplerStr += L"  \"bool usevariance\" [\""+ CString( MtBool[vusevariance]) + L"\"]\n"; // TODO;
        if ( vExpert )
        {
            samplerStr += L"  \"integer maxconsecrejects\" ["+ CString( vmaxconsecrejects ) + L"]\n";
        }
    }
    else if ( vSampler == 1 ) //-- sampler; erpt
    {
        if ( vbasampler > 0 ) //-- base sampler; low or random
        {
            samplerStr += L"  \"integer chainlength\" ["+ CString( vchainlength ) + L"]\n";
            samplerStr += L"  \"string basesampler\" [\""+ CString( MtBsampler[vbasampler] ) + L"\"]\n";
            samplerStr += L"  \"string pixelsampler\" [\""+ CString( aPxSampler[vPixsampler] ) + L"\"]\n";
            samplerStr += L"  \"integer pixelsamples\" ["+ CString( vPixelsamples ) + L"]\n";

        }
        else //-- base sampler; metropolis
        {
            samplerStr += L"  \"integer chainlength\" ["+ CString( vchainlength ) + L"]\n";
            samplerStr += L"  \"string basesampler\" [\""+ CString( MtBsampler[vbasampler] ) + L"\"]\n";
            if ( vExpert )
            {
                samplerStr += L"  \"float mutationrange\" [ "+ CString( vmutationrange ) + L"]\n"; // test
                samplerStr += L"  \"integer maxconsecrejects\" ["+ CString( vmaxconsecrejects ) + L"]\n";
            }
        }
    }
    else if ( vSampler > 1 ) //-- sampler; lowdiscrepance
    {
        samplerStr += L"  \"string pixelsampler\" [\""+ CString( aPxSampler[vPixsampler] ) + L"\"]\n";
        samplerStr += L"  \"integer pixelsamples\" ["+ CString( vPixelsamples ) + L"]\n";
    }
    else //-- error mesage
    {
        app.LogMessage( L"Not Sampler for exporter", siErrorMsg );
    }
    return samplerStr;
}

//--
CString luxsi_Accelerator()
{
    CString AccelStr;
    //--------------------------------------------------
    AccelStr = L"\nAccelerator \""+ CString(MtAccel[vAccel]) + L"\"\n";
    //--------------------------------------------------
    if ( vAccel == 0  &&  vExpert ) //-- qbvh
    {
        AccelStr += L"  \"integer maxprimsperleaf\" ["+ CString( vmaxprimsperleaf ) + L"]\n";
        AccelStr += L"  \"integer fullsweepthreshold\" ["+ CString( vfullsweepthr ) + L"]\n";
        AccelStr += L"  \"integer skipfactor\" ["+ CString( vskipfactor ) + L"]\n";
    }
    else if (( vAccel == 1 ) || ( vExpert )) //-- bvh
    {
        AccelStr += L"  \"integer intersectcost\" ["+ CString( vintersectcost ) + L"]\n";
        AccelStr += L"  \"integer traversalcost\" ["+ CString( vtraversalcost ) + L"]\n";
        AccelStr += L"  \"integer costsamples\" ["+ CString( vcostsamples ) + L"]\n"; // lbcostsamples
    }
//    else if (( vAccel == 2 ) || ( vExpert )) //-- grid
//   {
//        f << "  \"bool refineimmediately\" [\""<< MtBool[vrefineinmed] <<"\"]\n";
//    }
    else //-- KD Tree
    {
        AccelStr += L"  \"integer intersectcost\" ["+ CString( vintersectcost ) + L"]\n";
        AccelStr += L"  \"integer traversalcost\" ["+ CString( vtraversalcost ) + L"]\n";
        AccelStr += L"  \"float emptybonus\" ["+ CString( vemptybonus ) + L"]\n";
        AccelStr += L"  \"integer maxprims\" ["+ CString( vmaxprims ) + L"]\n";
        AccelStr += L"  \"integer maxdepth\" ["+ CString( vacmaxdepth ) + L"]\n\n";
    }
    return AccelStr;
}
//-
CString luxsi_Filter()
{
    CString filterStr;
    //-
    const char *aFilter [5] = { "box", "gaussian", "mitchell", "sinc", "triangle" };
    //--
    filterStr = L"\nPixelFilter \""+ CString( aFilter[vfilter] ) + L"\"\n";
    //--
    if ( vfexpert )
    { 
        //-- box -- commons values
        filterStr += L"  \"float xwidth\" ["+ CString( vXwidth ) + L"]\n";
        filterStr += L"  \"float ywidth\" ["+ CString( vYwidth ) + L"]\n";
       
        if ( vfilter == 1 ) //-- gaussian
        {
            filterStr += L"  \"float alpha\" ["+ CString( vFalpha ) +L"]\n";
        }
        else if ( vfilter == 2 ) //-- mitchell
        {
            filterStr += L"  \"float B\" ["+ CString( vF_B ) + L"]\n";
            filterStr += L"  \"float C\" ["+ CString( vF_C ) + L"]\n";
            filterStr += L"  \"bool supersample\" [\""+ CString( MtBool[vSupers] ) + L"\"]\n";
        }
        else //-- sinc
        {
            filterStr += L"  \"float tau\" ["+ CString( vTau ) + L"]\n";
        }
        //-- triangle -- commons values
    }
    return filterStr;
}
//--
void writeLuxsiBasics()
{
    /** write basic scene description */
    //--
    const char *ARmode [2] = { "GUI", "console"};
    const char *Aengine [2] = { "classic", "hybrid" };
    const char *ArgbT [3] = { "Y", "RGB", "RGBA" }; // iterator  vRtga
    const char *AZb_norm [3] = { "Camera Start/End clip", "Min/Max", "None" }; //exr
    
    //--
    CString imageFile = luxsi_normalize_path(vFileLxs);
    
    f << "\nFilm \"fleximage\"\n"; //----
    f << "  \"integer xresolution\" ["<< vXRes <<"]\n  \"integer yresolution\" ["<< vYRes <<"]\n";
    f << "  \"string filename\" [\""<< imageFile.GetAsciiString() <<"\"]\n";
    
    f << "  \"integer writeinterval\" ["<< vSave <<"]\n";
    f << "  \"integer displayinterval\" ["<< vDis <<"]\n";

    f << "  \"bool write_exr\" [\""<< MtBool[vExr] <<"\"]\n";
    if (vExr)
    {
        f << "  \"string write_exr_zbuf_normalizationtype\" [\""<< AZb_norm[vExr_Znorm] <<"\"]\n";
    } //----/ TODO: more options for Exr /------>
    
    /** write image render  to png format*/
    f << "  \"bool write_png\" [\""<< MtBool[vPng] <<"\"]\n";
    //-
    const char *ArgbP [4] = { "Y", "YA", "RGB", "RGBA" };
    
    if (vPng)
    {
        f << "  \"string write_png_channels\" [\""<< ArgbP[vRpng] <<"\"]\n";
        f << "  \"bool write_png_16bit\" [\""<< MtBool[vWpng_16] <<"\"]\n";
        f << "  \"bool write_png_gamutclamp\" [\""<< MtBool[vPng_gamut] <<"\"]\n";
    }

    /**  write image render to tga format */
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

    /* Render mode */
    CString renderMode = L"sampler";
    if ( vSurfaceInt == 6 ) renderMode = L"sppm";
    //-
    f << "Renderer \""<< renderMode.GetAsciiString() <<"\"\n";

    //-- filter -->
    CString filterData = luxsi_Filter();
    f << filterData.GetAsciiString();
    
    //-- sampler -->
    CString samplerData = luxsi_Sampler();
    f << samplerData.GetAsciiString();
    
    //-- surface integrator -->
    luxsi_SurfaceIntegrator();

    //-- Accelerator -->
    CString accelData = luxsi_Accelerator();
    f << accelData.GetAsciiString();
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
    for (int i=0; i < cShaders.GetCount(); i++)
    {
        //-
        Shader camShader(cShaders[i]);
        CString vCSID((camShader.GetProgID()).Split(L".")[1]);
        if ( luxdebug ) app.LogMessage(L" Lens shader in use: "+ CString(vCSID));
        //-
        if (vCSID==L"sib_dof") 
        {
            //-- Depth_of_field shader found
            vdof_mode = camShader.GetParameterValue(L"mode");
            //--
            if ( vdof_mode = 0 ) //- custom
            {
                /* custom_near_focus, custom_far_focus, custom_coc */
            }
            if ( vdof_mode = 1 ) //- auto
            {
                vFdist = camShader.GetParameterValue(L"auto_focal_distance");
            }
            if ( vdof_mode = 2 ) //- lens
            {
                /* len_focal_distance, len_focal_lenght, len_fstop, len_coc */
                vFdist = camShader.GetParameterValue(L"len_focal_distance");
            }
            //-- commons
            vLensr = camShader.GetParameterValue(L"strenght");        
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
    f << "  "<< new_pos_ci[0] <<" "<< -new_pos_ci[2] <<" "<< new_pos_ci[1] <<"\n";
    f << "  0 0 1 \n"; //<< CString(up.GetZ()).GetAsciiString() << "\n"; // not working correct
        
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
CString writeLuxsiSurface(X3DObject o)
{
    //-- WIP: lack a lots of updates..
    //- or use other methode (like Yafxsi :)
    CString surfaceData;

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
    //surfaceData.Clear();
    //-
    surfaceData = L"\nAttributeBegin\n";
    //CVector3 axis;
    //double rot = gt.GetRotationAxisAngle(axis);
    //-- TODO; changed for matrix
    surfaceData += L"\nTransformBegin\n";
    surfaceData += L"\nTranslate "
        + CString( gt.GetPosX() ) + L" "
        + CString( gt.GetPosY() ) + L" "
        + CString( gt.GetPosZ()) + L"\n";
    //-
    if (rotar != 0)
    {
        surfaceData += L"Rotate "
            + CString( rotar )/*(rot*180/PI)*/ + L" "
            + CString( axis[0]) + L" "
            + CString( axis[1]) + L" "
            + CString( axis[2]) + L"\n";
    }
    if (gt.GetSclX()!=1 || gt.GetSclY()!=1 || gt.GetSclZ()!=1) 
    {
        surfaceData += L"Scale "
            + CString( gt.GetSclX() ) + L" "
            + CString( gt.GetSclY() ) + L" "
            + CString( gt.GetSclZ() ) + L"\n";
    }    
	//-
    float end_v = o.GetParameterValue(L"endvangle");
	float start_v = o.GetParameterValue(L"startvangle");	
    //--
    surfaceData += L" Shape  \"sphere\" \n";
    surfaceData += L"  \"float radius\" ["+ CString( vradius ) + L"]\n";
    surfaceData += L"  \"float zmin\" [ -90 ]\n";
    surfaceData += L"  \"float zmax\" [ 90 ]\n";
    surfaceData += L"  \"float phimax\" ["+ CString( vphimax ) + L"]\n";
    //--
    surfaceData += L"TransformEnd\n";
    surfaceData += L"\nAttributeEnd\n";
    //--
    return surfaceData;
}
//--
CString write_header_files()
{
    //-- commons header for files .lxm and .lxo
    CString _header = L"";
    _header += L"\n# Created by LuXSI; Luxrender Exporter for Autodesk Softimage. \n";
    _header += L"# Copyright (C) 2007 - 2012 of all Authors: \n";
    _header += L"# Michael Gangolf, \"miga\". \n";
    _header += L"# Pedro Alcaide, \"povmaniaco\". \n \n";
    //-
    return _header;
}
//--
void luxsi_mat_preview(bool onlyExport)
{
    //--
    CString 
        vFile_scene_folder = L"",   //- this file is create a '/resources' folder
        vFile_mat_preview = L"",    //- file name for exporter material data 
        mat_data_preview = L"";     //- container string for material data
    
    //--  base folder
    vFile_scene_folder = app.GetInstallationPath(siUserAddonPath);
    //--
    vFile_mat_preview = vFile_scene_folder + L"/LuXSI/resources/scene_preview_mat.lxm";
    //--
    if ( luxdebug ) app.LogMessage(L"File for material preview: "+ vFile_mat_preview);
   
    mat_data_preview = writeLuxsiShader();
    //-
    if ( luxdebug ) app.LogMessage(L"Data for Material Preview: "+ mat_data_preview);
    //--
    if ( luxsi_find(aMatList, vmatPreview ) )
    {
        std::ofstream fmat;
		fmat.open(vFile_mat_preview.GetAsciiString());
        fmat << "# Material preview for LuXSI";
        fmat << mat_data_preview.GetAsciiString();
        fmat.close();

        // test
        if ( !onlyExport )
        {
            //- scene file
            CString scenePreviewFile = vFile_scene_folder + L"/LuXSI/resources/scene.lxs";
        
            //- exe path
            CString lux_bin = vLuXSIPath + L"/luxrender.exe";
        
            //- command line
            CString exec = lux_bin + L" \""+ scenePreviewFile + L"\"";

            //- show commandline for debug...
            if ( luxdebug ) app.LogMessage(exec);

            //- start render
            loader(exec.GetAsciiString()); 
        }
    }
    else
    {
        app.LogMessage(L"Not material named 'Preview'", siWarningMsg);
    }
}
//--
void writeLuxsiScene(double ftime)
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
            aInstance;  //- for instance objects
            
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

        //- cleaned global CStringArray  for instance list.
        aInstanceList.Clear();
        //--
        if ( luxdebug ) app.LogMessage(L"[DEBUG]: Created and cleaned arrays..");

        root = app.GetActiveSceneRoot();
        //--
        itemsArray += root.FindChildren( L"", L"", emptyArray, true );
        for ( int i=0; i < itemsArray.GetCount(); i++ )
        {
            X3DObject o(itemsArray[i]);
            //- test for search all objects type.
            app.LogMessage( L"\tObject Name: " + o.GetName() + L" Type: " + o.GetType()
                + L" parent: "+ X3DObject(o.GetParent()).GetType());
            //--
            Property visi = o.GetProperties().GetItem(L"Visibility");
            bool view_visbl = (bool)visi.GetParameterValue(L"viewvis");
            bool rend_visbl = (bool)visi.GetParameterValue(L"rendvis");

            //-- Collection objects / visibilty check
            if (o.GetType()==L"polymsh")
            {
                if (vIsHiddenObj || (!vIsHiddenObj && ( view_visbl && rend_visbl )))
                {
                    aObj.Add(o); // for create link into LXO file
                }
            }
            if (o.GetType()==L"hair")
            {
                if (vIsHiddenObj || (!vIsHiddenObj && ( view_visbl && rend_visbl )))
                {
                    app.LogMessage(L"Object type [ Hair ], name: "+ o.GetName());
                    aObj.Add(o); // for create link into LXO file
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
                //- instances
                if (Model(o).GetModelKind()==2 )
                {
                    if (vIsHiddenObj || (!vIsHiddenObj && (view_visbl && rend_visbl )))
                    {
                        aInstance.Add(o);   
                    }
                }
            }
        } 
        //-- end for visibility check
        if ( (aObj.GetCount() + aSurfaces.GetCount()) == 0 )
        {
            app.LogMessage(L"Any objects/surfaces to export! Check a list of 'Export hidden items..' in GUI", siFatalMsg );
        }
        else if (aCam.GetCount()==0)
        {
            app.LogMessage(L"Any camera to export! Check a visible cameras in scene or switch ON 'export hidden cameras'", siFatalMsg );
        }
        else
        {
            /** vFileExport is the path introduced by the user in the graphic interface.
            *   It is the base of the path.
            */
            vFileLxs.Clear();
            vFileLxs = vFileExport;
            //- default extension..
            int ext = 0;
            ext = int(vFileLxs.ReverseFindString("."));
            
            //- use only for exporter animation -----------------------------------------//
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: ftime is: "+ CString(ftime));
            //-
            if ( ftime != DBL_MAX )
            {
                /* Add frame number to outfile name */
                vFileLxs = vFileLxs.GetSubString(0, ext) + (L"_"+ CString(ftime) + L".lxs");

                /* Now, call luxsi_normalize_path(), for use 'relative path', with the links inside LXQ file.
                */
                CString inc_LXQ = luxsi_normalize_path(vFileLxs) + L".lxs";
                //-
                if ( lqueue )
                {
                    queue_list += inc_LXQ + L"\n";
                }
                if ( luxdebug ) app.LogMessage(L"OUT Filename: "+ inc_LXQ );
            }//--------------------------------------------------------------------------//

            /** Setup name and extension for include files inside .lxs file.
            *   luxsi_normalize_path() return filename + framenumber if exist,
            *   but not include the extension.
            */
            CString path_base = luxsi_normalize_path(vFileLxs);
            CString inc_LXM = path_base + L"_mat.lxm";  // material definitions
            CString inc_LXO = path_base + L"_geo.lxo";  // geometry definitions
            CString inc_LXV = path_base + L"_vol.lxv";  // volume definitions

            /** For animation, reset 'ext' value to new filename.
            *   The lenght of new filename as change (name + frame).
            *   Not use normalized path, ofstream need full path.
            */
            ext = int(vFileLxs.ReverseFindString("."));
            //- add extension for material file..
            CString vFileLXM = vFileLxs.GetSubString(0, ext) + (L"_mat.lxm");
            //- add extension for geometry file..
            CString vFileLXO = vFileLxs.GetSubString(0, ext) + (L"_geo.lxo");
            //- add extension for volume file..
            CString vFileVOL = vFileLxs.GetSubString(0, ext) + L"_vol.lxv";

            /** For include link to PLY files into LXO file, setup global vFileGeo
            *   with vFileLxs value, for include frame number.
            *   Name of object and extension, is added later.
            */
            vFileGeo = vFileLxs;
            
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

            //-- basics scene values
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
            //--
            CString surfaceData = L"";
            for (int i=0; i < aSurfaces.GetCount(); i++) 
            {
                surfaceData += writeLuxsiSurface(aSurfaces[i]);
                if (pb.IsCancelPressed() ) break;
                pb.Increment();
            }
            //- ready for write --------------------------------------------#
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: Open file for geometry: "+ vFileLXO);

            //- open geometry file..
            f.open(vFileLXO.GetAsciiString()); 

            //-- ..insert header
            f << _header.GetAsciiString();

            //-- ..add polymesh
            f << geometryData.GetAsciiString();

            //-- ..add surfaces
            f << surfaceData.GetAsciiString();
            
            //-- ..add pointclouds
            f << pointCloudData.GetAsciiString();

            //-- ..add instances
            f << instanceData.GetAsciiString();
            
            //-- ..and close _geom file
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
void loader(const char szArgs[])
{
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

            //- by default load unique .lxs file..
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

void luxsiRender(CString fileScene)
{
    /* Use only GUI mode, obviusly..
    */
    CString exec = vLuXSIPath + L"/luxrender.exe \""+ fileScene + L"\"";

    //- show commandline for debug...
    if ( luxdebug ) app.LogMessage(exec);
    //-
    loader(exec.GetAsciiString());
}
