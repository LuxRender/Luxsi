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
extern ofstream f;
extern float vContrast;
extern CString vChanel;
extern bool luxdebug;

extern CStringArray aMatList;
//-
extern string replace(string input);
//-
extern bool luxsi_find(CStringArray in_array, CString in_string);

//-
void luxsi_texture()
{
    //--
    Scene scene = app.GetActiveProject().GetActiveScene();
    Library matlib = scene.GetActiveMaterialLibrary();

    CRefArray materials = matlib.GetItems();
    //- ( 1 ) for materials..
    for ( LONG i=0; i < materials.GetCount(); i++ )
    {
        Texture vTexture;
        CString vChanType=L"";

        Shader vBumpTex;
        CString texFact=L"";
        bool vNorm=false;

        //---------------------------
        Material mat( materials[i] );
        //---------------------------
        if ( int(mat.GetUsedBy().GetCount())== 0 ) continue;

        CString MatName(mat.GetName());

        if ( !luxsi_find(aMatList, MatName ) ) aMatList.Add(MatName);
        else continue;
        //--
        CRefArray shad(mat.GetShaders()); // Array of all shaders attached to the material [e.g. phong]

        //- ( 2 ) for shaders..
        for ( long j = 0; j < shad.GetCount(); j++)
        {
            Shader s(shad[j]); // j en vez de 0 ?
            //---
            CString vMatID((s.GetProgID()).Split(L".")[1]);
            CRefArray texMat = s.GetShaders();

            //-( 3 ) for textures..
            for (int k=0; k < texMat.GetCount(); k++)
            {
                vTexture = texMat[k];
                CString vWhat(vTexture.GetProgID().Split(L".")[1]);
                //-
                if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1")
                {
                    //--
                    ImageClip2 vImgClip(vTexture.GetImageClip() );
                    Source vImgClipSrc(vImgClip.GetSource());
                    CString vFileName = vImgClipSrc.GetParameterValue( L"path");

                    //- test for search 'mapping' type -----------------------------
                    //CString textproj  = vTexture.GetParameterValue(L"tspace_id");
                    //app.LogMessage(CString(textproj.GetAsciiString()));
                    //--------------------------------------------------------------

                    //- TODO; synch by shader
                    CString _tex_name = mat.GetName() + L"_"+ vTexture.GetName();

                    //- old test for bump chanel
                    if ((bool)vTexture.GetParameterValue(L"bump_inuse")!=false)
                    {
                        vNorm = true;
                        vChanel = L"bumpmap";
                        vChanType = L"float";
                        texFact = vTexture.GetParameterValue(L"factor");
                    }
                    else
                    {
                        //-
                        vChanType = L"color";
                    }
                    //----/ rewrite all /-------->
                    f << "\nTexture \"" << _tex_name.GetAsciiString() <<"\" \""<< vChanType.GetAsciiString() <<"\" \"imagemap\" \n";
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
                    //-
                    vNorm = false;
                }
            }
            /*
            f.write('\n' +
            'Texture "%s" "color" "imagemap"\n' % tex_name +
            '    "string filename" ["%s"]\n' % _file_name +
            '    "float gamma" [2.2000]\n' +
            '    "string mapping" ["spherical"]\n' +
            '    "float udelta" [0.0000]\n' +
            '    #"vector v1" [1.000 0.000 0.000]\n' +
            '    #"vector v2" [0.000 1.000 0.000]\n' +
            '    "float vdelta" [0.0000]\n'
            )
            f.close()
            */
        }
    }
}
