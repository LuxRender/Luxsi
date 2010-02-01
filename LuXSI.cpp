/*
LuXSI - Softimage XSI Export plug-in for the LuxRender (http://www.luxrender.org) renderer

Copyright (C) 2008  Michael Gangolf

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
#include <xsi_segment.h>
#include <xsi_materiallibrary.h>
#include <xsi_particlecloud.h> 
#include <xsi_particlecloudprimitive.h> 
#include <xsi_application.h>
#include <xsi_pluginregistrar.h>
#include <xsi_geometryaccessor.h>
#include <xsi_menu.h> 
#include <sstream>
#include <fstream>
#include <xsi_model.h> 
#include <xsi_source.h> 
#include <xsi_triangle.h> 
#include <xsi_material.h>
#include <xsi_kinematics.h> 
#include <xsi_project.h> 
#include <xsi_camera.h>
#include <xsi_group.h>
#include <xsi_null.h> 
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <xsi_light.h> 
#include <xsi_polygonmesh.h> 
#include <xsi_texture.h> 
#include <xsi_imageclip2.h> 
#include <xsi_progressbar.h> 
#include <xsi_uitoolkit.h> 
#include <xsi_pass.h> 
#include <xsi_scene.h> 
#include <xsi_nurbssurfacemesh.h> 
#include <xsi_nurbssurface.h>
#include <xsi_controlpoint.h> 
#include <xsi_ogllight.h> 
#include <xsi_trianglevertex.h>
#include <xsi_polygonface.h>
#include <xsi_vertex.h>
#include <xsi_polygonnode.h>
#include <xsi_utils.h>
#include <math.h>
#include <string.h>
#include <xsi_scene.h>
#include <xsi_library.h>
#include <xsi_source.h>

#include <xsi_projectitem.h>
#include <xsi_siobject.h>
#include <xsi_base.h>


#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
	#include <winioctl.h>
#endif

using namespace XSI; 
using namespace MATH; 
using namespace std;

#define PI 3.14159265

void writeLuxsiBasics();
void writeLuxsiCam(X3DObject o);
void writeLuxsiLight(X3DObject o);
int writeLuxsiInstance(X3DObject o);

int writeLuxsiObj(X3DObject o);
void writeClouds(X3DObject o);
void writeLuxsiShader();
void luxsi_write();
void luxsi_execute();
CString readIni();
void update_LuXSI_values(CString   paramName, Parameter changed,PPGLayout lay);

Application app;
Model root( app.GetActiveSceneRoot() );
ofstream f;
stringstream sLight, sObj, sMat;
Null null;
CustomProperty prop ;


int vBounces=10,vPixelsamples=2,vSampler=2,vPresets=0,vDis=8,vThreads=2,vEye_depth=2,vLight_depth=2,vmaxRejects=256,vSurf=3,vXRes=800,vYRes=600,vFilt=2,vMaxDepth=256,vSamples=4,vSave=120,vPixelsampler=1;
bool vMLT=true,vIsLinux=true,vIsGI=true,vRrft=true,vProg=true,vIsCaustic=false,vIsHiddenCam=false,vIsHiddenLight=false,vIsHiddenObj=false, vExportDone=false;
bool vResume=false,vUseJitter=true,vIsHiddenSurfaces=false,vSFPreview=true,vIsHiddenClouds=false,vExpOne=true,vAmbBack=false,vExr=true,vPng=false,vTga=true;
float vCSize=0.4f,vGITolerance=0.025f,vSpacingX=0.1f,vSpacingY=0.1f,vContrast=2.2f;
float vmutation=0.1f, vrrprob=0.65f;

CRefArray aGroups;
CStringArray aMatList,aInstanceList;

UIToolkit kit = app.GetUIToolkit();
ProgressBar pb = kit.GetProgressBar();
CString vSun=L"",vHDRI=L"",vLuXSIPath=L"",vFileObjects=L"";

XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"Michael Gangolf");
	in_reg.PutName(L"LuXSI");
	in_reg.PutEmail(L"miga@migaweb.de");
	in_reg.PutURL(L"http://www.migaweb.de");
	in_reg.PutVersion(0,6);
	in_reg.RegisterProperty(L"LuXSI");
	in_reg.RegisterMenu(siMenuMainFileExportID,L"LuXSI_Menu",false,false);
	//RegistrationInsertionPoint - do not remove this line
	
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus XSIUnloadPlugin( const PluginRegistrar& in_reg )
{
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.");
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus LuXSI_Define( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	
	Parameter oParam;
	prop = ctxt.GetSource();
	
	
	prop.AddParameter(L"use_hidden_obj",CValue::siBool,siPersistable,L"",L"",vIsHiddenObj,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"use_hidden_light",CValue::siBool,siPersistable,L"",L"",vIsHiddenLight,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"use_hidden_cam",CValue::siBool,siPersistable,L"",L"",vIsHiddenCam,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"exp_one",CValue::siBool,siPersistable,L"",L"",vExpOne,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"Width",CValue::siInt4,siPersistable,L"",L"",vXRes,0l,2048l,0l,1024l,oParam);
	prop.AddParameter(L"Height",CValue::siInt4,siPersistable,L"",L"",vYRes,0l,2048l,0l,768l,oParam);
	prop.AddParameter(L"gamma",CValue::siFloat,siPersistable,L"",L"",vContrast,0,10,0,3,oParam);
	prop.AddParameter(L"max_depth",CValue::siInt4,siPersistable,L"",L"",vMaxDepth,0,4096,0,1024,oParam);
	prop.AddParameter(L"samples",CValue::siInt4,siPersistable,L"",L"",vSamples,0,1024,0,128,oParam);
	prop.AddParameter(L"progressive",CValue::siBool,siPersistable,L"",L"",vProg,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"rrft",CValue::siBool,siPersistable,L"",L"",vRrft,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter( L"Filt", CValue::siInt4, siPersistable, L"", L"",vFilt, oParam ) ;
	prop.AddParameter( L"sampler", CValue::siInt4, siPersistable, L"", L"",vSampler, oParam ) ;
	prop.AddParameter( L"presets", CValue::siInt4, siPersistable, L"", L"",vPresets, oParam ) ;
	prop.AddParameter( L"sintegrator", CValue::siInt4, siPersistable, L"", L"",vSurf, oParam ) ;
	prop.AddParameter( L"pixelsampler", CValue::siInt4, siPersistable, L"", L"",vPixelsampler, oParam ) ;
	prop.AddParameter( L"pixelsamples", CValue::siInt4, siPersistable, L"", L"",vPixelsamples, oParam ) ;
	
	prop.AddParameter( L"eye_depth", CValue::siInt4, siPersistable, L"", L"",vEye_depth, oParam ) ;
	prop.AddParameter( L"light_depth", CValue::siInt4, siPersistable, L"", L"",vLight_depth, oParam ) ;
	
	prop.AddParameter(L"savint",CValue::siInt4,siPersistable,L"",L"",vSave,0l,200l,0l,200l,oParam);
	prop.AddParameter(L"disint",CValue::siInt4,siPersistable,L"",L"",vDis,0l,200l,0l,200l,oParam);
	prop.AddParameter(L"AmbBack",CValue::siBool,siPersistable,L"",L"",vAmbBack,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"save_exr",CValue::siBool,siPersistable,L"",L"",vExr,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"save_png",CValue::siBool,siPersistable,L"",L"",vPng,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"save_tga",CValue::siBool,siPersistable,L"",L"",vTga,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"mlt",CValue::siBool,siPersistable,L"",L"",vMLT,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"resume",CValue::siBool,siPersistable,L"",L"",vResume,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"maxrej",CValue::siInt4,siPersistable,L"",L"",vmaxRejects,0l,10048l,0l,512l,oParam);
	prop.AddParameter(L"mutation",CValue::siFloat,siPersistable,L"",L"",vmutation,0l,1l,0l,1l,oParam);
	
	// set temp filename
	CString objPath;
	vFileObjects = app.GetInstallationPath(siUserPath);
	#ifdef __unix__
		vFileObjects += L"/tmp.lxs";
	#else
		vFileObjects += L"\\tmp.lxs";
	#endif
	
	vLuXSIPath=readIni(); // get luxrender path out of the ini
	
	prop.AddParameter( L"fObjects", CValue::siString, siPersistable, L"", L"", vFileObjects, oParam ) ;
	
	prop.AddParameter( L"fLuxPath", CValue::siString, siPersistable, L"", L"", vLuXSIPath, oParam ) ;
	
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus LuXSI_DefineLayout( CRef& in_ctxt )
{
	PPGLayout lay = Context(in_ctxt).GetSource() ;
        
	lay.Clear();
	
	// main tab
	
	lay.AddTab(L"Main");
	
	lay.AddGroup(L"Image");
	lay.AddRow();
	lay.AddItem(L"Width");
	lay.AddItem(L"Height");
	lay.EndRow();
	lay.AddItem(L"gamma",L"Gamma");
	lay.EndRow();
	CValueArray vItem2(10);
	vItem2[0] = L"box" ; vItem2[1] = 0;
	vItem2[2] = L"gaussian" ; vItem2[3] = 1;
	vItem2[4] = L"mitchell" ; vItem2[5] = 2;
	vItem2[6] = L"sinc" ; vItem2[7] = 3;
	vItem2[8] = L"triangle" ; vItem2[9] = 4;
	lay.AddEnumControl(L"Filt",vItem2,L"Filter",siControlCombo ) ;
	lay.AddItem(L"disint",L"Display Interval");
	lay.EndGroup();
	
	lay.AddGroup(L"Export hidden...");
	lay.AddItem(L"use_hidden_obj", L"Objects");
	lay.AddItem(L"use_hidden_cam", L"Cameras");
	lay.AddItem(L"use_hidden_light", L"Lights");
	lay.EndGroup();
	
	lay.AddGroup(L"Save as");
	lay.AddItem(L"save_tga", L"TGA");
	lay.AddItem(L"save_exr", L"EXR");
	lay.AddItem(L"save_png", L"PNG");
	lay.AddItem(L"savint",L"Interval");
	
	lay.EndGroup();
	
	lay.AddItem(L"fObjects",L"Filename",siControlFilePath);
	
	PPGItem it = lay.GetItem( L"fObjects" );
	it.PutAttribute( siUIFileFilter, L"LuXSI Scenes|*.lxs" ) ;
	
	lay.AddRow();
	lay.AddButton(L"exe_luxsi",L"Export");
	lay.AddButton(L"render_luxsi",L"Render");
	lay.EndRow();
	
	lay.AddTab(L"Render settings");
			
	CValueArray aPresets(16);
	
	aPresets[0] = L"custom" ; aPresets[1] = 0;
	aPresets[2] = L"Preview - Direct Lighting" ; aPresets[3] = 1;
	aPresets[4] = L"Final - MLT/Bidir Path Tracing (interior) (recommended)" ; aPresets[5] = 2;
	aPresets[6] = L"Final - MLT/Path Tracing (exterior)" ; aPresets[7] = 3;
	aPresets[8] = L"Progressive - Bidir Path Tracing (interior)" ; aPresets[9] = 4;
	aPresets[10] = L"Progressive - Path Tracing (exterior)" ; aPresets[11] = 5;
	aPresets[12] = L"Bucket - Bidir Path Tracing (interior)" ; aPresets[13] = 6;
	aPresets[14] = L"Bucket - Path Tracing (exterior)" ; aPresets[15] = 7;

	/*
	aPresets[16] = L"Anim - Distributed/GI low Q" ; aPresets[17] = 8;
	aPresets[18] = L"Anim - Distributed/GI medium Q" ; aPresets[19] = 9;
	aPresets[20] = L"Anim - Distributed/GI high Q" ; aPresets[21] = 10;
	aPresets[22] = L"Anim - Distributed/GI very high Q" ; aPresets[23] = 11;
	*/
	lay.AddEnumControl(L"presets",aPresets,L"Render Presets",siControlCombo ) ;
	
	
	lay.AddItem(L"AmbBack",L"Use Ambience as background");
	
	lay.AddGroup(L"Sampler");
		CValueArray vItem3(8);
		vItem3[0] = L"erpt" ; vItem3[1] = 0;
		vItem3[2] = L"lowdiscrepancy" ; vItem3[3] = 1;
		vItem3[4] = L"metropolis" ; vItem3[5] = 2;
		vItem3[6] = L"random" ; vItem3[7] = 3;
		lay.AddEnumControl(L"sampler",vItem3,L"Sampler",siControlCombo ) ;
		lay.AddItem(L"maxrej",L"Max rejects");
		lay.AddItem(L"mutation",L"Mutation");
	lay.EndGroup();
	
	lay.AddGroup(L"Integrator");
		CValueArray vItem6(10);
		vItem6[0] = L"bidirectional" ; vItem6[1] = 0;
		vItem6[2] = L"distributedpath" ; vItem6[3] = 1;
		vItem6[4] = L"emission" ; vItem6[5] = 2;
		vItem6[6] = L"path" ; vItem6[7] = 3;
		vItem6[8] = L"single" ; vItem6[9] = 4;
		lay.AddEnumControl(L"sintegrator",vItem6,L"Integrator",siControlCombo ) ;
		lay.AddItem(L"max_depth",L"Max depth");
		lay.AddItem(L"eye_depth",L"Eye depth");
		lay.AddItem(L"light_depth",L"Light depth");
	lay.EndGroup();

	lay.AddGroup(L"Pixel Sampler");
		CValueArray vItem5(12);
		vItem5[0] = L"linear" ; vItem5[1] = 0;
		vItem5[2] = L"vegas" ; vItem5[3] = 1;
		vItem5[4] = L"random" ; vItem5[5] = 2;
		vItem5[6] = L"lowdiscrepancy" ; vItem5[7] = 3;
		vItem5[8] = L"tilepx" ; vItem5[9] = 4;
		vItem5[10] = L"hilbert" ; vItem5[11] = 5;
		lay.AddEnumControl(L"pixelsampler",vItem5,L"Pixelsampler",siControlCombo ) ;
		lay.AddItem(L"pixelsamples",L"Pixelsamples");
	lay.EndGroup();
	
	lay.AddRow();
	lay.AddButton(L"exe_luxsi",L"Export");
	lay.AddButton(L"render_luxsi",L"Render");
	lay.EndRow();
	
	lay.AddTab(L"System");
	lay.AddItem(L"fLuxPath",L"Path to Luxrender",siControlFilePath);
	it = lay.GetItem( L"fLuxPath" );
	it.PutAttribute( siUIOpenFile, 1 ) ;
	it.PutAttribute( siUIFileMustExist, 1 ) ;
	it.PutAttribute( siControlFilePath , "test" ) ;
	
	lay.AddItem(L"resume", L"Resume render");
	
	
	lay.AddRow();
	lay.AddButton(L"exe_luxsi",L"Export");
	lay.AddButton(L"render_luxsi",L"Render");
	lay.EndRow();
	
	
	return CStatus::OK;
}

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
			update_LuXSI_values(param.GetScriptName(),param,lay);
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
		lay.PutAttribute(L"Refresh",true);
		CRefArray params = prop.GetParameters();;
		for (int i=0;i<params.GetCount();i++){
			// Update values on init
			Parameter param(params[i]);
			update_LuXSI_values(param.GetScriptName(),param,lay);
		}
		app.LogMessage( L"Button pressed: " + buttonPressed.GetAsText() ) ;
		
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
		
		update_LuXSI_values(paramName,changed,lay);
	}
	
	return CStatus::OK ;
}

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

