/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 - 2012  Michael Gangolf
Code contributor ; Pedro Alcaide, aka povmaniaco

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

#include "include\luxsi_main.h"

using namespace XSI;
using namespace std;

//-
extern Application app;
//-
extern float vContrast;
//-
extern CString vChanel;
//-
extern string replace(string input);
//-

CString luxsi_texture(Material mat, Shader s, CString texStr)
{
    //--
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
                 
            //- 
            if ( bool(tex.GetParameterValue(L"bump_inuse")) == true && vChanel == L"bumpmap")
            {
                //--
                vChanType = L"float";
            }
                    
            //----/ rewrite all /-------->
            texStr += L"\nTexture \"" + _tex_name + L"\" \""+ vChanType + L"\" \"imagemap\" \n";
            texStr += L"  \"string wrap\" [\"repeat\"] \n";//TODO; create option at spdl shader
            //f << "    \"string chanel\" [\"mean\"] \n"; // not work??
            texStr += L"  \"string filename\" [\""+ CString(replace(vFileName.GetAsciiString()).c_str()) + L"\"] \n";
            texStr += L"  \"float gamma\" ["+ CString( vContrast ) + L"]\n";
            texStr += L"  \"float gain\" [1.000000]\n";
            texStr += L"  \"string filtertype\" [\"bilinear\"] \n";// TODO; create option
            texStr += L"  \"string mapping\" [\"uv\"] \n";// TODO; create option / search data in XSI
            texStr += L"  \"float vscale\" [-1.0]\n";
            texStr += L"  \"float uscale\" [1.0] \n";
            texStr += L"  \"float udelta\" [0.000000] \n";
            texStr += L"  \"float vdelta\" [1.000000] \n";                    
        }
    }
    return texStr;
}