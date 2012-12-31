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

#include "include\luxsi_camera.h"

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
//--
CString writeLuxsiCam(X3DObject o)
{
    /****************************
    * Status: work in progress
    ****************************/
    //- values
    CString dofData, camData; 
    float vfov, aspect;
    //-
    X3DObject camInterest;
    Camera cam;
    cam = o.GetChildren()[0]; // camera
    camInterest = o.GetChildren()[1]; // target
    //camInterest = cam.GetInterest(); // other mode        

    /* Camera position */
    KinematicState  cam_global_state    = cam.GetKinematics().GetGlobal();
    CTransformation cam_global_transf   = cam_global_state.GetTransform(ftime);
    CVector3 camPos = cam_global_transf.GetTranslation();

    /* Camera target position */
    KinematicState  target_global_state     = camInterest.GetKinematics().GetGlobal();
    CTransformation target_global_transf    = target_global_state.GetTransform(ftime);
    CVector3 targetPos = target_global_transf.GetTranslation();
    
    //- shader nodes connect to camera
    CRefArray cShaders = cam.GetShaders();
    if ( cShaders.GetCount() > 0 )
    {
        dofData = luxsiCameraShaders(cShaders);
    }    
    //--
    if ((int)cam.GetParameterValue(L"fovtype")==1) 
    {
        // calculate the proper FOV (horizontal -> vertical)
        aspect = float(cam.GetParameterValue(L"aspect"));
        float hfov = float(cam.GetParameterValue(L"fov"));
        vfov = float( 2 * ( atan( (1/aspect) * tan(hfov / 2 * PI/180)) ) * (180/PI) );
    } 
    else
    {
        // keep vertical FOV
        vfov = float(cam.GetParameterValue(L"fov"));
    }
    //- test from liblux
    //float frame = aspect;
    float screen[4];
	if (aspect > 1.f){
		screen[0] = -aspect; screen[1] = aspect; screen[2] = -1.f; screen[3] = 1.f;
	} else {
		screen[0] = -1.f; screen[1] = 1.f; screen[2] = -1.f / aspect; screen[3] = 1.f / aspect;
	}
    //--
    int camera_proj = cam.GetParameterValue(L"proj");
    //--
    camData = L"LookAt "
        + CString(camPos[0]) + L" "
        + CString(-camPos[2]) + L" "
        + CString(camPos[1]) + L"\n";
    //-
    camData += L" "
        + CString(targetPos[0]) + L" "
        + CString(-targetPos[2]) + L" "
        + CString(targetPos[1]) + L"\n";
    //-
    camData += L" 0 0 1 \n"; //TODO
    //--    
    if ( camera_proj == 1 )
    {
        camData += L"Camera \"perspective\" \n";
        camData += L"  \"float fov\" ["+ CString( vfov ) + L"] \n";
        camData += L"  \"float screenwindow\" ["
            + CString(screen[0]) + L" "
            + CString(screen[1]) + L" "
            + CString(screen[2]) + L" "
            + CString(screen[3]) + L"]\n";
	    //camData += L"float shutteropen" [0.000000000000000]
	    //camData += L"float shutterclose" [0.041666666666667]
        //- clipping ?
        camData += L"  \"float hither\" ["+ CString(cam.GetParameterValue(L"near"))+ L"]\n";
	    camData += L"  \"float yon\" ["+ CString(cam.GetParameterValue(L"far"))+ L"]\n";
        //-
        if (!dofData.IsEmpty()) camData += dofData;
    }
    else //-- orthographic
    {        
        camData += L"Camera \"orthographic\"\n";
        /*
	    //- sin dof / sin clipping
        "float screenwindow" [-3.65 3.65 -2.53 2.53]
	    "bool autofocus" ["false"]
	    "float shutteropen" [0.0]
	    "float shutterclose" [0.0416]
        */
        
    }
    /* panoramic = environment ?
        Camera "environment"
    */
    return camData;
}

//--