CVector3 convertMatrix(CVector3 v){
	CMatrix3 m2(1.0,0.0,0.0,  0.0,0.0,1.0,   0.0,-1.0,0.0);
    return v.MulByMatrix3InPlace (m2);
}

void update_LuXSI_values(CString paramName, Parameter changed,PPGLayout lay){
	// update variables when PPG value changed
	if (paramName==L"presets") {
		switch ((int)changed.GetValue()) {
			case 1: 
				// Preview
				vSampler=1;
				vSurf=3;
				vMaxDepth=5;
				vPixelsampler=3;
				break;
			case 2: 
				// final 1
				vSampler=2;
				vmaxRejects=512;
				vmutation=0.6f;
			
				vSurf=0;
				vLight_depth=10;
				vEye_depth=10;
				vBounces=10;
				
				break;
			case 3: 
				// final 2
				vSampler=2;
				vmaxRejects=512;
				vmutation=0.6f;
			
				vSurf=3;
				vMaxDepth=10;
				vBounces=10;
				break;
			case 4: 
				// progr 1
				vSampler=1;
				vmaxRejects=512;
			
				vSurf=0;
				vLight_depth=10;
				vEye_depth=10;
				vBounces=10;

				vPixelsampler=3;
				vPixelsamples=1;
				break;
			case 5: 
				// progr 2
				vSampler=1;
				vmaxRejects=512;
			
				vSurf=3;
				vBounces=10;
				vMaxDepth=10;

				vPixelsampler=3;
				vPixelsamples=1;
				break;
			case 6: 
				// bucket 1
				vSampler=1;
				vmaxRejects=512;
			
				vSurf=0;
				vLight_depth=10;
				vEye_depth=8;
				vBounces=8;

				vPixelsampler=5;
				vPixelsamples=64;
				break;
			case 7:
				// bucket 2
				vSampler=1;
				vmaxRejects=512;
			
				vSurf=3;
				vMaxDepth=8;
				vBounces=8;

				vPixelsampler=5;
				vPixelsamples=64;
				break;
		}
		Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(vMaxDepth); 
		Parameter(prop.GetParameters().GetItem( L"sintegrator" )).PutValue(vSurf); 
		Parameter(prop.GetParameters().GetItem( L"light_depth" )).PutValue(vLight_depth); 
		Parameter(prop.GetParameters().GetItem( L"eye_depth" )).PutValue(vEye_depth); 
		Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(vSampler); 
		Parameter(prop.GetParameters().GetItem( L"maxrej" )).PutValue(vmaxRejects); 
		Parameter(prop.GetParameters().GetItem( L"pixelsampler" )).PutValue(vPixelsampler);
		Parameter(prop.GetParameters().GetItem( L"pixelsamples" )).PutValue(vPixelsamples); 
		Parameter(prop.GetParameters().GetItem( L"mutation" )).PutValue(vmutation); 
	}
	
	if (paramName==L"Width"){
		vXRes=changed.GetValue();
	} else if (paramName==L"Height"){
		vYRes=changed.GetValue();
	} else if (paramName==L"Filt"){
		vFilt=changed.GetValue();
	} else if (paramName==L"gamma"){
		vContrast=changed.GetValue();
	} else if (paramName==L"use_hidden_obj"){
		vIsHiddenObj=changed.GetValue();
	} else if (paramName==L"use_hidden_surf"){
		vIsHiddenSurfaces=changed.GetValue();
	} else if (paramName==L"use_hidden_clouds"){
		vIsHiddenClouds=changed.GetValue();
	} else if (paramName==L"use_hidden_cam"){
		vIsHiddenCam=changed.GetValue();
	} else if (paramName==L"use_hidden_light"){
		vIsHiddenLight=changed.GetValue();
	}  else if (paramName==L"fObjects"){
		vFileObjects=changed.GetValue();
	}  else if (paramName==L"max_depth"){
		vMaxDepth=changed.GetValue();
	} else if (paramName==L"progressive"){
		vProg=changed.GetValue();
	} else if (paramName==L"pixelsamples"){
		vSamples=changed.GetValue();
	}  else if (paramName==L"rrft"){
		vRrft=changed.GetValue();
	}  else if (paramName==L"savint"){
		vSave=changed.GetValue();
	} else if (paramName==L"mlt"){
		vMLT=changed.GetValue();
	}else if (paramName==L"AmbBack"){
		vAmbBack=changed.GetValue();
	} else if (paramName==L"maxrej"){
		vmaxRejects=changed.GetValue();
	} else if (paramName==L"save_png"){
		vPng=changed.GetValue();
	} else if (paramName==L"save_exr"){
		vExr=changed.GetValue();
	} else if (paramName==L"save_tga"){
		vTga=changed.GetValue();
	} else if (paramName==L"pixelsampler"){
		vPixelsampler=changed.GetValue();
	}else if (paramName==L"sintegrator"){
		vSurf=changed.GetValue();
	}else if (paramName==L"disint"){
		vDis=changed.GetValue();
	}else if (paramName==L"light_depth"){
		vLight_depth=changed.GetValue();
	}else if (paramName==L"eye_depth"){
		vEye_depth=changed.GetValue();
	}else if (paramName==L"fLuxPath"){
		vLuXSIPath=changed.GetValue();
		
	}else if (paramName==L"mutation"){
		vmutation=changed.GetValue();
	}else if (paramName==L"resume"){
		vResume=changed.GetValue();
	}
}

