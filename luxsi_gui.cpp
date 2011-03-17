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
#include "luxsi_main.h"

using namespace std;
using namespace XSI;
//using namespace MATH;


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
		lay.AddRow();
			lay.AddItem(L"disint",L"Display Int.");
			lay.AddItem(L"savint",L"Save Int.");
		lay.EndRow();
		lay.AddRow();
			lay.AddItem(L"hSpp",L"Halt at S/px");
			lay.AddItem(L"hTime",L"Halt at time");
		lay.EndRow();
			lay.AddItem(L"resume", L"Resume render");
	lay.EndGroup();

	lay.AddGroup(L"Export hidden...");
		lay.AddItem(L"use_hidden_obj", L"Objects");
		lay.AddItem(L"use_hidden_cam", L"Cameras");
		lay.AddItem(L"use_hidden_light", L"Lights");
	lay.EndGroup();
	lay.AddGroup(L"Subdivided mesh options"); // TODO:  to shader spdl file, per individuals flags?
		lay.AddItem(L"smooth_mesh", L"Export smooth meshes");
		lay.AddItem(L"sharp_bound", L"Preserve sharp edges");
	lay.EndGroup();

	lay.AddGroup(L"Settings output"); //[----
	lay.AddRow(); // new form;row with two groups
		lay.AddGroup(); //-- tga
			lay.AddItem(L"save_tga", L"TGA");
			CValueArray vItrtga(6);
				vItrtga[0] = L"Y" ;	vItrtga[1] = 0;
				vItrtga[2] = L"RGB" ;	vItrtga[3] = 1;
				vItrtga[4] = L"RGBA" ;	vItrtga[5] = 2;
			lay.AddEnumControl(L"mode_rtga",vItrtga,L"Mode",siControlCombo ) ;
			lay.AddItem(L"tga_gamut", L"Gamut Clamp");
		lay.EndGroup(); //-----]
		lay.AddGroup();	//-- Exr
			lay.AddItem(L"save_exr", L"EXR");
			CValueArray vItZb_nor(6);
				vItZb_nor[0] = L"Camera Start/End clip" ;	vItZb_nor[1] = 0;
				vItZb_nor[2] = L"Min/Max" ;			vItZb_nor[3] = 1;
				vItZb_nor[4] = L"None" ;			vItZb_nor[5] = 2;
			lay.AddEnumControl(L"mode_Znorm",vItZb_nor,L"Mode Zbuffer",siControlCombo ) ;
		lay.EndGroup(); //-----]
	lay.EndRow();//--------------------
	lay.AddGroup(); //-- png
		lay.AddRow(); //----
			lay.AddItem(L"save_png_16", L"16 Bits");
			CValueArray vItrpng(8);
				vItrpng[0] = L"Y" ;	vItrpng[1] = 0;
				vItrpng[2] = L"YA" ;	vItrpng[3] = 1;
				vItrpng[4] = L"RGB" ;	vItrpng[5] = 2;
				vItrpng[6] = L"RGBA" ;	vItrpng[7] = 3;
			lay.AddEnumControl(L"mode_rpng",vItrpng,L"Mode",siControlCombo ) ;
		lay.EndRow(); //----
		lay.AddRow(); //----
			lay.AddItem(L"png_gamut", L"Gamut Clamp");
			lay.AddItem(L"save_png", L"PNG");
		lay.EndRow(); //----
	lay.EndGroup(); //-----]
		lay.AddItem(L"fObjects",L"Filename",siControlFilePath);
		PPGItem it = lay.GetItem( L"fObjects" );
		it.PutAttribute( siUIFileFilter, L"LuXSI Scenes|*.lxs" ) ;
	lay.EndGroup(); //-- save image 
		lay.AddRow();
			lay.AddButton(L"exe_luxsi",L"Export to file");
			lay.AddButton(L"render_luxsi",L"Render scene");
		lay.EndRow();
	//------------------------------//
		lay.AddTab(L"Render ");		
	//------------------------------//
	CValueArray aPresets(16);
		aPresets[0] = L"custom" ;       aPresets[1] = 0;
		aPresets[2] = L"Preview - Direct Lighting" ; aPresets[3] = 1;
		aPresets[4] = L"Final - MLT/Bidir Path Tracing (interior) (recommended)" ; aPresets[5] = 2;
		aPresets[6] = L"Final - MLT/Path Tracing (exterior)" ; aPresets[7] = 3;
		aPresets[8] = L"Progressive - Bidir Path Tracing (interior)" ; aPresets[9] = 4;
		aPresets[10] = L"Progressive - Path Tracing (exterior)" ; aPresets[11] = 5;
		aPresets[12] = L"Bucket - Bidir Path Tracing (interior)" ; aPresets[13] = 6;
		aPresets[14] = L"Bucket - Path Tracing (exterior)" ; aPresets[15] = 7;
    lay.AddEnumControl(L"presets",aPresets,L"R. Presets",siControlCombo ) ;

