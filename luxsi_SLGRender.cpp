/***********************************************************************
This file is part of LuXSI;
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

***********************************************************************/

//--
#include "include\luxsi_SLGRender.h"


//- locals functions
CString SLGRenderScene(CString in_sceneFile);

//-
CVector3 storeLightDirection(X3DObject obj, CString pivot)
{
	CMatrix4 lDir;
	//-
	if ( pivot == L"local")
	{
		lDir = obj.GetKinematics().GetLocal().GetTransform().GetMatrix4();
	}
	else
	{
		lDir = obj.GetKinematics().GetGlobal().GetTransform().GetMatrix4();
	}
	//-
	CVector3 lightDirection(lDir.GetValue(2,0), -lDir.GetValue(2,2), lDir.GetValue(2,1));
	//-
	return lightDirection;
}
//-
CString createSLGMaterials()
{
	CString matData;
	//- scene materials
	CRefArray sceneMaterials = sceneCollectionMaterials();
	//-------------------------------------
	matData += L"#-[ materials ]---->\n";
	//-------------------------------------
	for ( long j = 0; j < sceneMaterials.GetCount(); j++)
	{
		//-
		Material slgMat(sceneMaterials[j]);
		//-
		CString matname = slgMat.GetName();
		//-
		CRefArray shaderArray(slgMat.GetShaders());
		//-
		for ( long k=0; k < shaderArray.GetCount(); k++)
		{
			Shader s(shaderArray[k]);
			CString vShadID((s.GetProgID()).Split(L".")[1]);
			//
			float r, g, b, a;

			//--------------------------
			if (vShadID == L"lux_matte")
			{
				matData += L"scene.materials."+ matname + L".type = matte\n";
				//-
				s.GetColorParameterValue("Kd", r, g, b, a );
				matData += L"scene.materials."+ matname + L".kd = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
			}//-----------------------------------------
			else if (vShadID == L"lux_mattetranslucent")
			{
				matData += L"scene.materials." + matname + L".type = mattetranslucent\n";
				//-
				s.GetColorParameterValue("Kr", r, g, b, a );
				matData += L"scene.materials."+ matname + L".kr = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
				//-
				s.GetColorParameterValue("Kt", r, g, b, a );
				matData += L"scene.materials." + matname + L".kt = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
			}//-----------------------------
			else if(vShadID == L"lux_glass")
			{
				CString shadertype = L"glass";
				if ( s.GetParameterValue(L"architectural")) shadertype = L"archglass";
				//-
				matData += L"scene.materials."+ matname + L".type = "+ shadertype + L"\n";

				//- reflectance
				s.GetColorParameterValue("Kr", r, g, b, a );
				matData += L"scene.materials."+ matname + L".kr = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";

				//- transmitance
				s.GetColorParameterValue("Kt", r, g, b, a );
				matData += L"scene.materials."+ matname + L".kt = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";

				//- ior out
				float ioroutside = float(s.GetParameterValue(L"filmindex")); // ??
				matData += L"scene.materials."+ matname + L".ioroutside = "+ CString(ioroutside) + L"\n";
				//- ior in
				float iorinside = float(s.GetParameterValue(L"index"));
				matData += L"scene.materials."+ matname + L".iorinside = "+ CString(iorinside) + L"\n";
			}
			else if(vShadID == L"lux_mirror")
			{
				s.GetColorParameterValue("Kr", r, g, b, a );
				matData += L"scene.materials."+ matname + L".type = mirror\n";
				matData += L"scene.materials."+ matname + L".kr = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n"; // review
			}
			else if(vShadID == L"lux_metal")
			{
				s.GetColorParameterValue("Kr", r, g, b, a );
				float exp = float(s.GetParameterValue(L"exponent"));
				//-
				matData += L"scene.materials." + matname + L".type = metal\n";
				matData += L"scene.materials." + matname + L".kr = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
				//-
				matData += L"scene.materials." + matname + L".exp = "+ CString(exp) + L"\n";
				/*
				props.SetString("scene.materials." + name + ".type", "metal2");
				props.SetString("scene.materials." + name + ".n", n->GetName());
				props.SetString("scene.materials." + name + ".k", k->GetName());
				props.SetString("scene.materials." + name + ".uroughness", nu->GetName());
				props.SetString("scene.materials." + name + ".vroughness", nv->GetName());
				*/
			}

			else if(vShadID == L"lux_glossy")
			{
				matData += L"scene.materials." + matname + L".type = glossy2\n";
				//-
				s.GetColorParameterValue("Kd", r, g, b, a );
				matData += L"scene.materials." + matname + L".kd = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
				//-
				s.GetColorParameterValue("Ks", r, g, b, a );
				matData += L"scene.materials." + matname + L".ks = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
				//-
				matData += L"scene.materials." + matname + L".uroughness = 0.1\n";
				matData += L"scene.materials." + matname + L".vroughness = 0.1\n";
				s.GetColorParameterValue("Ka", r, g, b, a );
				matData += L"scene.materials." + matname + L".ka = "
				           + CString(r) + L" "+ CString(g) + L" "+ CString(b)+ L"\n";
				//matData += L"scene.materials." + matname + L".d = ", depth->GetName());
				matData += L"scene.materials." + matname + L".index = 1\n";
			}
			else //- use default
			{
				matData += L"scene.materials."+ matname + L".type = matte\n";
				matData += L"scene.materials."+ matname + L".kd = 0.9 0.8 0.9\n";
			}
			//- here..
			if (pb.IsCancelPressed() ) break;
			pb.Increment();
		}
		//- or pb placed here?

	}
	return matData;
}

