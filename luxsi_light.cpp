/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 2012  Michael Gangolf 
Code contributor; Pedro Alcaide (aka povmaniaco)

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
using namespace MATH;

// extern
extern ofstream f;
extern bool vUse_IES;
extern CString ies_file;
extern Application app;

//-
extern string replace(string input);
extern CString findInGroup(CString name);
//- locals.
CString luxsi_area_transf(X3DObject o, float size_X, float size_Y);
//-
CString find_XSI_env(CString env_file);

//--
void writeLuxsiLight(X3DObject o)
{
    //- light object
    KinematicState  from_global_kinex_state;
    CTransformation light_from_global_transf;
    //- set state for 'point from'
    from_global_kinex_state = o.GetKinematics().GetGlobal();
    light_from_global_transf = from_global_kinex_state.GetTransform();
    
    //- point from
    CVector3 light_from; 
    light_from.MulByTransformationInPlace(light_from_global_transf);
    
    //- light object interest 
    X3DObject light_interest;
    light_interest = X3DObject(o.GetParent()).GetChildren()[1];
    //- set state for 'point to'
    KinematicState  to_global_kinex_state;
    CTransformation light_to_global_transf;
    //-
    to_global_kinex_state = light_interest.GetKinematics().GetGlobal();
    light_to_global_transf = to_global_kinex_state.GetTransform();
    
    //- point to
    CVector3 light_to; 
    light_to.MulByTransformationInPlace(light_to_global_transf);
    
    //--
    KinematicState  local_state = o.GetKinematics().GetLocal(); 
    CTransformation local_transf = local_state.GetTransform();
   
    //--
    float a = 0.0, b = 0.0, c = 0.0, alpha = 0.0;
    Shader s((Light(o).GetShaders())[0]);
    //- same name for xsi and Lux shaders: 'color'
    s.GetColorParameterValue(L"color", a, b, c, alpha);

    //-- finding for light group name
    CString light_Name = findInGroup(o.GetName());
    //-
    CString group_name = o.GetName().GetAsciiString();
    //-
    if (light_Name != L"") 
    {
        group_name = light_Name.GetAsciiString(); 
    } 
    //--
    int lType = Light(o).GetParameterValue(L"Type");
    app.LogMessage(L" light type: "+ CString(lType) );
    
    //-- test
    //bool vSiArealight = o.GetParameterValue(L"LightArea");
    //- end
    //bool vSiArea_vis = o.GetParameterValue(L"LightAreaVisible");
    //int vlight_geo = o.GetParameterValue(L"LightAreaGeom");

    float vLightCone = o.GetParameterValue(L"LightCone"); 
    float vIntensity = s.GetParameterValue(L"intensity");
    float vSpotblend = s.GetParameterValue(L"spread");
    //--
	CString Light_Shader_ID((s.GetProgID()).Split(L".")[1]);
	app.LogMessage(L"Light shader used: "+ Light_Shader_ID);
	//-
    f << "\nLightGroup \""<< group_name.GetAsciiString() <<"\" \n";
    
	//- for use only image environment light
    CString xsi_env = find_XSI_env(L"");
    //-
    if ( xsi_env != L"")
    {
        f << "LightSource \"infinite\"\n";
        f << "  \"float gain\" [" << vIntensity << "]\n";
        f << "  \"float importance\" [1.0]\n"; // TODO
        f << "  \"string mapname\" [\"" << replace(xsi_env.GetAsciiString()) << "\"]\n";
        f << "  \"string mapping\" [\"latlong\"]\n"; // TODO
        f << "  \"float gamma\" [1.0]\n";
        f << "  \"integer nsamples\" [1]\n"; // TODO
    } 
    //-
    if (lType == 2) //-- spot
    {
        //lux_Spot_Light();
        //-- values
        f << "\nLightSource \"spot\"\n";
        f << "  \"float gain\" ["<< vIntensity <<"]\n";
        f << "  \"color L\" ["<< a <<"  "<< b <<"  "<< c <<"]\n";
        //--
        if ( vUse_IES )
        {
            f << "  \"string iesname\" [\""<< replace(ies_file.GetAsciiString()) <<"\"]\n";
        }
        f << "  \"point from\" ["<< light_from[0] <<" "<< -light_from[2] <<" "<< light_from[1] <<"]\n";
        f << "  \"point to\" ["<< light_to[0] <<" "<< -light_to[2] <<" "<< light_to[1] <<"]\n";
        f << "  \"float coneangle\" ["<< vLightCone << "]\n";
        f << "  \"float conedeltaangle\" ["<< vLightCone - vSpotblend <<"]\n";
    } 
    else if  (lType == 1) //-- infinite
    {
        if ( xsi_env != L"" )
        {
            f << "LightSource \"infinite\"\n";
            f << "  \"float gain\" [" << vIntensity <<"]\n";
            f << "  \"float importance\" [1.0]\n"; // TODO
            f << "  \"string mapname\" [\""<< replace(xsi_env.GetAsciiString()) <<"\"]\n";
            f << "  \"string mapping\" [\"latlong\"]\n"; // TODO
            f << "  \"float gamma\" [1.0]\n";
            f << "  \"integer nsamples\" [1]\n"; // TODO
        } 
        else
        {
            CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();
            f << "\nLightSource \"sunsky\"\n";
            f << "  \"integer nsamples\" [4]\n";
            f << "  \"vector sundir\" [ "<< sunTransMat.GetValue(2,0) <<" "<< sunTransMat.GetValue(2,1) <<" "<< sunTransMat.GetValue(2,2) << " ]\n";
            f << "  \"float gain\" ["<< vIntensity <<"]\n";
            f << "  \"color L\" ["<< a <<"  "<< b <<"  "<< c <<"]\n";
        }
    } 
    else if (lType == 0) // Point light node
    {
        //-- for Softimage light area; only work into 'point' or 'spot' lights
        //if ( vSiArealight )
        if ( bool(o.GetParameterValue(L"LightArea")) == true)
        {
            //-- samples U + V / 2
            int U_samples = o.GetParameterValue(L"LightAreaSampU");
            int V_samples = o.GetParameterValue(L"LightAreaSampV");
            //--
            float size_X(o.GetParameterValue(L"LightAreaXformSX"));
            float size_Y(o.GetParameterValue(L"LightAreaXformSY"));            
    
            //--
            f << "\nAreaLightSource \"area\"\n";
	        f << "  \"float gain\" [" << vIntensity << "]\n";
            f << "  \"float importance\" [1.0]\n"; // TODO
	        f << "  \"float power\" ["<< float(o.GetParameterValue(L"LightEnergyIntens"))/100 <<"]\n";
	        f << "  \"float efficacy\" [17.0]\n";
	        f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
	        f << "  \"integer nsamples\" ["<< (U_samples + V_samples)/2 <<"]\n";
            //--
	        if ( vUse_IES )
            {
                f << "  \"string iesname\" [\"" << replace(ies_file.GetAsciiString()) << "\"]\n";
            }
            //-
            int vlight_geo = o.GetParameterValue(L"LightAreaGeom");
            //-- 'hide geometry' only is used for not generate shadows with other light
            if ( vlight_geo == 1 ) //-- square/rectangle
            {
                CString aPoints = luxsi_area_transf(o, size_X, size_Y);
                //--
                f << "\nShape \"trianglemesh\"\n";
	            f << "  \"integer indices\" [0 1 2 0 2 3]\n";
	            f << "  \"point P\" ["<< aPoints.GetAsciiString() <<"]\n";
            }
            else if ( vlight_geo == 3 )// sphere
            {
                f << "\nTransformBegin \n";
                f << "Translate "<< light_from[0] <<" "<< -light_from[2] <<" "<< light_from[1] <<"\n";
            
                f << "\nShape \"sphere\"\n";
                f << "  \"float radius\" ["<< size_X <<"]\n";
                //-
                f << "\nTransformEnd \n"; 
            }
            else
            {
                app.LogMessage(L"Not valid shape, use square or sphere", siWarningMsg);
            }
       
       
        /*
        else if (Light_Shader_ID == L"lux_point_light")
        {
            //--
            //
            //lux_point_light.color.green", 0.121, "")
            //lux_point_light.gain", 106.742, "")
            //lux_point_light.importance", 106.742, "")
            //lux_point_light.iesfile", "H:\\wip\\15.ies", "")
            //lux_point_light.flipz", True, "")
            //lux_point_light.power", 86.384, "")
            //lux_point_light.efficacy", 0.191, "")
            
            //
            f << "LightSource \"point\"\n";
	        f << "  \"float gain\" [" << float(s.GetParameterValue(L"gain")) <<"]\n";
            f << "  \"float importance\" [" << float(s.GetParameterValue(L"importance")) <<"]\n"; // TODO
	        f << "  \"float power\" ["<< float(s.GetParameterValue(L"power")) <<"]\n";
	        f << "  \"float efficacy\" ["<< float(s.GetParameterValue(L"efficacy")) <<"]\n";
	        f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
	        //f << "  \"integer nsamples\" ["<< (U_samples + V_samples)/2 <<"]\n";
            //--
            CString ies_path = s.GetParameterValue(L"iesfile");
            //-
            if (ies_path != L"")
            {
                f << "  \"string iesname\" [\""<< replace(ies_path.GetAsciiString()) <<"\"]\n";
            }
            //-
            if (bool(s.GetParameterValue(L"usesphere")) == true)
            {
                
               // lux_point_light.usesphere", True, "")
               // lux_point_light.radius", 88.849, "")
               // lux_point_light.shadowray", 4.238, "")
               // lux_point_light.hiddengeo", True, "")
               //
                f << "\nShape \"sphere\"\n";
                f << "  \"float radius\" ["<< float(s.GetParameterValue(L"radius")) <<"]\n";
                //f << "  \"float shadowr\" ["<< float(s.GetParameterValue(L"shadowray")) <<"]\n";
                //f << "  \"bool hiddengeo\" ["<< float(s.GetParameterValue(L"hiddengeo")) <<"]\n";
            }
            */
        }
        else //- simple point light, no area
        {
            f << "\nLightSource \"point\"\n";
            f << "  \"float gain\" [" << vIntensity << "]\n";
            f << "  \"color L\" [" << a << "  " << b << "  " << c << "]\n";
            //--
            if ( vUse_IES )
            {
                f << "  \"string iesname\" [\"" << replace(ies_file.GetAsciiString()) << "\"]\n";
                f << "  \"bool flipz\" [\"true\"]\n"; // TODO; option into 'special' tab
            }
            f << "  \"point from\" [" << light_from.GetX() << " " << -light_from.GetZ() << " " << light_from.GetY() << "]\n";
        }
    }
}
//--
CString luxsi_area_transf(X3DObject o, float size_X, float size_Y)
{
    //- initial state
    KinematicState area_state = o.GetKinematics().GetGlobal(); 
    //- transform
    CTransformation area_transf = area_state.GetTransform();
    //-
    CVector3 p1, p2, p3, p4;
    //-
    p1.Set(-size_X/2, size_Y/2, 0.0);   p1.MulByTransformationInPlace(area_transf);
    p2.Set(-size_X/2, -size_Y/2, 0.0);  p2.MulByTransformationInPlace(area_transf);
    p3.Set(size_X/2, -size_Y/2, 0.0);   p3.MulByTransformationInPlace(area_transf);
    p4.Set(size_X/2, size_Y/2, 0.0);    p4.MulByTransformationInPlace(area_transf);
    //-         
    CString aPoints = L"";
    aPoints += CString( p1[0] ) + L" "+ CString( -p1[2] ) + L" "+ CString( p1[1] ) + L" "; 
    aPoints += CString( p2[0] ) + L" "+ CString( -p2[2] ) + L" "+ CString( p2[1] ) + L" ";  
    aPoints += CString( p3[0] ) + L" "+ CString( -p3[2] ) + L" "+ CString( p3[1] ) + L" ";  
    aPoints += CString( p4[0] ) + L" "+ CString( -p4[2] ) + L" "+ CString( p4[1] ); 
                       
    return aPoints;
}
//--
CString find_XSI_env(CString env_file)
{
    CString vFile_env = L"";
    //- is need?
    env_file = L""; 

    /* /-- for search parameters..
    CRefArray prop_pass = app.GetActiveProject().GetActiveScene().GetActivePass().GetParameters();
    for (int pp=0; pp < prop_pass.GetCount(); pp++)
    {
        Parameter pname(prop_pass[pp]);
        CString prop_pas_name = pname.GetName();
       // app.LogMessage(L"Property name: "+ prop_pas_name);    
    }*/

    CRefArray aEnv = app.GetActiveProject().GetActiveScene().GetActivePass().GetNestedObjects();
    //-
    for (int i=0; i < aEnv.GetCount(); i++)
    {
        //- test
        //CString sio_name = SIObject(aEnv[i]).GetName();
        //app.LogMessage(L"SIOBject name: "+ sio_name);            
        //-
        if (SIObject(aEnv[i]).GetName()==L"Environment Shader Stack")
        {
            CRefArray aImages = SIObject(aEnv[i]).GetNestedObjects();
            for (int j=0;j<aImages.GetCount();j++)
            {
                if (SIObject(aImages[j]).GetType()==L"Shader")
                {
                    Shader s(aImages[j]);
                    CRefArray aEnvImg = s.GetImageClips();
                    for (int k=0; k < aEnvImg.GetCount(); k++)
                    {
                        ImageClip2 vImgClip(aEnvImg[k]);
                        Source vImgClipSrc(vImgClip.GetSource());
                        //--
                        vFile_env = vImgClipSrc.GetParameterValue( L"path");
                        //--
                        if (vFile_env !=L"")
                        {
                            string::size_type hdr = string(CString(vFile_env).GetAsciiString()).find( ".hdr", 0 );
                            string::size_type exr = string(CString(vFile_env).GetAsciiString()).find( ".exr", 0 );
	                        if ( hdr != string::npos || exr != string::npos )
                            {
                                env_file = vFile_env;
                            }
                            else
                            {
                                app.LogMessage(L" Not valid IBL file: "+ vFile_env, siWarningMsg);
                            }
                        }
                        else
                        {
                            app.LogMessage(L" None file selected for environment", siWarningMsg);
                        }
                    }
                }
            }
        }
    }
    return env_file;
}
//--