//	lay.AddItem(L"AmbBack",L"Use Ambience as background");
	
//-- Render mode, internal:console / external:gui	
	lay.AddGroup(L"LuxRender Engine");
		lay.AddRow();
		CValueArray Acombo(4);
			Acombo[0] = L"GUI";	 Acombo[1] = 0;
			Acombo[2] = L"Console";	 Acombo[3] = 1;
		lay.AddEnumControl( L"brmode", Acombo, L"Render", siControlCombo ) ;

		CValueArray vAengine(4);
			vAengine[0] = L"Classic" ;	vAengine[1] = 0;
			vAengine[2] = L"Hybrid" ;	vAengine[3] = 1;
		lay.AddEnumControl( L"bengine", vAengine, L"Engine", siControlCombo ) ;
		lay.EndRow();
		//--
		lay.AddItem(L"fLuxPath",L"Path to Luxrender",siControlFilePath);
		PPGItem	lpath = lay.GetItem( L"fLuxPath" );
			lpath.PutAttribute( siUIOpenFile, 1 ) ;
			lpath.PutAttribute( siUIFileMustExist, 1 ) ;
			lpath.PutAttribute( siControlFilePath , "test" ) ;
		//--

		lay.AddRow();
			lay.AddItem( L"bthreads", L"Threads" );
			lay.AddItem( L"bautode", L"Auto-Threads" );
		lay.EndRow();
	lay.EndGroup();