/**/
CString createSLGLights()
{
	CString lightData;
	//- lights
	CRefArray slgLights = sceneCollectionsLights();
	//-
	lightData += L"#-[ lights ]---->\n";
	//-
	for ( long i = 0; i < slgLights.GetCount(); i++)
	{
		X3DObject oLight(slgLights[i]);
		//-
		Shader s((Light(oLight).GetShaders())[0]);
		int lightype = Light(oLight).GetParameterValue(L"Type");
		CString slgLightID = s.GetProgID().Split(L".")[1];
		//-
		CVector3 lightDir = storeLightDirection(oLight, L"local");

		if (lightype == 1) // infinite
		{
			lightData += L"scene.sunlight.dir = "
			             + CString(lightDir[0]) + L" "+ CString(lightDir[1]) + L" "+ CString(lightDir[2]) + L"\n";
			//-
			lightData += L"scene.sunlight.turbidity = 2.2\n";
			lightData += L"scene.sunlight.gain = .5 .5 .5\n";
			lightData += L"scene.sunlight.relsize = 2\n";
		}
		else
		{
			lightData += L"scene.skylight.dir = "
			             + CString(lightDir[0]) + L" "+ CString(lightDir[1]) + L" "+ CString(lightDir[2]) + L"\n";
			//-
			lightData += L"scene.skylight.turbidity = 2.2\n";
			lightData += L"scene.skylight.gain = 1. 1. 1.\n";
		}
		//- pb
		if (pb.IsCancelPressed() ) break;
		pb.Increment();
	}
	return lightData;
}
/**/
CRefArray slgCollectionsObjects()
{
	/*
	Esta parte es solo para pruebas con polygonmesh.
	Cuando tengamos soporte para instancias o hair, usaremos el collections objects normal.
	La idea; limitar el uso de findchildren, ya que este hace una exploracion recursiva
	de toda la escena, consumiendo muchos recursos.
	*/
	CRefArray slgItemsArray, slgObj;
	//--
	root = Application().GetActiveSceneRoot();
	//--
	slgItemsArray += root.FindChildren( L"", L"polymsh", CStringArray(), true );
	for ( int i=0; i < slgItemsArray.GetCount(); i++ )
	{
		X3DObject o(slgItemsArray[i]);
		//-
		if ( is_visible(o, L"polymsh")) slgObj.Add(o);
	}
	//-
	return slgObj;
}
/**/
CString SLGRenderScene(CString sceneFile)
{
	CString sceneData;
	CRefArray slgCameras = sceneCollectionsCameras();
	//-
	for ( long i= 0; i < slgCameras.GetCount(); i++)
	{
		/*
		scene.camera.lookat = 0.5 -1.4 1.7 0.0 0.0 0.4
		scene.camera.fieldofview = 60.0
		scene.camera.lensradius = 0.005
		scene.camera.focaldistance = 4.0
		#scene.camera.screenwindow = 0.5 1.0 0.0 1.0
		scene.camera.cliphither", 1e-3f
		scene.camera.clipyon", 1e30f
		*/
		//- camera root
		X3DObject slgCam(slgCameras[i]);
		//- camera
		Camera cam = slgCam.GetChildren()[0];
		//- pos
		CVector3 Look = cameraLookAt(cam, L"Look");
		//- target
		CVector3 At = cameraLookAt(cam, L"At");
		//- up
		CVector3 Up = cameraLookAt(cam, L"Up");
		//-
		sceneData += L"#-[ camera ]---->\n";
		sceneData += L"scene.camera.lookat = "
		             + CString(Look[0]) + L" "+ CString(-Look[2]) + L" "+ CString(Look[1]) + L" "
		             + CString(At[0]) + L" "+ CString(-At[2]) + L" "+ CString(At[1]) + L"\n";
		//-
		sceneData += L"scene.camera.up = "
		             + CString(Up[0]) + L" "+ CString(-Up[2]) + L" "+ CString(Up[1]) + L"\n";
		//-
		float aspect = cam.GetParameterValue(L"aspect");
		//-
		CVector4 screen(-1.0, 1.0, -1.f/aspect, 1.f/aspect);
		//-
		if (aspect > 1.0) screen.Set(-aspect, aspect, -1.0, 1.0);
		//-
		sceneData += L"scene.camera.screenwindow = "
		             + CString(screen[0]) + L" "+ CString(screen[1]) + L" "
		             + CString(screen[2]) + L" "+ CString(screen[3]) + L"\n";
		//-
		float fov = float(cam.GetParameterValue(L"fov"));
		sceneData += L"scene.camera.fieldofview = "+ CString(fov) + L"\n";
		//-
		if (pb.IsCancelPressed() ) break;
		pb.Increment();
	}

	//--[ add materials ]----------->
	sceneData += createSLGMaterials();

	//--[ scene objects ]---->
	CRefArray slgObjects = slgCollectionsObjects();
	//-
	sceneData += L"#-[ scene objects ]----->\n";
	for (int i = 0; i < slgObjects.GetCount(); i++)
	{
		//- each object...
		X3DObject o(slgObjects[i]);
		//- your name..
		CString objName = o.GetName();
		//- and your material name...
		CString matObjName = o.GetMaterial().GetName();

		//- file PLY name is.. the name of filescene + obj name + .ply
		CString plyfile = sceneFile +"_"+ objName +".ply";
		//-
		sceneData += L"#--[ "+ objName + L" ]-->\n";
		sceneData += L"scene.objects."+ objName + L".ply = "+ plyfile + L"\n";
		sceneData += L"scene.objects."+ objName + L".material = "+ matObjName + L"\n";
		//-
		if ( exportNormals) sceneData += L"scene.objects."+ objName + L".useplynormals = 1\n";

		//- go to write geometry files, one for each object
		writeLuxsiObj(o);
		//-
		if (pb.IsCancelPressed() ) break;
		pb.Increment();
	}

	//--[ add lights ]------------>
	sceneData += createSLGLights();

	//--[ write scene file ]------>
	int ext = int(sceneFile.ReverseFindString("."));
	CString sceneFilePath = sceneFile.GetSubString(0, ext) + (L".scn");
	//-
	std::ofstream scn;
	scn.open(sceneFilePath.GetAsciiString(), std::ios::out);
	scn << sceneData.GetAsciiString();
	scn.close();

	//- close pb
	pb.PutVisible( false );

	//-
	return L"";
}
/**/
CString SLGFileconfig(CString filePath)
{
	//-- init progress bar
	pb.PutValue(0);
	pb.PutMaximum( 100 );
	pb.PutStep(1);
	pb.PutVisible( true );
	pb.PutCaption( L"Processing SLG scene data.." );
	pb.PutCancelEnabled(true);

	//-- create base path; extract extension to file
	int ext = int(filePath.ReverseFindString("."));
	CString basePath = filePath.GetSubString(0, ext);

	CString sceneFilePath = basePath + L".scn";
	CString configFilePath = basePath + L".cfg";
	CString imageFilePath = basePath + L".png";

	CString cfgData;
	//
	cfgData += L"image.width = "+ CString(vXRes)    +"\n";
	cfgData += L"image.height = "+ CString(vYRes)   +"\n";
	cfgData += L"image.filename = "+ imageFilePath  +"\n";

	// Use a value > 0 to enable batch mode
	//batch.halttime = 0
	//cfgData += L"batch.haltspp = 60\n";
	//
	cfgData += L"scene.file = "+ sceneFilePath + L"\n";

	cfgData += L"opencl.cpu.use = 1\n";
	cfgData += L"opencl.gpu.use = 1\n";
	// Select the OpenCL platform to use (0=first platform available, 1=second, etc.)
	//opencl.platform.index = 0
	// The string select the OpenCL devices to use (i.e. first "0" disable the first
	// device, second "1" enable the second).
	//opencl.devices.select = 10
	// Use a value of 0 to enable default value
	cfgData += L"opencl.gpu.workgroup.size = 64\n";
	cfgData += L"screen.refresh.interval = "+ CString(vDis) +"\n";
	// Set the gamma correction value used for PNG/PPM file and screen
	cfgData += L"film.gamma = "+ CString(vContrast) +"\n";
	//-
	//if ( pathCPU
	cfgData += L"path.maxdepth = "+ CString(vmaxdepth) +"\n";
	cfgData += L"path.russianroulette.depth = 3\n";
	cfgData += L"path.russianroulette.cap = .5\n";

	/*
	   BiDirVMCPU -------------------->
	   cfgData += L"bidirvm.lightpath.count", 16 * 1024));
	   cfgData += L"bidirvm.startradius.scale", .003f) * renderConfig->scene->dataSet->GetBSphere().rad;
	   cfgData += L"bidirvm.alpha", .95f);

	   CBiDirHybrid --------------->>
	   cfgData += L"cbidir.eyepath.count", 5);
	   cfgData += L"cbidir.lightpath.count", 5);
	*/

	/*  !! REMEMBER !! ofstream need the complete path!!
	*/
	std::ofstream cfg;
	cfg.open(configFilePath.GetAsciiString(), std::ios::out);
	cfg << cfgData.GetAsciiString();
	cfg.close();

	//- write scene file
	SLGRenderScene(basePath);

	//- finally, return the name of cfg file, used for a render params.
	return configFilePath;
}
