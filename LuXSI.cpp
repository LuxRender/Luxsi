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
#include <xsi_scene.h>
#include <xsi_library.h>
#include <xsi_source.h>

#include <xsi_projectitem.h>
#include <xsi_siobject.h>
#include <xsi_base.h>

using namespace XSI; 
using namespace MATH; 
using namespace std;

#define PI 3.14159265

void writeLuxsiBasics();
void writeLuxsiCam(X3DObject o);
void writeLuxsiLight(X3DObject o);
int writeLuxsiObj(X3DObject o);
void writeClouds(X3DObject o);
void writeLuxsiShader();

void luxsi_write();
void update_LuXSI_values(CString   paramName, Parameter changed,PPGLayout lay);


Application app;
Model root( app.GetActiveSceneRoot() );
std::ofstream f;
std::stringstream sLight, sObj, sMat;

Null null;
CustomProperty prop ;

int vDis=12,vThreads=2,vmaxRejects=256,vSurf=3,vXRes=640,vYRes=480,vFilt=1,vMaxDepth=256,vSamples=4,vPxSampler=1,vSave=120,vSampler=4;
bool vMLT=true,vIsLinux=true,vIsGI=true,vRrft=true,vProg=true,vIsCaustic=false,vIsHiddenCam=false,vIsHiddenLight=false,vIsHiddenObj=false;
bool vUseJitter=true,vIsHiddenSurfaces=false,vSFPreview=true,vIsHiddenClouds=false,vExpOne=true,vAmbBack=false,vExr=true,vIgi=false,vTga=true;
float vCSize=0.4f,vGITolerance=0.025f,vSpacingX=0.1f,vSpacingY=0.1f,vContrast=2.2f;
float vlmprob=0.4f, vrrprob=0.65f;


CStringArray aMatList;
UIToolkit kit = app.GetUIToolkit();
ProgressBar pb = kit.GetProgressBar();
CString vSun=L"",vHDRI=L"",vLuXSIPath=L"",vFileObjects=L"";


XSIPLUGINCALLBACK CStatus XSILoadPlugin( PluginRegistrar& in_reg )
{
	in_reg.PutAuthor(L"Michael 'miga' Gangolf");
	in_reg.PutName(L"LuXSI");
	in_reg.PutEmail(L"miga@migaweb.de");
	in_reg.PutURL(L"http://www.migaweb.de");
	in_reg.PutVersion(0,5);
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
	prop.AddParameter( L"surfaceintegrator", CValue::siInt4, siPersistable, L"", L"",vSurf, oParam ) ;
	prop.AddParameter( L"pxsampler", CValue::siInt4, siPersistable, L"", L"",vPxSampler, oParam ) ;
	
	prop.AddParameter(L"savint",CValue::siInt4,siPersistable,L"",L"",vSave,0l,200l,0l,200l,oParam);
	prop.AddParameter(L"disint",CValue::siInt4,siPersistable,L"",L"",vDis,0l,200l,0l,200l,oParam);
	prop.AddParameter(L"AmbBack",CValue::siBool,siPersistable,L"",L"",vAmbBack,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"save_exr",CValue::siBool,siPersistable,L"",L"",vExr,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"save_igi",CValue::siBool,siPersistable,L"",L"",vIgi,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"save_tga",CValue::siBool,siPersistable,L"",L"",vTga,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"mlt",CValue::siBool,siPersistable,L"",L"",vMLT,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"maxrej",CValue::siInt4,siPersistable,L"",L"",vmaxRejects,0l,10048l,0l,512l,oParam);
	prop.AddParameter(L"lmprob",CValue::siFloat,siPersistable,L"",L"",vlmprob,0l,10l,0l,10l,oParam);
	
	prop.AddParameter( L"fObjects", CValue::siString, siPersistable, L"", L"", CValue(), oParam ) ;
	
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
	lay.AddItem(L"disint",L"Display Interval (sec)");
	lay.EndGroup();
	
	
	
	
	lay.AddGroup(L"Export hidden...");
	lay.AddItem(L"use_hidden_obj", L"Objects");
	lay.AddItem(L"use_hidden_cam", L"Cameras");
	lay.AddItem(L"use_hidden_light", L"Lights");
	lay.EndGroup();
	
	
	lay.AddGroup(L"Save as");
	lay.AddItem(L"save_tga", L"TGA");
	lay.AddItem(L"save_exr", L"EXR");
	lay.AddItem(L"save_igi", L"IGI");
	lay.AddItem(L"savint",L"Interval (sec)");
	
	lay.EndGroup();
	
	lay.AddItem(L"fObjects",L"Filename",siControlFilePath);
	
	PPGItem it = lay.GetItem( L"fObjects" );
	it.PutAttribute( siUIFileFilter, L"LuXSI Scenes|*.lxs" ) ;
	lay.AddRow();
	lay.AddButton(L"exe_luxsi",L"Export");
	lay.EndRow();
	
	
	
	lay.AddTab(L"Render settings");
	
	lay.AddGroup(L"Presets");
	lay.AddRow();
	lay.AddButton(L"pre_dir",L"Preview (DirectLight)");
	lay.AddButton(L"pre_path",L"Preview (Path)");
	lay.EndRow();
	lay.AddRow();
	lay.AddButton(L"final_path",L"Final (Path)");
	lay.AddButton(L"final_low",L"Final Low (MLT)");
	lay.EndRow();
	lay.AddRow();
	lay.AddButton(L"final_med",L"Final Med (MLT)");
	lay.AddButton(L"final_high",L"Final High (MLT)");
	lay.EndRow();
	lay.AddRow();
	lay.AddButton(L"ref_path",L"Reference Path");
	lay.AddButton(L"ref_mlt",L"Reference MLT");
	lay.EndRow();
	lay.EndGroup();
	
	lay.AddItem(L"max_depth",L"Max depth");
	lay.AddItem(L"AmbBack",L"Use Ambience as background");
	CValueArray vItem3(10);
	vItem3[0] = L"lowdiscrepancy" ; vItem3[1] = 0;
	vItem3[2] = L"halton" ; vItem3[3] = 1;
	vItem3[4] = L"random" ; vItem3[5] = 2;
	vItem3[6] = L"erpt" ; vItem3[7] = 3;
	vItem3[8] = L"metropolis" ; vItem3[9] = 4;
	lay.AddEnumControl(L"sampler",vItem3,L"Sampler",siControlCombo ) ;
	
	
	CValueArray vItem5(12);
	vItem5[0] = L"linear" ; vItem5[1] = 0;
	vItem5[2] = L"vegas" ; vItem5[3] = 1;
	vItem5[4] = L"random" ; vItem5[5] = 2;
	vItem5[6] = L"lowdiscrepancy" ; vItem5[7] = 3;
	vItem5[8] = L"tilepx" ; vItem5[9] = 4;
	vItem5[10] = L"hilbert" ; vItem5[11] = 5;
	lay.AddEnumControl(L"pxsampler",vItem5,L"Pixelsampler",siControlCombo ) ;
	lay.AddItem(L"samples",L"Pixelsamples");
	
	
	CValueArray vItem6(8);
	vItem6[0] = L"directlighting" ; vItem6[1] = 0;
	vItem6[2] = L"bidirectional" ; vItem6[3] = 1;
	vItem6[4] = L"particletracing" ; vItem6[5] = 2;
	vItem6[6] = L"path" ; vItem6[7] = 3;
	//vItem6[8] = L"pathn" ; vItem6[9] = 4;
	lay.AddEnumControl(L"surfaceintegrator",vItem6,L"Surface Integrator",siControlCombo ) ;
	
	
	lay.AddGroup(L"MLT");
	lay.AddItem(L"maxrej",L"Max rejects");
	lay.AddItem(L"lmprob",L"LMprob");
	lay.EndGroup();
	
	
	
	lay.AddRow();
	lay.AddButton(L"exe_luxsi",L"Export");
	lay.EndRow();
	
	return CStatus::OK;
}

