/*
This file is part of LuXSI.
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
XSIPLUGINCALLBACK CStatus LuXSI_Menu_Init( CRef& in_ctxt )
{
    Context ctxt( in_ctxt );
    Menu oMenu;
    oMenu = ctxt.GetSource();
    MenuItem oNewItem;
    oMenu.AddCallbackItem(L"LuXSI",L"LuXSI_MenuClicked",oNewItem);
    return CStatus::OK;
}

//--
XSIPLUGINCALLBACK CStatus LuXSI_MenuClicked( XSI::CRef& )
{
    CValueArray addpropArgs(5) ;
    addpropArgs[0] = L"LuXSI"; // Type of Property
    addpropArgs[3] = L"LuXSI"; // Name for the Property
    addpropArgs[1] = L"Scene_Root";
    bool vAlreadyThere = false;
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
//-
XSIPLUGINCALLBACK CStatus LuXSI_PPGEvent( const CRef& in_ctxt )
{
    //--
    ftime = DBL_MAX;
    //-
    PPGEventContext ctxt( in_ctxt ) ;
    //PPGLayout lay = Context(in_ctxt).GetSource() ; // UNUSED

    PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;
    //----------------------------------------->
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
            update_main_values(pname, param, ctxt);
        }
    }//---------------------------------------------------->
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
                //
                luxsi_mat_preview(false);
                //
                // make slg preview scene?
                // slg_mat_preview();
            }
            else
            {
                //-- lux...
                luxsiRender(vblxs_file);                
            }
        }
        if ( buttonOption == L"export_luxsi" || buttonOption == L"render_luxsi")
        {
            //- always..
            createLuxsiScene(ftime);

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
                createLuxsiScene(ftime);
                
                //- test MsgBox
                //long st = 3;
                //kit.MsgBox(L"Message text..", siMsgOk, L"Warning!!", st);
            }
            int ext = int(vFileExport.ReverseFindString("."));
            //- for 'queue' files
            vFileQueue = vFileExport.GetSubString(0,ext) + L".lxq";
            //-
            std::ofstream flxq;
            flxq.open(vFileQueue.GetAsciiString());
            //-
            flxq << queue_list.GetAsciiString();
            flxq.close();
            //-
            if( buttonOption == L"render_ani") luxsi_execute();
        }
    }//----------------------------------------------->
    else if ( eventID == PPGEventContext::siTabChange )
    {
        CValue tabLabel = ctxt.GetAttribute( L"Tab" );
    }//----------------------------------------------------->
    else if ( eventID == PPGEventContext::siParameterChange )
    {
        Parameter changed = ctxt.GetSource();
        prop = changed.GetParent();
        CString paramName = changed.GetScriptName();

        if ( luxdebug ) app.LogMessage( L"Parameter Changed: " + paramName );

        update_main_values(paramName, changed, ctxt);
    }
    return CStatus::OK;
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
    } else if (paramName == L"smooth_mesh")     { exportNormals  = changed.GetValue();
    } else if (paramName == L"sharp_bound")     { vSharp_bound  = changed.GetValue();
    //- geometry
    } else if (paramName == L"over_geo")        { overrGeometry = changed.GetValue();
    } else if (paramName == L"bplymesh")        { vplymesh = changed.GetValue();
    
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
//-
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
    //- slg 
    } else if (paramName == L"rusianrouletedepth") { vrusianrouletdepth   = changed.GetValue();

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
    //-- SLG surface int
    //else if ( paramName == L"slgint") { vslgint = changed.GetValue();
    //}
    //-- volume int
    else if ( paramName == L"bvolumeint") { vvolumeint = changed.GetValue();
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
    else if (paramName == L"bmaxconsrej")  { vmaxconsecrejects  = changed.GetValue();
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
    //else 
    //{
    //    dynamic_luxsi_UI(changed, paramName, ctxt);
    //}
}
//--
CString luxsi_SurfaceIntegrator()
{
    //- Non SLG, only for Lux modes
    const char *MtSurf [7] = { "bidirectional", "path", "directlighting",
        "distributedpath", "igi", "exphotonmap", "sppm" };
    //-
    const char *MtRRst [3] = { "none", "probability", "efficiency"};
    //-
    const char *MtlightST [7] = { "one", "all", "auto", "importance", 
        "powerimp", "allpowerimp", "logpowerimp"};

    CString surfData;
    //-------------------------------------------------------------------------------
    surfData = L"\nSurfaceIntegrator \""+ CString( MtSurf[ vSurfaceInt ] ) + L"\"\n";
    //-------------------------------------------------------------------------------
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
        surfData += L"  \"integer eyedepth\" ["+ CString( vEye_depth ) + L"]\n";
        surfData += L"  \"integer lightdepth\" ["+ CString( vLight_depth ) + L"]\n";
        if ( vsexpert )
        {
            surfData += L"  \"float eyerrthreshold\" ["+ CString( vEyeRRthre ) + L"]\n"; 
            surfData += L"  \"float lightrrthreshold\" ["+ CString( vLightRRthre ) + L"]\n";
        }
    }
    else if ( vSurfaceInt == 1 )//-- path
    {
        surfData += L"  \"integer maxdepth\" ["+ CString( vmaxdepth ) + L"]\n";
        surfData += L"  \"float rrcontinueprob\" [0.649999976158142]\n"; // TODO:
        surfData += L"  \"string rrstrategy\" [\""+ CString( MtRRst[ vRRstrategy ] ) + L"\"]\n";
        surfData += L"  \"bool includeenvironment\" [\""+ CString( MtBool[ vInc_env ] ) + L"\"]\n";
        //-
        if ( vsexpert )
        {
            surfData += L"  \"integer shadowraycount\" ["+ CString( vshadowraycount ) + L"]\n";
        }
    }
    else if ( vSurfaceInt == 2 ) //-- directlighting
    {
        surfData += L"  \"integer maxdepth\" ["+ CString( vmaxdepth ) + L"]\n";
        //-
        if ( vsexpert )
        {
            surfData += L"  \"integer shadowraycount\" ["+ CString( vshadowraycount ) + L"]\n";
        }
    }
    else if ( vSurfaceInt == 3 ) //-- distributedpath
    {
        surfData += L"  \"integer directsamples\" ["+ CString( vdirectsamples ) + L"]\n";
        surfData += L"  \"integer indirectsamples\" ["+ CString( vindirectsamples ) + L"]\n";
        surfData += L"  \"integer diffusereflectdepth\" ["+ CString( vdiffusereflectdepth ) + L"]\n";
        surfData += L"  \"integer diffusereflectsamples\" ["+ CString( vdiffusereflectsamples ) + L"]\n";
        surfData += L"  \"integer diffuserefractdepth\" ["+ CString( vdiffuserefractdepth ) + L"]\n";
        surfData += L"  \"integer diffuserefractsamples\" ["+ CString( vdiffuserefractsamples ) + L"]\n";
        surfData += L"  \"integer glossyreflectdepth\" ["+ CString( vglossyreflectdepth ) + L"]\n";
        surfData += L"  \"integer glossyreflectsamples\" ["+ CString( vglossyreflectsamples ) + L"]\n";
        surfData += L"  \"integer glossyrefractdepth\" ["+ CString( vglossyrefractdepth ) + L"]\n";
        surfData += L"  \"integer glossyrefractsamples\" ["+ CString( vglossyrefractsamples ) + L"]\n";
        surfData += L"  \"integer specularreflectdepth\" ["+ CString( vspecularreflectdepth ) + L"]\n";
        surfData += L"  \"integer specularrefractdepth\" ["+ CString( vspecularrefractdepth ) + L"]\n";
        // 
        if ( vsexpert )
        {
            surfData += L"  \"bool directsampleall\" [\""+ CString( MtBool[ vdirectsampleall ] ) + L"\"]\n";
            surfData += L"  \"bool directdiffuse\" [\""+ CString( MtBool[ vdirectdiffuse ] ) + L"\"]\n";
            surfData += L"  \"bool directglossy\" [\""+ CString( MtBool[vdirectglossy] ) + L"\"]\n";
        
            surfData += L"  \"bool indirectsampleall\" [\""+ CString( MtBool[ vindirectsampleall ] ) + L"\"]\n";
            surfData += L"  \"bool indirectdiffuse\" [\""+ CString( MtBool[ vindirectdiffuse ] ) + L"\"]\n";
            surfData += L"  \"bool indirectglossy\" [\""+ CString( MtBool[ vindirectglossy ] ) + L"\"]\n";
            surfData += L"  \"bool diffusereflectreject\" [\""+ CString( MtBool[0] ) + L"\"]\n"; // TODO
            surfData += L"  \"float diffusereflectreject_threshold\" ["+ CString( vdiff_reflect_reject_thr ) + L"]\n";
            surfData += L"  \"bool diffuserefractreject\" [\""+ CString( MtBool[0] ) + L"\"]\n";
            surfData += L"  \"float diffuserefractreject_threshold\" ["+ CString( vdiff_refract_reject_thr ) + L"]\n";
            surfData += L"  \"bool glossyreflectreject\" [\""+ CString( MtBool[0] ) + L"\"]\n";
            surfData += L"  \"float glossyreflectreject_threshold\" ["+ CString( vglossy_reflect_reject_thr ) + L"]\n";
            surfData += L"  \"bool glossyrefractreject\" [\""+ CString( MtBool[0] ) + L"\"]\n";
            surfData += L"  \"float glossyrefractreject_threshold\" ["+ CString( vglossy_refract_reject_thr ) + L"]\n";
            //--
        }
    }
    else if ( vSurfaceInt == 4 ) //-- igi
    {
        surfData += L"  \"integer nsets\" ["+ CString( vnsets ) + L"]\n";
        surfData += L"  \"integer nlights\" ["+ CString( vnlights ) + L"]\n";
        surfData += L"  \"integer maxdepth\" ["+ CString( vmaxdepth ) + L"]\n";
        surfData += L"  \"float mindist\" ["+ CString( vmindist ) + L"]\n";
    }
    else if ( vSurfaceInt == 5 ) //-- exphotonmap
    {
        surfData += L"  \"integer maxdepth\" ["+ CString( vmaxeyedepth ) + L"]\n"; // vmax eye depth ?
        surfData += L"  \"integer maxphotondepth\" ["+ CString( vmaxphotondepth ) + L"]\n";
        surfData += L"  \"integer directphotons\" ["+ CString( vdirectphotons ) + L"] \n";
        surfData += L"  \"integer causticphotons\" ["+ CString( vcausticphotons ) + L"] \n";
        surfData += L"  \"integer indirectphotons\" ["+ CString( vindirectphotons ) + L"] \n";
        surfData += L"  \"integer radiancephotons\" ["+ CString( vradiancephotons ) + L"] \n";
        surfData += L"  \"integer nphotonsused\" ["+ CString( vnphotonsused ) + L"] \n";
        surfData += L"  \"float maxphotondist\" ["+ CString( vmaxphotondist ) + L"]\n";
        surfData += L"  \"bool finalgather\" [\""+ CString( MtBool[ vfinalgather ] ) + L"\"]\n";
        //-
        if ( vfinalgather )
        {
            surfData += L"  \"integer finalgathersamples\" ["+ CString( vfinalgathersamples ) + L"] \n";
            surfData += L"  \"float gatherangle\" ["+ CString( vgatherangle ) + L"] \n";
        }
        if ( vsexpert )
        {
            //-- test
            const char *MtRendering [] = { "path", "directlighting" };
            //--
            surfData += L"  \"integer shadowraycount\" [\""+ CString( vshadowraycount ) + L"\"]\n"; 
            surfData += L"  \"string renderingmode\" [\""+ CString( MtRendering[ vrenderingmode ] ) + L"\"]\n";
            surfData += L"  \"string rrstrategy\" [\""+ CString( MtRRst[vRRstrategy] ) + L"\"]\n"; 
            surfData += L"  \"float distancethreshold\" [\""+ CString( vdistancethreshold ) + L"\"]\n"; 
            surfData += L"  \"bool dbg_enabledirect\" [\""+ CString( MtBool[vdbg_direct] ) + L"\"]\n";
            surfData += L"  \"bool dbg_enableradiancemap\" [\""+ CString( MtBool[vdbg_radiancemap] ) + L"\"]\n";
            surfData += L"  \"bool dbg_enableindircaustic\" [\""+ CString( MtBool[vdbg_indircaustic] ) + L"\"]\n";
            surfData += L"  \"bool dbg_enableindirdiffuse\" [\""+ CString( MtBool[vdbg_indirdiffuse] ) + L"\"]\n";
            surfData += L"  \"bool dbg_enableindirspecular\" [\""+ CString( MtBool[vdbg_indirspecular] ) + L"\"]\n";
        }        
    }
    else // sppm
    {
        //- 
        surfData += L"  \"integer maxeyedepth\" ["+ CString( vbmaxeyedepht ) + L"]\n"; 
	    surfData += L"  \"integer maxphotondepth\" ["+ CString( vbmaxphoton ) + L"]\n";
	    surfData += L"  \"integer photonperpass\" ["+ CString( vbphotonsxpass ) + L"]\n";
	    surfData += L"  \"integer hitpointperpass\" ["+ CString( vbpointxpass ) + L"]\n"; 
	    surfData += L"  \"float startradius\" ["+ CString( vbstartradius ) + L"]\n";
	    surfData += L"  \"float alpha\" ["+ CString( vbalpha ) + L"]\n";
	    surfData += L"  \"bool includeenvironment\" [\""+ CString( MtBool[vbincenvironment] ) + L"\"]\n";
	    surfData += L"  \"bool directlightsampling\" [\""+ CString( MtBool[vbdlsampling] ) + L"\"]\n"; 
        //-
        /*
        //--- expert
        "bool storeglossy" ["false"]
	    "bool useproba" ["true"]
	    "integer wavelengthstratificationpasses" [8]
	    "string lookupaccel" ["kdtree"]
	    "float parallelhashgridspare" [1.000000000000000]
	    "string pixelsampler" ["hilbert"]
	    "string photonsampler" ["halton"]
        */
        if ( vsexpert )// use default values.. atm!
        {
	        surfData += L"  \"bool storeglossy\" [\"false\"]\n";
	        surfData += L"  \"bool useproba\" [\"true\"]\n";
	        surfData += L"  \"integer wavelengthstratificationpasses\" [8]\n";
	        surfData += L"  \"string lookupaccel\" [\"kdtree\"]\n";
	        surfData += L"  \"float parallelhashgridspare\" [1.000000000000000]\n";
	        surfData += L"  \"string pixelsampler\" [\"hilbert\"]\n";
	        surfData += L"  \"string photonsampler\" [\"halton\"]\n";
        }
    }
    //- light strategy for all, except sppm
    if ( vSurfaceInt < 6 )
    {
        surfData += L"  \"string lightstrategy\" [\""+ CString(MtlightST[vLight_str]) + L"\"]\n";
    }
    return surfData;
}
//-
CString luxsi_VolumeIntegrator()
{
    const char *volint[]={"none", "single", "multi", "emission"};
    CString volData;
    //-
    volData = L"\nVolumeIntegrator \""+ CString(volint[vvolumeint]) + L"\"\n";
    volData += L"   \"float stepsize\" [1.0000]\n";
    //-
    return volData;
}
//--
CString luxsi_Sampler()
{
    CString samplerStr;
    //-
    const char *MtBsampler[]= { "lowdiscrepancy", "random" };
    //-
    const char *aPxSampler [] = { "linear", "vegas", "lowdiscrepancy", "tile", "hilbert" };
    //-
    const char *aSampler [] = { "metropolis", "erpt", "lowdiscrepancy", "random" };
    
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
    //-----------------------------------------------------------------
    AccelStr = L"\nAccelerator \""+ CString(MtAccel[vAccel]) + L"\"\n";
    //-----------------------------------------------------------------
    if ( vAccel == 0  &&  vacexpert ) //-- qbvh
    {
        AccelStr += L"  \"integer maxprimsperleaf\" ["+ CString( vmaxprimsperleaf ) + L"]\n";
        AccelStr += L"  \"integer fullsweepthreshold\" ["+ CString( vfullsweepthr ) + L"]\n";
        AccelStr += L"  \"integer skipfactor\" ["+ CString( vskipfactor ) + L"]\n";
    }
    else if ( vAccel == 1  ||  vacexpert ) //-- bvh
    {
        AccelStr += L"  \"integer intersectcost\" ["+ CString( vintersectcost ) + L"]\n";
        AccelStr += L"  \"integer traversalcost\" ["+ CString( vtraversalcost ) + L"]\n";
        AccelStr += L"  \"integer costsamples\" ["+ CString( vcostsamples ) + L"]\n"; // lbcostsamples
    }
//    else if (( vAccel == 2 ) || ( vacexpert )) //-- grid
//   {
//        f << "  \"bool refineimmediately\" [\""<< MtBool[vrefineinmed] <<"\"]\n";
//    }
    else //-- KD Tree
    {
        if ( vacexpert )
        {
            AccelStr += L"  \"integer intersectcost\" ["+ CString( vintersectcost ) + L"]\n";
            AccelStr += L"  \"integer traversalcost\" ["+ CString( vtraversalcost ) + L"]\n";
            AccelStr += L"  \"float emptybonus\" ["+ CString( vemptybonus ) + L"]\n";
            AccelStr += L"  \"integer maxprims\" ["+ CString( vmaxprims ) + L"]\n";
            AccelStr += L"  \"integer maxdepth\" ["+ CString( vacmaxdepth ) + L"]\n\n";
        }
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
CString writeLuxsiBasics()
{
    /** write basic scene description */
    //--
    const char *ARmode [2] = { "GUI", "console"};
    const char *Aengine [2] = { "classic", "hybrid" };
    const char *ArgbT [3] = { "Y", "RGB", "RGBA" }; // iterator  vRtga
    const char *AZb_norm [3] = { "Camera Start/End clip", "Min/Max", "None" }; //exr
    /* 
	Film "fleximage"
	"integer xresolution" [960]
	"integer yresolution" [540]
	"float gamma" [2.200000000000000]
	"float colorspace_white" [0.314275000000000 0.329411000000000]
	"float colorspace_red" [0.630000000000000 0.340000000000000]
	"float colorspace_green" [0.310000000000000 0.595000000000000]
	"float colorspace_blue" [0.155000000000000 0.070000000000000]
	"string filename" ["probas.Scene.00001"]
	"bool write_resume_flm" ["false"]
	"bool restart_resume_flm" ["false"]
	"bool write_flm_direct" ["false"]
	"bool write_exr_halftype" ["true"]
	"bool write_exr_applyimaging" ["true"]
	"bool write_exr_ZBuf" ["false"]
	"string write_exr_compressiontype" ["PIZ (lossless)"]
	"string write_exr_zbuf_normalizationtype" ["None"]
	"bool write_exr" ["false"]
	"bool write_png" ["true"]
	"string write_png_channels" ["RGB"]
	"bool write_png_16bit" ["false"]
	"bool write_tga" ["false"]
	"string ldr_clamp_method" ["cut"]
	"integer displayinterval" [10]
	"integer writeinterval" [180]
	"integer flmwriteinterval" [900]
	"integer tilecount" [0]
	"string tonemapkernel" ["autolinear"]
	*/
    //--
    CString basicData;
    CString imageFile = luxsi_normalize_path(vFileLxs);
    
    basicData = L"\nFilm \"fleximage\"\n"; //----
    basicData += L"  \"integer xresolution\" ["+ CString( vXRes ) + L"]\n";
    basicData += L"  \"integer yresolution\" ["+ CString( vYRes ) + L"]\n";
    basicData += L"  \"string filename\" [\""+ imageFile + L"\"]\n";
    
    basicData += L"  \"integer writeinterval\" ["+ CString( vSave ) + L"]\n";
    basicData += L"  \"integer displayinterval\" ["+ CString( vDis ) + L"]\n";

    basicData += L"  \"bool write_exr\" [\""+ CString( MtBool[vExr] ) + L"\"]\n";
    //-
    if (vExr)
    {
        basicData += L"  \"string write_exr_zbuf_normalizationtype\" [\""+ CString( AZb_norm[vExr_Znorm] ) + L"\"]\n";
    } //----/ TODO: more options for Exr /------>
    
    /** write image render  to png format*/
    basicData += L"  \"bool write_png\" [\""+ CString( MtBool[vPng] ) + L"\"]\n";
    //-
    const char *ArgbP [4] = { "Y", "YA", "RGB", "RGBA" };
    
    if (vPng)
    {
        basicData += L"  \"string write_png_channels\" [\""+ CString( ArgbP[vRpng] ) + L"\"]\n";
        basicData += L"  \"bool write_png_16bit\" [\""+ CString( MtBool[vWpng_16] ) + L"\"]\n";
        basicData += L"  \"bool write_png_gamutclamp\" [\""+ CString( MtBool[vPng_gamut] ) + L"\"]\n";
    }

    /**  write image render to tga format */
    basicData += L"  \"bool write_tga\" [\""+ CString( MtBool[vTga] ) + L"\"]\n";
    if (vTga)
    {
        basicData += L"  \"string write_tga_channels\" [\""+ CString( ArgbT[vRtga] ) + L"\"]\n";
        basicData += L"  \"bool write_tga_gamutclamp\" [\""+ CString( MtBool[vTga_gamut] ) + L"\"]\n";
    } //----/ TODO: more options for Tga /------>

    basicData += L"  \"bool write_resume_flm\" [\""+ CString( MtBool[vResume] ) + L"\"]\n";
    basicData += L"  \"bool premultiplyalpha\" [\""+ CString( MtBool[0] ) + L"\"]\n"; // TODO;
    basicData += L"  \"integer haltspp\" ["+ CString( vhaltspp ) + L"]\n";
    basicData += L"  \"integer halttime\" ["+ CString( vhalttime ) + L"]\n";

    basicData += L"  \"float gamma\" ["+ CString( vContrast ) + L"]\n\n";

    /* Render mode */
    CString renderMode = L"sampler";
    if ( vSurfaceInt == 6 ) renderMode = L"sppm";
    //if ( vSurfaceInt > 6 ) renderMode = L"slg";
    //-
    basicData += L"Renderer \""+ renderMode + L"\"\n";

    //-- Filter -->
    basicData += luxsi_Filter();
    
    //-- Sampler -->
    basicData += luxsi_Sampler();
    
    //-- Surface integrator -->
    basicData += luxsi_SurfaceIntegrator();

    //-- Volume Integrator -->
    basicData += luxsi_VolumeIntegrator();

    //-- Accelerator -->
    basicData += luxsi_Accelerator();
    //-
 return basicData;
}
//--
CString header_info()
{
    //-- commons header for files .lxm and .lxo
    CString head;
    head += L"\n# Created by LuXSI; Luxrender Exporter for Autodesk Softimage. \n";
    head += L"# Copyright (C) 2007 - 2013 of all Authors: \n";
    head += L"# Michael Gangolf, \"miga\". \n";
    head += L"# Pedro Alcaide, \"povmaniaco\". \n \n";
    //-
    return head;
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
//-
void sceneCollectionsObjects()
{
    //--
    sceneItemsArray.Clear();
    aObj.Clear();
    //aCam.Clear();
    aSurfaces.Clear();
    aClouds.Clear();
    aModels.Clear();
    aHair.Clear();

    //- cleaned global CStringArray  for instance list.
    //aInstanceList.Clear();
    //--
    if ( luxdebug ) app.LogMessage(L"[DEBUG]: Created and cleaned arrays..");

    root = app.GetActiveSceneRoot();
    //--
    sceneItemsArray += root.FindChildren( L"", L"", CStringArray(), true );
    for ( int i=0; i < sceneItemsArray.GetCount(); i++ )
    {
        X3DObject o(sceneItemsArray[i]);
        //- test for search all objects type.
        if ( luxdebug ) app.LogMessage( L"Object Name: [" 
            + o.GetName() + L"] Type: [" 
            + o.GetType()+ L"] parent: "
            + X3DObject(o.GetParent()).GetName());
           
        //-- Collection objects / visibilty check
        if (o.GetType()==L"polymsh")
        {
            if ( is_visible(o, L"polymsh"))     aObj.Add(o);
        }
        else if (o.GetType()==L"hair")
        {
            if ( is_visible(o, L"polymsh"))     aHair.Add(o);// TODO: 
        }
        else if (o.GetType() == L"surfmsh" )
        {
            if ( is_visible(o, L"surfmsh"))     aSurfaces.Add(o);
        }
        else if (o.GetType()== L"pointcloud")
        {
            if ( is_visible(o, L"pointcloud"))  aClouds.Add(o); 
        }
        else if (o.GetType()==L"#model")
        {
            if ( is_visible(o, L"polymsh"))     aModels.Add(o);                
        }
        else
        {
            if ( luxdebug )app.LogMessage(L" Object type [ "+ o.GetType() + L" ] named [ "+ o.GetName() + L" ] not listed");
        }
        //- TODO; placed here advice for empty scene
    } 
}
//--
void createLuxsiScene(double ftime)
{
    /*! write objects, materials, lights, cameras
    */
    if (!vFileExport.IsEmpty())
    {
        /* Add collections objects.
        */
        sceneCollectionsObjects();
        
        //-- 
        if ( aObj.GetCount() == 0 )
        {
            app.LogMessage(L"Any objects to export! Check a list of 'Export hidden items..' in GUI", siFatalMsg );
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
            
            //-------------------------------//
            // Set file names for Animation  //
            //-------------------------------//
            if ( luxdebug ) app.LogMessage(L"[DEBUG]: ftime is: "+ CString(ftime));
            //-
            if ( ftime != DBL_MAX )
            {
                /* Add frame number to outfile name */
                vFileLxs = vFileLxs.GetSubString(0, ext) + (L"_"+ CString(ftime) + L".lxs");

                /* Now, call luxsi_normalize_path(), for use 'relative path', with the links inside LXQ file.
                */
                CString inc_LXQ = luxsi_normalize_path(vFileLxs) + L".lxs";
                //if ( vEngine == 0) inc_LXQ = luxsi_normalize_path(vFileLxs) + L".cfg"; // or .scn ?                
                //-
                if ( lqueue )
                {
                    queue_list += inc_LXQ + L"\n";
                }
                if ( luxdebug ) app.LogMessage(L"OUT Filename: "+ inc_LXQ );
            }//--------------------------------------------------------------//

            /** For animation, reset 'ext' value to new filename.
            *   The lenght of new filename as change (name + frame).
            *   Not use normalized path, ofstream need the full path.
            */
            // ext = int(vFileLxs.ReverseFindString("."));
          
            /** For include link to geometry files into .LXO file, 
            *   add vFileLxs value to global 'vFileGeo' for include frame number.
            *   Name of object and extension, is added later.
            */
            vFileGeo = vFileLxs;
            
            //-------------------//
            // Init progress bar //
            //-------------------//
            pb.PutValue(0);
            pb.PutMaximum( sceneItemsArray.GetCount());
            pb.PutStep(1);
            pb.PutVisible( true );
            pb.PutCaption( L"Processing data for exporter.." );
            pb.PutCancelEnabled(true);
            
            //------------------------------------------------------------
            CString SLGScene;  
            //- slg modes
            if ( vSurfaceInt > 6 ) // vEngine == 1 )
            {
                /*  write slg data and change .lxs  to .cfg
                 */
                vFileLxs = SLGFileconfig(vFileLxs);
                //-
                app.LogMessage("The name for CFG file is: "+ vFileLxs);
                //-
                vExportDone = true;
            }
            else
            {
                /* Write LXS 
                */                
                writeLxsFile(vFileLxs);

                /* Write LXM file with all material data
                */
                writeLxmFile(vFileLxs);
            
                /* Write LXO file for all geometry data
                */
                writeLxoFile(vFileLxs, aObj, aModels, aClouds, aSurfaces, aHair);
                
                //- done..
                vExportDone = true;
            }
            //- hidden ProgressBar
            pb.PutVisible( false );
        }
    }
    else
    {
        app.LogMessage(L"Filename is empty", siFatalMsg );
    }
}
//--
int writeLxoFile(CString fileLxs, CRefArray meshes, CRefArray models, 
                 CRefArray clouds, CRefArray surfaces, CRefArray hairs)
{
    //- Ghatering all geometry data
    
    CString geometryData;
    for (int i = 0; i < meshes.GetCount(); i++) 
    {
        geometryData += writeLuxsiObj(meshes[i]);
        if (pb.IsCancelPressed() ) break;
        pb.Increment();
    }
    //- Models and Instances
    CString instanceData;
    if (models.GetCount() >0)
    {
        instanceData += writeLuxsiInstance(models);
    }
    //- Pointcloud objects
    CString pointCloudData;
    for (int i = 0; i < clouds.GetCount(); i++) 
    {
        pointCloudData += writeLuxsiCloud(clouds[i]);
        if (pb.IsCancelPressed() ) break; 
        pb.Increment();
    }
    //- Surface / Primitives
    CString surfaceData;
    for (int i=0; i < surfaces.GetCount(); i++) 
    {
        surfaceData += writeLuxsiSurface(surfaces[i]);
        if (pb.IsCancelPressed() ) break;
        pb.Increment();
    }
    //- 'Hair emitter' objects
    CString hairData;
    for ( int i = 0; i < hairs.GetCount(); i++)
    {
        hairData  += writeLuxsiHair(hairs[i]);
        if (pb.IsCancelPressed() ) break;
        pb.Increment();
    }
            
    //-------------------------//
    int ext = int(vFileLxs.ReverseFindString("."));
    //- material file..
    CString vFileLXO = fileLxs.GetSubString(0, ext) + (L"_geo.lxo");
    //-
    std::ofstream flxo;
    flxo.open(vFileLXO.GetAsciiString(),'w'); 

    //- Header
    flxo << "#\n# LuxRender Geometry file\n";

    //- Polygon Mesh
    flxo << geometryData.GetAsciiString();
           
    //- Surfaces
    if ( !surfaceData.IsEmpty() )
    {
        flxo << surfaceData.GetAsciiString();
    }
    //- Pointclouds
    if ( !pointCloudData.IsEmpty() )
    {
        flxo << pointCloudData.GetAsciiString();
    }
    //- Instances
    if ( !instanceData.IsEmpty() )
    {
        flxo << instanceData.GetAsciiString();
    }
    //- Hair
    if ( !hairData.IsEmpty() )
    {
        flxo << hairData.GetAsciiString();
    }
    //-
    flxo.close();
    //-
    return 0;
}
//-
int writeLxsFile(CString fileLxs)
{
    // path to files into lxs file
    CString path_base = luxsi_normalize_path(fileLxs);
    CString inc_LXM = path_base + L"_mat.lxm";  // material definitions
    CString inc_LXO = path_base + L"_geo.lxo";  // geometry definitions
    //CString inc_LXV = path_base + L"_vol.lxv";  // volume definitions
    
    //-- camera
    CString camData = writeLuxsiCam();
    //- basics
    CString basicData = writeLuxsiBasics();
    //- lights
    CString lightData = writeLuxsiLight();
    
    std::ofstream flxs;
    flxs.open(fileLxs.GetAsciiString(),'w');
    //-
    if (flxs.good()) // is OK?
    {
        //- insert header
        flxs << "#\n# LuXSI / LuxRender scene file\n";
        //- Camera
        flxs << camData.GetAsciiString();
        //- Basic scene data
        flxs << basicData.GetAsciiString();
        //-
        flxs << "\nWorldBegin \n";
        //- includes
        flxs << "\nInclude \""<< inc_LXM.GetAsciiString() <<"\" \n";
        flxs << "\nInclude \""<< inc_LXO.GetAsciiString() <<"\" \n";
        //flxs << "Include \""<< inc_LXV.GetAsciiString() <<"\" \n";

        //- lights
        flxs << "\nAttributeBegin \n";

        flxs << lightData.GetAsciiString();

        flxs << "\nAttributeEnd \n";
        //-
        flxs << "\nWorldEnd";

        flxs.close();
    }
    else
    {
        // test
        app.LogMessage(L"Unable to open LXS file.");
        flxs.close();
    }
    return 0;
}
//--
int writeLxmFile(CString fileLxs)
{
    //- path to material file.. for ofstream
    int ext = int(fileLxs.ReverseFindString("."));
    //- 
    CString vFileLXM = fileLxs.GetSubString(0, ext) + (L"_mat.lxm");
    //-
    CString shaderData = writeLuxsiShader();
            
    /* Open LXM file 
    */
    std::ofstream flxm;
    flxm.open(vFileLXM.GetAsciiString(), 'w'); 

    //- header
    flxm << "#\n# LuxRender Material file\n";
                            
    //- material data
    flxm << shaderData.GetAsciiString();
    flxm.close();
    //-
    return 0;
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
        CString Lux_Binarie = L"", exec = L"";
        //-
        if ( vSurfaceInt < 6 ) // Lux modes
        {
            if (vRmode == 0 )
            {
                Lux_Binarie = vLuXSIPath + L"/luxrender.exe";
            } 
            else 
            {
                Lux_Binarie = vLuXSIPath + L"/luxconsole.exe";
            }
            //- by default load unique .lxs file..
            exec = Lux_Binarie +" \""+ vFileLxs + "\"";

            //- or use queue list for load a list files..
            if (lqueue) exec = Lux_Binarie + L" -L \""+ vFileQueue + "\"";

            //- reset queue
            lqueue = false; 
        }
        else // SLG modes
        {
            /*  Ignore render mode ( atm..) use only slg gui.
                rmode 1 = slg console(batch mode)
            */
            Lux_Binarie = vLuXSIPath + L"/slg4.exe";
            //-
            exec = Lux_Binarie + L" -o \""+ vFileLxs + "\"";
        }
        //-
        app.LogMessage(exec);
        //- execute..
        loader(exec.GetAsciiString());
    }
    else
    {
        app.LogMessage(L" Not data file exported, retry exporting scene, before render ", siErrorMsg );
    }
}

//-
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
