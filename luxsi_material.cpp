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
//using namespace MATH;

//--
void writeLuxsiShader()
{
    // Writes shader
 
    Scene scene = app.GetActiveProject().GetActiveScene();
    Library matlib = scene.GetActiveMaterialLibrary();

    CRefArray materials = matlib.GetItems();
    
    for ( LONG i=0; i < materials.GetCount(); i++ )
    {
        Texture vTexture;
        CString shaderStr;
        CString vFileBump;
        CString vChanel=L"", vChanType=L"", shaderTexture=L"";

        float b_red = 0.0f, b_green = 0.0f,b_blue = 0.0f, b_alpha = 0.0f;
        float sp_red = 0.0f, sp_green = 0.0f, sp_blue = 0.0f, sp_alpha = 0.0f;
        float refl_red = 0.0f, refl_green = 0.0f, refl_blue = 0.0f, refl_alpha = 0.0f;
        float mRough = 0.0f;
        ImageClip2 vBumpFile;
        Shader vBumpTex;
        CString texFact=L"";
        bool vIsSet=false;
        bool vText=false, vNorm=false;

        //-------------------------
        Material m( materials[i] );
        //-------------------------
        if ( int(m.GetUsedBy().GetCount())== 0 ) continue;
                
        CString MatName(m.GetName());

        if ( !luxsi_find(aMatList, MatName ) ) aMatList.Add(MatName); 
        else continue;
        //--
        CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
        //--
        for ( long j = 0; j < shad.GetCount(); j++)
        {
            Shader s(shad[0]);
            //---
            CString vMatID((s.GetProgID()).Split(L".")[1]);
            
            //-------------------------
            if (vMatID == L"lux_glass") 
            {
                shaderStr = write_lux_glass(s, shaderStr);
                shaderType = L"glass";
                vIsSet = true;
            }
            //-----------------------------------
            else if (vMatID == L"lux_roughglass") 
            {
                //--
                shaderStr = write_lux_roughglass(s, shaderStr);
                shaderType = L"roughglass";
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
            else if (vMatID==L"lux_substrate") 
            {
                shaderStr = write_lux_substrate(s, shaderStr);
                shaderType = L"substrate";
                vIsSet = true;
            }
            //-----------------------------------------
            else if (vMatID == L"lux_mattetranslucent") 
            {
                shaderStr = write_lux_mattetranslucent(s, shaderStr);
                shaderType = L"mattetranslucent";
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
            else if (vMatID == L"mia_material_phen") 
            {
                shaderStr = write_mia_material_phen(s, shaderStr);
                //--
                vIsSet=true;
            }
            else if (vMatID==L"material-phong") 
            {
                s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
                shaderStr += L" \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]";
                shaderStr += L" \"float uroughness\" ["+ CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]";
                shaderStr += L" \"float vroughness\" ["+CString((float)(s.GetParameterValue(L"shiny"))/10)+L"]\n";
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
                shaderStr += L" \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
                shaderStr += L" \"float sigma\" [0]\n";
                //--
                vIsSet=true;
            }
            //----------------------------------
            else if (vMatID == L"material-ward")
            {
                s.GetColorParameterValue(L"diffuse", red, green, blue, alpha );
                shaderStr += L" \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
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
                shaderStr += L" \"color Kd\" ["+ CString(red) + L" " + CString(green) + L" "+ CString(blue) + L"]\n";
                shaderStr += L" \"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
            } 
            else if (vMatID==L"material-strauss")
            {
                s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                shaderStr += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderStr += L"  \"float sigma\" [0]\n";
                //--
                shaderType = L"matte";
                vIsSet = true;
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

            else if (vMatID==L"lux_matte") 
            {
                s.GetColorParameterValue(L"kd", red, green, blue, alpha );
                shaderStr += L"  \"color Kd\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                shaderStr += L"  \"float sigma\" [" + CString((float)s.GetParameterValue(L"sigma")) + L"]\n";
                //----------
                shaderType = L"matte";
                vIsSet = true;
            }
            //-----------------------------
            else if (vMatID==L"lux_mirror") 
            {
                s.GetColorParameterValue(L"kr", red, green, blue, alpha );
                shaderStr += L"  \"color Kr\" [" + CString(red) + L" " + CString(green) + L" " + CString(blue) + L"]\n";
                //--
                shaderType = L"mirror";
                vIsSet = true;
            }
            else
            {
                // fall back shader
                shaderType=L"matte";
                shaderStr += L"  \"color Kd\" [0.7 0.7 0.7]\n";
                shaderStr += L"  \"float sigma\" [0.1]\n";
            }
            
            //-- search for texture image
            //CRefArray vImags=m.GetShaders();
            // for (int i=0; i < vImags.GetCount(); i++)
            //{
            //CRefArray vImags2=Shader(vImags[i]).GetShaders();
            CRefArray texMat = s.GetShaders();
            for (int j=0; j < texMat.GetCount(); j++)
            {
                vTexture = texMat[j];
                //CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
                CString vWhat(vTexture.GetProgID().Split(L".")[1]);
                if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1")
                {
                    //--
                    ImageClip2 vImgClip(vTexture.GetImageClip() );
                    Source vImgClipSrc(vImgClip.GetSource());
                    CString vFileName = vImgClipSrc.GetParameterValue( L"path");

                    CString textproj  = vTexture.GetParameterValue(L"tspace_id");
                    app.LogMessage(CString(textproj.GetAsciiString())); // TODO; search "type" projection

                    if ((bool)vTexture.GetParameterValue(L"bump_inuse")!=false) 
                    {
                        vNorm = true; vChanel = L"bumpmap"; vChanType = L"float";
                        texFact = vTexture.GetParameterValue(L"factor");
                    }
                    else
                    {
                        vText=true; vChanel = L"Kd"; vChanType = L"color";
                        s.GetColorParameterValue(L"diffuse",red,green,blue,alpha );
                        texFact = L" 0.8 0.8 0.8 "; //provisional
                        //texFact = L" "+ CString(red) + L" " + CString(green) + L" " + CString(blue) + L" "; // not work
                    }
                    //----/ rewrite all /-------->
                    f << "\nTexture \"" << MatName.GetAsciiString() << "::"<< vChanel.GetAsciiString() <<"\" \""<< vChanType.GetAsciiString() <<"\" \"imagemap\" \n";
                    f << "  \"string wrap\" [\"repeat\"] \n";//TODO; create option at spdl shader
                    //f << "    \"string chanel\" [\"mean\"] \n"; // not work??
                    f << "  \"string filename\" [\"" << replace(vFileName.GetAsciiString()) << "\"] \n";
                    f << "  \"float gamma\" ["<< vContrast <<"]\n";
                    f << "  \"float gain\" [1.000000]\n";
                    f << "  \"string filtertype\" [\"bilinear\"] \n";// TODO; create option
                    f << "  \"string mapping\" [\"uv\"] \n";// TODO; create option / search data in XSI
                    f << "  \"float vscale\" [-1.0]\n";
                    f << "  \"float uscale\" [1.0] \n";
                    f << "  \"float udelta\" [0.000000] \n";
                    f << "  \"float vdelta\" [1.000000] \n";
                    f << "Texture \""<< MatName.GetAsciiString() << "::"<< vChanel.GetAsciiString() <<".scale""\" \""<< vChanType.GetAsciiString() <<"\" \"scale\" ";
                    f << "\"texture tex1\"  [\""<< MatName.GetAsciiString() << "::"<< vChanel.GetAsciiString() <<"\"]  \""<< vChanType.GetAsciiString() <<" tex2\" ["<< texFact.GetAsciiString() <<"] \n";
                    shaderTexture += L"  \"texture "+ vChanel + L"\" [\""+ m.GetName().GetAsciiString() + L"::"+ vChanel + L".scale\"]\n";
                    vNorm=false;

                    //vText=true;
                }
            }
            //}
    
            //
            // write shader block
            //
    
            f << "\n MakeNamedMaterial \""<< m.GetName().GetAsciiString() << "\" \n";
            f << "  \"string type\" [\""<< shaderType.GetAsciiString() <<"\"]\n";
            f << shaderStr.GetAsciiString();
            if (shaderTexture !=L"")
            {
                f << shaderTexture.GetAsciiString();
            }
        }
    }// test
}
//--
CValue mat_value(Shader s, CString compo)
{
    //-
    s.GetColorParameterValue(compo, red, green, blue, alpha );
    compo = s.GetParameterValue(compo);
    return compo;
}
//
CString write_lux_glass(Shader s, CString shStr)
{
    //--
    s.GetColorParameterValue(L"kr", red, green, blue, alpha );
    shStr += L" \"color Kr\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    //-
    s.GetColorParameterValue(L"kt", red, green, blue, alpha );
    shStr += L" \"color Kt\" ["+ CString(red) + L" "+ CString(green) +  L" "+ CString(blue) + L"]\n";
    //-
    shStr += L" \"float film\" ["+ CString(s.GetParameterValue(L"film")) + L"]\n";
    shStr += L" \"float filmindex\" ["+ CString(s.GetParameterValue(L"filmindex")) + L"]\n";
    shStr += L" \"float index\" ["+ CString(s.GetParameterValue(L"index")) + L"]\n";
    shStr += L" \"float cauchyb\" ["+ CString(s.GetParameterValue(L"cauchyb")) + L"]\n";
    shStr += L" \"bool architectural\" [\""+ CString(s.GetParameterValue(L"architectural")) + L"\"]\n";
    //--
    return shStr;
}
//--
CString write_lux_roughglass(Shader s, CString shStr)
{
    //--
    s.GetColorParameterValue(L"kt", red, green, blue, alpha );
    //--
    shStr += L" \"color Kt\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    shStr += L" \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"uroughness"))) + L"]\n";
    shStr += L" \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"vroughness"))) + L"]\n";

    s.GetColorParameterValue(L"kr",red,green,blue,alpha );
    //--
    shStr += L" \"color Kr\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    shStr += L" \"float index\" ["+ CString((float)s.GetParameterValue(L"index")) + L"]\n";
    shStr += L" \"float cauchyb\" ["+ CString((float)s.GetParameterValue(L"cauchyb")) + L"]\n";
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
    const char *A_carpaint [8] = {"2k acrylack", "blue", "blue matte", "bmw339","ford f8", "opel titan", "polaris silber", "white"};
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
    //- specular color
    s.GetColorParameterValue(L"ks", red, green, blue, alpha );
    shStr += L" \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    shStr += L" \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
    shStr += L" \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"roughness"))) + L"]\n";
    //- reflection color
    s.GetColorParameterValue(L"kr", red, green, blue, alpha );
    shStr += L" \"color Kr\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    //--
    return shStr;
}
//--
CString write_lux_substrate( Shader s, CString shStr)
{
    //--
    //- diffuse color
    s.GetColorParameterValue(L"kd", red, green, blue, alpha );
    shStr += L" \"color Kd\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    shStr += L" \"float uroughness\" ["+ CString(float(s.GetParameterValue(L"uroughness"))) + L"]\n";
    shStr += L" \"float vroughness\" ["+ CString(float(s.GetParameterValue(L"vroughness"))) + L"]\n";
    //- specular color
    s.GetColorParameterValue(L"ks", red, green, blue, alpha );
    shStr += L" \"color Ks\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    //--
    return shStr;
}
//--
CString write_lux_mattetranslucent(Shader s, CString shStr)
{
    //-- reflectivity
    s.GetColorParameterValue(L"kr", red, green, blue, alpha );
    shStr += L" \"color Kr\" ["+ CString(red) + L" "+ CString(green) + L" "+ CString(blue) + L"]\n";
    //-- transmitivity
    s.GetColorParameterValue(L"kt", red, green, blue, alpha );
    shStr += L" \"color Kt\" ["+ CString(red) + L" "+ CString(green) +  L" "+ CString(blue) + L"]\n";
    shStr += L" \"float sigma\" ["+ CString(float(s.GetParameterValue(L"sigma"))) + L"]\n";
    //--
    return shStr;
}