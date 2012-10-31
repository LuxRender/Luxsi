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

#include "include\luxsi_dynamicUI.h"

//--
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
void dynamic_filter_UI( Parameter changed, CString paramName, PPGEventContext ctxt)
{
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
void dynamic_surfaceInt_UI(Parameter changed, CString paramName, PPGEventContext ctxt)
{
    //--   
    const char *u_intgrator [] = {
        /*ditributepath 26*/"bdirectsampleall", "bdirectsamples", "bindirectsampleall", "bindirectsamples",
        "bdiffusereflectdepth", "bdiffusereflectsamples", "bdiffuserefractdepth", "bdiffuserefractsamples",
        "bdirectdiffuse", "bindirectdiffuse", "bglossyreflectdepth", "bglossyreflectsamples", "bglossyrefractdepth",
        "bglossyrefractsamples", "bdirectglossy", "bindirectglossy", "bspecularreflectdepth", "bspecularrefractdepth",
        "bdiff_reflect_reject_thr", "bdiff_refract_reject_thr", "bglossy_reflect_reject_thr", "bglossy_refract_reject_thr",
        "bdiff_reflect_reject", "bdiff_refract_reject", "bglossy_reflect_reject", "bglossy_refract_reject",
        /*exphotonmap 20*/"bstrategy", "bshadowraycount", "bmaxphotondepth", "bmaxeyedepth", "bmaxphotondist",
        "bnphotonsused", "bindirectphotons", "bdirectphotons", "bcausticphotons", "bradiancephotons", "bfinalgather",
        "brenderingmode", "bfinalgathersamples", "bgatherangle", "bdistancethreshold", "bdbg_enabledirect", "bdbg_enableradiancemap",
        "bdbg_enableindircaustic", "bdbg_enableindirdiffuse", "bdbg_enableindirspecular",
        /*sppm 15*/"bmaxeyedepht", "bmaxphoton", "bpointxpass", "bphotonsxpass", "bstartradius", "balpha", "bdlsampling",
        "bincenvironment", "bstoreglossy", "buseproba", "bwavelengthstratificationpasses", "blookupaccel", 
        "bparallelhashgridspare", "bpixelsampler", "bphotonsampler",
        /*path 5*/"blight_str", "binc_env", "brrstrategy", "bmaxdepth", "brrcon_prob",
        /*bidirectional 4*/"blight_depth", "beye_depth", "beyerrthre", "blightrrthre",
        /*IGI 3*/"bnsets", "bnlights", "bmindist"};
       
    //-
    for ( long in = 0; in < 73;)
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
            show_params(L"blight_str");
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
            show_params(L"bshadowraycount"); //- revised
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
        //-
        show_params(L"bmaxeyedepht");
        show_params(L"bmaxphoton");
        show_params(L"bpointxpass");
        show_params(L"bphotonsxpass");
        show_params(L"bstartradius");
        show_params(L"balpha");
        show_params(L"bdlsampling");
        show_params(L"bincenvironment");
        //-
        if (vsexpert )
        {
            show_params(L"bstoreglossy");
            show_params(L"buseproba");
            show_params(L"bwavelengthstratificationpasses");
            show_params(L"blookupaccel");
            show_params(L"bparallelhashgridspare");
            show_params(L"bpixelsampler");
            show_params(L"bphotonsampler");
        }
        //--
        ctxt.PutAttribute(L"Refresh", true );
    }
    //- for all, except sppm
    if ( vSurfaceInt < 6 ) show_params(L"blight_str");
}
//--
void dynamic_sampler_UI( Parameter changed, CString paramName, PPGEventContext ctxt)
{
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

//-
void dynamic_Accel_UI(Parameter changed, CString paramName, PPGEventContext ctxt)
{
    //-- Accelerator
    const char *ui_accel [9] = {
        /* qbvh */"bmaxprimsperleaf", "bfullsweepthreshold", "bskipfactor",
        /* bvh */"bcostsamples", "bintersectcost", "btraversalcost", "bmaxprims", "bacmaxdepth", "bemptybonus" };
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
    //------------
    // Status: OK
    //------------

    //- tga ------------------->
    hide_params( L"mode_rtga" ); 
    hide_params( L"tga_gamut" );
    //--
    if ( vTga )
    {
        show_params( L"mode_rtga" );
        show_params( L"tga_gamut" );
        //-
        ctxt.PutAttribute( L"Refresh", true );
    }
    //-- exr ------------------->
    hide_params( L"mode_Znorm" );
    //-
    if ( vExr )
    {
        show_params( L"mode_Znorm" );
        //--
        ctxt.PutAttribute( L"Refresh", true );
    }
    //- png -------------------->
    hide_params( L"mode_rpng"   ); 
    hide_params( L"save_png_16" ); 
    hide_params( L"png_gamut"   );
    //-
    if ( vPng )
    {
        show_params( L"mode_rpng"   );
        show_params( L"save_png_16" );
        show_params( L"png_gamut"   );
        //-
        ctxt.PutAttribute( L"Refresh", true );
    }
}

//--