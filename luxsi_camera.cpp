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

#include "include\luxsi_camera.h"

//-
CRefArray sceneCollectionsCameras()
{
    //-
    CRefArray sceneCams, aCams;
    root = app.GetActiveSceneRoot();
    sceneCams += root.FindChildren( L"", L"CameraRoot", CStringArray(), true );
    //-
    for ( long i = 0; i < sceneCams.GetCount(); i++)
    {
        X3DObject obj(sceneCams[i]);
        //-
        if ( is_visible(obj, L"camera")) aCams.Add(obj);
    }
    return aCams;
}
//--
CVector3 cameraLookAt(Camera cam, CString date)
{
    CVector3 cameraData;
    //
    if (date == L"Look")
    {
        cameraData = cam.GetKinematics().GetGlobal().GetTransform(ftime).GetTranslation();
    }
    else if (date == L"At")
    {
        X3DObject camInterest = cam.GetInterest();
        cameraData = camInterest.GetKinematics().GetGlobal().GetTransform(ftime).GetTranslation();
    }
    else if (date == L"Up")
    {
        CMatrix4 camMatrix = cam.GetKinematics().GetGlobal().GetTransform(ftime).GetMatrix4();
        //- 
        cameraData.Set(camMatrix.GetValue(1,0), camMatrix.GetValue(1,1), camMatrix.GetValue(1,2));
    }
    else
    {
        app.LogMessage(L"Incorrect camera vector type");
    }
    return cameraData;
}
//-- 
CString luxsiCameraShaders(CRefArray camShaders)
{
    CString dofData;
    //-
    for (int i=0; i < camShaders.GetCount(); i++)
    {
        //-
        Shader cs(camShaders[i]); 
        CString vCSID((cs.GetProgID()).Split(L".")[1]);
        if ( luxdebug ) app.LogMessage(L" Lens shader in use: "+ CString(vCSID));
        //-
        if ( vCSID == L"sib_dof") 
        {
            int vdof_mode = cs.GetParameterValue(L"mode");
            //--
            if ( vdof_mode = 0 ) //- custom
            {
                /* custom_near_focus, custom_far_focus, custom_coc */
            }
            if ( vdof_mode = 1 ) //- auto
            {
                dofData += floatToString(cs, L"focaldistance", L"auto_focal_distance");
            }
            if ( vdof_mode = 2 ) //- lens
            {
                /* len_focal_distance, len_focal_lenght, len_fstop, len_coc */
                dofData += floatToString(cs, L"focaldistance", L"len_focal_distance");
            }
            //-- commons
            dofData += floatToString(cs, L"lensradius", L"strenght");
        }
        else if (vCSID == L"lux_camera_dof")
        {
            const char *distrib[] = {"uniform", "gaussian", "inverse gaussian", "exponential", "inverse exponential"};
            int _dist = cs.GetParameterValue(L"distribution");
            
            //object_dof /* TODO */
	        bool autofocus = cs.GetParameterValue(L"autofocus");
            dofData += L"  \"bool autofocus\" [\""+ CString(autofocus) + L"\"]\n";    
            //-
            if (!autofocus) dofData += floatToString(cs, L"focaldistance", L"distance");
            dofData += L"  \"string distribution\" [\""+ CString(distrib[_dist]) + L"\"]\n";    
            dofData += integerToString(cs, L"power");
            //-
            dofData += floatToString(cs, L"lensradius");
            dofData += integerToString(cs, L"blades");
	        //-
        }
    }
    return dofData;
}
//-
CString writeLuxsiCam()
{
    /****************************
    * Status: work in progress
    ****************************/
    //- values
    CString camShaderData, camData; 
    float vfov, aspect;

    CRefArray aCamera = sceneCollectionsCameras();
    //-
    for ( long i = 0; i < aCamera.GetCount(); i++)
    {
        X3DObject camRoot(aCamera[i]);    
        //-
        Camera cam = camRoot.GetChildren()[0]; // camera
    
        /* Position */
        CVector3 look = cameraLookAt(cam, L"Look");
        /* Target */
        CVector3 target = cameraLookAt(cam, L"At");
        /* Up */
        CVector3 up = cameraLookAt(cam, L"Up");
    
        //- shader nodes connect to camera
        CRefArray cShaders = cam.GetShaders();
        //-
        if ( cShaders.GetCount() > 0 )
        {
            camShaderData = luxsiCameraShaders(cShaders);
        }

        //- compute FOV
        aspect = float(cam.GetParameterValue(L"aspect"));
        vfov = float(cam.GetParameterValue(L"fov"));
        //-
        if ( int(cam.GetParameterValue(L"fovtype")) == 1) 
        {
            // calculate the proper FOV (horizontal -> vertical)
            float hfov = float(cam.GetParameterValue(L"fov"));
            vfov = float( 2 * ( atan( (1/aspect) * tan(hfov / 2 * PI/180)) ) * (180/PI) );
        } 
        //- calculate screen window ( from liblux )
        CVector4 screenW(-1.f, 1.f, -1.f/aspect, 1.f/aspect);
        //-
	    if (aspect > 1.f) screenW.Set(-aspect, aspect, -1.f, 1.f);
    
        //-- projection type
        int camera_proj = cam.GetParameterValue(L"proj");

        //- pos
        camData = L"\nLookAt "
            + CString(look[0]) + L" "
            + CString(-look[2]) + L" "
            + CString(look[1]) + L"\n";

        //- target
        camData += L"\t"
            + CString(target[0]) + L" "
            + CString(-target[2]) + L" "
            + CString(target[1]) + L"\n"; 

        //- up
        camData += L"\t"
            + CString(up[0]) + L" "
            + CString(-up[2]) + L" "
            + CString(up[1]) + L"\n";

        //--    
        if ( camera_proj == 1 )
        {
            camData += L"\nCamera \"perspective\" \n";
            camData += L"  \"float fov\" ["+ CString( vfov ) + L"] \n";
            camData += L"  \"float screenwindow\" ["
                + CString(screenW[0]) + L" "
                + CString(screenW[1]) + L" "
                + CString(screenW[2]) + L" "
                + CString(screenW[3]) + L"]\n";
	        //camData += L"float shutteropen" [0.00]
	        //camData += L"float shutterclose" [0.04]

            //- clipping ?
            camData += L"  \"float hither\" ["+ CString(cam.GetParameterValue(L"near"))+ L"]\n";
	        camData += L"  \"float yon\" ["+ CString(cam.GetParameterValue(L"far"))+ L"]\n";
        
            //- add Camera shaders
            if (!camShaderData.IsEmpty()) camData += camShaderData;
        }
        else //-- orthographic
        {        
            camData += L"\nCamera \"orthographic\"\n";
            camData += L"  \"float screenwindow\" ["
                + CString(screenW[0]) + L" "
                + CString(screenW[1]) + L" "
                + CString(screenW[2]) + L" "
                + CString(screenW[3]) + L"]\n";
        /*
	    //- not dof / not clipping
        "float screenwindow" [-3.65 3.65 -2.53 2.53]
	    "bool autofocus" ["false"]
	    "float shutteropen" [0.0]
	    "float shutterclose" [0.0416]
        */
        
        }
        /* panoramic = environment ?
            Camera "environment"
        */
    }
    return camData;
}