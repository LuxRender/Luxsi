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

#include "include\luxsi_main.h"

using namespace XSI;
using namespace std;
//using namespace luxsi;

/**/
extern Application app;

/* for gamma value
*/
extern float vContrast;

/**/
extern CString vChanel;

/* for normalize path files ( slash character )
*/
extern std::string luxsi_replace(string input);

/* Is 'normal' scene or 'Preview'
*/
extern bool is_preview;

CString luxsi_texture(Material mat, Shader s)
{
    //-- test
    CString texStr;
    //- end
    CRefArray texMat = s.GetShaders();

    //-( 3 ) for textures..
    for (int k=0; k < texMat.GetCount(); k++)
    {
        Texture tex = texMat[k];
        CString vTexID(tex.GetProgID().Split(L".")[1]);
        // test
        CString texFact=L"";
        //bool vNorm=false;
        CString vChanType=L"color";
              
        if (vTexID == L"txt2d-image-explicit" || vTexID == L"Softimage.txt2d-image-explicit.1")
        {
            //--
            ImageClip2 vImgClip(tex.GetImageClip() );
            Source vImgClipSrc(vImgClip.GetSource());
            CString vFileName = vImgClipSrc.GetParameterValue( L"path");

            //- test for search 'mapping' type -----------------------------
            //CString textproj  = vTexture.GetParameterValue(L"tspace_id");
            //app.LogMessage(CString(textproj.GetAsciiString()));
            //--------------------------------------------------------------

            //- TODO; synch by shader
            CString _tex_name = mat.GetName() + L"_"+ tex.GetName();

            //- trick for Preview material
            CString uvmap = L"uv";
            if ( is_preview ) uvmap = L"spherical";
                 
            //- 
            if ( bool(tex.GetParameterValue(L"bump_inuse")) == true && vChanel == L"bumpmap")
            {
                //--
                vChanType = L"float";
            }
                    
            //----/ rewrite all /-------->
            texStr += L"\nTexture \"" + _tex_name + L"\" \""+ vChanType + L"\" \"imagemap\" \n";
            texStr += L" \"string wrap\" [\"repeat\"] \n";//TODO; create option at spdl file
            //f << "    \"string chanel\" [\"mean\"] \n"; // not work??
            texStr += L" \"string filename\" [\""+ CString(luxsi_replace(vFileName.GetAsciiString()).c_str()) + L"\"] \n";
            texStr += L" \"float gamma\" ["+ CString( vContrast ) + L"]\n";
            texStr += L" \"float gain\" [1.000000]\n";
            texStr += L" \"string filtertype\" [\"bilinear\"] \n";// TODO; create option
            texStr += L" \"string mapping\" [\""+ uvmap +"\"] \n";
            texStr += L" \"float vscale\" [-1.0]\n";
            texStr += L" \"float uscale\" [1.0] \n";
            texStr += L" \"float udelta\" [0.000000] \n";
            texStr += L" \"float vdelta\" [1.000000] \n";                    
        }
    }
    return texStr;
}