XSIPLUGINCALLBACK CStatus LuXSI_Menu_Init( CRef& in_ctxt )
{
	Context ctxt( in_ctxt );
	Menu oMenu;
	oMenu = ctxt.GetSource();
	MenuItem oNewItem;
	oMenu.AddCallbackItem(L"LuXSI",L"OnLuXSI_MenuClicked",oNewItem);
	return CStatus::OK;
}

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
	
	return 0;
}


void writeLuxsiBasics(){
	//
	// write basic scene description
	//
	vHDRI=L"";
	
	char aFilter[8][20]={"box","gaussian","mitchell","sinc","triangle"};
	
	
	char aBool[2][6]={"false","true"};
	char aSampler[4][15]={"erpt", "lowdiscrepancy","metropolis","random"};
	char aPxSampler[6][15]={"linear","vegas","random","lowdiscrepancy","tilepx","hilbert"};
	char aSurf[5][16]={"bidirectional","distributedpath","emission","path","single"};
	
	string fname=vFileObjects.GetAsciiString();
	

	int loc=(int)fname.rfind(".");
	
	f << "Film \"fleximage\"\n";
	f << "  \"integer xresolution\" [" <<  vXRes << "]\n  \"integer yresolution\" [" <<  vYRes<<"]\n";
	
	f << "  \"string filename\" [\""<< replace(fname.substr(0,loc)) << "\"]\n";
	f << "  \"integer writeinterval\" ["<< vSave << "]\n";
	f << "  \"integer displayinterval\" ["<< vDis << "]\n";
	
	if (vExr){
		f << "  \"bool write_exr\" [\"true\"]\n";
	} else {
		f << "  \"bool write_exr\" [\"false\"]\n";
	}
	
	if (vTga){
		f << "  \"bool write_tga\" [\"true\"]\n";
	} 
	if (vPng){
		f << "  \"bool write_png\" [\"true\"]\n";
	} else {
		f << "  \"bool write_png\" [\"false\"]\n";
	}	
	
	f << "  \"bool write_resume_flm\" [\""<< aBool[vResume] << "\"]\n";
	f << "  \"bool premultiplyalpha\" [\"false\"]\n";
	f << "  \"integer haltspp\" [0]\n";
	
	
	f << "  \"float gamma\" [" << vContrast << "]\n\n";
	
	f << "\nPixelFilter \""<< aFilter[vFilt] << "\"\n  \"float xwidth\" [2.000000]\n  \"float ywidth\" [2.000000]\n\n";

	f << "Sampler \""<< aSampler[vSampler] <<"\"\n";
		
	if (vSampler!=2) {f << "  \"string pixelsampler\" [\""<< aPxSampler[vPixelsampler] <<"\"]\n";}
		
		
		
	switch(vSampler) {
		case 0: f << "  \"integer pixelsamples\" ["<< vSamples <<"]\n";break;
		case 2: 
			f << "  \"float largemutationprob\" ["<< vmutation <<"]\n";
			f << "  \"integer maxconsecrejects\" ["<< vmaxRejects<<"]\n";		
			break;
	}
		
	f << "\nSurfaceIntegrator \"" << aSurf[vSurf] << "\"\n";
	
	switch(vSurf) {
		case 0: // bidirectional
			f << "  \"integer eyedepth\" ["<< vEye_depth << "] \n";
			f << "  \"integer lightdepth\" ["<< vLight_depth << "] \n";
			break;
		case 1: break;
		case 2: break;
		case 3: 
			// path
			f << "  \"integer maxdepth\" ["<< vMaxDepth << "] \n";
			//f << "\"float rrcontinueprob\" ["<<vrrprob << "] ";
			break;
		case 4: break;
	}
	f << "\nAccelerator \"tabreckdtree\"\n";
	f << "  \"integer intersectcost\" [80]\n";
	f << "  \"integer traversalcost\" [1]\n";
	f << "  \"float emptybonus\" [0.200000]\n";
	f << "  \"integer maxprims\" [1]\n";
	f << "  \"integer maxdepth\" [-1]\n\n";
	
	f << "WorldBegin\n\n";
		
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
	if (vHDRI!=L""){
		f << "AttributeBegin\n";
		f << "LightSource \"infinite\" \"color L\" [1 1 1] \"integer nsamples\" [1]\n";
		f << "\"string mapname\" [\"" << replace(vHDRI.GetAsciiString()) << "\"]\n";
	} else if (vAmbBack) {
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
	}	
}

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
	
	CVector3 vnegZ(0,0,1);

	// Operations to calculate look at position.
	vnegZ.MulByMatrix3InPlace(c.GetKinematics().GetGlobal().GetTransform().GetRotationMatrix3());
	vnegZ.NormalizeInPlace();
	vnegZ.ScaleInPlace((double) c.GetParameterValue(L"interestdist"));
	vnegZ.AddInPlace(c.GetKinematics().GetGlobal().GetTransform().GetTranslation());
	
	
	CTransformation localTransformation = o2.GetKinematics().GetLocal().GetTransform();
	KinematicState  gs = o2.GetKinematics().GetGlobal();
	CTransformation gt = gs.GetTransform();
	//MapObjectPoseToWorldSpace(  gt, localTransformation);
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
	
	CVector3 new_pos = convertMatrix(gt.GetTranslation());
	CVector3 new_pos_ci = convertMatrix(ci_gt.GetTranslation());
	
	f << "LookAt " << CString(new_pos.GetX()).GetAsciiString() << " " <<CString(new_pos.GetY()).GetAsciiString() << " " << CString(new_pos.GetZ()).GetAsciiString() << " " << CString(new_pos_ci.GetX()).GetAsciiString() << " " <<CString(new_pos_ci.GetY()).GetAsciiString() << " " << CString(new_pos_ci.GetZ()).GetAsciiString()  << " 0 0 " << 	CString(up.GetZ()).GetAsciiString() << "\n";
	f << "Camera \"perspective\" \"float fov\" [" << vfov << "] \"float lensradius\" ["<< CString(vLensr).GetAsciiString()  <<"] \"float focaldistance\" ["<< CString(vFdist).GetAsciiString() <<"]"; 
	f << "\n\n";
}

