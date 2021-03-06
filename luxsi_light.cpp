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

#include "include\luxsi_lights.h"

using namespace std;
using namespace XSI;
using namespace MATH;

//--
void sceneCollectionLights()
{
    //-
    Model root = app.GetActiveSceneRoot();
    CRefArray sceneObjectLights = root.FindChildren( L"", L"light", CStringArray(), true);
    for( long i = 0; i < sceneObjectLights.GetCount(); i++)
    {
        X3DObject o(sceneObjectLights[i]);
    }
}
//-

CString writeLuxsiLight()
{
    sceneLight.Clear();
    Alights.Clear();
    root = app.GetActiveSceneRoot();
    CString lightData;

    /* create an array objects with the 'lights' of the scene.
    */
    sceneLight += root.FindChildren( L"", L"light", CStringArray(), true );
    
    //-- check visibility
    for ( int i=0; i < sceneLight.GetCount(); i++ )
    {
        X3DObject o(sceneLight[i]);
        //-
        if ( !is_visible(o, L"light")) continue;
            
        //- light object
        KinematicState  from_global_kinex_state;
        CTransformation light_from_global_transf;
        //- set state for 'point from'
        from_global_kinex_state = o.GetKinematics().GetGlobal();
        light_from_global_transf = from_global_kinex_state.GetTransform();

        //- point from
        CVector3 light_from;
        light_from.MulByTransformationInPlace(light_from_global_transf);

        //- light object interest. Target object for 'infinite' or 'spot' lights
        X3DObject light_interest = Light(o).GetInterest();
            
        //- set state for 'point to'
        KinematicState  to_global_state;
        CTransformation light_to_global_transf;
        //-
        to_global_state = light_interest.GetKinematics().GetGlobal();
        light_to_global_transf = to_global_state.GetTransform();

        //- point to
        CVector3 light_to;
        light_to.MulByTransformationInPlace(light_to_global_transf);

        //--
        Shader s((Light(o).GetShaders())[0]);
        /** Same name for xsi and Lux shaders: 'color'
        *   if no have any shader, the color is black (0,0,0).
        */
        s.GetColorParameterValue(L"color", red, green, blue, alpha);

        //-- finding for light group name
        CString group_name = o.GetName();
        //-
        if ( findInGroup(o.GetName()) != L"")
        {
            group_name = findInGroup(o.GetName());
        }
        //--
        int lType = Light(o).GetParameterValue(L"Type");

        //--
        float vLightCone = o.GetParameterValue(L"LightCone"); // is only for spot light??
        vIntensity = s.GetParameterValue(L"intensity");
        float vSpotblend = s.GetParameterValue(L"spread"); // move to spot light??

        //-- search 'Programmatic Identification (ID)' for Light Shader Nodes
        Light_Shader_ID =s.GetProgID().Split(L".")[1];
        //--
        if ( luxdebug ) app.LogMessage(L"Light shader used: "+ Light_Shader_ID);
        //-
        lightData += L"\nLightGroup \""+ group_name + L"\" \n";

        //- for use Image Based Light ( IBL )
        //if ( luxdebug ) app.LogMessage(L"XSI environment finded: "+ find_XSI_env());
            
        //-
        CString xsi_env = find_XSI_env();
        //-
        if ( xsi_env != L"" )
        {
            lightData += L"LightSource \"infinite\"\n";
            lightData += L"  \"float gain\" ["+ CString( vIntensity ) + L"]\n";
            lightData += L"  \"float importance\" [1.0]\n"; // TODO
            lightData += L"  \"string mapname\" [\""+ CString( luxsi_replace(xsi_env.GetAsciiString()).c_str() ) + L"\"]\n";
            lightData += L"  \"string mapping\" [\"latlong\"]\n"; // TODO
            lightData += L"  \"float gamma\" [1.0]\n";
            lightData += L"  \"integer nsamples\" [1]\n"; // TODO
        }
        //-
        else if (lType == 2) //-- spot
        {
            //-- values
            lightData += L"\nLightSource \"spot\"\n";
            lightData += L"  \"float gain\" ["+ CString( vIntensity ) + L"]\n";
            lightData += L"  \"color L\" ["
                + CString( red )    + L" "
                + CString( green )  + L" "
                + CString( blue )   + L"]\n";
            //--
            lightData += L"  \"point from\" ["
                + CString( light_from[0] )  + L" "
                + CString( -light_from[2] ) + L" "
                + CString( light_from[1] )  + L"]\n";
            lightData += L"  \"point to\" ["
                + CString( light_to[0] )    + L" "
                + CString( -light_to[2] )   + L" "
                + CString( light_to[1] )    + L"]\n";
            lightData += L"  \"float coneangle\" ["+ CString( vLightCone ) + L"]\n";
            lightData += L"  \"float conedeltaangle\" ["+ CString( vLightCone - vSpotblend ) + L"]\n";
        }
        else if  (lType == 1) //-- infinite
        {
            CMatrix4 sunTransMat = o.GetKinematics().GetLocal().GetTransform().GetMatrix4();
            //-
            lightData += L"\nLightSource \"sunsky\"\n";
            lightData += L"  \"integer nsamples\" [4]\n";
            lightData += L"  \"vector sundir\" ["
                + CString( sunTransMat.GetValue(2,0) )  + L" "
                + CString( -sunTransMat.GetValue(2,2) ) + L" "
                + CString( sunTransMat.GetValue(2,1) )  + L"]\n";
            //-
            lightData += L"  \"float gain\" ["+ CString( vIntensity ) + L"]\n";
            lightData += L"  \"color L\" ["
                + CString( red )    + L" "
                + CString( green )  + L" "
                + CString( blue )   + L"]\n";
        }
        else // light type 0; point
        {
            //-
            lightData += luxsi_point_light( o, s, light_from);
        }
    }    
    return lightData;
}
//--
CString luxsi_point_light(X3DObject o, Shader s, CVector3 light_from)
{
    CString pLightData;
    /** For Softimage light area; only work into 'point' or 'spot' lights
    *   first check all type shaders connects
    *   if have a LuxRender type I can config the light with the node options
    */
    //-
    if ( luxdebug ) app.LogMessage(L"Color values: "+ CString(red) + L" "+ CString(green) + L" "+ CString(blue));
    //-
    if ( bool(o.GetParameterValue(L"LightArea")) == true)
    {
        //-- samples U + V / 2
        int U_samples = o.GetParameterValue(L"LightAreaSampU");
        int V_samples = o.GetParameterValue(L"LightAreaSampV");
        //--
        float size_X = o.GetParameterValue(L"LightAreaXformSX");
        float size_Y = o.GetParameterValue(L"LightAreaXformSY");
        //-
        pLightData += L"\nAreaLightSource \"area\"\n";
        //-
        if ( Light_Shader_ID == L"soft_light")
        {
            //-
            pLightData += L"  \"float gain\" [1.00]\n";
            pLightData += L"  \"float importance\" [1.0]\n"; // TODO
            pLightData += L"  \"float power\" ["+ CString( float(s.GetParameterValue(L"intensity"))*10 ) + L"]\n";
            pLightData += L"  \"float efficacy\" [17.0]\n"; //TODO
            pLightData += L"  \"color L\" ["+ CString( red ) + L" "+ CString( green ) + L" "+ CString( blue ) + L"]\n";
        
        }
        else if ( Light_Shader_ID == L"lux_point_light")
        {
            /** if is arealight with the 'lux' node connect..
            *   use only the appropiates values from node?
            */
            pLightData += L"  \"float gain\" ["+ CString( float(s.GetParameterValue(L"gain")) ) + L"]\n";
            pLightData += L"  \"float importance\" ["+ CString( float(s.GetParameterValue(L"importance")) ) + L"]\n"; // TODO
            pLightData += L"  \"float power\" ["+ CString( float(s.GetParameterValue(L"power")) ) + L"]\n";
            pLightData += L"  \"float efficacy\" ["+ CString( float(s.GetParameterValue(L"efficacy")) ) + L"]\n";
            pLightData += L"  \"color L\" ["+ CString( red ) + L" "+ CString( green ) + L" "+ CString( blue ) + L"]\n";

            //- IES file seems not work fine into area spherical light
            CString ies_path = s.GetParameterValue(L"iesfile");
            //-
            if (ies_path != L"")
            {
                pLightData += L"  \"string iesname\" [\""+ CString( luxsi_replace(ies_path.GetAsciiString()).c_str() ) + L"\"]\n";
            }
        }
        else
        {
            //-- log error and use default values.
            app.LogMessage(L"Not valid 'node' connect. Default values in use..", siWarningMsg);
            //-
            pLightData += L"  \"color L\" [.91 .91 .91]\n";
            pLightData += L"  \"integer nsamples\" [4]\n";
        }

        /** now,  the geometry..
        *   'hide geometry' only is used for not generate shadows with other light
        */
        int vlight_geo = o.GetParameterValue(L"LightAreaGeom");
        //--
        if ( vlight_geo == 1 ) //-- square/rectangle
        {
            CString aPoints = luxsi_area_light_transform(o, size_X, size_Y);
            //--
            pLightData += L"\nShape \"trianglemesh\"\n";
            pLightData += L"  \"integer indices\" [0 1 2 0 2 3]\n";
            pLightData += L"  \"point P\" ["+ CString( aPoints.GetAsciiString() ) + L"]\n";
        }
        else if ( vlight_geo == 3 )// sphere
        {
            /** in LuxBlend this option is only for use in 'point light'??
            *   really, sphere is an area light object, like XSI
            */
            pLightData += L"\nTransformBegin \n";
            pLightData += L"Translate "+ CString( light_from[0] ) + L" "+ CString( -light_from[2] ) + L" "+ CString( light_from[1] ) + L"\n";

            pLightData += L"\nShape \"sphere\"\n";
            pLightData += L"  \"float radius\" ["+ CString( size_X ) + L"]\n";
            //-
            pLightData += L"\nTransformEnd \n";
        }
        else
        {
            app.LogMessage(L"Not valid shape, use square or sphere", siWarningMsg);
        }
    }
    else
    {
        /** Basic point light, with 'node' by defaul to XSI( soft_light )
        */
        if ( Light_Shader_ID == L"soft_light")
        {
            pLightData += L"\nLightSource \"point\"\n";
            pLightData += L"  \"float gain\" ["+ CString( vIntensity * 10 ) + L"]\n";
            pLightData += L"  \"color L\" ["
                + CString( red ) + L" "
                + CString( green ) + L" "
                + CString( blue ) + L"]\n";
            pLightData += L"  \"point from\" ["
                + CString( light_from[0] ) + L" "
                + CString( -light_from[2] ) + L" "
                + CString( light_from[1] ) + L"]\n";
        }
        else if ( Light_Shader_ID == L"lux_point_light")
        {
            //-
            bool usesphere = s.GetParameterValue(L"usesphere");
            CString ies_path = s.GetParameterValue(L"iesfile");
            
            if ( !usesphere ) pLightData += L"\nLightSource \"point\"\n";
            if ( usesphere ) pLightData += L"\nAreaLightSource \"area\"\n"; 
            //-
            pLightData += L"  \"float gain\" ["+ CString( float(s.GetParameterValue(L"gain")) ) + L"]\n";
            pLightData += L"  \"float importance\" ["+ CString( float(s.GetParameterValue(L"importance")) ) + L"]\n"; // TODO
            pLightData += L"  \"float power\" ["+ CString( float(s.GetParameterValue(L"power")) ) + L"]\n";
            pLightData += L"  \"float efficacy\" ["+ CString( float(s.GetParameterValue(L"efficacy")) ) + L"]\n";
            pLightData += L"  \"color L\" ["+ CString( red ) + L" "+ CString( green ) + L" "+ CString( blue ) + L"]\n";
            if (!usesphere)
            {
                pLightData += L"  \"point from\" ["
                    + CString( light_from[0] )  + L" "
                    + CString( -light_from[2] ) + L" "
                    + CString( light_from[1] )  + L"]\n";
            }
            else
            {
                pLightData += L"  \"integer nsamples\" ["+ CString( float(s.GetParameterValue(L"shadowray")) ) + L"]\n";
            }
            if ( ies_path != L"")
            {
                pLightData += L"  \"bool flipz\" [\""+ CString( MtBool[(s.GetParameterValue(L"flipz"))] ) + L"\"]\n";
                pLightData += L"  \"string iesname\" [\""+ CString( luxsi_replace(ies_path.GetAsciiString()).c_str() ) + L"\"]\n";
            }
            //-
            if ( usesphere )
            {
                //-
                pLightData += L"\nTransformBegin \n";
                pLightData += L"Translate "
                    + CString( light_from[0] )  + L" "
                    + CString( -light_from[2] ) + L" "
                    + CString( light_from[1] )  + L"\n";

                pLightData += L"\nShape \"sphere\"\n";
                pLightData += L"  \"float radius\" ["+ CString( float (s.GetParameterValue(L"radius")) ) + L"]\n";
                //-
                pLightData += L"\nTransformEnd \n";
            }
        }
        else
        {
            //- no shader..
            app.LogMessage(L"Not valid Node connect", siWarningMsg);
        }
    }
    return pLightData;
}
//--
CString luxsi_area_light_transform(X3DObject o, float size_X, float size_Y)
{
    //- initial state
    KinematicState area_state = o.GetKinematics().GetGlobal();
    //- transform
    CTransformation area_transf = area_state.GetTransform();
    //-
    CVector3 p1, p2, p3, p4;
    //-
    p1.Set(-size_X/2, size_Y/2, 0.0);
    p1.MulByTransformationInPlace(area_transf);
    p2.Set(-size_X/2, -size_Y/2, 0.0);
    p2.MulByTransformationInPlace(area_transf);
    p3.Set(size_X/2, -size_Y/2, 0.0);
    p3.MulByTransformationInPlace(area_transf);
    p4.Set(size_X/2, size_Y/2, 0.0);
    p4.MulByTransformationInPlace(area_transf);
    //-
    CString aPoints = L"";
    aPoints += CString( p1[0] ) + L" "+ CString( -p1[2] ) + L" "+ CString( p1[1] ) + L" ";
    aPoints += CString( p2[0] ) + L" "+ CString( -p2[2] ) + L" "+ CString( p2[1] ) + L" ";
    aPoints += CString( p3[0] ) + L" "+ CString( -p3[2] ) + L" "+ CString( p3[1] ) + L" ";
    aPoints += CString( p4[0] ) + L" "+ CString( -p4[2] ) + L" "+ CString( p4[1] );

    return aPoints;
}
//--
CString find_XSI_env()
{
    CString vFile_env = L"";
    //- is need?
    //env_file = L"";

    /** for search parameters..
    *   CRefArray prop_pass = app.GetActiveProject().GetActiveScene().GetActivePass().GetParameters();
    *   for (int pp=0; pp < prop_pass.GetCount(); pp++)
    *   {
    *       Parameter pname(prop_pass[pp]);
    *       CString prop_pas_name = pname.GetName();
    *   // app.LogMessage(L"Property name: "+ prop_pas_name);
    *   }
    */

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
            for (int j=0; j<aImages.GetCount(); j++)
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
                        CString env_file = vImgClipSrc.GetParameterValue( L"path");
                        //--
                        string::size_type hdr = string(CString(env_file).GetAsciiString()).find( ".hdr", 0 );
                        string::size_type exr = string(CString(env_file).GetAsciiString()).find( ".exr", 0 );
                        if ( hdr != string::npos || exr != string::npos )
                        {
                            vFile_env = env_file;
                        }
                        else
                        {
                            app.LogMessage(L" Not valid IBL file: "+ env_file, siWarningMsg);
                        }
                    }
                }
            }
        }
    }
    return vFile_env;
}
