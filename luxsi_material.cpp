/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 2011 2012  Michael Gangolf
Code contributor: Pedro Alcaide

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

#include "include\luxsi_material.h"

using namespace std;
using namespace XSI;
using namespace MATH;


void writeLuxsiShader()
{
    //- begin values..
    aMatList.Clear();

    //- Search materials library.
    Scene scene = app.GetActiveProject().GetActiveScene();
    Library matlib = scene.GetActiveMaterialLibrary();
    CRefArray materials = matlib.GetItems();
    scene.GetMaterialLibraries();

    //- write default bumpmap texture
    f <<"Texture \"mate_bump+normal_generated\" \"float\" \"multimix\" \n";
    f <<"	\"texture tex1\" [\"\"] # bumpmap\n";
    f <<"	\"texture tex2\" [\"\"] # normalmap\n";
    f <<"	\"float weights\" [1.000000000000000 1.000000000000000]\n";

    for ( LONG i=0; i < materials.GetCount(); i++ )
    {
        //-
        shaderStr = L"";
        vChanel = L"";
        shaderType = L"";

        //-------------------------
        //Material m( materials[i] );
        mat = materials[i];
        //-------------------------
        if ( int(mat.GetUsedBy().GetCount())== 0 ) continue;

        CString MatName(mat.GetName());

        if ( !luxsi_find(aMatList, MatName ) ) aMatList.Add(MatName);
        else continue;
        //--
        CRefArray shad(mat.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
        //--
        for ( long j = 0; j < shad.GetCount(); j++)
        {
            Shader s(shad[j]);
            //- name
            CString shader_name = s.GetName();
            app.LogMessage(L"Nombre del shader: "+ shader_name);

            //- shader node ID
            //CString vMatID((s.GetProgID()).Split(L".")[1]);
            CString vMatID((s.GetProgID()).Split(L".")[1]);
            app.LogMessage(L" Shader ID: "+ vMatID);

            //-------------------------
            if (vMatID == L"lux_glass" || vMatID == L"lux_roughglass")
            {
                shaderStr = write_lux_glass(s, shaderStr, vMatID);
                //-
                vIsSet = true;
            }
            //------------------------------
            else if (vMatID == L"lux_metal")
            {
                //--
                shaderStr = write_lux_metal(s, shaderStr);
                shaderType = L"metal";
                vIsSet = true;
            }
            //-----------------------------------
            else if (vMatID == L"lux_shinymetal")
            {
                //--
                shaderStr = write_lux_shinymetal(s, shaderStr);
                shaderType = L"shinymetal";
                vIsSet = true;
            }
            //--------------------------------
            else if (vMatID == L"lux_substrate")
            {
                shaderStr = write_lux_substrate(s, shaderStr);
                shaderType = L"substrate";
                vIsSet = true;
            }
            //-----------------------------------------
            else if ( vMatID == L"lux_matte" || vMatID == L"lux_mattetranslucent")
            {
                shaderStr = write_lux_matte(s, vMatID);
                //-
                vIsSet = true;
            }
            //----------------------------------
            else if (vMatID == L"lux_car_paint")
            {
                shaderStr = write_lux_car_paint(s, shaderStr);
                shaderType = L"carpaint";
                vIsSet=true;
            }
            //--------------------------------------
            else if (vMatID==L"lux_mirror")
            {
                s.GetColorParameterValue(L"kr", red, green, blue, alpha );
                shaderStr = L"  \"color Kr\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                //--
                shaderType = L"mirror";
                vIsSet = true;
            }
            //--------------------------------------
            else if (vMatID == L"mia_material_phen")
            {
                shaderStr = write_mia_material_phen(s, shaderStr);
                //--
                vIsSet=true;
            }
            else if (vMatID==L"material-phong")
            {
                s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
                shaderStr = L" \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
                shaderStr += L" \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]";
                shaderStr += L" \"float vroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]\n";
                s.GetColorParameterValue(L"specular", red, green, blue, alpha );
                shaderStr += L" \"color Ks\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                //--
                shaderType = L"glossy";
                vIsSet = true;
            }
            else if (vMatID==L"material-lambert")
            {
                shaderType=L"matte";
                s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
                shaderStr = L"	\"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                shaderStr += L"	\"float sigma\" [0]\n";
                //--
                vIsSet=true;
            }
            //----------------------------------
            else if (vMatID == L"material-ward")
            {
                s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
                shaderStr = L" \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                shaderStr += L" \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_u"))/10) + L"]\n";
                shaderStr += L" \"float vroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny_v"))/10) + L"]\n";
                shaderStr += L" \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                //--
                shaderType = L"glossy";
                vIsSet = true;
            }
            //--------------------------------------
            else if (vMatID == L"material-constant")
            {
                //- diffuse color
                s.GetColorParameterValue(L"color", red, green, blue, alpha );
                shaderStr = L" \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                shaderStr += L" \"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
            }
            else if (vMatID==L"material-strauss")
            {
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                shaderStr = L"	\"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                shaderStr += L"	\"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
            }
            else if (vMatID == L"XSINormalMap2")
            {
                //--
                app.LogMessage(L"XSINormalMap2 node connected. Write texture 'normalmap'");
            }
            else if (vMatID == L"sib_zbump")
            {
                app.LogMessage(L"'sib_zbump'(Bumpmap) node connected. Write texture 'bumpmap'");
            }

            /* //-- if yet vMatID is defined......
            else if (!vIsSet)
            {
            if (s.GetParameterValue(L"refract_inuse")=="-1")
            {
            //check if material is transparent: phong/lamber/blin/constant/cooktorrance/strauss
            float ior=0.0f;
            s.GetColorParameterValue(L"transparency",red,green,blue,alpha );
            if (red>0 || green>0 || blue>0)
            {
            shaderType=L"glass";
            shaderStr += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            s.GetColorParameterValue(L"reflectivity",sp_red,sp_green,sp_blue,sp_alpha );
            shaderStr += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
            shaderStr += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"index_of_refraction")) + L"]\n";
            shaderStr += L"  \"float cauchyb\" [0]\n";
            //--
            if ((float)s.GetParameterValue(L"trans_glossy")>0 )
            {
            shaderType=L"roughglass";
            shaderStr += L"  \"float uroughness\" ["+ CString((float)s.GetParameterValue(L"trans_glossy"))+ L"]";
            shaderStr += L"  \"float vroughness\" ["+ CString((float)s.GetParameterValue(L"trans_glossy"))+ L"]\n";
            }
            }
            }
            }
            else if (!vIsSet)
            {
            if ( (float)s.GetParameterValue(L"transparency") > 0.0f )
            {
            float ior=0.0f;
            // glass mia-arch shader
            s.GetColorParameterValue(L"refr_color",red,green,blue,alpha );
            s.GetColorParameterValue(L"refl_color",sp_red,sp_green,sp_blue,sp_alpha );
            shaderStr += L"  \"color Kt\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
            shaderStr += L"  \"color Kr\" [" + CString(sp_red) + L" "  + CString(sp_green) +  L" "  + CString(sp_blue) + L"]\n";
            shaderStr += L"  \"float index\" [" + CString((float)s.GetParameterValue(L"refr_ior")) + L"]\n";
            shaderStr += L"  \"float cauchyb\" [0]\n";
            shaderType=L"glass";
            if ((float)s.GetParameter(L"refr_gloss").GetValue()<1 )
            {
            shaderType=L"roughglass";
            shaderStr += L"  \"float uroughness\" ["+ CString(1.0f - float(s.GetParameterValue(L"refr_gloss"))) + L"]";
            shaderStr += L"  \"float vroughness\" ["+ CString(1.0f - float(s.GetParameterValue(L"refr_gloss"))) + L"]\n";
            }
            }
            vIsSet=true;
            }
            else if (!vIsSet)
            {
            // check if its a reflecting material
            float a,b,c,d;
            s.GetColorParameterValue(L"diffuse",a,b,c,d );
            if (s.GetParameterValue(L"reflect_inuse")== "-1" )
            {
            if (vMatID==L"mia_material_phen")
            {
            s.GetColorParameterValue(L"refl_color",red,green,blue,alpha );
            mRough = 1 - float(s.GetParameterValue(L"refl_gloss"));
            red = red*a;
            green = green*b;
            blue = blue*c;
            }
            else
            {
            s.GetColorParameterValue(L"reflectivity",red,green,blue,alpha );
            mRough = (float)s.GetParameterValue(L"reflect_glossy");
            }
            if (red>0 || green>0 || blue>0)
            {
            shaderType=L"shinymetal";
            shaderStr += L" \"color Kr\" ["+ CString(a) + L" "+ CString(b) + L" "+ CString(c) + L"] ";
            shaderStr += L" \"float uroughness\" ["+ CString(mRough/10) + L"]\n";
            shaderStr += L" \"float vroughness\" ["+ CString(mRough/10)+ L"]\n";
            shaderStr += L" \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
            vIsSet=true;
            }
            }
            }*/

            //----------------------------

            else
            {
                // fall back shader
                app.LogMessage(L"No know shader for LuxRender: [ "+ vMatID + L" ]. Writen the default shader values");
                shaderType = L"matte";
                shaderStr = L"  \"color Kd\" [0.7 0.7 0.7]\n";
                shaderStr += L"  \"float sigma\" [0.1]\n";
            }
        }
        //-  write shader block
        f << "\n MakeNamedMaterial \""<< MatName.GetAsciiString() <<"\" \n";
        f << "	\"string type\" [\""<< shaderType.GetAsciiString() <<"\"]\n";
        f << "	"<< shaderStr.GetAsciiString();
    }
}

