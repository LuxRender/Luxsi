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

#include "include\luxsi_material.h"

using namespace std;
using namespace XSI;
using namespace MATH;

/*
mat_names = {
	'matte': 'Matte',
	'mattetranslucent': 'Matte Translucent',
	'glossy': 'Glossy',
	'glossycoating': 'Glossy Coating',
	'glossytranslucent': 'Glossy Translucent',
	'glass': 'Glass',
	'glass2': 'Glass2',
	'roughglass': 'Rough Glass',
	'mirror': 'Mirror',
	'carpaint': 'Car Paint',
	'metal': 'Metal',
	'metal2': 'Metal2',
	'shinymetal': 'Shiny Metal',
	'velvet': 'Velvet',
	'scatter': 'Scatter',
	'mix': 'Mix',
	'layered': 'Layered',
	'null': 'Null',
}
master_color_map = {
		'carpaint': 'Kd',
		'glass': 'Kt',
		'roughglass': 'Kt',
		'glossy': 'Kd',
		'glossytranslucent': 'Kd',
		'matte': 'Kd',
		'mattetranslucent': 'Kr',
		'metal2': 'Kr',
		'shinymetal': 'Kr',
		'mirror': 'Kr',
		'scatter': 'Kd',
		'velvet': 'Kd',
	}
*/

