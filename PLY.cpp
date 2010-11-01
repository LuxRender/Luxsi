
#include <xsi_application.h>
#include <xsi_context.h>
#include <xsi_pluginregistrar.h>
#include <xsi_status.h>
#include <xsi_menu.h> 
#include <xsi_model.h> 
#include <xsi_ppglayout.h>
#include <xsi_ppgeventcontext.h>
#include <sstream>
#include <fstream>
#include <xsi_camera.h>
#include <xsi_kinematics.h> 
#include <xsi_utils.h>
#include <xsi_project.h>
#include <xsi_scene.h>
#include <xsi_pass.h>
#include <xsi_shader.h>
#include <xsi_renderer.h> 
#include <xsi_vector3.h> 
#include <xsi_geometryaccessor.h>
#include <xsi_polygonmesh.h> 
#include <xsi_polygonnode.h>
#include <xsi_segment.h>
#include <xsi_particlecloudprimitive.h> 


using namespace XSI; 
using namespace std;
using namespace MATH;

CString vFileObjects="";
CString path="";
Application app;
Project proj = app.GetActiveProject();
Scene scn = proj.GetActiveScene();
Model root = scn.GetRoot();
		
CustomProperty prop ;
ofstream _fileScene;
ofstream _fileObjects;

void exportFile();

SICALLBACK XSILoadPlugin( PluginRegistrar& in_reg ) {
	in_reg.PutAuthor(L"Michael Gangolf");
	in_reg.PutName(L"PLY");
	in_reg.PutEmail(L"miga@migaweb.de");
	in_reg.PutURL(L"http://www.migaweb.de");
	in_reg.PutVersion(0,1);
	in_reg.RegisterProperty(L"PLY");
	in_reg.RegisterMenu(siMenuMainFileExportID,L"PLY_Menu",false,false);
	
	//RegistrationInsertionPoint - do not remove this line

	return CStatus::OK;
}

SICALLBACK XSIUnloadPlugin( const PluginRegistrar& in_reg ) {
	CString strPluginName;
	strPluginName = in_reg.GetName();
	Application().LogMessage(strPluginName + L" has been unloaded.",siVerboseMsg);
	return CStatus::OK;
}

SICALLBACK PLY_Menu_Init( CRef& in_ctxt ) {
	Context ctxt( in_ctxt );
	Menu oMenu;
	oMenu = ctxt.GetSource();
	MenuItem oNewItem;
	oMenu.AddCallbackItem(L"PLY",L"OnPLY_MenuClicked",oNewItem);
	return CStatus::OK;
}