//-- procces_mat
CValue _process(Shader s, CString vfloat)
{
    //- only for test..
    CValue in_string=s.GetParameterValue(vfloat);
    //-
    return in_string;
}

//--
CString mat_value(Shader s, CString _K)
{
    //-
    CString _component = L"";
    //--
    CString texName = find_shader_used(s, _K );

    //--
    Texture tex_shader = find_tex_used(s, texName);
    //-
    CString t_name = tex_shader.GetName();
    //-
    app.LogMessage(L"Connect..: "+ _K + L" to texture: " + tex_shader.GetName());

    //-
    if (t_name != L"")
    {
        t_name = mat.GetName()+ L"_"+ t_name;

        vChanel = _K; // texture type ( Kd, Kr, Kt...)
        _component += L" \"texture "+ _K + L"\" [\""+ t_name + L"\"]\n";
    }
    else
    {
        s.GetColorParameterValue(_K, red, green, blue, alpha );
        //-
        _component += L" \"color "+ _K
                      + L"\" ["+ CString(red)
                      + L" "+ CString(green)
                      + L" "+ CString(blue) + L"]\n";
    }
    //-
    return _component;
}
//
CString write_lux_glass(Shader s, CString shStr, CString vMatID)
{
    //-- reflectivity component
    shStr += CString(mat_value(s, L"Kr"));

    //-- transmitivity component
    shStr += CString(mat_value(s, L"Kt"));
    //-
    //CValue sr = _process(s, L"index");
    //--
    //shStr += L" \"float index\" ["+ CString(sr) + L"]\n";
    shStr += L" \"float index\" ["+ CString(_process(s, L"index")) + L"]\n";
    shStr += L" \"float index\" ["+ CString(s.GetParameterValue(L"index")) + L"]\n";
    shStr += L" \"float cauchyb\" ["+ CString(s.GetParameterValue(L"cauchyb")) + L"]\n";
    //-
    shaderType = L"glass";
    //--
    if ( vMatID == L"lux_roughglass" )
    {
        shStr += L" \"float uroughness\" ["+ CString(s.GetParameterValue(L"uroughness")) + L"]\n";
        shStr += L" \"float vroughness\" ["+ CString(s.GetParameterValue(L"vroughness")) + L"]\n";
        shStr += L" \"bool dispersion\" [\""+ CString(s.GetParameterValue(L"dispersion")) + L"\"]\n";
        //-
        shaderType = L"roughglass";
    }
    else
    {
        shStr += L" \"float film\" ["+ CString(s.GetParameterValue(L"film")) + L"]\n";
        shStr += L" \"float filmindex\" ["+ CString(s.GetParameterValue(L"filmindex")) + L"]\n";
        shStr += L" \"bool architectural\" [\""+ CString(s.GetParameterValue(L"architectural")) + L"\"]\n";
        //--
    }
    //--
    return shStr;
}
//--
CString write_lux_metal(Shader s, CString shStr)
{
    //--
    const char *ametal [5] = {"amorphous carbon", "silver", "gold", "copper", "aluminium"};
    int nmetal = s.GetParameterValue(L"mname");
    //--
    shStr += L"  \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
    shStr += L"  \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
    if (s.GetParameterValue(L"NKfile") == true )
    {
        //shaderStr += L"  \"string filename\" [\""+ /* NK file */ + L"\"]\n";
    }
    else
    {
        shStr += L"  \"string name\" [\""+ CString(ametal[nmetal]) + L"\"]\n";
    }
    shStr += L"  \"string type\" [\"metal\"] \n";
    //--
    return shStr;
}
//--
CString write_lux_car_paint(Shader s, CString shStr)
{
    //-- car paint
    const char *A_carpaint [] = {"2k acrylack", "blue", "blue matte", "bmw339","ford f8", "opel titan", "polaris silber", "white"};
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
        shStr += L"  \"color Kd\" ["+ CString(r) + L" "+ CString(g) + L" "+ CString(b) + L"] \n";
        shStr += L"  \"color Ks1\" ["+ CString(spr) + L" "+ CString(spg) + L" "+ CString(spb) + L"] \n";
        shStr += L"  \"color Ks2\" ["+ CString(spr2) + L" "+ CString(spg2) + L" "+ CString(spb2) + L"] \n";
        shStr += L"  \"color Ks3\" ["+ CString(spr2) + L" "+ CString(spg2) + L" "+ CString(spb2) + L"] \n";
        shStr += L"  \"float M1\" ["+ CString(float(s.GetParameterValue(L"m1"))) + L"] \n";
        shStr += L"  \"float M2\" ["+ CString(float(s.GetParameterValue(L"m2"))) + L"] \n";
        shStr += L"  \"float M3\" ["+ CString(float(s.GetParameterValue(L"m3"))) + L"] \n";
        shStr += L"  \"float R1\" ["+ CString(float(s.GetParameterValue(L"r1"))) + L"] \n";
        shStr += L"  \"float R2\" ["+ CString(float(s.GetParameterValue(L"r2"))) + L"] \n";
        shStr += L"  \"float R3\" ["+ CString(float(s.GetParameterValue(L"r3"))) + L"] \n";
    }
    else
    {
        shStr += L"  \"string name\" [\""+ CString(A_carpaint[presets]) + L"\"]\n"; // presets
    }
    //--
    return shStr;
}
//--
CString write_mia_material_phen(Shader s, CString shStr)
{
    //--
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
CString write_lux_shinymetal(Shader s, CString shStr)
{
    //- specular component ( see 'Ks' variable in SPDL file )
    shStr += CString(mat_value(s, L"Ks"));

    //-- reflection component, ( see 'Kr' variable in SPDL file )
    shStr += CString(mat_value(s, L"Kr"));
    //--
    shStr += L" \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
    shStr += L" \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
    //--
    return shStr;
}
//--
CString write_lux_substrate( Shader s, CString shStr)
{
    //- diffuse component ( see 'Kd' variable in SPDL file )
    shStr += CString(mat_value(s, L"Kd"));

    //-- specular component, ( see 'Ks' variable in SPDL file )
    shStr += CString(mat_value(s, L"Ks"));
    //--
    shStr += L" \"float uroughness\" ["+ CString(s.GetParameterValue(L"uroughness")) + L"]\n";
    shStr += L" \"float vroughness\" ["+ CString(s.GetParameterValue(L"vroughness")) + L"]\n";

    //--
    return shStr;
}
//--
CString write_lux_matte(Shader s, CString vMatID)
{
    /* 
        s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
        shaderStr = L"	\"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
        shaderStr += L"	\"float sigma\" [0]\n";
    */
    //--
    CString shStr=L"";

    //- matte...
    if (vMatID == L"lux_matte" )
    {
        //- diffuse component
        shStr += CString(mat_value(s, L"Kd"));
        //-
        shaderType = L"matte";
    }
    //- ...matte translucent
    else
    {
        //-- reflectivity component
        shStr += CString(mat_value(s, L"Kr"));

        //-- transmitivity component
        shStr += CString(mat_value(s, L"Kt"));

        //-
        bool energy_conserving = false; // todo
        if ( bool(s.GetParameterValue(L"energy"))== true)
        {
            energy_conserving = true;
        }
        //-
        shStr += L" \"bool energyconserving\" [\""+ CString(energy_conserving) + L"\"]\n";
        shaderType = L"mattetranslucent";
    }
    //-
    shStr += L" \"float sigma\" ["+ CString(float(s.GetParameterValue(L"sigma"))) + L"]\n";

    return shStr;
}
