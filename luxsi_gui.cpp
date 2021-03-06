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

//--
XSIPLUGINCALLBACK CStatus LuXSI_DefineLayout( CRef& in_ctxt )
{
    PPGLayout lay = Context(in_ctxt).GetSource() ;

    lay.Clear();

    //-------------------//
    lay.AddTab(L"Main");
    //-------------------//
    lay.AddGroup(L"Image");
        lay.AddRow();
            lay.AddItem(L"Width");
            lay.AddItem(L"Height");
        lay.EndRow();
            lay.AddItem(L"gamma",L"Gamma");
        lay.EndRow();
        
        //-
        lay.AddRow(); // new form; row with two groups
            lay.AddGroup(); //-- tga
                lay.AddItem(L"save_tga", L"TGA");
                CValueArray vItrtga(6);
                    vItrtga[0] = L"Y";      vItrtga[1] = 0;
                    vItrtga[2] = L"RGB";    vItrtga[3] = 1;
                    vItrtga[4] = L"RGBA";   vItrtga[5] = 2;
                lay.AddEnumControl(L"mode_rtga",vItrtga,L"Mode",siControlCombo ) ;
                lay.AddItem(L"tga_gamut", L"Gamut Clamp");
            lay.EndGroup(); //- end tga

            lay.AddGroup(); //-- Exr
                lay.AddItem(L"save_exr", L"EXR");
                CValueArray vItZb_nor(6);
                    vItZb_nor[0] = L"Camera Start/End clip";	vItZb_nor[1] = 0;
                    vItZb_nor[2] = L"Min/Max" ;					vItZb_nor[3] = 1;
                    vItZb_nor[4] = L"None" ;					vItZb_nor[5] = 2;
                lay.AddEnumControl(L"mode_Znorm",vItZb_nor,L"Mode Zbuffer",siControlCombo ) ;
            lay.EndGroup(); //- end Exr
    lay.EndRow();//- end row double group

    lay.AddGroup(); //- png
        lay.AddRow(); //----
            lay.AddItem(L"save_png_16", L"16 Bits");
            CValueArray vItrpng(8);
                vItrpng[0] = L"Y" ;    vItrpng[1] = 0;
                vItrpng[2] = L"YA" ;    vItrpng[3] = 1;
                vItrpng[4] = L"RGB" ;    vItrpng[5] = 2;
                vItrpng[6] = L"RGBA" ;    vItrpng[7] = 3;
            lay.AddEnumControl(L"mode_rpng", vItrpng, L"Mode", siControlCombo ) ;
        lay.EndRow(); //----
        lay.AddRow(); //----
            lay.AddItem(L"png_gamut", L"Gamut Clamp");
            lay.AddItem(L"save_png", L"PNG");
        lay.EndRow(); //----
    lay.EndGroup(); //- end png

        lay.AddItem(L"fileExport",L"Out File", siControlFilePath);
        PPGItem it = lay.GetItem( L"fileExport" );
        it.PutAttribute( siUIOpenFile, false);
        it.PutAttribute( siUIFileMustExist, true); // advice for re-write files
        it.PutAttribute( siUIInitialDir, 0);
        it.PutAttribute( siUIFileFilter, L"LuXSI Scenes|*.lxs" ) ;
    
    lay.EndGroup(); //- Image

    lay.AddGroup(L"Export hidden items...");
        lay.AddRow();
            lay.AddItem(L"use_hidden_obj",		L"Objects");
            lay.AddItem(L"use_hidden_cam",		L"Cameras");
        lay.EndRow();
        lay.AddRow();
            lay.AddItem(L"use_hidden_light",	L"Lights");
            lay.AddItem(L"use_hidden_surf",		L"Surfaces");
        lay.EndRow();
    lay.EndGroup(); //- export hidden..

    lay.AddGroup(L"Geometry"); //L"Mesh options");
        lay.AddItem(L"smooth_mesh", L"Export smooth meshes");
        lay.AddItem(L"sharp_bound", L"Preserve sharp edges");
        lay.AddRow();
            lay.AddItem(L"bplymesh", L"Use PLY mesh");
            lay.AddItem(L"over_geo", L"Override Geometry");
        lay.EndRow();
    lay.EndGroup();

    
        lay.AddRow();
            lay.AddButton(L"export_luxsi", L"Export to file");
            lay.AddButton(L"render_luxsi", L"Render scene");
        lay.EndRow();
    
    //------------------------------// 
        lay.AddTab(L"General ");
    //------------------------------//
    //-- Render mode, internal:console / external:gui
    lay.AddGroup(L"LuxRender Engine");
        lay.AddRow();
        CValueArray Acombo(4);
            Acombo[0] = L"GUI";     Acombo[1] = 0; //-- TODO;
            Acombo[2] = L"Console"; Acombo[3] = 1;
        lay.AddEnumControl( L"brmode", Acombo, L"Render", siControlCombo ) ;

        CValueArray vAengine(4);
            vAengine[0] = L"Classic";   vAengine[1] = 0;
            vAengine[2] = L"Hybrid";    vAengine[3] = 1;
        lay.AddEnumControl( L"bengine", vAengine, L"Engine", siControlCombo ) ;
        lay.EndRow();
        
        //--
        lay.AddRow();
            lay.AddItem( L"bthreads", L"Threads" );
            lay.AddItem( L"bautode", L"Auto-Threads" );
        lay.EndRow();
        /* new place */
        lay.AddRow();
            lay.AddItem(L"disint", L"Display Int.");
            lay.AddItem(L"savint", L"Save Int.");
        lay.EndRow();
            lay.AddItem(L"fLuxPath", L"Luxrender Path", siControlFolder);
            PPGItem lpath = lay.GetItem( L"fLuxPath" ); 
            lpath.PutLabelPercentage(55);
    lay.EndGroup();

        //-----------------/
        lay.AddSpacer(1,0);
        //-----------------/
        lay.AddGroup(L"Animation / Particles");
            lay.AddRow();
                lay.AddItem(L"hSpp", L"Halt at S/px");
                lay.AddItem(L"hTime", L"Halt at time");
            lay.EndRow();
            lay.AddRow();
                lay.AddButton(L"render_ani", L"Render anim");
                lay.AddItem(L"bframestep", L"Frame Step", siUINoSlider);
                lay.AddButton(L"export_ani", L"Export anim");
            lay.EndRow();
            lay.AddRow();
                lay.AddItem(L"resume", L"Resume render");
                lay.AddItem(L"loglevel", L"Debug Msg");
            lay.EndRow();
        lay.EndGroup();

 
    //---------------------// 
    lay.AddTab(L"Render ");
    //---------------------//
    CValueArray aPresets(24);
        aPresets[0] = L"Custom parameter";                              aPresets[1] = 0;
        aPresets[2] = L"Preview - Instant Global Illumination";         aPresets[3] = 1;
        aPresets[4] = L"Preview - Direct Lighting (No GI)";             aPresets[5] = 2;
        aPresets[6] = L"Preview - Ex-Photonmap";                        aPresets[7] = 3;
        aPresets[8] = L"Final - MLT/Bidir Path Tracing (int) (recom.)"; aPresets[9] = 4;
        aPresets[10] = L"Final - MLT/Path Tracing (exterior)";          aPresets[11] = 5;
        aPresets[12] = L"Progressive - Bidir Path Tracing (interior)";  aPresets[13] = 6;
        aPresets[14] = L"Progressive - Path Tracing (exterior)";        aPresets[15] = 7;
        aPresets[16] = L"Bucket - Bidir Path Tracing (interior)";       aPresets[17] = 8;
        aPresets[18] = L"Bucket - Path Tracing (exterior)";             aPresets[19] = 9;
        aPresets[20] = L"Hybrid - Path Tracing (test)";                 aPresets[21] = 10;
        aPresets[22] = L"Hybrid - Bidir. Path Tracing (test)";          aPresets[23] = 11;
    lay.AddEnumControl(L"bpresets", aPresets, L"R. Presets", siControlCombo ) ;

    lay.AddGroup(L"Surface Integrator"); //-- surface
        lay.AddRow();
            CValueArray vItem6(18);
                vItem6[0]  = L"Bidirectional";      vItem6[1] = 0;
                vItem6[2]  = L"Pathtracing";        vItem6[3] = 1;
                vItem6[4]  = L"Directlighting";     vItem6[5] = 2;
                vItem6[6]  = L"Distributedpath";    vItem6[7] = 3;
                vItem6[8]  = L"Inst. Global Illu."; vItem6[9] = 4;
                vItem6[10] = L"Ex-Photonmap";       vItem6[11] = 5;
                vItem6[12] = L"SPPM";               vItem6[13] = 6;
                vItem6[14] = L"Hybrid (PT)";        vItem6[15] = 7;
                vItem6[16] = L"Hybrid (BDPT)";      vItem6[17] = 8;
            lay.AddEnumControl( L"bsurfaceint", vItem6, L"Integrator", siControlCombo ) ;
            lay.AddItem(L"bsexpert", L"Advanced").PutWidthPercentage(6);
        lay.EndRow();
    lay.AddGroup(); //-- no label
            CValueArray vAlist(14); //-- light strategy
                vAlist[0]  = L"one";         vAlist[1] = 0;
                vAlist[2]  = L"all";         vAlist[3] = 1;
                vAlist[4]  = L"auto";        vAlist[5] = 2;
                vAlist[6]  = L"importance";  vAlist[7] = 3;
                vAlist[8]  = L"powerimp";    vAlist[9] = 4;
                vAlist[10] = L"allpowerimp"; vAlist[11] = 5;
                vAlist[12] = L"logpowerimp"; vAlist[13] = 6;
            lay.AddEnumControl( L"blight_str", vAlist, L"Light Strategy",siControlCombo).PutLabelPercentage(70);//-- combo

            //-- bidireccional / path / directlighting
            lay.AddItem( L"bmaxdepth", L"Max. depth").PutLabelPercentage(70);
            lay.AddItem( L"bshadowraycount", L"Shadow Ray Count").PutLabelPercentage(70);
            lay.AddItem( L"beye_depth",   L"Eye Depth").PutLabelPercentage(70);
            lay.AddItem( L"blight_depth", L"Light Depth").PutLabelPercentage(70);
            lay.AddItem( L"beyerrthre",   L"Eye RR Thresh.").PutLabelPercentage(70);
            lay.AddItem( L"blightrrthre", L"Light RR Thresh.").PutLabelPercentage(70);
            
            //-- distributedpath
            lay.AddItem(L"bdirectsamples", L"Direct light Sampling").PutLabelPercentage(80);
        lay.AddRow();
            lay.AddItem(L"bdirectsampleall", L"Sample all");
            lay.AddItem(L"bdirectdiffuse", L"Diffuse");
            lay.AddItem(L"bdirectglossy", L"Glossy");
        lay.EndRow();
            lay.AddItem(L"bindirectsamples", L"Indirect light Sampling").PutLabelPercentage(80);
        lay.AddRow();
            lay.AddItem(L"bindirectsampleall", L"Sample all");
            lay.AddItem(L"bindirectdiffuse", L"Diffuse");
            lay.AddItem(L"bindirectglossy", L"Glossy");
        lay.EndRow();

    lay.AddGroup(L"Diffuse settings");
        lay.AddRow();
            lay.AddItem(L"bdiffusereflectsamples", L"Refl. sampl").PutLabelPercentage(80);
            lay.AddItem(L"bdiffusereflectdepth", L"Refl. depth").PutLabelPercentage(80);
        lay.EndRow();
        lay.AddRow();  
            lay.AddItem(L"bdiffuserefractsamples", L"Refr. sampl").PutLabelPercentage(80);
            lay.AddItem(L"bdiffuserefractdepth", L"Refr. depth").PutLabelPercentage(80);
        lay.EndRow();
    lay.EndGroup();
    
    lay.AddGroup(L"Glossy settings");
        lay.AddRow();
            lay.AddItem(L"bglossyreflectsamples", L"Refl. sampl").PutLabelPercentage(80);
            lay.AddItem(L"bglossyreflectdepth", L"Refl. depth").PutLabelPercentage(80);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem(L"bglossyrefractsamples", L"Refr. sampl").PutLabelPercentage(80);
            lay.AddItem(L"bglossyrefractdepth", L"Refr. depth").PutLabelPercentage(80);
        lay.EndRow();
    lay.EndGroup();

    lay.AddGroup(L"Specular settings");
        lay.AddRow();            
            lay.AddItem(L"bspecularreflectdepth", L"Refl. sampl").PutLabelPercentage(80);
            lay.AddItem(L"bspecularrefractdepth", L"Refr. depth").PutLabelPercentage(80);
        lay.EndRow();
    lay.EndGroup();

    lay.AddGroup(L"Rejection settings");
        lay.AddRow();
            lay.AddItem( L"bdiff_reflect_reject", L"Diffuse reflect reject").PutLabelPercentage(70); 
            lay.AddItem( L"bdiff_reflect_reject_thr", L"").PutAttribute(siUINoLabel, true);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem( L"bdiff_refract_reject", L"Diffuse refract reject").PutLabelPercentage(70);
            lay.AddItem( L"bdiff_refract_reject_thr", L"").PutAttribute(siUINoLabel, true);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem( L"bglossy_reflect_reject", L"Glossy reflect reject").PutLabelPercentage(70);
            lay.AddItem( L"bglossy_reflect_reject_thr", L"").PutAttribute(siUINoLabel, true);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem( L"bglossy_refract_reject", L"Glossy refract reject").PutLabelPercentage(70);
            lay.AddItem( L"bglossy_refract_reject_thr", L"").PutAttribute(siUINoLabel, true);
        lay.EndRow();
    lay.EndGroup();

            // Instant Global Illumination
            lay.AddItem(L"bnsets", L"Number of sets").PutLabelPercentage(70);
            lay.AddItem(L"bnlights",L"Number of lights").PutLabelPercentage(70);
            lay.AddItem(L"bmindist", L"Min. Distance").PutLabelPercentage(70);  
            
            //-- exphotonmap
            lay.AddItem( L"bstrategy", L"");
            //lay.AddItem( L"bshadowraycount", L"Shadow Ray Count").PutLabelPercentage(70);
            lay.AddItem( L"bmaxeyedepth",       L"Max. eye depth").PutLabelPercentage(70);
            lay.AddItem( L"bmaxphotondepth",    L"Max. photon depth").PutLabelPercentage(70);
            lay.AddItem( L"bdirectphotons",     L"Direct photons").PutLabelPercentage(70);
            lay.AddItem( L"bcausticphotons",    L"Caustic photons").PutLabelPercentage(70);
            lay.AddItem( L"bindirectphotons",   L"Indirect photons").PutLabelPercentage(70);
            lay.AddItem( L"bradiancephotons",   L"Radiance photons").PutLabelPercentage(70);
            lay.AddItem( L"bnphotonsused",      L"Num. photons used").PutLabelPercentage(70);
            lay.AddItem( L"bmaxphotondist",     L"Max. photons dist").PutLabelPercentage(70);
            lay.AddItem( L"bfinalgather",       L"Final Gahter");
            lay.AddItem( L"bfinalgathersamples",L"F. Gather samples").PutLabelPercentage(70);
            lay.AddItem( L"bgatherangle",       L"Gather angle").PutLabelPercentage(70);
            //--
            CValueArray vItRend(4);
                vItRend[0] = L"path" ;           vItRend[1] = 0;
                vItRend[2] = L"directlighting" ; vItRend[3] = 1;
            lay.AddEnumControl( L"brenderingmode", vItRend, L"Rendering mode").PutLabelPercentage(70);
            //--
            CValueArray vItRRSt(6);
                vItRRSt[0] = L"none" ;        vItRRSt[1] = 0;
                vItRRSt[2] = L"probability" ; vItRRSt[3] = 1;
                vItRRSt[4] = L"efficiency" ;  vItRRSt[5] = 2;
            lay.AddEnumControl( L"brrstrategy", vItRRSt, L"RR Strategy",siControlCombo).PutLabelPercentage(70);
            lay.AddItem( L"brrcon_prob", "RR cont. probability").PutLabelPercentage(70);
            lay.AddItem( L"bdistancethreshold", "Distance threshold").PutLabelPercentage(70);
            
            //-- by path mode, place there for correct show in menu
            lay.AddItem( L"binc_env", L"Include Environment").PutWidthPercentage(30);
            //-- end
            lay.AddItem(L"bphotonmaps",L"Photonmaps Files",siControlFilePath);
            PPGItem phf = lay.GetItem( L"bphotonmaps" );
            phf.PutAttribute( siUIFileFilter, L"Photonmap files|*.map" ) ;
            //--
            lay.AddGroup(L"Enable Debug").PutLabelPercentage(60);
                lay.AddRow();
                lay.AddItem( L"bdbg_enabledirect", L"Direct");
                lay.AddItem( L"bdbg_enableradiancemap", L"Radiance map");
                lay.EndRow();
                lay.AddRow();
                lay.AddItem( L"bdbg_enableindircaustic", L"Indirect caustics");
                lay.AddItem( L"bdbg_enableindirdiffuse", L"Indirect diffuse");
                lay.EndRow();
                lay.AddItem( L"bdbg_enableindirspecular", L"Enable indirect specular");
            lay.EndGroup();
        //-- temporary SPPM menu
        
        lay.AddRow();            
            lay.AddItem(L"bmaxeyedepht", L"Max Eye D.").PutLabelPercentage(80);
            lay.AddItem(L"bmaxphoton", L"Max Photon D.").PutLabelPercentage(80);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem( L"bpointxpass", L"Hit point x pass").PutLabelPercentage(80);
            lay.AddItem( L"bphotonsxpass", L"Photons x pass").PutLabelPercentage(80);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem( L"bstartradius", L"Start radius").PutLabelPercentage(80);
            lay.AddItem( L"balpha", L"Alpha").PutLabelPercentage(80);
        lay.EndRow();
        lay.AddRow();
            lay.AddItem( L"bdlsampling", L"Direct Light Sampling").PutLabelPercentage(80);
            lay.AddItem( L"bincenvironment", L"Include env.").PutLabelPercentage(80);
        lay.EndRow();
        // advanced mode
            lay.AddItem( L"bstoreglossy", L""); // bool		
            lay.AddItem( L"buseproba", L""); // bool
            //integer(
            lay.AddItem( L"bwavelengthstratificationpasses", L"");
            //string(
            lay.AddItem( L"blookupaccel", L"");
            //float(
            lay.AddItem( L"bparallelhashgridspare", L"");
            //string(
            lay.AddItem( L"bpixelsampler", L"");
            //string(
            lay.AddItem( L"bphotonsampler", L"");      
        
        lay.EndGroup(); //-- no label
        /********************************/
    lay.EndGroup(); //-- surface
    //----------------------->
    lay.AddGroup(L"Sampler");
        lay.AddRow();
            CValueArray vItem3(8);
                vItem3[0] = L"metropolis";     vItem3[1] = 0;
                vItem3[2] = L"erpt";           vItem3[3] = 1;
                vItem3[4] = L"lowdiscrepancy"; vItem3[5] = 2;
                vItem3[6] = L"random";         vItem3[7] = 3;
            lay.AddEnumControl( L"bsampler", vItem3, L"Sampler", siControlCombo );
            lay.AddItem(L"bexpert", L"Advanced").PutWidthPercentage(6);
        lay.EndRow();
            lay.AddItem( L"bchainlength", L"Chainlength").PutLabelPercentage(60);
            CValueArray vItbase(4);
                vItbase[0] = L"lowdiscrepancy"; vItbase[1] = 0;
                vItbase[2] = L"random";         vItbase[3] = 1;
            lay.AddEnumControl( L"bbasampler", vItbase, L"Base sampler", siControlCombo ).PutLabelPercentage(60);
        //--
            CValueArray vItem5(10);
                vItem5[0] = L"linear";         vItem5[1] = 0;
                vItem5[2] = L"vegas";          vItem5[3] = 1;
                vItem5[4] = L"lowdiscrepancy"; vItem5[5] = 2;
                vItem5[6] = L"tile";           vItem5[7] = 3;
                vItem5[8] = L"hilbert";        vItem5[9] = 4;
            lay.AddEnumControl( L"bpixsampler", vItem5, L"Pixelsampler", siControlCombo ) ;
            lay.AddItem( L"pixelsamples", L"Pixelsamples");
        //--
            lay.AddItem( L"bmutation", L"Large Mut. prob.").PutLabelPercentage(60);
            lay.AddItem( L"bmaxrej", L"Max cons. rejects").PutLabelPercentage(60);
            lay.AddItem( L"buservarian", L"User Variance");
        //--
    lay.EndGroup();
    //--------------]
    //--------- integrator ----------->
    lay.AddGroup(L"Volume integrator");
        lay.AddRow();
            CValueArray vItvolume(6);
                vItvolume[0] = L"Multi";    vItvolume[1] = 0;
                vItvolume[2] = L"Single";   vItvolume[3] = 1;
                vItvolume[4] = L"Emission"; vItvolume[5] = 2;
            lay.AddEnumControl(L"bvolumeint",vItvolume,L"Volume Integrator",siControlCombo );
            lay.AddItem(L"bvolexpert", L"Advanced").PutWidthPercentage(6);
        lay.EndRow();
    lay.EndGroup();
    //-------------]
    //---- filter --------->
    lay.AddGroup(L"Filter");
        lay.AddRow();
            CValueArray vItfilter(10);
                vItfilter[0] = L"box";      vItfilter[1] = 0;
                vItfilter[2] = L"gaussian"; vItfilter[3] = 1;
                vItfilter[4] = L"mitchell"; vItfilter[5] = 2;
                vItfilter[6] = L"sinc";     vItfilter[7] = 3;
                vItfilter[8] = L"triangle"; vItfilter[9] = 4;
            lay.AddEnumControl(L"bfilter",vItfilter,L"Filter",siControlCombo ) ;
            lay.AddItem(L"bfexpert", L"Advanced").PutWidthPercentage(6);
        lay.EndRow();
        lay.AddRow(); // box
            lay.AddItem(L"bxwidth", L"X Width").PutLabelPercentage(40);
            lay.AddItem(L"bywidth", L"Y Width").PutLabelPercentage(40);
        lay.EndRow();// gauss
            lay.AddItem(L"bfalpha", L"Alpha");
        lay.AddRow(); // mitchell
            lay.AddItem(L"bF_B", L"B");
            lay.AddItem(L"bF_C", L"C");
        lay.EndRow();
            lay.AddItem(L"ssample", L"Supersample");
            lay.AddItem(L"bTau", L"Tau"); 
    lay.EndGroup(); 
    //-------------]
    //---- Accelerator --------->
    lay.AddGroup(L"Accelerator");
        lay.AddRow();
            CValueArray vItAccel(6);
                vItAccel[0] = L"QBVH";    vItAccel[1] = 0;
                vItAccel[2] = L"BVH";     vItAccel[3] = 1;
                vItAccel[4] = L"KD Tree"; vItAccel[5] = 2;
            lay.AddEnumControl( L"bAccel", vItAccel, L"Accelerator",siControlCombo);
            lay.AddItem(L"bacexpert", L"Advanced").PutWidthPercentage(6);
        lay.EndRow();
            //-- qbvh
            lay.AddItem(L"bmaxprimsperleaf", L"Max. prims leaft").PutLabelPercentage(60);
            lay.AddItem(L"bfullsweepthreshold", L"Full sweep threshold").PutLabelPercentage(60);
            lay.AddItem(L"bskipfactor", L"Skip factor").PutLabelPercentage(60); 
        //-- bvh /-- inter, traver, costsamp
//            lay.AddItem(L"btreetype", L""); // combo ?
            lay.AddItem(L"bcostsamples", L"Costsample"); 
        //-- KD Tree
            lay.AddItem(L"bintersectcost", L"Intersec Cost").PutLabelPercentage(60); 
            lay.AddItem(L"btraversalcost", L"Traversal Cost").PutLabelPercentage(60); 
        lay.AddRow();
            lay.AddItem(L"bmaxprims", L"Max. Prims"); 
            lay.AddItem(L"bacmaxdepth", L"Max. Depth");
        lay.EndRow();
            lay.AddItem(L"bemptybonus", L"Empty"); 
    lay.EndGroup(); 
    //-------------]
       /*    
		CValueArray vItTree(6);
            vItTree[0] = L"Binary" ; vItTree[1] = 0;
            vItTree[2] = L"Quad" ;   vItTree[3] = 1;
            vItTree[4] = L"Oct" ;    vItTree[5] = 2;
		lay.AddEnumControl( L"btreetype", vItTree, L"Tree type",siControlCombo);
      */

    //--
    lay.AddRow();
        lay.AddButton( L"export_luxsi", L"Export to file");
        lay.AddButton( L"render_luxsi", L"Render scene");
    lay.EndRow();
    
    //------------------------------/
    lay.AddTab(L"Material preview"); 
    //------------------------------/
        //------------>
		lay.AddGroup();
            lay.AddSpacer(5,2);
            lay.AddRow();
                lay.AddItem(L"bmatPreview", L"Material for preview").PutLabelPercentage(60);
                //lay.AddItem(L"", L"");
            lay.EndRow();
            lay.AddRow();
                lay.AddButton(L"blpreview", L"Render Preview");
                lay.AddButton(L"bExportPrev", L"Export Mat. definition");
            lay.EndRow();
            //--
            lay.AddItem(L"luxMatExport",L"Export to file", siControlFilePath);
            PPGItem exportMat = lay.GetItem( L"luxMatExport" );
            exportMat.PutAttribute( siUIFileFilter, L"LuxRender Material file|*.lxm" );
            exportMat.PutLabelPercentage(60);
            //--
        lay.EndGroup();
        //-------------]
        lay.AddSpacer(5,2);
        //--------------------------------------------->
        lay.AddGroup(L"Load LuxRender .LXS scene file");
            lay.AddSpacer(5,2);
			lay.AddItem(L"blxs_file", L"Find file..",siControlFilePath);
            PPGItem sel_item = lay.GetItem(L"blxs_file");
			sel_item.PutAttribute( siUIOpenFile, 1 ) ;
			sel_item.PutAttribute( siUIFileMustExist, 1 ) ;
			sel_item.PutAttribute( siControlFilePath , "test" ) ;
            sel_item.PutAttribute( siUIFileFilter, L"LuxRender Scene files|*.lxs" ) ;
            //---------------------------------------------------
            lay.AddButton(L"bre_render", L"Render loaded file");
            //---------------------------------------------------
            CValueArray prevItem(12);
		        prevItem[0] = L" TIFF"; prevItem[1] = 0;
			    prevItem[2] = L" TGA";  prevItem[3] = 1;
			    prevItem[4] = L" PNG";  prevItem[5] = 2;
			    prevItem[6] = L" JPG";  prevItem[7] = 3;
			    prevItem[8] = L" HDR";  prevItem[9] = 4;
			    prevItem[10] = L" EXR"; prevItem[11] = 5;
			lay.AddEnumControl(L"bsavefile", prevItem, L"Image Format", siControlCombo).PutLabelPercentage(60);
        lay.EndGroup();
        //-------------]
    
    //-------------------/
    lay.AddTab(L"About");
    //-------------------/
    lay.AddGroup();
        lay.AddStaticText(L" LuXSI; LuxRender Exporter for Autodesk Softimage.");
        lay.AddStaticText(L" Version v1.1 (Windows 32 and 64 bits).");
        lay.AddGroup();
        lay.AddStaticText(L" Copyright 2007 - 2012 of all Authors:");
            lay.AddStaticText(L" Michael Gangolf  \"miga\"\n miga@migaweb.de");
            lay.AddStaticText(L" Pedro Alcaide  \"povmaniaco\"\n p.alcaide@hotmail.com");
        lay.EndGroup();
        lay.AddGroup();
            lay.AddStaticText(L" Collaborators:");
            lay.AddStaticText(L" \"sshadows\"\n Leslaw Cymer \"l_cymer\"");
        lay.EndGroup();
        //-
        lay.AddStaticText(L" LuxRender is a GPL physically based\n and unbiased rendering engine.");
        lay.AddStaticText(L" http://www.luxrender.net");
    lay.EndGroup();
    //-------------]
    

    return CStatus::OK;
}