SICALLBACK OnPLY_MenuClicked( XSI::CRef& ) {	
	CValueArray addpropArgs(5) ;
	addpropArgs[0] = L"PLY"; // Type of Property
	addpropArgs[3] = L"PLY"; // Name for the Property
	addpropArgs[1] = L"Scene_Root";
	bool vAlreadyThere=false;
	CValue retVal ;
	CStatus st;
	
	// search for existing interface
	CRefArray vItems = app.GetActiveSceneRoot().GetProperties();
	for (int i=0;i<vItems.GetCount();i++){
		if (SIObject(vItems[i]).GetType()==L"PLY") {
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
		args[2] = "PLY";
		args[3] = siLock; 
		args[4] = false;
		
		app.ExecuteCommand( L"InspectObj", args, retVal ) ;
	}
	
	return st ;
}


SICALLBACK PLY_Define( CRef& in_ctxt ) {
	Context ctxt( in_ctxt );
	Parameter oParam;
	prop = ctxt.GetSource();
	prop.AddParameter( L"fObjects", CValue::siString, siPersistable, L"", L"", vFileObjects, oParam ) ;

	return CStatus::OK;
}

SICALLBACK PLY_DefineLayout( CRef& in_ctxt ){
	PPGLayout lay = Context(in_ctxt).GetSource() ;
    lay.Clear();
	
	lay.AddTab(L"Main");
	
	
	lay.AddItem(L"fObjects",L"Save as",siControlFilePath);
	PPGItem it = lay.GetItem( L"fObjects" );
	it.PutAttribute( siUIFileFilter, L"LuXSI Scenes|*.snc" ) ;
	
	
	lay.AddButton(L"exe_ply",L"Export");
	
	return CStatus::OK;
}

SICALLBACK PLY_PPGEvent( const CRef& in_ctxt )
{
	PPGEventContext ctxt( in_ctxt ) ;
	PPGLayout lay = Context(in_ctxt).GetSource() ;
	
	PPGEventContext::PPGEvent eventID = ctxt.GetEventID() ;
	
	if ( eventID == PPGEventContext::siOnInit ) {
		CustomProperty prop = ctxt.GetSource() ;	
		ctxt.PutAttribute(L"Refresh",true);
		
	}
	else if ( eventID == PPGEventContext::siButtonClicked )	{
		CValue buttonPressed = ctxt.GetAttribute( L"Button" ) ;	
		if (buttonPressed.GetAsText()==L"exe_ply"){
			exportFile();
		} 
		
		
		ctxt.PutAttribute(L"Refresh",true);
		lay.PutAttribute(L"Refresh",true);
		CRefArray params = prop.GetParameters();;
		app.LogMessage( L"Button pressed: " + buttonPressed.GetAsText() ) ;
		
	} else if ( eventID == PPGEventContext::siTabChange ) {
		CValue tabLabel = ctxt.GetAttribute( L"Tab" ) ;
	} else if ( eventID == PPGEventContext::siParameterChange )	{
		Parameter changed = ctxt.GetSource() ;	
		CustomProperty prop = changed.GetParent() ;	
		CString   paramName = changed.GetScriptName() ; 
		
		if (paramName==L"fObjects"){
			vFileObjects=changed.GetValue();
		} 
		
		
	}
	
	return CStatus::OK ;
}


void writeFile(int i, CString s){
	if (i==0) _fileScene << s.GetAsciiString() << "\n";
	if (i==1) _fileObjects << s.GetAsciiString() << "\n";
}

CVector3 convertMatrix(CVector3 v){
	CMatrix3 m2(1.0,0.0,0.0,  0.0,0.0,1.0,   0.0,-1.0,0.0);
    return v.MulByMatrix3InPlace (m2);
}

void outputCamera(){
	//
	// write camera
	//
	

	
	CScriptErrorDescriptor status ;
	CValueArray fooArgs(1) ;
	fooArgs[0] = L"" ;
	CValue retVal=false ;
	status = app.ExecuteScriptProcedure(L"function getS(){return GetValue(Application.ActiveProject.ActiveScene.ActivePass+\".Camera\")}",L"JScript",L"getS",fooArgs, retVal  ) ;
	Camera c = Camera(retVal);
	X3DObject o2=X3DObject(c.GetParent()).GetChildren()[0];
	
	
	CVector3 vnegZ(0,0,1);

	// Operations to calculate look at position.
	vnegZ.MulByMatrix3InPlace(c.GetKinematics().GetGlobal().GetTransform().GetRotationMatrix3());
	vnegZ.NormalizeInPlace();
	vnegZ.ScaleInPlace((double) c.GetParameterValue(L"interestdist"));
	vnegZ.AddInPlace(c.GetKinematics().GetGlobal().GetTransform().GetTranslation());
	
	
	CTransformation localTransformation = o2.GetKinematics().GetLocal().GetTransform();
	KinematicState  gs = o2.GetKinematics().GetGlobal();
	CTransformation gt = gs.GetTransform();
	CVector3 translation(localTransformation.GetTranslation());
	bool vDof=false;
	
	X3DObject ci(c.GetChildren()[1]);
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
	

	writeFile(0,"scene.camera.lookat = " + CString(new_pos.GetX()) + " " + CString(new_pos.GetY()) + " " + CString(new_pos.GetZ())+ " " + CString(new_pos_ci.GetX()) + " " +CString(new_pos_ci.GetY())+ " " + CString(new_pos_ci.GetZ()) + " 0 0 "+ CString(up.GetZ()));
	
	
}

void outputObjects(X3DObject o, CString fname){
	// gets a geometry accessor object with default parameters
	Geometry g(o.GetActivePrimitive().GetGeometry());
    CGeometryAccessor ga = PolygonMesh(g).GetGeometryAccessor();

	// get the number of vertices for each polygons
	CLongArray pvCount;
	ga.GetPolygonVerticesCount(pvCount);

	LONG vCount;
	vCount=ga.GetVertexCount();
	LONG pCount;
	pCount=ga.GetPolygonCount();
	CLongArray vIndices;
	ga.GetVertexIndices(vIndices);
	CDoubleArray vPos;
	ga.GetVertexPositions(vPos);

	CString sPos;
	for (LONG i=0; i<vCount; i+=3) {
		sPos += CString(vPos[i]) + " " + CString(vPos[i+1]) + " " + CString(vPos[i+2]) + " 0 0 0\n" ;
	} 

	CString s= fname+".ply";
	_fileObjects.open (s.GetAsciiString());
	writeFile(1,"ply");
	writeFile(1,"format ascii 1.0");
	writeFile(1,"comment SI Export");
	writeFile(1,"element vertex "+CString(vCount));
	writeFile(1,"property float x");
	writeFile(1,"property float y");
	writeFile(1,"property float z");
	writeFile(1,"property uchar red");
	writeFile(1,"property uchar green");
	writeFile(1,"property uchar blue");
	writeFile(1,"element face "+CString(pCount));
	writeFile(1,"property list uchar uint vertex_indices");
	writeFile(1,"end_header");
	writeFile(1,sPos);
	
	_fileObjects.close();
}

void exportFile(){
	CRefArray array;
	CStringArray emptyArray;		
	emptyArray.Clear();

	int loc_end=(int)vFileObjects.ReverseFindString(CString("."));
	int loc_start=(int)vFileObjects.ReverseFindString(CString("/")) + 1;
	CString fname = vFileObjects.GetSubString(loc_start,loc_end-loc_start);
	path = vFileObjects.GetSubString(0,loc_start);
	
	_fileScene.open (vFileObjects.GetAsciiString());
	outputCamera();
	writeFile(0,"scene.materials.matte.whitematte = 0.75 0.75 0.75");
	writeFile(0,"scene.materials.light.whitelight = 120.0 120.0 120.0");
	
	writeFile(0,"#scene.objects.whitelight.all = "+fname+"_lights.ply");
	
	
	array.Clear();

	array += root.FindChildren( L"", L"", emptyArray, true );
	for ( int i=0; i<array.GetCount();i++ ){
		X3DObject o(array[i]);
		//app.LogMessage( L"\tObject name: " + o.GetName() + L":" +o.GetType() + L" parent:"+X3DObject(o.GetParent()).GetType());
		
		// Collection objects
		if (o.GetType()==L"polymsh"){
			writeFile(0,"scene.objects.whitematte.all = "+fname+"_"+o.GetName()+".ply");
			outputObjects(o, path+fname+"_"+o.GetName());
		}
	}
	
	_fileScene.close();
	
}