//-- 
	lay.AddGroup(L"Sampler"); //***
		lay.AddRow();
			CValueArray vItem3(8);
				vItem3[0] = L"metropolis" ;     vItem3[1] = 0;
				vItem3[2] = L"erpt" ;           vItem3[3] = 1;
				vItem3[4] = L"lowdiscrepancy" ; vItem3[5] = 2;
				vItem3[6] = L"random" ;		vItem3[7] = 3;
			lay.AddEnumControl( L"bsampler", vItem3, L"Sampler", siControlCombo ) ;
			lay.AddItem(L"bexpert", L"Adv.").PutWidthPercentage(6);
		lay.EndRow();
			lay.AddItem( L"bchainlength", L"Chainlength");
			CValueArray vItbase(6);
				vItbase[0] = L"metropolis" ;	 vItbase[1] = 0;
				vItbase[2] = L"lowdiscrepancy" ; vItbase[3] = 1;
				vItbase[4] = L"random" ;	 vItbase[5] = 2;
			lay.AddEnumControl( L"bbasampler", vItbase, L"Base sampler", siControlCombo ) ;
		//--
			CValueArray vItem5(12);
				vItem5[0] = L"linear" ;			vItem5[1] = 0;
				vItem5[2] = L"vegas" ;			vItem5[3] = 1;
				vItem5[4] = L"random" ;			vItem5[5] = 2;
				vItem5[6] = L"lowdiscrepancy" ;         vItem5[7] = 3;
				vItem5[8] = L"tile" ;			vItem5[9] = 4;
				vItem5[10] = L"hilbert" ;		vItem5[11] = 5;
			lay.AddEnumControl( L"bpixsampler", vItem5, L"Pixelsampler", siControlCombo ) ;
			lay.AddItem( L"pixelsamples", L"Pixelsamples");
		//--
			lay.AddItem( L"bmutation", L"Large Mut. prob.");
			lay.AddItem( L"bmaxrej", L"Max rejects"); 
			lay.AddItem( L"buservarian", L"User Variance");
		//--
	lay.EndGroup(); //-- end render group
	//--
	lay.AddGroup(L"Surface Integrator");
		lay.AddRow();
			CValueArray vItem6(12);
				vItem6[0]  = L"bidirectional";	 vItem6[1] = 0;
				vItem6[2]  = L"path";		 vItem6[3] = 1;
				vItem6[4]  = L"directlighting";	 vItem6[5] = 2;
				vItem6[6]  = L"distributedpath"; vItem6[7] = 3;
				vItem6[8]  = L"igi";		 vItem6[9] = 4;
				vItem6[10] = L"exphotonmap";	 vItem6[11] = 5;
			lay.AddEnumControl( L"bsurfaceint", vItem6, L"Integrator", siControlCombo ) ;
            lay.AddItem(L"bsexpert", L"Adv.").PutWidthPercentage(6);
		lay.EndRow();
		lay.AddGroup();	
			CValueArray vAlist(14); //-- light strategy
				vAlist[0]  = L"one";		vAlist[1] = 0;
				vAlist[2]  = L"all";		vAlist[3] = 1;
				vAlist[4]  = L"auto";		vAlist[5] = 2;
				vAlist[6]  = L"importance";	vAlist[7] = 3;
				vAlist[8]  = L"powerimp";	vAlist[9] = 4;
				vAlist[10] = L"allpowerimp";	vAlist[11] = 5;
				vAlist[12] = L"logpowerimp";	vAlist[13] = 6;
			lay.AddEnumControl( L"blight_str", vAlist, L"Light strategy",siControlCombo).PutLabelPercentage(70);//-- combo
			            
	//-- path; max_depth, light_str, RR_strategy,
	//	lay.AddItem( L"shadow_rc", L"Shadow ray count"); // shadow ray count
        	        
                 lay.AddItem( L"binc_env", L"Include Env.").PutWidthPercentage(30); 
    //-- bidireccional
				lay.AddItem( L"beye_depth",   L"Eye Depth").PutLabelPercentage(70);
				lay.AddItem( L"blight_depth", L"Light Depth").PutLabelPercentage(70); 
				lay.AddItem( L"beyerrthre",   L"Eye RR Thresh.").PutLabelPercentage(70);
				lay.AddItem( L"blightrrthre", L"Light RR Thresh.").PutLabelPercentage(70);
	
	//	//-- exphotonmap
		 	lay.AddItem( L"bstrategy", L""); 
			lay.AddItem( L"bshadowraycount", L"");
			lay.AddItem( L"bmaxdepth", L"Max. depth").PutLabelPercentage(70);
			lay.AddItem( L"bmaxphotondepth", L"Max. photon depth").PutLabelPercentage(70);
			lay.AddItem( L"bdirectphotons",  L"Direct photons").PutLabelPercentage(70);
			lay.AddItem( L"bcausticphotons", L"Caustic photons").PutLabelPercentage(70);
                        lay.AddItem( L"bindirectphotons",L"Indirect photons").PutLabelPercentage(70);
			lay.AddItem( L"bradiancephotons",L"Radiance photons").PutLabelPercentage(70);
                        lay.AddItem( L"bnphotonsused",   L"Num. photons used").PutLabelPercentage(70);
                        lay.AddItem( L"bmaxphotondist",  L"Max. photons dist").PutLabelPercentage(70);
			lay.AddItem( L"bfinalgather", L"Final Gahter");
            //	lay.AddItem( L"brrstrategy", L"");
                        lay.AddItem( L"bfinalgathersamples", L"F. gather samples").PutLabelPercentage(70);
			lay.AddItem( L"bgatherangle", L"Gather angle").PutLabelPercentage(70);
            //--
            CValueArray vItRend(4);
		        vItRend[0] = L"path" ;			 vItRend[1] = 0;
		        vItRend[2] = L"directlighting" ; vItRend[3] = 1;
            lay.AddEnumControl( L"brenderingmode", vItRend, L"Rendering mode").PutLabelPercentage(70);
            //-- probas
            CValueArray vItRRSt(6);
		        vItRRSt[0] = L"none" ;		vItRRSt[1] = 0;
                    vItRRSt[2] = L"probability" ;	vItRRSt[3] = 1;
		        vItRRSt[4] = L"efficiency" ;	vItRRSt[5] = 2;
		    lay.AddEnumControl( L"brrstrategy", vItRRSt, L"RR Strategy",siControlCombo);
                lay.AddItem( L"brrcon_prob", "RR cont. probability");
            
            //-- probas

			lay.AddItem( L"bdbg_enabledirect", L"Debug: Enable direct");
			lay.AddItem( L"bdbg_enableradiancemap", L"Debug: Enable radiance map");
			lay.AddItem( L"bdbg_enableindircaustic", L"Debug: Enable indirect caustics");
			lay.AddItem( L"bdbg_enableindirdiffuse", L"Debug: Enable indirect diffuse");
			lay.AddItem( L"bdbg_enableindirspecular", L"Debug: Enable indirect specular");
			
	        // igi
			lay.AddItem(L"bnsets", L"");// 4]\n";
			lay.AddItem(L"bnlights",L""); // 64]\n";
			lay.AddItem(L"bstrategy",L""); // L0.100000]\n";
	     // distributedpath
			lay.AddItem(L"bdirectsampleall", L"");//\"" << MtBool[1] << "\"]\n";
			lay.AddItem(L"bdirectsamples", L"");//1]\n";
			lay.AddItem(L"bindirectsampleall", L"");//\"" << MtBool[1] << "\"]\n";
 			lay.AddItem(L"bindirectsamples", L"");//1]\n";
 	     	        lay.AddItem(L"bdiffusereflectdepth", L"");//3]\n";
			lay.AddItem(L"bdiffusereflectsamples", L"");//1]\n";
			lay.AddItem(L"bdiffuserefractdepth", L"");//5]\n";
			lay.AddItem(L"bdiffuserefractsamples", L"");//1]\n";
			lay.AddItem(L"bdirectdiffuse", L"");//\"" << MtBool[1] << "\"]\n";
			lay.AddItem(L"bindirectdiffuse", L"");//\"" << MtBool[1] << "\"]\n";
			lay.AddItem(L"bglossyreflectdepth", L"");//2]\n";
			lay.AddItem(L"bglossyreflectsamples", L"");//1]\n";
			lay.AddItem(L"bglossyrefractdepth", L"");//5]\n";
			lay.AddItem(L"bglossyrefractsamples", L"");//1]\n";
			lay.AddItem(L"bdirectglossy", L"");//\"" << MtBool[1] << "\"]\n";
			lay.AddItem(L"bindirectglossy", L"");//\"" << MtBool[1] << "\"]\n";
			lay.AddItem(L"bspecularreflectdepth", L"");//3]\n";
			lay.AddItem(L"bspecularrefractdepth", L"");//5]\n";
					
		lay.EndGroup();
	lay.EndGroup();

	//---------
	//-- filter
	lay.AddGroup(L"Filter"); //***
		CValueArray vItfilter(10);
			vItfilter[0] = L"box" ;		vItfilter[1] = 0;
			vItfilter[2] = L"gaussian" ;	vItfilter[3] = 1;
			vItfilter[4] = L"mitchell" ;	vItfilter[5] = 2;
			vItfilter[6] = L"sinc" ;	vItfilter[7] = 3;
			vItfilter[8] = L"triangle" ;	vItfilter[9] = 4;
		lay.AddEnumControl(L"Filt",vItfilter,L"Filter",siControlCombo ) ;
	//-- only for adv. mode. / if conflict by names, add prefix "b": "bx_width"..
		lay.AddRow(); // box
			lay.AddItem(L"bxwidth", L"X Width");
			lay.AddItem(L"bywidth", L"Y Width");
		lay.EndRow();// gauss
			lay.AddItem(L"bfalpha", L"Alpha"); 
		lay.AddRow(); // mitchell
			lay.AddItem(L"bF_B", L"B");
			lay.AddItem(L"bF_C", L"C");
		lay.EndRow();
			lay.AddItem(L"ssample", L"Supersample");
			lay.AddItem(L"bTau", L"Tau"); // x sinc
	lay.EndGroup(); //---]
	
	lay.AddGroup(L"Accelerator");
		CValueArray vItAccel(8);
			vItAccel[0] = L"QBVH" ;		vItAccel[1] = 0;
			vItAccel[2] = L"BVH" ;		vItAccel[3] = 1;
			vItAccel[4] = L"Grid" ;		vItAccel[5] = 2;
			vItAccel[6] = L"KD Tree" ;	vItAccel[7] = 3;
		lay.AddEnumControl( L"bAccel", vItAccel, L"Accelerator",siControlCombo);
	lay.EndGroup(); //---]
		
		//-- qbvh
			lay.AddItem(L"bmaxprimspl", L"Max. prims per leaft"); //" [4]\n";
		lay.AddRow();
			lay.AddItem(L"bfullswthre", L"Full sweep threshold"); // [16]\n";
			lay.AddItem(L"bskipfactor", L"Skip factor"); // [1]\n";
		lay.EndRow();
			//-- bvh /-- inter, traver, costsamp
		//	lay.AddItem(L"bcostsamp", L"Costsample"); // [0]
			//-- grid
			lay.AddItem(L"brefimmediat", L"Refine Immediately"); // [\"false\"]\n";
			//-- KD Tree
		lay.AddRow();
			lay.AddItem(L"binter_cost", L"Intersec Cost"); //[80]// int
			lay.AddItem(L"btraver_cost", L"Traversal Cost"); // [1]\n";
		lay.EndRow();
		lay.AddRow();
			lay.AddItem(L"bmaxprim", L"Max. Prims"); //[1]\n";
			lay.AddItem(L"bacmaxdepth", L"Max. Depth"); // [-1]\n\n";
		lay.EndRow();
			lay.AddItem(L"bemptyb", L"Empty"); // float
		
				
	
	//--
	lay.AddRow();
		lay.AddButton( L"exe_luxsi", L"Export to file");
		lay.AddButton( L"render_luxsi", L"Render scene");
	lay.EndRow();

/*
	lay.AddTab(L"System");
		

	
	lay.AddRow();
		lay.AddButton(L"exe_luxsi",L"Export to file");
		lay.AddButton(L"render_luxsi",L"Render scene");
	lay.EndRow();

*/

	return CStatus::OK;
}
