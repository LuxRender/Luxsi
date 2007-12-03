/*
LuXSI - Softimage XSI Export plug-in for the LuxRender (http://www.luxrender.org) renderer
		compiled version at: http://www.migaweb.de


Copyright (C) 2007  Michael Gangolf

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

using namespace XSI; 
using namespace MATH; 
using namespace std;


void writeLuxsiBasics();
void writeLuxsiCam(X3DObject o);
void writeLuxsiLight(X3DObject o);
int writeLuxsiObj(X3DObject o);
void writeClouds(X3DObject o);
int writeLuxsiShader(X3DObject o);

void luxsi_write();
void update_LuXSI_values(CString   paramName, Parameter changed,PPGLayout lay);


Application app;
Model root( app.GetActiveSceneRoot() );
std::ofstream f;
std::stringstream sLight, sObj, sMat;

Null null;
CustomProperty prop ;

int vXRes=640,vYRes=480,vFilt=1,vMaxDepth=256,vSamples=4,vSave=120,vTone=1,vMaxrej=128,vSampler=0;
bool vMLT=false,vIsLinux=true,vIsGI=true,vRrft=true,vBloom=false, vProg=true,vIsCaustic=false,vIsHiddenCam=false,vIsHiddenLight=false,vIsHiddenObj=false;
bool vUseJitter=true,vIsHiddenSurfaces=false,vSFPreview=true,vIsHiddenClouds=false,vExpOne=true,vAmbBack=false,vExr=false,vIgi=false,vTga=true;
float vTPre=1.0f,vTPos=1.0f,vTBurn=6.0f,vCSize=0.4f,vGITolerance=0.025f,vSpacingX=0.1f,vSpacingY=0.1f,vContrast=2.0f,vDither=0.5f,vBloomWidth=0.1f,vBloomRadius=0.1f,vLmprob=0.25f;

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
	in_reg.PutVersion(0,3);
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
	//prop.AddParameter(L"use_hidden_surf",CValue::siBool,siPersistable,L"",L"",vIsHiddenSurfaces,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"use_hidden_light",CValue::siBool,siPersistable,L"",L"",vIsHiddenLight,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"use_hidden_cam",CValue::siBool,siPersistable,L"",L"",vIsHiddenCam,CValue(),CValue(),CValue(),CValue(),oParam);
	//prop.AddParameter(L"use_hidden_clouds",CValue::siBool,siPersistable,L"",L"",vIsHiddenClouds,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"exp_one",CValue::siBool,siPersistable,L"",L"",vExpOne,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"Width",CValue::siInt4,siPersistable,L"",L"",vXRes,0l,2048l,0l,1024l,oParam);
	prop.AddParameter(L"Height",CValue::siInt4,siPersistable,L"",L"",vYRes,0l,2048l,0l,768l,oParam);
	prop.AddParameter(L"gamma",CValue::siFloat,siPersistable,L"",L"",vContrast,0,10,0,3,oParam);
	prop.AddParameter(L"max_depth",CValue::siInt4,siPersistable,L"",L"",vMaxDepth,0,4096,0,1024,oParam);
	prop.AddParameter(L"samples",CValue::siInt4,siPersistable,L"",L"",vSamples,0,100,0,10,oParam);
	prop.AddParameter(L"progressive",CValue::siBool,siPersistable,L"",L"",vProg,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"rrft",CValue::siBool,siPersistable,L"",L"",vRrft,CValue(),CValue(),CValue(),CValue(),oParam);
	
        prop.AddParameter( L"Filt", CValue::siInt4, siPersistable, L"", L"",vFilt, oParam ) ;
	prop.AddParameter( L"sampler", CValue::siInt4, siPersistable, L"", L"",vSampler, oParam ) ;
	prop.AddParameter( L"tone", CValue::siInt4, siPersistable, L"", L"",vTone, oParam ) ;
	prop.AddParameter(L"dither",CValue::siFloat,siPersistable,L"",L"",vDither,0,10,0,1,oParam);
	
	prop.AddParameter(L"Bloom",CValue::siBool,siPersistable,L"",L"",vBloom,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"bWid",CValue::siFloat,siPersistable,L"",L"",vBloomWidth,0l,1l,0l,1l,oParam);
	prop.AddParameter(L"bRad",CValue::siFloat,siPersistable,L"",L"",vBloomRadius,0l,1l,0l,1l,oParam);
	
	prop.AddParameter(L"tBurn",CValue::siFloat,siPersistable,L"",L"",vTBurn,0l,10l,0l,10l,oParam);
	prop.AddParameter(L"tPre",CValue::siFloat,siPersistable,L"",L"",vTPre,0l,10l,0l,10l,oParam);
	prop.AddParameter(L"tPos",CValue::siFloat,siPersistable,L"",L"",vTPos,0l,10l,0l,10l,oParam);
	
	prop.AddParameter(L"savint",CValue::siInt4,siPersistable,L"",L"",vSave,0l,200l,0l,200l,oParam);
	prop.AddParameter(L"AmbBack",CValue::siBool,siPersistable,L"",L"",vAmbBack,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"save_exr",CValue::siBool,siPersistable,L"",L"",vExr,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"save_igi",CValue::siBool,siPersistable,L"",L"",vIgi,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"save_tga",CValue::siBool,siPersistable,L"",L"",vTga,CValue(),CValue(),CValue(),CValue(),oParam);
	
	prop.AddParameter(L"mlt",CValue::siBool,siPersistable,L"",L"",vMLT,CValue(),CValue(),CValue(),CValue(),oParam);
	prop.AddParameter(L"maxrej",CValue::siInt4,siPersistable,L"",L"",vMaxrej,0l,2048l,0l,256l,oParam);
	prop.AddParameter(L"lmprob",CValue::siFloat,siPersistable,L"",L"",vLmprob,0l,10l,0l,10l,oParam);
	
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
	lay.AddRow();
	lay.AddItem(L"dither",L"Dither");
	lay.AddItem(L"samples",L"Samples");
	lay.EndRow();
	
	lay.AddRow();
	CValueArray vItem2(10);
        vItem2[0] = L"box" ; vItem2[1] = 0;
	vItem2[2] = L"gaussian" ; vItem2[3] = 1;
	vItem2[4] = L"mitchell" ; vItem2[5] = 2;
	vItem2[6] = L"sinc" ; vItem2[7] = 3;
	vItem2[8] = L"triangle" ; vItem2[9] = 4;
	lay.AddEnumControl(L"Filt",vItem2,L"Filter",siControlCombo ) ;
	
	CValueArray vItem3(8);
        vItem3[0] = L"lowdiscrepancy" ; vItem3[1] = 0;
	vItem3[2] = L"random" ; vItem3[3] = 1;
	vItem3[4] = L"stratfied" ; vItem3[5] = 2;
	vItem3[6] = L"bestcandidate" ; vItem3[7] = 3;
	lay.AddEnumControl(L"sampler",vItem3,L"Sampler",siControlCombo ) ;
	lay.EndRow();
	lay.EndGroup();
	
	
	lay.AddGroup(L"Export hidden...");
	lay.AddItem(L"use_hidden_obj", L"Objects");
	//lay.AddItem(L"use_hidden_surf", L"Surfaces");
	lay.AddItem(L"use_hidden_cam", L"Cameras");
	lay.AddItem(L"use_hidden_light", L"Lights");
	//lay.AddItem(L"use_hidden_clouds", L"Clouds");
	lay.EndGroup();
	
	
	lay.AddGroup(L"Save as");
	lay.AddItem(L"save_tga", L"TGA");
	lay.AddItem(L"save_exr", L"EXR");
	lay.AddItem(L"save_igi", L"IGI");
	lay.AddItem(L"savint",L"Interval");
	lay.EndGroup();
	
	lay.AddItem(L"fObjects",L"Filename",siControlFilePath);
	
	PPGItem it = lay.GetItem( L"fObjects" );
	it.PutAttribute( siUIFileFilter, L"LuXSI Scenes|*.lxs" ) ;
	lay.AddRow();
	Command cmd();
	lay.AddButton(L"exe_luxsi",L"Export");
	lay.EndRow();
	
	
	lay.AddTab(L"Render settings");
	
	lay.AddItem(L"max_depth",L"Max depth");
	
	
	lay.AddItem(L"progressive",L"Progressive");
	lay.AddItem(L"rrft",L"RRFTrasmit");
	lay.AddItem(L"AmbBack",L"Use Ambience as background");
	
	lay.AddGroup(L"MLT");
	lay.AddItem(L"mlt",L"enable MLT");
	lay.AddItem(L"maxrej",L"Max rejects");
	lay.AddItem(L"lmprob",L"LMprob");
	
	lay.EndGroup();
	
	lay.AddGroup(L"Tonemapping");
	CValueArray vItem4(10);
        vItem4[0] = L"none" ; vItem4[1] = 0;
	vItem4[2] = L"Reinhard" ; vItem4[3] = 1;
	vItem4[4] = L"maxwhite" ; vItem4[5] = 2;
	vItem4[6] = L"highcontrast" ; vItem4[7] = 3;
	vItem4[8] = L"nonlinear" ; vItem4[9] = 4;
	
	lay.AddEnumControl(L"tone",vItem4,L"Methode",siControlCombo ) ;
	lay.AddRow();
	lay.AddItem(L"tPre",L"Prescale");
	lay.AddItem(L"tPos",L"Postscale");
	lay.EndRow();
	lay.AddItem(L"tBurn",L"Burn");
	lay.EndGroup();
	
	lay.AddGroup(L"Bloom");
	lay.AddItem(L"Bloom",L"enable");
	lay.AddRow();
	lay.AddItem(L"bWid",L"Width");
	lay.AddItem(L"bRad",L"Radius");
	lay.EndRow();
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
	}  else if (paramName==L"Width"){
		vBloomWidth=changed.GetValue();
	}  else if (paramName==L"Radius"){
		vBloomRadius=changed.GetValue();
	}  else if (paramName==L"Bloom"){
		vBloom=changed.GetValue();
	} else if (paramName==L"savint"){
		vSave=changed.GetValue();
	} else if (paramName==L"tBurn"){
		vTBurn=changed.GetValue();
	} else if (paramName==L"tPre"){
		vTPre=changed.GetValue();
	} else if (paramName==L"tPos"){
		vTPos=changed.GetValue();
	} else if (paramName==L"tone"){
		vTone=changed.GetValue();
	} else if (paramName==L"mlt"){
		vMLT=changed.GetValue();
	}else if (paramName==L"AmbBack"){
		vAmbBack=changed.GetValue();
	} else if (paramName==L"maxrej"){
		vMaxrej=changed.GetValue();
	} else if (paramName==L"lmprob"){
		vLmprob=changed.GetValue();
	} else if (paramName==L"save_igi"){
		vIgi=changed.GetValue();
	} else if (paramName==L"save_exr"){
		vExr=changed.GetValue();
	} else if (paramName==L"save_tga"){
		vTga=changed.GetValue();
	} else if (paramName==L"sampler"){
		vSampler=changed.GetValue();
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
		args[3] = siLock ; 
		args[4] = false ;
		
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
	char aSampler[4][20]={"lowdiscrepancy","random","stratified","bestcandidate"};
	
	string fname=vFileObjects.GetAsciiString();
	int loc=fname.rfind(".");
	
	f << "Film \"multiimage\"\n";
	f << "  \"integer xresolution\" [" <<  vXRes << "] \"integer yresolution\" [" <<  vYRes<<"]\n";
	
	if (vExr){
		f << "      \"string hdr_filename\" [\""<< fname.substr(0,loc) << ".exr\"]\n";
		f << "        \"integer hdr_writeinterval\" ["<< vSave << "]\n";
	}
	if (vIgi){
		f << "      \"string igi_filename\" [\""<< fname.substr(0,loc) << ".igi\"]\n";
		f << "        \"integer igi_writeinterval\" ["<< vSave << "]\n";
	}
	if (vTga){
		f << "      \"string ldr_filename\" [\""<< fname.substr(0,loc) << ".tga\"]\n";
		f << "        \"integer ldr_writeinterval\" ["<< vSave << "]\n";
	}
	f << "        \"integer ldr_displayinterval\" [10]\n";
	
	switch(vTone) {
	case 0: break;
	case 1:
		f << "  \"string tonemapper\" [\"reinhard\"] \"float reinhard_prescale\" [1.0] \"float reinhard_postscale\" [1.0] \"float reinhard_burn\" [6.0]\n";
		break;
	case 2:
		f << "  \"string tonemapper\" [\"maxwhite\"]\n";
		break;
	case 3:
		f << "  \"string tonemapper\" [\"highcontrast\"]\n";
		break;
	case 4:
		f << "  \"string tonemapper\" [\"nonlinear\"] \"float nonlinear_maxY\" [0.03]";
		break;
	}
	
	if (vBloom) {
		f << "    \"float bloomwidth\" ["<< vBloomWidth<<"]\n";
		f << "    \"float bloomradius\" ["<< vBloomRadius<<"]\n";	
	}
	f << "      \"float gamma\" [" << vContrast << "]\n";
	f << "      \"float dither\" ["<< vDither <<"]\n";
	f << "PixelFilter \""<< aFilter[vFilt] << "\" \"float xwidth\" [2.000000] \"float ywidth\" [2.000000]\n";
	f << "Sampler \""<< aSampler[vSampler] <<"\" \"bool progressive\" [\"" << aBool[vProg] << "\"]\n";
	
	if (vSampler==0) {
		f << "\"integer pixelsamples\" ["<< vSamples <<"]\n";
	}
	
	f << "SurfaceIntegrator \"path\" \"integer maxdepth\" ["<< vMaxDepth << "] \"bool rrforcetransmit\" [\""<< aBool[vRrft] <<"\"] \"float rrcontinueprob\" [0.500000]\n";
	if (vMLT) {
		f << "\"bool metropolis\" [\"true\"] \"float largemutationprob\" ["<< vLmprob <<"] \"integer maxconsecrejects\" ["<< vMaxrej <<"]\n";
	}
	f << "Accelerator \"kdtree\"\n";
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
		f << "AttributeEnd\n";
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
		f << "AttributeEnd\n";	
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
	
	
	
	
	// lookat: posX posY posZ targetX targetY targetZ upX upY upZ
	f << "LookAt " << CString(gt.GetTranslation().GetX()).GetAsciiString() << " " <<CString(gt.GetTranslation().GetY()).GetAsciiString() << " " << CString(gt.GetTranslation().GetZ()).GetAsciiString() << " " << CString(ci_gt.GetTranslation().GetX()).GetAsciiString() << " " <<CString(ci_gt.GetTranslation().GetY()).GetAsciiString()  << " " <<CString(ci_gt.GetTranslation().GetZ()).GetAsciiString()  << " 0 " << CString(up.GetY()*up.GetY()).GetAsciiString() << " 0 \n";
	f << "Camera \"perspective\" \"float fov\" [" << (c.GetParameterValue(CString(L"fov")).GetAsText()).GetAsciiString() << "] \"float lensradius\" ["<< CString(vLensr).GetAsciiString()  <<"] \"float focaldistance\" ["<< CString(vFdist).GetAsciiString() <<"]"; 
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
	X3DObject ci(o.GetChildren()[1]);
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
		// calculate interest
		X3DObject li;
		CVector3 intPos;
		CVector3 tranlation(0,0,-1);
		CTransformation target=localTransformation.AddLocalTranslation(tranlation);
		
		
		CVector3 tranlation1(0,-1,0);
		CTransformation up=o.GetKinematics().GetGlobal().GetTransform().AddLocalTranslation(tranlation1);
		
		CVector3 tranlation2(1,0,0);
		CTransformation east=o.GetKinematics().GetGlobal().GetTransform().AddLocalTranslation(tranlation2);
		
		KinematicState  gs2 = o.GetKinematics().GetGlobal();
		CTransformation gt2 = gs2.GetTransform();
		MapObjectPoseToWorldSpace(  gt2, up);
		intPos=up.GetTranslation();
		lMore=L"Rotate 90 1 0 0\nRotate " + CString(intPos.GetX()) + L" 1 0 0 \n Rotate " + CString(intPos.GetY()) +L" 0 1 0 \n Rotate "+ CString(intPos.GetZ())+L" 0 0 1";
		lRow1=L"LightSource \"sunsky\" \"integer nsamples\" [ 4 ]\n";
		lRow2=L"\"vector sundir\" [ 0 0 1 ]";
		lRow3=L"\"float turbidity\" [2.000000]\n";
		
		
		
	} else {
		//
		// Pointlight
		//
		lRow1=L"LightSource \"infinite\" \"color L\" ["+ CString(a) + L"  " + CString(b) + L"  " +  CString(c)+L"] \"integer nsamples\" [1]";
		
	}
	
	
	sLight <<"AttributeBegin \n";
	if (lMore.GetAsText()!=L"") {
		sLight << lMore.GetAsText().GetAsciiString()<< " \n";
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
	sLight <<"AttributeEnd\n\n";	
}




CRef miga_findShader(CRefArray in_spShaderArray,CString in_strShaderName)
{
	CRefArray l_spFilteredShaderArray;
	in_spShaderArray.Filter(L"",CStringArray(),in_strShaderName,l_spFilteredShaderArray);
	if(l_spFilteredShaderArray.GetCount()>0)
		return l_spFilteredShaderArray[0];
	
	for(long i=0;i<in_spShaderArray.GetCount();i++)
	{
		Shader spShader(in_spShaderArray[i]);
		Shader l_spShader  = miga_findShader(l_spShader.GetShaders(),in_strShaderName);
		if(l_spShader != null)
			return l_spShader;
	}
	return CRef();
}


int writeLuxsiShader(X3DObject o){
	//
	// Writes shader
	//
	Texture vTexture;
	bool vIsSet=false;
	bool vText=false,vIsSubD=true;
	bool vIsMod=false;
	float b_red=0.0f,b_green=0.0f,b_blue=0.0f,b_alpha=0.0f,red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f,sp_red=0.0f,sp_green=0.0f,sp_blue=0.0f,sp_alpha=0.0f,vModScale=0.0f,refl_red=0.0f,refl_green=0.0f,refl_blue=0.0f,refl_alpha=0.0f;
	ImageClip2 vBumpFile;
	Shader vBumpTex;
	CValue vDiffType,vMore,vCol,vMore2,vMore3,vTexStr,vMore1,vMore4,vMore5;
	CString vNormals=L"",vTris=L"",vUV=L"",vMod=L"";
	
	CRefArray mats(o.GetMaterials()); // Array of all materials of the object
	Material m=mats[0];
	CRefArray shad(m.GetShaders());	// Array of all shaders attached to the material [e.g. phong]
	Shader s(shad[0]);
	CString vMatID((s.GetProgID()).Split(L".")[1]);
	int ret=0;
	
	CStatus vIsNewShader;
	vIsNewShader=find(aMatList,m.GetName());
	
	
	
	if (s.GetParameter(L"refract_inuse").GetValue()=="-1" || (float)s.GetParameter(L"transparency").GetValue()>0.0f){
		
		s.GetColorParameterValue(L"transparency",red,green,blue,alpha );
		s.GetColorParameterValue(L"refr_color",b_red,b_green,b_blue,b_alpha );
		
		
		
		if (red>0 || green>0 || blue>0 || b_red>0 || b_green>0 || b_blue>0) {
			//glass
			float ior=0.0f;
			if (vMatID==L"mia_material_phen") { 
				s.GetColorParameterValue(L"refr_color",red,green,blue,alpha );
				s.GetColorParameterValue(L"refr_color",sp_red,sp_green,sp_blue,sp_alpha );
				ior = (float)s.GetParameter(L"refr_ior").GetValue();
				
			} else {
				s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
				ior = (float)s.GetParameter(L"index_of_refraction").GetValue();
			}
			
			
			vMore1= L"Texture \"Kt-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(red)+L" "+CString(green)+L" "+CString(blue)+L"]\n";
			vMore2= L"Texture \"Kr-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(sp_red)+L" "+CString(sp_green)+L" "+CString(sp_blue)+L"]\n";
			vMore3= L"Texture \"index-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" ["+CString(ior)+L"]\n";
			ret=1;
			vIsSet=true;
		}
	}
	
	if (!vIsSet) {
		if (s.GetParameter(L"reflect_inuse").GetValue()=="-1" || (float)s.GetParameter(L"reflectivity").GetValue()>0.0f){
			if (vMatID==L"mia_material_phen") { 
				float a,b,c,d;
				s.GetColorParameterValue(L"diffuse",a,b,c,d );
				
				s.GetColorParameterValue(L"refl_color",red,green,blue,alpha );
				red=red*a;
				green=green*b;
				blue=blue*c;
				
			} else {
				s.GetColorParameterValue(L"reflectivity",red,green,blue,alpha );
			}
			if (red>0 || green>0 || blue>0) {
				//mirror
				vMore1= L"Texture \"Kr-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(red)+L" "+CString(green)+L" "+CString(blue)+L"]\n";
				ret=3;
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
		
			
			
			
			if (string(CString(s.GetName()).GetAsciiString()).find( "FORDF8", 0 ) != string::npos) {
				//
			} else if (string(CString(s.GetName()).GetAsciiString()).find( "POLARIS", 0 ) != string::npos) {
				//
			}else if (string(CString(s.GetName()).GetAsciiString()).find( "OPELTITAN", 0 ) != string::npos) {
				//
			}else if (string(CString(s.GetName()).GetAsciiString()).find( "BMW339", 0 ) != string::npos) {
				//
			}else if (string(CString(s.GetName()).GetAsciiString()).find( "2KACRYLACK", 0 ) != string::npos) {
				//
			}else if (string(CString(s.GetName()).GetAsciiString()).find( "WHITE", 0 ) != string::npos) {
				//
			}else if (string(CString(s.GetName()).GetAsciiString()).find( "BLUE", 0 ) != string::npos) {
				vMore1=L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" [0.0079 0.023 0.1]\n \       
				Texture \"Ks1-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" [0.0011 0.0015 0.0019]\n  \
				Texture \"Ks2-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" [0.025 0.03 0.043]\n      \
				Texture \"Ks3-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" [0.059 0.074 0.082]\n      \
				Texture \"R1-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [1.000]\n \
				Texture \"R2-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.094]\n \
				Texture \"R3-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.170]\n \
				Texture \"M1-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.150]\n \
				Texture \"M2-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.043]\n \
				Texture \"M3-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.020]";  

			}else if (string(CString(s.GetName()).GetAsciiString()).find( "BLUEMATTE", 0 ) != string::npos) {
				//
			} else {
				float spr,spg,spb,spa,spr2,spg2,spb2,spa2,r,g,b,a;
				s.GetColorParameterValue(L"spec",spr,spg,spb,spa );
				s.GetColorParameterValue(L"spec_sec",spr2,spg2,spb2,spa2 );
				s.GetColorParameterValue(L"base_color",r,g,b,a );
				
				
				vMore1=L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(r)+L" "+CString(g)+L" "+CString(b)+L"]\n \       
				Texture \"Ks1-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(spr)+L" "+CString(spg)+L" "+CString(spb)+L"]\n  \
				Texture \"Ks2-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(spr2)+L" "+CString(spg2)+L" "+CString(spb2)+L"]\n      \
				Texture \"Ks3-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(spr2)+L" "+CString(spg2)+L" "+CString(spb2)+L"]\n      \
				Texture \"R1-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [1.000]\n \
				Texture \"R2-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.094]\n \
				Texture \"R3-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.170]\n \
				Texture \"M1-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.150]\n \
				Texture \"M2-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.043]\n \
				Texture \"M3-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0.020]";  
			}
			ret=6;
			
		} else if (vMatID==L"mia_material_phen") {
			// arch vis
			s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
			s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
			
			vMore1= L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(red)+L" "+CString(green)+L" "+CString(blue)+L"]";
			vMore2= L"Texture \"Ks-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(sp_red)+L" "+CString(sp_green)+L" "+CString(sp_blue)+L"]";
			vMore3= L"Texture \"roughness-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" ["+CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]";
			ret=5;
			
		} else if (vMatID==L"material-phong") {
			s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
			s.GetColorParameterValue(L"specular",sp_red,sp_green,sp_blue,sp_alpha );
			
			vMore1= L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(red)+L" "+CString(green)+L" "+CString(blue)+L"]";
			vMore2= L"Texture \"Ks-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(sp_red)+L" "+CString(sp_green)+L" "+CString(sp_blue)+L"]";
			vMore3= L"Texture \"roughness-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" ["+CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]";
			ret=5;
			
		}  else if (vMatID==L"material-lambert"){
			s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
			vMore1= L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" ["+CString(red)+L" "+CString(green)+L" "+CString(blue)+L"]";	
			vMore2= L"Texture \"sigma-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0]";
			ret=4;
		} else if (vMatID==L"material-ward"){
			
		} else if (vMatID==L"XSIAmbientOcclusion"){
			
		}
		
		else if (vMatID==L"material-constant"){
			s.GetColorParameterValue(L"color",red,green,blue,alpha );
		} else if (vMatID==L"material-strauss"){
			s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
		} else {
			// fall back shader
			vMore1= L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"constant\" \"color value\" [0.7 0.7 0.7]";
			vMore2= L"Texture \"sigma-"+ m.GetName() +L"\" \"float\" \"constant\" \"float value\" [0]";
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
				
				vMore1=L"Texture \"Kd-"+ m.GetName() +L"\" \"color\" \"imagemap\" \"string filename\" [\""+vFileName.GetAsText()+L"\"] \"float vscale\" [-1.0]";
				vText=true;
			}
		}
	}
	
	//
	// write shader block
	//
	if (vIsNewShader==false){
		if (vMore1.GetAsText()!=L"") {
			sMat << vMore1.GetAsText().GetAsciiString()<<"\n";
		}
		if (vMore2.GetAsText()!=L"") {
			sMat << vMore2.GetAsText().GetAsciiString()<<"\n";
		}
		if (vMore3.GetAsText()!=L"") {
			sMat << vMore3.GetAsText().GetAsciiString()<<"\n";
		}
		if (vMore4.GetAsText()!=L"") {
			sMat << vMore4.GetAsText().GetAsciiString()<<"\n";
		}
		if (vMore5.GetAsText()!=L"") {
			sMat << vMore5.GetAsText().GetAsciiString()<<"\n";
		}
		aMatList.Add(m.GetName());
		sMat << "\n";
	}
	return ret;
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
	float red=0.0f,green=0.0f,blue=0.0f,alpha=0.0f;
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
	LONG nPolyCount = ga.GetPolygonCount();
	
	CValueArray aShader(6);
	aShader[0] = L"matte" ;
        aShader[1] = L"glass" ;
	aShader[2] = L"substrate";
	aShader[3] = L"mirror";
	aShader[4] = L"matte";
	aShader[5] = L"plastic";
	aShader[6] = L"carpaint";
	int shader;
	if (points.GetCount()>0 || tris.GetCount()>0) {
		sObj << "# Object : " << (o.GetName()).GetAsciiString() ;
		sObj << "\nAttributeBegin\n";
		
		
		if ((float)s.GetParameter(L"inc_inten").GetValue()>0) {
			// check for Meshlight
			vIsMeshLight=true;
			s.GetColorParameterValue(L"incandescence",red,green,blue,alpha ); 
		} else {
			shader = writeLuxsiShader(o);
		}
		if (vType==L"obj"){
			
			CValue tmp;
			
			if (shader==1) {
				tmp=L" \"texture Kr\" \"Kr-" + m.GetName()+L"\" \"texture Kt\" \"Kt-"+ m.GetName()+L"\" \"texture index\" \"index-"+ m.GetName()+L"\"";
			} else if (shader==2) {
				tmp=L" \"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture uroughness\" \"uroughness-"+m.GetName()+L"\" \"texture vroughness\" \"vroughness-"+m.GetName()+L"\"";
			} else if (shader==3) {
				tmp=L" \"texture Kr\" \"Kr-"+m.GetName()+ L"\"";
			}else if (shader==4) {
				tmp=L" \"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture sigma\" \"sigma-"+ m.GetName()+L"\"";
			} else if (shader==5) {
				tmp=L" \"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks\" \"Ks-"+m.GetName()+L"\" \"texture roughness\" \"roughness-"+m.GetName()+L"\"";
			} else if (shader==6) {
				tmp=L"\"texture Kd\" \"Kd-"+m.GetName()+ L"\" \"texture Ks1\" \"Ks1-"+m.GetName()+ L"\" \"texture Ks2\" \"Ks2-"+m.GetName()+ L"\" \"texture Ks3\" \"Ks3-"+m.GetName()+ L"\" \"texture R1\" \"R1-"+m.GetName()+ L"\" \"texture R2\" \"R2-"+m.GetName()+ L"\" \"texture R3\" \"R3-"+m.GetName()+ L"\" \"texture M1\" \"M1-"+m.GetName()+ L"\" \"texture M2\" \"M2-"+m.GetName()+ L"\" \"texture M3\" \"M3-"+m.GetName()+ L"\"";
			}
			if (vIsMeshLight) {
				sObj << "AreaLightSource \"area\" \"integer nsamples\" [1] \"color L\" ["<<(red*(float)s.GetParameterValue(L"inc_inten")) <<" "<<(green*(float)s.GetParameterValue(L"inc_inten")) <<" "<<(blue*(float)s.GetParameterValue(L"inc_inten"))<<"]\n";
			} else {
				sObj << "Material \""<< aShader[shader].GetAsText().GetAsciiString() << "\" " << tmp.GetAsText().GetAsciiString() << "\n";
			}
			
			
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
			for (LONG j=0; j<nod.GetCount(); j+=3){
				
				
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
				vPoints +=  L" "+ CString(pos[0]) + L" "+  CString(pos[1]) + L" "+ CString(pos[2])+L"\n"; 
				vNormals +=  L" "+ CString(norm[0]) + L" "+  CString(norm[1]) + L" "+ CString(norm[2])+L"\n";
				if (vText) vUV += L" "+ CString(uvs[0]) + L" "+  CString(uvs[1])+L"\n";
			}
			
			
			//
			// write 
			//
			
			string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
			if (loc != string::npos) {
				sObj << " PortalShape ";
			} else {
				sObj << " Shape ";
			}
			sObj << " \"trianglemesh\" \"integer indices\" [\n";
			sObj << vTris.GetAsciiString();
			sObj << " ] \"point P\" [\n" ;
			sObj << vPoints.GetAsciiString();
			sObj << "] \"normal N\" [\n";
			sObj << vNormals.GetAsciiString();
			if(vText){
				sObj << "] \"float uv\" [\n";
				sObj << vUV.GetAsciiString();
			}
			sObj << "]\nAttributeEnd\n";
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
		f.open (vFileObjects.GetAsciiString());
		
		CRefArray array,aObj,aLight,aCam,aSurfaces,aClouds,aInstance;
		sLight.str("");
		sObj.str("");
		sMat.str("");
		
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
		
		app.LogMessage( L"\tObject count: "+CString(aObj.GetCount()));
		pb.PutValue(0);
		pb.PutMaximum( aObj.GetCount() );
		pb.PutVisible( true );
		pb.PutCaption( L"Processing" );
		pb.PutCancelEnabled(true); 
		for (int i=0;i<aLight.GetCount();i++) writeLuxsiLight(aLight[i]);
		
		for (int i=0;i<aObj.GetCount();i++) {
			if (writeLuxsiObj(aObj[i],L"obj")==-1) break;
			pb.Increment();
		}
		
		
		for (int i=0;i<aSurfaces.GetCount();i++) {
			if (writeLuxsiObj(aSurfaces[i],L"surface")==-1) break;
			pb.Increment();
		}
		//for (int i=0;i<aClouds.GetCount();i++) {
		//	if (writeLuxsiObj(aClouds[i],L"cloud")==-1) break;
		//	pb.Increment();
		//}
		for (int i=0;i<aCam.GetCount();i++) writeLuxsiCam(aCam[i]);
		
		writeLuxsiBasics();
		
		pb.PutVisible( false );
		//
		// write to file
		//
		
		f << sLight.str();
		sLight.flush();
		f << sMat.str();
		f << sObj.str();
		sObj.flush();
		sMat.flush();

		f << "WorldEnd";
		f.close();
	} else {}
}