//- Search materials library.
CRefArray sceneCollectionMaterials()
{
    //- clear Arrays..
    sceneMatLibraries.Clear();
    aMats.Clear();
    //-
    Scene scene = app.GetActiveProject().GetActiveScene();
    sceneMatLibraries = scene.GetMaterialLibraries();

    //- Acces to all material libraries into a scene;
    for (long lib = 0; lib < sceneMatLibraries.GetCount(); lib++)
    {
        Library matLibrary(sceneMatLibraries[lib]);
        materialLibs  = matLibrary.GetItems();
        //-
        for ( long ml = 0; ml < materialLibs.GetCount(); ml++)
        {
            Material libMat(materialLibs[ml]);
            aMats.Add(libMat);
        }
    }
    return aMats;
}
//-
CString writeLuxsiShader()
{
    //- create all Materials Collection.
    CRefArray sceneMats = sceneCollectionMaterials();

    //-first of all..
    aMatList.Clear();
    //- clear data
    materialData.Clear();

    //-
    for ( LONG i=0; i < sceneMats.GetCount(); i++ )
    {
        //- clear shader containers..
        shaderStr.Clear();
        texStr.Clear();
        vChanel.Clear();
        shaderType.Clear();

        //------------------
        mat = sceneMats[i];

        /* Write only materials in use.
        */
        if ( int(mat.GetUsedBy().GetCount())== 0 ) continue;

        CString MatName(mat.GetName());
        /*
        * Prevent to duplicate material definitions.
        */
        if ( luxsi_find(aMatList, MatName )) continue;
        /*
        * filter Preview material name // MatName != L"Preview")
        */        
        if (is_preview && MatName != vmatPreview) continue;
        //-
        aMatList.Add(MatName);
        //--
        CRefArray shad(mat.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
        //--
        for ( long j = 0; j < shad.GetCount(); j++)
        {
            Shader s(shad[j]);
            //- name
            CString shader_name = s.GetName();
            if ( luxdebug ) app.LogMessage(L"Name of shader: "+ shader_name);

            //- shader node ID
            CString vMatID((s.GetProgID()).Split(L".")[1]);
            if ( luxdebug ) app.LogMessage(L"Shader ID: "+ vMatID);

            //-- specific LuxRender shaders  --------------------------////
            if (vMatID == L"lux_glass" || vMatID == L"lux_roughglass")
            {
                shaderStr = write_lux_glass(s, vMatID);
                //-
                vIsSet = true;
            }
            //--
            else if (vMatID == L"lux_metal")
            {
                shaderStr = write_lux_metal(s);
                //-
                shaderType = L"metal";
                vIsSet = true;
            }
            //-- metal 2
            /*
            Texture "Material_nk" "fresnel" "preset"
	            "string name" ["aluminium"]

            MakeNamedMaterial "Material"
	            "float uroughness" [0.075000002980232]
	            "float vroughness" [0.075000002980232]
	            "texture fresnel" ["Material_nk"]
	            "string type" ["metal2"]
            */
            else if (vMatID == L"lux_shinymetal")
            {
                shaderStr = write_lux_shinymetal(s);
                //-
                shaderType = L"shinymetal";
                vIsSet = true;
            }
            //--
            else if (vMatID == L"lux_substrate")
            {
                shaderStr = write_lux_substrate(s);
                //-
                shaderType = L"substrate";
                vIsSet = true;
            }
            else if (vMatID == L"lux_velvet")
            {
                /*
                Material* Velvet::CreateMaterial(const Transform &xform,
		        const ParamSet &mp) {
	                boost::shared_ptr<Texture<SWCSpectrum> > Kd(mp.GetSWCSpectrumTexture("Kd", RGBColor(.3f)));
	                boost::shared_ptr<Texture<float> > P1(mp.GetFloatTexture("p1", -2.f));
	                boost::shared_ptr<Texture<float> > P2(mp.GetFloatTexture("p2", 20.f));
	                boost::shared_ptr<Texture<float> > P3(mp.GetFloatTexture("p3", 2.f));
	                boost::shared_ptr<Texture<float> > Thickness(mp.GetFloatTexture("thickness", 0.1f));
	                return new Velvet(Kd, P1, P2, P3, Thickness, mp);
                }
                */
                //-- velvet
                shaderStr = mat_value(s, L"Kd", L"Kd")+
                    floatToString(s, L"thickness")+
                    floatToString(s, L"p1")+
                    floatToString(s, L"p2")+
                    floatToString(s, L"p3");
                shaderType = L"velvet";
                vIsSet = true;                
            }
            else if (vMatID == L"lux_scatter")
            {
                //-- scatter
                shaderStr = mat_value(s, L"Kd", L"Kd")+
                    floatToString(s, L"g");
                shaderType = L"scatter";
                vIsSet = true;
            }
            else if (vMatID == L"lux_glossy")
            {
                shaderStr = write_lux_glossy(s);
                //-
                vIsSet = true;                
            }
            /*
            //-- glossycoated
            MakeNamedMaterial "base"
	            "color Kd" [0.63999999 0.08297527 0.13193677]
	            "float sigma" [0.000000000000000]
	            "string type" ["matte"]

            MakeNamedMaterial "Material"
	            "bool multibounce" ["false"]
	            "color Ks" [0.04000000 0.04000000 0.04000000]
	            "float index" [0.000000000000000]
	            "float uroughness" [0.075000002980232]
	            "float vroughness" [0.075000002980232]
	            "string basematerial" ["base"]
	            "string type" ["glossycoating"]

            */

            else if ( vMatID == L"lux_matte" || vMatID == L"lux_mattetranslucent")
            {
                shaderStr = write_lux_matte(s, vMatID);
                //-
                vIsSet = true;
            }
            //-- for meshlight test -------------------------
            else if (vMatID == L"lux_emitter_mat")
            {
                shaderStr = mat_value(s, L"Kd", L"Kd");
                shaderType = L"matte";
                vIsSet = true;
            }
            //-----------------------------------------------
            else if (vMatID == L"lux_car_paint")
            {
                shaderStr = write_lux_car_paint(s);
                //-
                shaderType = L"carpaint";
                vIsSet = true;
            }
            //--
            else if (vMatID==L"lux_mirror")
            {
                shaderStr = mat_value(s, L"Kr", L"Kr")+
                    floatToString(s, L"film")+
                    floatToString(s, L"filmindex");
                //-
                shaderType = L"mirror";
                vIsSet = true;
            }
            //--
            else if (vMatID == L"mia_material_phen")
            {
                shaderStr = write_mia_material_phen(s);
                //--
                vIsSet=true;
            }
            //--
            else if (vMatID==L"material-phong")
            {
                shaderStr += mat_value(s, L"Kd", L"diffuse");
                shaderStr += L" \"float uroughness\" ["+ CString((float(s.GetParameterValue(L"shiny"))/10)) + L"]\n";
                shaderStr += L" \"float vroughness\" ["+ CString((float(s.GetParameterValue(L"shiny"))/10)) + L"]\n";
                shaderStr += mat_value(s, L"Ks", L"specular");
                //--
                shaderType = L"glossy";
                vIsSet = true;
            }
            //--
            else if (vMatID==L"material-lambert")
            {
                shaderStr += mat_value(s, L"Kd", L"diffuse");
                shaderStr += L" \"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
            }
            //--
            else if (vMatID == L"material-ward")
            {
                shaderStr += mat_value(s, L"Kd", L"diffuse");
                shaderStr += L" \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_u"))/10) + L"]\n";
                shaderStr += L" \"float vroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_v"))/10) + L"]\n";
                //--------------
                //- for revised
                //--------------
                shaderStr += L" \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                //--
                shaderType = L"glossy";
                vIsSet = true;
            }
            //--
            else if (vMatID == L"material-constant")
            {
                shaderStr += mat_value(s, L"Kd", L"color");
                shaderStr += L" \"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
            }
            else if (vMatID==L"material-strauss")
            {
                shaderStr += mat_value(s, L"Kd", L"diffuse");
                shaderStr += L" \"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
            }
            /*
            else if (vMatID == L"XSINormalMap2")
            {
                //--
                app.LogMessage(L"XSINormalMap2 node connected. Write texture 'normalmap'");
            }
            else if (vMatID == L"sib_zbump")
            {
                app.LogMessage(L"'sib_zbump'(Bumpmap) node connected. Write texture 'bumpmap'");
            }
            */
            else
            {
                // fall back shader
                app.LogMessage(L"Non valid shader for LuxRender: ["+ vMatID + L"]. Use the default shader values", siErrorMsg);
                //-
                shaderType = L"matte"; 
                shaderStr = L" \"color Kd\" [0.7 0.7 0.7]\n";
                shaderStr += L" \"float sigma\" [0.1]\n";
            }
            
            /** If have texture connected...
            *   The cycle to process all the connected textures to the shader,
            *   is executed inside the luxsi_texture() function.
            */
            if ( s.GetShaders().GetCount() > 0 ) texStr = luxsi_texture(mat, s); 
            

        }//- process Preview Material ----------------------------------/
        if ( is_preview && MatName == vmatPreview )
        {
            CString prev_material;  //- for material preview data
            //-
            if (luxdebug) app.LogMessage(L"[DEBUG]: Process data for Material Preview");

            //- texture component
            if ( texStr != L"") prev_material = texStr + L"\n";
            
            //- shader component
            prev_material += L"\nMakeNamedMaterial \"sphere_mat\" \n";
            prev_material += L" \"string type\" [\""+ shaderType + L"\"]\n";
            prev_material += shaderStr;
                        
            return prev_material;
            break; // placed here ?? Investigate about this option
        }//--------------------------------------------------------------/
        else
        {
            //-  request texture data 
            if (texStr != L"") 
            {
                materialData += texStr;
            }
            //- write shader
            materialData += L"\nMakeNamedMaterial \""+ MatName + L"\" \n";
            materialData += L" \"string type\" [\""+ shaderType + L"\"]\n";
            materialData += shaderStr;
        }
    }
    return materialData;
}
//--
CString mat_value(Shader s, CString in_texture, CString in_shader_port)
{
    //-------------
    //- status: Ok
    //-------------
    if ( luxdebug ) app.LogMessage(L"in texture: "+ in_texture + L" in node color: "+ in_shader_port);
    
    CString _component = L"";
    //--
    CString texName = find_shader_used(s, in_shader_port );

    //--
    Texture tex_shader = find_tex_used(s, texName);
    //-
    CString t_name = tex_shader.GetName(); 
    //-
    if ( luxdebug ) app.LogMessage(L"Connect..: "+ in_shader_port + L" port, to texture: "+ tex_shader.GetName());
    
    //-
    if ( tex_shader.GetName() != L"")
    {
        CString tex_name = mat.GetName()+ L"_"+ tex_shader.GetName(); // name of texture connect to port

        vChanel = in_texture; // texture type ( Kd, Kr, Kt...)
        _component += L" \"texture "+ in_texture + L"\" [\""+ tex_name + L"\"]\n";
    }
    else
    {
        s.GetColorParameterValue(in_shader_port, red, green, blue, alpha );
        //-
        _component += L" \"color "+ in_texture
                      + L"\" ["+ CString(red)
                      + L" "+ CString(green)
                      + L" "+ CString(blue) + L"]\n";
    }
    //-
    return _component;
}
//
CString write_lux_glass(Shader s, CString vMatID)
{
    //--------------
    //- status: Ok
    //--------------
    CString shStr;
    //-- reflectivity component
    shStr = mat_value(s, L"Kr", L"Kr");

    //-- transmitivity component
    shStr += mat_value(s, L"Kt", L"Kt");
    //-
    shStr += floatToString(s, L"index")+
        floatToString(s, L"cauchyb");
    //-
    shaderType = L"glass";
    //--
    if ( vMatID == L"lux_roughglass" )
    {
        shStr += floatToString(s, L"uroughness")+
            floatToString(s, L"vroughness");
        shStr += L" \"bool dispersion\" [\""+ CString(s.GetParameterValue(L"dispersion")) + L"\"]\n";
        //-
        shaderType = L"roughglass";
    }
    else
    {
        shStr += floatToString(s, L"film")+
            floatToString(s, L"filmindex");
        shStr += L" \"bool architectural\" [\""+ CString(s.GetParameterValue(L"architectural")) + L"\"]\n";
        //--
    }
    //--
    return shStr;
}
//--
CString write_lux_metal(Shader s)
{
    //-----------------------------
    //  Status: Add NK files option
    //-----------------------------
    CString metalStr;
    //-
    const char *ametal [5] = {"amorphous carbon", "silver", "gold", "copper", "aluminium"};
    int nmetal = s.GetParameterValue(L"mname");
    //--
    //shStr += floatToString(s, L"uroughness")+
    //    floatToString(s, L"vroughness");
    //-
    if (s.GetParameterValue(L"NKfile") == true )
    {
        //shaderStr += L"  \"string filename\" [\""+ /* NK file */ + L"\"]\n";
    }
    else
    {
        metalStr += L" \"string name\" [\""+ CString(ametal[nmetal]) + L"\"]\n";
    }
    //--
    return metalStr;
}
//--
CString write_lux_car_paint(Shader s)
{
    //--------------
    //  status: Ok
    //--------------
    CString shStr = L"";
    //-- car paint
    const char *A_carpaint [] = {"Manual Settings","2k acrylack", "blue", "blue matte",
        "bmw339", "ford f8", "opel titan", "polaris silber", "white"
    };
    int presets = s.GetParameterValue(L"presets");

    //-- especular primary
    float spr = 0.0, spg = 0.0, spb = 0.0, spa = 0.0;
    s.GetColorParameterValue(L"spec", spr, spg, spb, spa );
    //-- spec secondary
    float spr2= 0.0, spg2 = 0.0, spb2 = 0.0, spa2 = 0.0;
    s.GetColorParameterValue(L"spec_sec", spr2, spg2, spb2, spa2 );
    //- diffuse
    s.GetColorParameterValue(L"kd", red, green, blue, alpha );
    //-- reflect
    float r = 0.0, g = 0.0, b = 0.0, a = 0.0;
    s.GetColorParameterValue(L"reflectivity", r, g, b, a );
    //--
    if ( presets == 0 )
    {
        shStr += L" \"color Kd\" ["+ CString(r) + L" "+ CString(g) + L" "+ CString(b) + L"] \n";
        shStr += L" \"color Ks1\" ["+ CString(spr) + L" "+ CString(spg) + L" "+ CString(spb) + L"] \n";
        shStr += L" \"color Ks2\" ["+ CString(spr2) + L" "+ CString(spg2) + L" "+ CString(spb2) + L"] \n";
        shStr += L" \"color Ks3\" ["+ CString(spr2) + L" "+ CString(spg2) + L" "+ CString(spb2) + L"] \n";
        shStr += floatToString(s, L"M1", L"m1")+
            floatToString(s, L"M2", L"m2")+
            floatToString(s, L"M3", L"m3")+ 
            floatToString(s, L"R1", L"r1")+
            floatToString(s, L"R2", L"r2")+
            floatToString(s, L"R3", L"r3");
    }
    else
    {
        shStr += L" \"string name\" [\""+ CString(A_carpaint[presets]) + L"\"]\n"; // presets
    }
    //--
    return shStr;
}
//--
CString write_mia_material_phen(Shader s)
{
    //---------------------
    //- status: FOR REVISED
    //---------------------
    CString shStr = L"";
    //-
    float sp_red, sp_green, sp_blue, sp_alpha, mRough;
    //- arch vis
    s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
    s.GetColorParameterValue(L"refl_color", sp_red, sp_green, sp_blue, sp_alpha );
    //-
    mRough =  s.GetParameterValue(L"reflect_glossy");
    float refl = s.GetParameterValue(L"reflectivity");
    float brdf = s.GetParameterValue(L"brdf_0_degree_refl");
    //--
    if ( refl > 0 )
    {
        sp_red = refl * sp_red * brdf;
        sp_green = refl * sp_green * brdf;
        sp_blue = refl * sp_blue * brdf;
        //--
        shStr += L" \"color Kr\" [" + CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
        shStr += L" \"float uroughness\" ["+ CString(1-mRough) + L"]\n";
        shStr += L" \"float vroughness\" ["+ CString(1-mRough) + L"]\n";
        shStr += L" \"color Ks\" ["+ CString(sp_red) + L" "+ CString(sp_green) + L" "+ CString(sp_blue) + L"]\n";
        //--
        shaderType = L"shinymetal";
    }
    else
    {
        //- diffuse color
        //s.GetColorParameterValue(L"kd",red,green,blue,alpha ); // TODO; revised
        shStr += L" \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
        shStr += L" \"float sigma\" ["+ CString(mRough) + L"]\n";
        //-
        shaderType = L"matte";

    }
    //--
    return shStr;
}
//--
CString write_lux_shinymetal(Shader s)
{
    //--------------
    //- status: Ok
    //--------------
    CString shStr = L"";
    //- specular component
    shStr += mat_value(s, L"Ks", L"Ks");

    //-- reflection component
    shStr += mat_value(s, L"Kr", L"Kr");
    //--
    shStr += floatToString(s, L"uroughness")+
        floatToString(s, L"vroughness");
    //--
    return shStr;
}
//--
CString write_lux_substrate( Shader s)
{
    //--------------
    //- status: Ok
    //--------------
    CString shStr = L"";
    //- diffuse component 
    shStr += mat_value(s, L"Kd", L"Kd");

    //-- specular component
    shStr += mat_value(s, L"Ks", L"Ks");
    //--
    shStr += floatToString(s, L"uroughness")+
        floatToString(s, L"vroughness");
    //--
    return shStr;
}
//--
CString write_lux_matte(Shader s, CString vMatID)
{
    //-------------
    //  Status: OK
    //-------------
    
    CString shStr=L"";

    //- matte...
    if (vMatID == L"lux_matte" )
    {
        //- diffuse component
        shStr += mat_value(s, L"Kd", L"Kd");
        //-
        shaderType = L"matte";

        // test for bump
        if( bool(s.GetParameterValue(L"use_bump")) == true )
        {
            CString _bump = mat_value(s, L"bumpmap", L"bump_data");
            //-
            shStr += _bump; 
        }
    }
    //- ...matte translucent
    else
    {
        //-- reflectivity component
        shStr += mat_value(s, L"Kr", L"Kr");

        //-- transmitivity component
        shStr += mat_value(s, L"Kt", L"Kt");

        //-
        bool energy_conserving = false;
        if ( bool(s.GetParameterValue(L"energy"))== true)
        {
            energy_conserving = true;
        }
        //-
        shStr += L" \"bool energyconserving\" [\""+ CString(energy_conserving) + L"\"]\n";
        shaderType = L"mattetranslucent";
    }
    //-
    shStr += floatToString(s, L"sigma");

    return shStr;
}
//--
CString write_lux_glossy(Shader s)
{
    //-- for glossy and glossy translucent shaders.
    CString glossyData;
    //-
    int bounce = s.GetParameterValue(L"multibounce");
    //-
    glossyData = mat_value(s, L"Kd", L"Kd");
    glossyData += mat_value(s, L"Ks", L"Ks");
    glossyData += L" \"bool multibounce\" [\""+ CString(MtBool[bounce]) + L"\"]\n";
	glossyData += floatToString(s, L"index")+
        floatToString(s, L"uroughness")+
        floatToString(s, L"vroughness");                
    //-
	shaderType = L"glossy";
    //-
    if ( bool(s.GetParameterValue(L"translucent"))== true)
    {
        bool onesided = s.GetParameterValue(L"onesided");
        glossyData += mat_value(s, L"Kt", L"Kt");
        glossyData += L" \"bool onesided\" [\""+ CString(MtBool[onesided]) + L"\"]\n";
        //-
        shaderType = L"glossytranslucent";
    }
    else
    {
        glossyData += floatToString(s, L"sigma");
    }
    return glossyData;
}