void writeLuxsiLight(X3DObject o){
	//
	// write light
	//

	CTransformation localTransformation = o.GetKinematics().GetLocal().GetTransform();
	KinematicState  gs = o.GetKinematics().GetGlobal();
	CTransformation gt = gs.GetTransform();
	CVector3 translation(gt.GetTranslation());
	CValue lType,lPos,lPower;
	float a=0,b=0,c=0,alpha=0;
	Shader s((Light(o).GetShaders())[0]);
	OGLLight myOGLLight(Light(o).GetOGLLight());
	s.GetColorParameterValue(L"color",a,b,c,alpha ); 
	CString lName = findInGroup(o.GetName());
	f << "LightGroup \"";
		if (lName!=L"") {
			f << lName.GetAsciiString();
		} else {
			f << (o.GetName()).GetAsciiString();
		}
	f << "\"";

	if (myOGLLight.GetType()==siLightSpot ){
		//
		// Spotlight
		//
		
		X3DObject li;
		CVector3 intPos;
		
		//Get Interest
		li= X3DObject(o.GetParent()).GetChildren()[1];
		CTransformation lt = li.GetKinematics().GetLocal().GetTransform();
		//KinematicState  gs2 = li.GetKinematics().GetGlobal();
		//CTransformation gt2 = gs2.GetTransform();
		intPos=lt.GetTranslation();
		
		CRefArray shad(Light(o).GetShaders());
		Shader s(shad[0]);
				
		f << "\nLightSource \"spot\"\n";
		f << "  \"point from\" [" << (float)translation.GetX() << " " << (float)translation.GetY() << " "  << (float)translation.GetZ()  << "] \"point to\" ["<< (float)intPos.GetX() << " " << (float)intPos.GetY() << " "<< (float)intPos.GetZ() << "]\n";
		f << "  \"float coneangle\" [" << (float)o.GetParameterValue((L"LightCone")) << "]\n";
		f << "  \"float conedeltaangle\" [" << ((float)o.GetParameterValue(L"LightCone")- (float)s.GetParameter(L"spread").GetValue() ) << "]\n";
		f << "  \"color I\" [" << a << "  " << b << "  " << c << "] \"float gain\" ["  << (float)s.GetParameterValue(L"intensity") << "]\n";
	} else if  (myOGLLight.GetType()==siLightInfinite ) {
		//
		//sunlight
		//	
		CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();
		
		f << "\nLightSource \"sunsky\"\n";
		f << "  \"integer nsamples\" [4]\n";
		f << "  \"vector sundir\" [ "<< (float)sunTransMat.GetValue(2,0) << " " << -(float)sunTransMat.GetValue(2,2) << " " << (float)sunTransMat.GetValue(2,1) << " ] \"float gain\" [" << (float)s.GetParameterValue(L"intensity") << "]\n";
		
	} else {
		//
		// Pointlight
		//
		CVector3 intPos;
		CTransformation localTransformation2 = o.GetKinematics().GetLocal().GetTransform();
		KinematicState  gs2 = o.GetKinematics().GetGlobal();
		CTransformation gt2 = gs2.GetTransform();
		intPos=gt2.GetTranslation();
		f << "\nLightSource \"point\"\n";
		f << "  \"point from\" [" << intPos.GetX() << " " << intPos.GetY() << " " << intPos.GetZ() << "] \"color I\" [" << a << "  " << b << "  " << c << "] \"float gain\" ["<< (float)s.GetParameterValue(L"intensity") << "]\n";
		
	}
}