XSIPLUGINCALLBACK CStatus LuXSI_PPGEvent( const CRef& in_ctxt )
{
	
	Application app ;
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
		} else if (buttonPressed.GetAsText()==L"pre_dir"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(0); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(5); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(0); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(3); 
		} else if (buttonPressed.GetAsText()==L"pre_path"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(0); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(3); 
			vrrprob = 1.0f;
		} else if (buttonPressed.GetAsText()==L"final_low"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(12); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(4); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(3); 
			vrrprob = 0.65f;
			Parameter(prop.GetParameters().GetItem( L"lmprob" )).PutValue(0.4); 
			Parameter(prop.GetParameters().GetItem( L"maxrej" )).PutValue(256); 
		} else if (buttonPressed.GetAsText()==L"final_med"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(12); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(4); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(3); 
			vrrprob = 0.65f;
			Parameter(prop.GetParameters().GetItem( L"lmprob" )).PutValue(0.25); 
			Parameter(prop.GetParameters().GetItem( L"maxrej" )).PutValue(256); 
		}  else if (buttonPressed.GetAsText()==L"final_high"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(12); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(4); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(3); 
			vrrprob = 0.65f;
			Parameter(prop.GetParameters().GetItem( L"lmprob" )).PutValue(0.1); 
			Parameter(prop.GetParameters().GetItem( L"maxrej" )).PutValue(256); 
		}else if (buttonPressed.GetAsText()==L"ref_path"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(1024); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(4); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(2); 
			vrrprob = 0.65f;
		}else if (buttonPressed.GetAsText()==L"ref_mlt"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(1024); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(4); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler")).PutValue(3); 
			vrrprob = 0.65f;
			Parameter(prop.GetParameters().GetItem( L"lmprob" )).PutValue(0.25); 
			Parameter(prop.GetParameters().GetItem( L"maxrej" )).PutValue(8192); 
		} else if (buttonPressed.GetAsText()==L"final_path"){
			Parameter(prop.GetParameters().GetItem( L"surfaceintegrator" )).PutValue(3); 
			Parameter(prop.GetParameters().GetItem( L"max_depth" )).PutValue(12); 
			Parameter(prop.GetParameters().GetItem( L"sampler" )).PutValue(0); 
			Parameter(prop.GetParameters().GetItem( L"pxsampler" )).PutValue(3); 
			vrrprob = 0.65f;
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

void update_LuXSI_values(CString paramName, Parameter changed,PPGLayout lay){
	// update variables when PPG value changed
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
	} else if (paramName==L"samples"){
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
	} else if (paramName==L"lmprob"){
		vlmprob=changed.GetValue();
	} else if (paramName==L"save_igi"){
		vIgi=changed.GetValue();
	} else if (paramName==L"save_exr"){
		vExr=changed.GetValue();
	} else if (paramName==L"save_tga"){
		vTga=changed.GetValue();
	} else if (paramName==L"sampler"){
		vSampler=changed.GetValue();
	} else if (paramName==L"pxsampler"){
		vPxSampler=changed.GetValue();
	}else if (paramName==L"surfaceintegrator"){
		vSurf=changed.GetValue();
	}else if (paramName==L"disint"){
		vDis=changed.GetValue();
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
	Application app;
	
	CValueArray addpropArgs(5) ;
	addpropArgs[0] = L"LuXSI"; // Type of Property
	addpropArgs[3] = L"LuXSI"; // Name for the Property
	addpropArgs[1] = L"Scene_Root";
	
	CValue retVal ;
	CStatus st = app.ExecuteCommand( L"SIAddProp", addpropArgs, retVal ) ;
	
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


void writeLuxsiBasics(){
	//
	// write basic scene description
	//
	vHDRI=L"";
	
	char aFilter[8][20]={"box","gaussian","mitchell","sinc","triangle"};
	
	
	char aBool[2][6]={"false","true"};
	char aSampler[5][15]={"lowdiscrepancy","halton","random","erpt","metropolis"};
	char aPxSampler[6][15]={"linear","vegas","random","lowdiscrepancy","tilepx","hilbert"};
	char aSurf[7][16]={"directlighting","bidirectional","patricletracing","path"};
	
	string fname=vFileObjects.GetAsciiString();
	int loc=fname.rfind(".");
	
	f << "Film \"fleximage\"\n";
	f << "  \"integer xresolution\" [" <<  vXRes << "] \"integer yresolution\" [" <<  vYRes<<"]\n";
	
	f << "  \"string filename\" [\""<< fname.substr(0,loc) << "\"]\n";
	f << "  \"integer writeinterval\" ["<< vSave << "]\n";
	f << "  \"integer displayinterval\" ["<< vDis << "]\n";
	
	if (vExr){
		f << "  \"bool write_tonemapped_exr\" [\"true\"]\n";
		f << "  \"bool write_untonemapped_exr\" [\"true\"]\n";
	} else {
		f << "  \"bool write_tonemapped_exr\" [\"false\"]\n";
	}
	
	if (vTga){
		f << "  \"bool write_tonemapped_tga\" [\"true\"]\n";
	} 
	if (vIgi){
		f << "  \"bool write_tonemapped_igi\" [\"true\"]\n";
		f << "  \"bool write_untonemapped_igi\" [\"true\"]\n";
	} 
	f << "  \"bool write_resume_flm\" [\"false\"]\n";
	f << "  \"float gamma\" [" << vContrast << "]\n\n";
	
		
		
		
		
		f << "PixelFilter \""<< aFilter[vFilt] << "\" \"float xwidth\" [2.000000] \"float ywidth\" [2.000000]\n";
		f << "Sampler \""<< aSampler[vSampler] <<"\" ";
		
		if (vSampler!=4) {f << "\"string pixelsampler\" [\""<< aPxSampler[vPxSampler] <<"\"] ";}
		
		
		
		switch(vSampler) {
		case 0: f << "\"integer pixelsamples\" ["<< vSamples <<"]";break;
		case 4: f << "\"float largemutationprob\" ["<< vlmprob<<"] ";
			f << "\"integer maxconsecrejects\" ["<< vmaxRejects<<"] ";		
			break;
		}
		
		f << "\nSurfaceIntegrator \"" << aSurf[vSurf] << "\" \"integer maxdepth\" ["<< vMaxDepth << "] ";
		switch(vSurf) {
		case 0: break;
		case 1: break;
		case 2: break;
		case 3: 
			//f << "\"float rrcontinueprob\" ["<<vrrprob << "] ";
			break;
		case 4: break;
		}
		f << "\nAccelerator \"kdtree\"\n\n";
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
			f << "\"string mapname\" [\"" << vHDRI.GetAsciiString() << "\"]\n";
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
	X3DObject o2 (o.GetChildren()[0]);
	CTransformation localTransformation = o2.GetKinematics().GetLocal().GetTransform();
	KinematicState  gs = o2.GetKinematics().GetGlobal();
	CTransformation gt = gs.GetTransform();
	MapObjectPoseToWorldSpace(  gt, localTransformation);
	CVector3 translation(localTransformation.GetTranslation());
	bool vDof=false;
	Camera c(o.GetChildren()[0]);
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
	f << "LookAt " << CString(gt.GetTranslation().GetX()).GetAsciiString() << " " <<CString(-gt.GetTranslation().GetZ()).GetAsciiString() << " " << CString(gt.GetTranslation().GetY()).GetAsciiString() << " " << CString(ci_gt.GetTranslation().GetX()).GetAsciiString() << " " <<CString(-ci_gt.GetTranslation().GetZ()).GetAsciiString()  << " " <<CString(ci_gt.GetTranslation().GetY()).GetAsciiString()  << " 0 0 " << 	CString(up.GetZ()*up.GetZ()).GetAsciiString() << "\n";
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
	CValue lMore=L"",lMore2=L"",lRow1=L"",lRow2=L"",lRow3=L"",lRow4=L"",lRow5=L"";
	CValue lType,lPos,lPower;
	float a=0,b=0,c=0,alpha=0;
	Shader s((Light(o).GetShaders())[0]);
	OGLLight myOGLLight(Light(o).GetOGLLight());
	s.GetColorParameterValue(L"color",a,b,c,alpha ); 
	
	
	
	if (myOGLLight.GetType()==siLightSpot ){
		//
		// Spotlight
		//
		
		X3DObject li;
		CVector3 intPos;
		
		//Get Interest
		li= X3DObject(o.GetParent()).GetChildren()[1];
		CTransformation localTransformation2 = li.GetKinematics().GetLocal().GetTransform();
		KinematicState  gs2 = li.GetKinematics().GetGlobal();
		CTransformation gt2 = gs2.GetTransform();
		//MapObjectPoseToWorldSpace(  gt2, localTransformation2);
		intPos=gt2.GetTranslation();
		
		CRefArray shad(Light(o).GetShaders());
		Shader s(shad[0]);
		
		lRow1=L"LightSource \"spot\" \"point from\" ["+CString(translation.GetX()) + L" " + CString(translation.GetY()) + L" " +CString(translation.GetZ()) +L"] \"point to\" ["+CString(intPos.GetX()) + L" " + CString(intPos.GetY()) +L" "+ CString(intPos.GetZ())+L"]";
		lRow2=L"\"float coneangle\" ["+ CString((float)o.GetParameterValue(CString(L"LightCone"))) + L"] \"float conedeltaangle\" ["+ CString( (float)o.GetParameterValue(CString(L"LightCone")) - (float)s.GetParameter(L"spread").GetValue() ) + L"]";
	} else if  (myOGLLight.GetType()==siLightInfinite) {
		//
		//sunlight
		//
		
		CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();
		
		lRow1=L"LightSource \"sunsky\" \"integer nsamples\" [4] ";
		lRow2=L"\"vector sundir\" [ "+CString((float)sunTransMat.GetValue(2,0))+L" "+CString((float)-sunTransMat.GetValue(2,2))+L" "+CString((float)sunTransMat.GetValue(2,1))+L" ]";
		
	} else {
		//
		// Pointlight
		//
		lRow1=L"LightSource \"infinite\" \"color L\" ["+ CString(a) + L"  " + CString(b) + L"  " +  CString(c)+L"] \"integer nsamples\" [1]";
		
	}
	
	if (lMore.GetAsText()!=L"") {
		sLight << "  " << lMore.GetAsText().GetAsciiString()<< " \n";
	}
	
	sLight <<"  " << lRow1.GetAsText().GetAsciiString()<< " \n";
	if (lMore2.GetAsText()!=L"") {
		sLight << lMore2.GetAsText().GetAsciiString()<< " \n";
	}
	if (lRow2.GetAsText()!=L"") {
		sLight <<"  " << lRow2.GetAsText().GetAsciiString()<< " \n";
	}
	if (lRow3.GetAsText()!=L"") {
		sLight <<"  " << lRow3.GetAsText().GetAsciiString()<< " \n";
	}
	if (lRow4.GetAsText()!=L"") {
		sLight <<"  " << lRow4.GetAsText().GetAsciiString()<< " \n";
	}
	if (lRow5.GetAsText()!=L"") {
		sLight <<"  " << lRow5.GetAsText().GetAsciiString()<< " \n";
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
	aShader[2] = L"substrate";
	aShader[3] = L"shinymetal";
	aShader[4] = L"matte";
	aShader[5] = L"plastic";
	aShader[6] = L"carpaint";
	aShader[7] = L"roughglass";
	aShader[8] = L"mirror";
	aShader[9] = L"metal";
	aShader[10] = L"mattetranslucent";
	int ret=0;
	
	for ( LONG i=0; i < materials.GetCount(); i++ ) {
		Texture vTexture;
		CString addString;
	
	float b_red=0.0f,b_green=0.0f,b_blue=0.0f,b_alpha=0.0f,red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f,sp_red=0.0f,sp_green=0.0f,sp_blue=0.0f,sp_alpha=0.0f,vModScale=0.0f,refl_red=0.0f,refl_green=0.0f,refl_blue=0.0f,refl_alpha=0.0f;
	float mRough=0.0f;
	ImageClip2 vBumpFile;
	Shader vBumpTex;
		bool vIsSet=false;
		bool vText=false,vIsSubD=true;
		CValue vDiffType,vMore,vCol,vMore2,vMore3,vTexStr,vMore1,vMore4,vMore5;
		Material m( materials[i] );
		
		CRefArray shad(m.GetShaders());	// Array of all shaders attached to the material [e.g. phong]
		Shader s(shad[0]);
		CString vMatID((s.GetProgID()).Split(L".")[1]);
		char sname[256];
		strcpy(sname,m.GetName().GetAsciiString());
		
		
		if (vMatID==L"lux_glass") {
			s.GetColorParameterValue(L"kt",red,green,blue,alpha );
			f << "Texture \"Kt-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			s.GetColorParameterValue(L"kr",red,green,blue,alpha );
			f << "Texture \"Kr-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			f << "Texture \"index-"<<  sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"index").GetValue() << "]\n";
			f << "Texture \"cauchyb-"<<  sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"cauchyb").GetValue() << "]\n";
			if ((bool)s.GetParameter(L"architectural").GetValue()==true) {
				 addString += L"\"bool architectural\" [\"true\"]";
			}
			ret=1;
			vIsSet=true;	
		} else if (vMatID==L"lux_roughglass") {
			s.GetColorParameterValue(L"kt",red,green,blue,alpha );
			f << "Texture \"Kt-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			s.GetColorParameterValue(L"kr",red,green,blue,alpha );
			f << "Texture \"Kr-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"uroughness").GetValue() << "]\n";
			f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"vroughness").GetValue() << "]\n";
			f << "Texture \"index-"<<  sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"index").GetValue() << "]\n";
			f << "Texture \"cauchyb-"<<  sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"cauchyb").GetValue() << "]\n";
			ret=7;
			vIsSet=true;	
		} else if (vMatID==L"lux_matte") {
			s.GetColorParameterValue(L"kd",red,green,blue,alpha );
			f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<red<<" "<<green<<" "<<blue<<"]\n";	
			f << "Texture \"sigma-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< (float)s.GetParameter(L"sigma").GetValue() <<"]\n";
			ret=4;
			vIsSet=true;	
		} else if (vMatID==L"lux_mirror") {
			s.GetColorParameterValue(L"kr",red,green,blue,alpha );
			f << "Texture \"Kr-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<red<<" "<<green<<" "<<blue<<"]\n";	
			ret=8;
			vIsSet=true;	
		} else if (vMatID==L"lux_metal") {
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
		} else if (vMatID==L"lux_shinymetal") {
			s.GetColorParameterValue(L"ks",red,green,blue,alpha );
			f << "Texture \"Ks-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			s.GetColorParameterValue(L"kr",red,green,blue,alpha );
			f << "Texture \"Kr-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"roughness").GetValue() << "]\n";
			f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"roughness").GetValue() << "]\n";
			ret=3;
			vIsSet=true;	
		} else if (vMatID==L"lux_substrate") {
			s.GetColorParameterValue(L"kd",red,green,blue,alpha );
			f << "Texture \"Kd-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			s.GetColorParameterValue(L"ks",red,green,blue,alpha );
			f << "Texture \"Ks-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
			f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"uroughness").GetValue() << "]\n";
			f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"vroughness").GetValue() << "]\n";
			ret=2;
			vIsSet=true;	
		}else if (vMatID==L"lux_mattetranslucent") {
			s.GetColorParameterValue(L"kr",red,green,blue,alpha );
			f << "Texture \"Kr-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<red<<" "<<green<<" "<<blue<<"]\n";	
			s.GetColorParameterValue(L"kt",red,green,blue,alpha );
			f << "Texture \"Kt-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<red<<" "<<green<<" "<<blue<<"]\n";	
			f << "Texture \"sigma-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< (float)s.GetParameter(L"sigma").GetValue() <<"]\n";
			ret=10;
			vIsSet=true;	
		}
		
		if (!vIsSet) {
			if (s.GetParameter(L"refract_inuse").GetValue()=="-1") { 
			//check if material is transparent: phong/lamber/blin/constant/cooktorrance/strauss
			float ior=0.0f;
			s.GetColorParameterValue(L"transparency",red,green,blue,alpha );
			if (red>0 || green>0 || blue>0) {
				ret=1; // glass
				s.GetColorParameterValue(L"reflectivity",sp_red,sp_green,sp_blue,sp_alpha );
				ior = (float)s.GetParameter(L"index_of_refraction").GetValue();
				f << "Texture \"Kt-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
				f << "Texture \"Kr-" << sname << "\" \"color\" \"constant\" \"color value\" [" << sp_red << " " << sp_green << " " << sp_blue << "]\n";
				f << "Texture \"index-"<<  sname << "\" \"float\" \"constant\" \"float value\" [" << ior << "]\n";
				if ((float)s.GetParameter(L"trans_glossy").GetValue()>0 ) {
					f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"trans_glossy").GetValue() << "]\n";
					f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (float)s.GetParameter(L"trans_glossy").GetValue() << "]\n";
					ret=7; // roughglass
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
				ior = (float)s.GetParameter(L"refr_ior").GetValue();
				f << "Texture \"Kt-"<< sname <<"\" \"color\" \"constant\" \"color value\" ["<< red << " " <<green<<" "<<blue<<"]\n";
				f << "Texture \"Kr-"<< sname <<"\" \"color\" \"constant\" \"color value\" ["<< sp_red << " "<<sp_green<<" "<<sp_blue<<"]\n";
				f << "Texture \"index-"<< sname <<"\" \"float\" \"constant\" \"float value\" [" << ior <<"]\n";
				ret=1;  // glass
				if ((float)s.GetParameter(L"refr_gloss").GetValue()<1 ) {
					f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (1.0f-(float)s.GetParameter(L"refr_gloss").GetValue()) << "]\n";
					f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" [" << (1.0f-(float)s.GetParameter(L"refr_gloss").GetValue()) << "]\n";
					ret=7; // roughglass
				}
				vIsSet=true;
			}
		}
		
		
		if (!vIsSet) {
			if (s.GetParameter(L"reflect_inuse").GetValue()=="-1" || (float)s.GetParameter(L"reflectivity").GetValue()>0.0f){
				if (vMatID==L"mia_material_phen") { 
					float a,b,c,d;
					s.GetColorParameterValue(L"diffuse",a,b,c,d );
					
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
					// plastic
					//if (mRough==0) mRough=0.000001;
					f << "Texture \"Kd-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red << " " << green << " " << blue << "]\n";
					f << "Texture \"Ks-" << sname << "\" \"color\" \"constant\" \"color value\" [" << red <<" " << green << " " << blue << "]\n";
					f << "Texture \"uroughness-" << sname << "\" \"float\" \"constant\" \"float value\" ["<< mRough << "]\n";
					f << "Texture \"vroughness-" << sname << "\" \"float\" \"constant\" \"float value\" ["<< mRough << "]\n";
					
					ret=5;
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
			} else if (vMatID==L"mia_material_phen") {
				// arch vis
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
				mRough=(float)s.GetParameter(L"reflect_glossy").GetValue();
				f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<< red <<" "<< green<< " "<< blue<<"]\n";
				f << "Texture \"Ks-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<sp_red<<" "<<sp_green<<" "<<sp_blue<<"]\n";
				f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< mRough<<"]\n";
				f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< mRough<<"]\n";
				ret=2;
				
			} else if (vMatID==L"material-phong") {
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				s.GetColorParameterValue(L"specular",sp_red,sp_green,sp_blue,sp_alpha );
				
				f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<< red<<" "<<green<<" "<<blue<<"]\n";
				f << "Texture \"Ks-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<< sp_red<<" "<<sp_green<<" "<<sp_blue<<"]\n";
				f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< (float)(s.GetParameterValue(L"shiny"))/10<<"]\n";
				f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< (float)(s.GetParameterValue(L"shiny"))/10<<"]\n";
				ret=5;
				
			}  else if (vMatID==L"material-lambert"){
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<red<<" "<<green<<" "<<blue<<"]\n";	
				f << "Texture \"sigma-"<< sname << "\" \"float\" \"constant\" \"float value\" [0]\n";
				ret=4;
			} else if (vMatID==L"material-ward"){
				// anisotrop
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
				//s.GetColorParameterValue(L"glossy",sp_red,sp_green,sp_blue,alpha );
				f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<< red<<" "<<green<<" "<<blue<<"]\n";
				f << "Texture \"Ks-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<< red<<" "<<green<<" "<<blue<<"]\n";
				f << "Texture \"uroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< (float)(s.GetParameterValue(L"shiny_u"))/10<<"]\n";
				f << "Texture \"vroughness-"<< sname << "\" \"float\" \"constant\" \"float value\" ["<< (float)(s.GetParameterValue(L"shiny_v"))/10<<"]\n";
				ret=2;
			} else if (vMatID==L"material-constant"){
				s.GetColorParameterValue(L"color",red,green,blue,alpha );
				f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" ["<<red <<" " << green << " " << blue << "]\n";
				f << "Texture \"sigma-"<< sname << "\" \"float\" \"constant\" \"float value\" [0]\n";
				ret=4;
			} else if (vMatID==L"material-strauss"){
				s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
			} else {
				// fall back shader
				f << "Texture \"Kd-"<< sname << "\" \"color\" \"constant\" \"color value\" [0.7 0.7 0.7]\n";
				f << "Texture \"sigma-"<< sname << "\" \"float\" \"constant\" \"float value\" [0]\n";
				ret=4;
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
					
					f << "Texture \"Kd-"<< sname << "\" \"color\" \"imagemap\" \"string filename\" [\"" << vFileName.GetAsText().GetAsciiString() << "\"] \"float vscale\" [-1.0]";
					vText=true;
				}
			}
		}
		
		//
		// write shader block
		//
		
				
		CValue tmp;
		switch (ret) {
		case 1: tmp=L"\"texture Kr\" \"Kr-" + m.GetName()+L"\" \"texture Kt\" \"Kt-"+ m.GetName()+L"\" \"texture index\" \"index-"+ m.GetName()+L"\" \"texture cauchyb\" \"cauchyb-"+ m.GetName()+L"\"";break;
		case 2: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\"";break;
		case 3: tmp=L"\"texture Kr\" \"Kr-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\"";break;
		case 4: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture sigma\" \"sigma-"+ m.GetName()+L"\"";break;
		case 5: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\"";break;
		case 6: tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks1\" \"Ks1-"+m.GetName()+ L"\" \"texture Ks2\" \"Ks2-"+m.GetName()+ L"\" \"texture Ks3\" \"Ks3-"+m.GetName()+ L"\" \"texture R1\" \"R1-"+m.GetName()+ L"\" \"texture R2\" \"R2-"+m.GetName()+ L"\" \"texture R3\" \"R3-"+m.GetName()+ L"\" \"texture M1\" \"M1-"+m.GetName()+ L"\" \"texture M2\" \"M2-"+m.GetName()+ L"\" \"texture M3\" \"M3-"+m.GetName()+ L"\"";break;
		case 7: tmp=L"\"texture Kr\" \"Kr-" + m.GetName()+L"\" \"texture Kt\" \"Kt-"+ m.GetName()+L"\" \"texture index\" \"index-"+ m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\" \"texture cauchyb\" \"cauchyb-"+ m.GetName()+L"\"";break;
		case 8: tmp=L"\"texture Kr\" \"Kr-" + m.GetName()+L"\"";break;
		case 9: tmp=L"\"texture name\" \"name-" + m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"roughness-"+m.GetName()+L"\"";break;
		case 10: tmp=L"\"texture Kr\" \"Kr-"+m.GetName()+ L"\" \"texture Kt\" \"Kt-"+m.GetName()+ L"\" \"texture sigma\" \"sigma-"+ m.GetName()+L"\"";break;
		}
		
		f << "MakeNamedMaterial \""<< m.GetName().GetAsciiString() << "\" \"string type\" [\""<< aShader[ret].GetAsText().GetAsciiString() <<"\"] " << tmp.GetAsText().GetAsciiString() << " " << addString.GetAsciiString()  << "\n\n";
		
		
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
	CDoubleArray points;
	CFloatArray normals;
	CLongArray tris,nod,out;
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
	CString vNormals=L"",vTris=L"",vUV=L"",vMod=L"",vPoints=L"";
	
	int vSubDValue=1;
	
	Property geopr=o.GetProperties().GetItem(L"Geometry Approximation");
	if ((int)geopr.GetParameterValue(L"gapproxmordrsl")>0 || (int)geopr.GetParameterValue(L"gapproxmosl")>0) {
		vIsSubD=true;
		if ((int)geopr.GetParameterValue(L"gapproxmordrsl")>0) {
			vSubDValue=(int)geopr.GetParameterValue(L"gapproxmordrsl");
		} else {
			vSubDValue=(int)geopr.GetParameterValue(L"gapproxmosl");
		}
	} else {
		vIsSubD=false;
	}
	
	if (vIsSubD==true) {
		// Get Subd Object
		ga = PolygonMesh(g).GetGeometryAccessor(siConstructionModeModeling,siCatmullClark,vSubDValue,true,false);
	} else {
		// Get Object
		ga = PolygonMesh(g).GetGeometryAccessor();
	}
	
	ga.GetVertexPositions(points);
	ga.GetTriangleVertexIndices(tris); 
	ga.GetTriangleNodeIndices(nod); 
	ga.GetNodeNormals(normals);
	
	CLongArray pvCount;
	CLongArray vIndices;
	ga.GetVertexIndices(vIndices);
	
	ga.GetPolygonVerticesCount(pvCount);
	long nPolyCount = ga.GetPolygonCount();
	
	
	//int shader;
	if (points.GetCount()>0 || tris.GetCount()>0) {
		f << "# Object : " << (o.GetName()).GetAsciiString() ;
		f << "\nAttributeBegin";
		
		
		if ((float)s.GetParameter(L"inc_inten").GetValue()>0) {
			// check for Meshlight
			vIsMeshLight=true;
			float red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f;
			s.GetColorParameterValue(L"incandescence",red,green,blue,alpha ); 
			f << "\nAreaLightSource \"area\" \"integer nsamples\" [1] \"color L\" ["<<(red*(float)s.GetParameterValue(L"inc_inten")) <<" "<<(green*(float)s.GetParameterValue(L"inc_inten")) <<" "<<(blue*(float)s.GetParameterValue(L"inc_inten"))<<"]\n";
		} else {
			//shader = writeLuxsiShader(o);
			f << "\nNamedMaterial \""<< m.GetName().GetAsciiString() <<"\"\n";
		}
		
		if (vType==L"obj"){
			
			//
			// write triangles
			//
			CTransformation localTransformation = ga.GetTransform();
			KinematicState  gs = o.GetKinematics().GetGlobal();
			//KinematicState  gs = o.GetKinematics().GetLocal();
			CTransformation gt = gs.GetTransform();
			
			CVector3Array ap(nod.GetCount());
			CVector3Array an(nod.GetCount());
			CVector3Array au(nod.GetCount());
			
			CFloatArray uvValues;
			CRefArray uvs = ga.GetUVs( );
			ClusterProperty uvProp = uvs[ 0 ];
			uvProp.GetValues( uvValues );
			
			
			long high=0;
			for (long j=0; j<nod.GetCount(); j+=3){
				
				
				vTris += L" " + CString(nod[j]) + L" "+CString(nod[j+1])+ L" "+ CString(nod[j+2])+L"\n";
				if (nod[j]>high) {high=nod[j];}
				if (nod[j+1]>high) {high=nod[j+1];}
				if (nod[j+2]>high) {high=nod[j+2];}
				
				CVector3 n1(normals[nod[j]*3],normals[nod[j]*3+1],normals[nod[j]*3+2]);
				CVector3 n2(normals[(nod[j+1])*3],normals[(nod[j+1])*3+1],normals[(nod[j+1])*3+2]);
				CVector3 n3(normals[(nod[j+2])*3],normals[(nod[j+2])*3+1],normals[(nod[j+2])*3+2]);
				
				
				ap[nod[j]]=CVector3(points[tris[j]*3],points[tris[j]*3+1],points[tris[j]*3+2]);
				ap[nod[j+1]]=CVector3(points[tris[j+1]*3],points[tris[j+1]*3+1],points[tris[j+1]*3+2]);
				ap[nod[j+2]]=CVector3(points[tris[j+2]*3],points[tris[j+2]*3+1],points[tris[j+2]*3+2]);
				
				
				au[nod[j]]= CVector3(uvValues[ nod[j]*3 ],uvValues[ nod[j]*3 + 1 ],uvValues[ nod[j]*3 + 2 ]);
				au[nod[j+1]]= CVector3(uvValues[ nod[j+1]*3 + 0 ],uvValues[ nod[j+1]*3 + 1 ],uvValues[ nod[j+1]*3 + 2 ]);
				au[nod[j+2]]= CVector3(uvValues[ nod[j+2]*3 + 0 ],uvValues[ nod[j+2]*3 + 1 ],uvValues[ nod[j+2]*3 + 2 ]);
				
				an[nod[j]]=n1;
				an[nod[j+1]]=n2;
				an[nod[j+2]]=n3;
				
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
			
			
			for (long j=0;j<=high;j++){
				CVector3 pos1(ap[j]);
				CVector3 pos(MapObjectPositionToWorldSpace(  gt,  pos1));
				CVector3 norm(an[j]);
				CVector3 uvs(au[j]);
				vPoints +=  L" "+ CString(pos[0]) + L" "+  CString(-pos[2]) + L" "+ CString(pos[1])+L"\n"; 
				vNormals +=  L" "+ CString(norm[0]) + L" "+  CString(-norm[2]) + L" "+ CString(norm[1])+L"\n";
				if (vText) vUV += L" "+ CString(uvs[0]) + L" "+  CString(uvs[1])+L"\n";
			}
			
			
			//
			// write 
			//
			
			string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
			if (loc != string::npos) {
				sLight << " PortalShape ";
				sLight << " \"trianglemesh\" \"integer indices\" [\n";
				sLight << vTris.GetAsciiString();
				sLight << " ] \"point P\" [\n" ;
				sLight << vPoints.GetAsciiString();
				sLight << "] \"normal N\" [\n";
				sLight << vNormals.GetAsciiString();
				sLight << "]\n";
			} else {
				f << " Shape ";
				
				f << " \"trianglemesh\" \"integer indices\" [\n";
				f << vTris.GetAsciiString();
				f << " ] \"point P\" [\n" ;
				f << vPoints.GetAsciiString();
				f << "] \"normal N\" [\n";
				f << vNormals.GetAsciiString();
				if(vText){
					f << "] \"float uv\" [\n";
					f << vUV.GetAsciiString();
				}
				f << "]";
			}
			f << "\nAttributeEnd\n";
		}
	}
	return 0;
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
		
		aObj.Clear();
		aSurfaces.Clear();
		aLight.Clear();
		aCam.Clear();
		aClouds.Clear();
		aInstance.Clear();
		
		array += root.FindChildren( L"", L"", emptyArray, true );
		for ( int i=0; i<array.GetCount();i++ ){
			X3DObject o(array[i]);
			//app.LogMessage( L"\tObject name: " + o.GetName() + L":" +o.GetType() );
			Property visi=o.GetProperties().GetItem(L"Visibility");
			// Collection objects
			if (o.GetType()==L"polymsh"){
				if (vIsHiddenObj || (vIsHiddenObj==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aObj.Add(o); 	// visibilty check
			}
			if (o.GetType()==L"CameraRoot"){
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
				// instances
				CRefArray foundsrcs = Model(o).GetGroups();
				
				if ( foundsrcs.GetCount() > LONG(0) ) {
					if (vIsHiddenObj || (vIsHiddenObj==false && ((bool)visi.GetParameterValue(L"viewvis")==true && (bool)visi.GetParameterValue(L"rendvis")==true))) aInstance.Add(o); 	// visibilty check
				}
			}
		}
		
		if (aObj.GetCount()==0) {
			app.LogMessage(L"No objects are exported! Check if objects are visible in XSI or switch on 'export hidden objects'",siFatalMsg );
			
		} else if (aCam.GetCount()==0) {
			app.LogMessage(L"No cameras are exported! Check if a camera is visible in XSI or switch on 'export hidden cameras'",siFatalMsg );
			
		} else {
			
			app.LogMessage( L"\tObject count: "+CString(aObj.GetCount()));
			pb.PutValue(0);
			pb.PutMaximum( aObj.GetCount() );
			pb.PutStep(1);
			pb.PutVisible( true );
			pb.PutCaption( L"Processing" );
			pb.PutCancelEnabled(true); 
			
			f.open (vFileObjects.GetAsciiString());
			
			for (int i=0;i<aLight.GetCount();i++) writeLuxsiLight(aLight[i]);

			for (int i=0;i<aCam.GetCount();i++) writeLuxsiCam(aCam[i]);
			
			writeLuxsiBasics();
			
			
			//
			// write to file
			//

			f << "AttributeBegin\n"<< sLight.str() <<"AttributeEnd\n\n";	
			sLight.flush();
			
			writeLuxsiShader();
			
			for (int i=0;i<aObj.GetCount();i++) {
				if (writeLuxsiObj(aObj[i],L"obj")==-1) break;
				if (pb.IsCancelPressed() ) break;
				pb.Increment();
			}
			pb.PutVisible( false );
			f << "WorldEnd";
			f.close();
		}
	} else {
		app.LogMessage(L"Filename is empty",siErrorMsg );
	}
}