void writeLuxsiShader(){
	//
	// Writes shader
	//

	Scene scene = app.GetActiveProject().GetActiveScene();
	Library matlib = scene.GetActiveMaterialLibrary();
	
	CRefArray materials = matlib.GetItems();
	char aBool[2][6]={"false","true"};
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
	
		float b_red=0.0f,b_green=0.0f,b_blue=0.0f,b_alpha=0.0f,red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f,sp_red=0.0f,sp_green=0.0f,sp_blue=0.0f,sp_alpha=0.0f,vModScale=0.0f,refl_red=0.0f,refl_green=0.0f,refl_blue=0.0f,refl_alpha=0.0f;
		float mRough=0.0f;
		ImageClip2 vBumpFile;
		Shader vBumpTex;
		bool vIsSet=false;
		bool vText=false,vIsSubD=true;
		CValue vDiffType,vMore,vCol,vMore2,vMore3,vTexStr,vMore1,vMore4,vMore5;
		Material m( materials[i] );
		
		if ( (int)m.GetUsedBy().GetCount()==0) {
			continue;
		}
		
		CRefArray shad(m.GetShaders());	// Array of all shaders attached to the material [e.g. phong]
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
			shaderString += L"  \"float index\" [" + CString((float)s.GetParameter(L"index").GetValue()) + L"]\n";
			shaderString += L"  \"float cauchyb\" [" + CString((float)s.GetParameter(L"cauchyb").GetValue()) + L"]\n";
			vIsSet=true;	
		} else if (vMatID==L"lux_matte") {
			shaderType=L"matte";
			s.GetColorParameterValue(L"kd",red,green,blue,alpha );
			shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
			shaderString += L"  \"float sigma\" [" + CString((float)s.GetParameter(L"sigma").GetValue()) + L"]\n";
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
			shaderString += L"  \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString((float)s.GetParameter(L"roughness").GetValue())+L"] \"float vroughness\" ["+CString((float)s.GetParameter(L"roughness").GetValue())+L"]\n";
			s.GetColorParameterValue(L"kr",red,green,blue,alpha );
			shaderString += L"  \"color Kr\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
			vIsSet=true;	
		} else if (vMatID==L"lux_substrate") {
			shaderType=L"substrate";
			s.GetColorParameterValue(L"kd",red,green,blue,alpha );
			shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString((float)s.GetParameter(L"uroughness").GetValue())+L"] \"float vroughness\" ["+CString((float)s.GetParameter(L"vroughness").GetValue())+L"]\n";
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
			if (s.GetParameter(L"refract_inuse").GetValue()=="-1") { 
			//check if material is transparent: phong/lamber/blin/constant/cooktorrance/strauss
			float ior=0.0f;
			s.GetColorParameterValue(L"transparency",red,green,blue,alpha );
			if (red>0 || green>0 || blue>0) {
				shaderType=L"glass";
				
				shaderString += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
				s.GetColorParameterValue(L"reflectivity",sp_red,sp_green,sp_blue,sp_alpha );
				shaderString += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
				shaderString += L"  \"float index\" [" + CString((float)s.GetParameter(L"index_of_refraction").GetValue()) + L"]\n";
				shaderString += L"  \"float cauchyb\" [0]\n";
							
				if ((float)s.GetParameter(L"trans_glossy").GetValue()>0 ) {
					shaderType=L"roughglass";
					shaderString += L"  \"float uroughness\" ["+ CString((float)s.GetParameter(L"trans_glossy").GetValue())+L"] \"float vroughness\" ["+CString((float)s.GetParameter(L"trans_glossy").GetValue())+L"]\n";
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
				
				if ((float)s.GetParameter(L"refr_gloss").GetValue()<1 ) {
					shaderType=L"roughglass";
					shaderString += L"  \"float uroughness\" ["+ CString(1.0f-(float)s.GetParameter(L"refr_gloss").GetValue())+L"] \"float vroughness\" ["+CString(1.0f-(float)s.GetParameter(L"refr_gloss").GetValue())+L"]\n";
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
					shaderString += L"  \"color Kr\" [" + CString(a) + L" " + CString(b) + L" " + CString(c) + L"] \"float uroughness\" ["+ CString(mRough/10)+L"] \"float vroughness\" ["+CString(mRough/10)+L"]\n";
					shaderString += L"  \"color Ks\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
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
				mRough=(float)s.GetParameter(L"reflect_glossy").GetValue();
				float refl=(float)s.GetParameter(L"reflectivity").GetValue();
				float brdf=(float)s.GetParameter(L"brdf_0_degree_refl").GetValue();
				
					
				if (refl>0) {
					shaderType=L"shinymetal";
					shaderString += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString(1-mRough)+L"] \"float vroughness\" ["+CString(1-mRough)+L"]\n";
					shaderString += L"  \"color Ks\" [" + CString(refl*sp_red*brdf) + L" "  + CString(refl*sp_green*brdf) +  L" "  + CString(refl*sp_blue*brdf) + L"]\n";
				} else {
					shaderType=L"matte";
					s.GetColorParameterValue(L"kd",red,green,blue,alpha );
					shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
					shaderString += L"  \"float sigma\" [" + CString(mRough) + L"]\n";
				}
			} else if (vMatID==L"material-phong") {
				shaderType=L"glossy";
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny"))/10)+L"] \"float vroughness\" ["+CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]\n";
				s.GetColorParameterValue(L"specular",red,green,blue,alpha );
				shaderString += L"  \"color Ks\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
			}  else if (vMatID==L"material-lambert"){
				shaderType=L"matte";
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
				shaderString += L"  \"float sigma\" [0]\n";
			} else if (vMatID==L"material-ward"){
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				shaderType=L"glossy";
				shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"] \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_u"))/10)+L"] \"float vroughness\" ["+CString((float)(s.GetParameterValue(L"shiny_v"))/10)+L"]\n";
				shaderString += L"  \"color Ks\" [" + CString(red) + L" "  + CString(green) +  L" "  + CString(blue) + L"]\n";
			} else if (vMatID==L"material-constant"){
				s.GetColorParameterValue(L"color",red,green,blue,alpha );
				shaderType=L"matte";
				shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
				shaderString += L"  \"float sigma\" [0]\n";
			} else if (vMatID==L"material-strauss"){
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				shaderString += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
				shaderString += L"  \"float sigma\" [0]\n";
			} else {
				// fall back shader
				shaderType=L"matte";
				shaderString += L"  \"color Kd\" [0.7 0.7 0.7]\n";
				shaderString += L"  \"float sigma\" [0]\n";
			}
		}
		
		
		CRefArray vImags=m.GetShaders();
		for (int i=0;i<vImags.GetCount();i++){
			CRefArray vImags2=Shader(vImags[i]).GetShaders();
			for (int j=0;j<vImags2.GetCount();j++){
				CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
				if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1"){
					vTexture=vImags2[j];
					
					ImageClip2 vImgClip(vTexture.GetImageClip() );
					Source vImgClipSrc(vImgClip.GetSource());
					CValue vFileName = vImgClipSrc.GetParameterValue( L"path");
					
					f << "Texture \"Kd-"<< sname << "\" \"color\" \"imagemap\" \"string filename\" [\"" << replace(vFileName.GetAsText().GetAsciiString()) << "\"] \"string wrap\" [\"repeat\"] \"string filtertype\" [\"bilinear\"] \"string mapping\" [\"uv\"] \"float vscale\" [-1.0]\n";
					vText=true;
				}
			}
		}
		
		//
		// write shader block
		//
		
		f << "MakeNamedMaterial \""<< m.GetName().GetAsciiString() << "\" \n";
		f << "  \"string type\" [\""<< shaderType.GetAsciiString() <<"\"]\n";
		f << shaderString.GetAsciiString();
		
		
		/*
		CValue tmp;
		switch (ret) {
		case 1: tmp=L"\"texture Kr\" \"Kr-" + m.GetName()+L"\" \"texture Kt\" \"Kt-"+ m.GetName()+L"\" \"texture index\" \"index-"+ m.GetName()+L"\" \"texture cauchyb\" \"cauchyb-"+ m.GetName()+L"\" ";break;
		case 2: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\" ";break;
		case 3: tmp=L"\"texture Kr\" \"Kr-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\"";break;
		case 4: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture sigma\" \"sigma-"+ m.GetName()+L"\"";break;
		case 6: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks1\" \"Ks1-"+m.GetName()+ L"\" \"texture Ks2\" \"Ks2-"+m.GetName()+ L"\" \"texture Ks3\" \"Ks3-"+m.GetName()+ L"\" \"texture R1\" \"R1-"+m.GetName()+ L"\" \"texture R2\" \"R2-"+m.GetName()+ L"\" \"texture R3\" \"R3-"+m.GetName()+ L"\" \"texture M1\" \"M1-"+m.GetName()+ L"\" \"texture M2\" \"M2-"+m.GetName()+ L"\" \"texture M3\" \"M3-"+m.GetName()+ L"\"";break;
		case 7: tmp=L"\"texture Kr\" \"Kr-" + m.GetName()+L"\" \"texture Kt\" \"Kt-"+ m.GetName()+L"\" \"texture index\" \"index-"+ m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\" \"texture cauchyb\" \"cauchyb-"+ m.GetName()+L"\" ";break;
		case 8: tmp=L"\"texture Kr\" \"Kr-" + m.GetName()+L"\" ";break;
		case 9: tmp=L"\"texture name\" \"name-" + m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"roughness-"+m.GetName()+L"\" ";break;
		case 10: tmp=L"\"texture Kr\" \"Kr-"+m.GetName()+ L"\" \"texture Kt\" \"Kt-"+m.GetName()+ L"\" \"texture sigma\" \"sigma-"+ m.GetName()+L"\" ";break;
		}
		*/
		
		
	}
}


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
	CRefArray shad(m.GetShaders());	// Array of all shaders attached to the material [e.g. phong]
	Shader s(shad[0]);
	CGeometryAccessor ga;
	CValue vObjW,vObjType,vObjSh2,vObjSh3,vObjSh;
	CString vUV=L"",vNormals=L"",vTris=L"",vMod=L"",vPoints=L"";
	CFacetRefArray facets( g.GetFacets() );

	int vSubDValue=1;
	
	Property geopr=o.GetProperties().GetItem(L"Geometry Approximation");
	if ((int)geopr.GetParameterValue(L"gapproxmordrsl")>0 || (int)geopr.GetParameterValue(L"gapproxmosl")>0) {
		vIsSubD=true;
		if ((int)geopr.GetParameterValue(L"gapproxmordrsl")>0) {
			vSubDValue=(int)geopr.GetParameterValue(L"gapproxmordrsl")+1;
		} else {
			vSubDValue=(int)geopr.GetParameterValue(L"gapproxmosl")+1;
		}
	} else {
		vIsSubD=false;
	}
	
	
	ga = PolygonMesh(g).GetGeometryAccessor();
	CRefArray gaUV = ga.GetUVs();
	ClusterProperty uv(gaUV[0]);
	CFloatArray uvValues;
	uv.GetValues( uvValues );
	
	ga.GetVertexPositions(point_count);
	ga.GetTriangleVertexIndices(tri_count); 
	
	ga.GetTriangleNodeIndices(nod); 
	ga.GetNodeNormals(normals);
	
	CLongArray pvCount;
	CLongArray vIndices;
	ga.GetTriangleNodeIndices(vIndices);
	
	ga.GetPolygonVerticesCount(pvCount);
	long nPolyCount = ga.GetPolygonCount();
	
	
	//app.LogMessage(L"GA Stuff:" + CString(pvCount) + L" " + CString(vIndices.GetCount()) + L" " + CString(nod.GetCount()));
	//Cube: vIndices.GetCount()=24 ----  nod.GetCount()=36
	
	//CRefArray cls = o.GetActivePrimitive().GetGeometry().GetClusters();	// get clusters
	CTransformation localTransformation = ga.GetTransform();
	KinematicState  gs = o.GetKinematics().GetGlobal();
	//KinematicState  gs = o.GetKinematics().GetLocal();
	CTransformation gt = gs.GetTransform();
	CMatrix4 mat4(gt.GetMatrix4());
	
	CVector3 axis; 
	double rot = gt.GetRotationAxisAngle(axis);
	
	
	//app.LogMessage(L"Clusters: "+CString(cls.GetCount()));
	
	
	
	if (point_count.GetCount()>0 || tri_count.GetCount()>0) {
		
		f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
		
		
		
		if ((float)s.GetParameter(L"inc_inten").GetValue()>0) {
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
		
		
		
		
			
			//
			// write triangles
			//
			if (vType==L"instance") {
				f << "Identity\n";
			}
				f << "Translate " << CString(gt.GetPosX()).GetAsciiString() << " " << CString(-gt.GetPosZ()).GetAsciiString() << " "<< CString(gt.GetPosY()).GetAsciiString() << "\n";
				if (rot!=0){
				f << "Rotate " << (rot*180/PI) << " "<< CString(axis[0]).GetAsciiString() << " " << CString(-axis[2]).GetAsciiString() << " "<< CString(axis[1]).GetAsciiString() << "\n";
				}
				if (gt.GetSclX()==gt.GetSclY()==gt.GetSclZ()==1) {} else {
					f << "Scale " << CString(gt.GetSclX()).GetAsciiString() << " " << CString(gt.GetSclZ()).GetAsciiString() << " "<< CString(gt.GetSclY()).GetAsciiString() << "\n";
				}
		//	}
			
			CTriangleRefArray triangles(g.GetTriangles());
			CLongArray indices( triangles.GetIndexArray() );
			//CUVArray aUV ( triangles.GetUVArray() );
			
			for ( LONG i=0; i<indices.GetCount(); i+=3 )	{
				vTris += L" "+ CValue(indices[i]).GetAsText() + L" "+CValue(indices[i+1]).GetAsText() + L" "+CValue(indices[i+2]).GetAsText() + L" "  + L"\n ";
			}
				
			
			for (LONG j=0;j<point_count.GetCount();j+=3){
					//bool b;
					//CVector3 pos(Point(points[j]).GetPosition());
					//CVector3 norm(Point(points[j]).GetNormal(b));
					//norm.NormalizeInPlace();
					vPoints +=  L" "+ CString(point_count[j]) + L" "+  CString(-point_count[j+2]) + L" "+ CString(point_count[j+1])+L"\n"; 
					//vNormals +=  L" "+ CString(norm[0]) + L" "+  CString(-norm[2]) + L" "+ CString(norm[1])+L"\n"; 
					//vUV +=  L" "+CString(uvValues[j*3+1]) + L" "+ CString(-uvValues[j*3])+L"\n";
					
				}
			/*
			CPointRefArray points;
			LONG lFacetCount = facets.GetCount();
				
			for ( LONG i=0; i<lFacetCount; i++ )	{
				Facet facet(facets[i]);
				CSampleRefArray samples( facet.GetSamples() );
				points = facet.GetPoints() ;
				
				LONG lSampleCount = samples.GetCount();
				for (LONG j=0; j<lSampleCount; j+=lSampleCount)	{
					vTris += L" "+ CValue(Sample(samples[j]).GetIndex()).GetAsText()+ L" " + CValue(Sample(samples[j+1]).GetIndex()).GetAsText()+ L" "+ CValue(Sample(samples[j+2]).GetIndex()).GetAsText() + L"\n ";
					if (lSampleCount==4) {
						vTris += L" "+ CValue(Sample(samples[j]).GetIndex()).GetAsText()+ L" " + CValue(Sample(samples[j+2]).GetIndex()).GetAsText()+ L" "+ CValue(Sample(samples[j+3]).GetIndex()).GetAsText() + L"\n ";
					}
				}
				
				for (LONG j=0;j<points.GetCount();j++){
					bool b;
					CVector3 pos(Point(points[j]).GetPosition());
					CVector3 norm(Point(points[j]).GetNormal(b));
					norm.NormalizeInPlace();
					vPoints +=  L" "+ CString(pos[0]) + L" "+  CString(-pos[2]) + L" "+ CString(pos[1])+L"\n"; 
					vNormals +=  L" "+ CString(norm[0]) + L" "+  CString(-norm[2]) + L" "+ CString(norm[1])+L"\n"; 
					//vUV +=  L" "+CString(uvValues[j*3+1]) + L" "+ CString(-uvValues[j*3])+L"\n";
					
				}
				
				
			}
			
			
			*/
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
			
			//
			// write 
			//
			
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
					f << "\"loopsubdiv\" \"integer nlevels\" [" <<  vSubDValue << "] \"bool dmnormalsmooth\" [\"true\"] \"bool dmsharpboundary\" [\"false\"]\n";
				} else {
					f << " \"trianglemesh\"\n";
				}
			
				f << "\"integer indices\" [\n";
				f << vTris.GetAsciiString();
				f << " ] \"point P\" [\n" ;
				f << vPoints.GetAsciiString();
				f << "] ";
				
				/*
				if(vText){
					f << " \"float uv\" [\n";
					//app.LogMessage(CString(vUV.GetCount()));
					//for (long i=0;i<vUV.GetCount();i++){
					//	f << vUV[i].GetAsciiString()<< "\n";
					//}
					f << vUV.GetAsciiString();
					f << " ]\n";
				}
				*/
				//f << "]";
			
			
		}
		f << "\nAttributeEnd #" << o.GetName().GetAsciiString() << "\n";

	}
	
	
	
	
	return 0;
}

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
	//KinematicState  gs = o.GetKinematics().GetLocal();
	CTransformation gt = gs.GetTransform();
	CMatrix4 mat4(gt.GetMatrix4());
	
	CVector3 axis; 
	double rot = gt.GetRotationAxisAngle(axis);
	f << "\nTranslate " << CString(gt.GetPosX()).GetAsciiString() << " " << CString(-gt.GetPosZ()).GetAsciiString() << " "<< CString(gt.GetPosY()).GetAsciiString() << "\n";
	if (rot!=0){
	f << "Rotate " << (rot*180/PI) << " "<< CString(axis[0]).GetAsciiString() << " " << CString(-axis[2]).GetAsciiString() << " "<< CString(axis[1]).GetAsciiString() << "\n";
	}
	if (gt.GetSclX()==gt.GetSclY()==gt.GetSclZ()==1) {} else {
		f << "Scale " << CString(gt.GetSclX()).GetAsciiString() << " " << CString(gt.GetSclZ()).GetAsciiString() << " "<< CString(gt.GetSclY()).GetAsciiString() << "\n";
	}
	f << "ObjectInstance \"" << Model(o).GetInstanceMaster().GetName().GetAsciiString() <<"\"\n";
	f << "AttributeEnd #" << o.GetName().GetAsciiString() << "\n\n";
}

CString readIni(){
	
	char x;
	CString data;
	ifstream load;
	
	CString iniPath;
	
	iniPath = app.GetInstallationPath(siUserPath);
	app.LogMessage(L"userdir:"+ iniPath);
	#ifdef __unix__
		iniPath += L"/LuXSI.ini";
	#else
		iniPath += L"\\LuXSI.ini";
	#endif
	app.LogMessage(L""+iniPath);
	load.open( iniPath.GetAsciiString() );
	
	while(load.get(x)) {
      data+=x;
   }
   app.LogMessage(L""+CString(data));
   load.close();
   return data;
}

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
		for ( int i=0; i<array.GetCount();i++ ){
			X3DObject o(array[i]);
			app.LogMessage( L"\tObject name: " + o.GetName() + L":" +o.GetType() + L" parent:"+X3DObject(o.GetParent()).GetType());
			Property visi=o.GetProperties().GetItem(L"Visibility");
			// Collection objects
			if (o.GetType()==L"polymsh"){
				if (vIsHiddenObj || (vIsHiddenObj==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) {
					aObj.Add(o); 	
				}
			}
			if (o.GetType()==L"CameraRoot"){
				if (vIsHiddenCam || (vIsHiddenCam==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aCam.Add(o);	// visibilty check
			}
			if (o.GetType()==L"camera" && X3DObject(o.GetParent()).GetType()!=L"CameraRoot"){
				if (vIsHiddenCam || (vIsHiddenCam==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aCam.Add(o);	// visibilty check
			}
			if (o.GetType()==L"light"){
				if (vIsHiddenLight || (vIsHiddenLight==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true)))  aLight.Add(o);	// visibilty check
			}
			if (o.GetType()==L"surfmsh"){
				if (vIsHiddenSurfaces || (vIsHiddenSurfaces==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aSurfaces.Add(o);	// visibilty check
			}
			if (o.GetType()==L"cloud"){
				if (vIsHiddenClouds || (vIsHiddenClouds==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aClouds.Add(o);	// visibilty check
			}
			if (o.GetType()==L"#model"){
				// model
				if (Model(o).GetModelKind()==2){
					// instances
					
					if (vIsHiddenObj || (vIsHiddenObj==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aInstance.Add(o); 	// visibilty check
				}
			}
		}
		
		if (aObj.GetCount()==0) {
			app.LogMessage(L"No objects are exported! Check if objects are visible in XSI or switch on 'export hidden objects'",siFatalMsg );
			
		} else if (aCam.GetCount()==0) {
			app.LogMessage(L"No cameras are exported! Check if a camera is visible in XSI or switch on 'export hidden cameras'",siFatalMsg );
			
		} else {
			
			//app.LogMessage( L"\tObject count: "+CString(aObj.GetCount()));
			//app.LogMessage( L"\tInstance count: "+CString(aInstance.GetCount()));
			pb.PutValue(0);
			pb.PutMaximum( aObj.GetCount()+aInstance.GetCount() );
			pb.PutStep(1);
			pb.PutVisible( true );
			pb.PutCaption( L"Processing" );
			pb.PutCancelEnabled(true); 
			
			f.open (vFileObjects.GetAsciiString());
			
			// write to file
			
			for (int i=0;i<aCam.GetCount();i++) writeLuxsiCam(aCam[i]);
			
			writeLuxsiBasics();
			f << "AttributeBegin\n";
			for (int i=0;i<aLight.GetCount();i++) writeLuxsiLight(aLight[i]);
			f << "AttributeEnd\n\n";	
			
			
			writeLuxsiShader();
			
			for (int i=0;i<aObj.GetCount();i++) {
				if (writeLuxsiObj(aObj[i],L"obj")==-1) break;
				if (pb.IsCancelPressed() ) break;
				pb.Increment();
			}
			
			for (int i=0;i<aInstance.GetCount();i++) {
				if (writeLuxsiInstance(aInstance[i])==-1) break;

				if (pb.IsCancelPressed() ) break;
				pb.Increment();
			}
			
			pb.PutVisible( false );
			f << "WorldEnd";
			f.close();
			vExportDone=true;
		}
	} else {
		app.LogMessage(L"Filename is empty",siErrorMsg );
	}
}



#if defined(_WIN32) || defined(_WIN64)
	void loader(const char szExe[], const char szArgs[])
	{
		//HANDLE hFile ;
		PROCESS_INFORMATION  pi;
		// start a program in windows
		STARTUPINFO  si = { sizeof(si) };
		CreateProcessA(szExe, (LPSTR)szArgs, 0, 0, FALSE, 0, 0, 0, LPSTARTUPINFOA(&si), &pi);
	}
#endif

void luxsi_execute(){
	
	if (vLuXSIPath!=L""){
		if (vExportDone) {
			app.LogMessage(vLuXSIPath +L" " + vFileObjects );
			
			#ifdef __unix__
				pid_t pid = fork();
				if( 0 == pid ) {
					system ( (vLuXSIPath +L" " + vFileObjects).GetAsciiString());
					exit(0); 
				}
			#else
				// win
				
				char pfad[500];
				char options[500];
				
				
				loader(vLuXSIPath.GetAsciiString(),replace(' "'+vFileObjects.GetAsciiString()+'"').c_str());
			#endif 
		
		}
	}else {
		app.LogMessage(L"Select the Luxrender path",siErrorMsg );
	}
}
