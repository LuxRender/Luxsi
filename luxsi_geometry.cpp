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
using namespace MATH;
using namespace std;

//-
extern ofstream f;

extern bool vplymesh;

extern CString vFileObjects;

extern CString vFilePLY;
extern bool vSmooth_mesh;
extern bool vSharp_bound;
extern int vAccel;
extern const char *MtBool[], *MtAccel[];

extern string replace(string in_input);

extern CString findInGroup(CString s);

//--
int writeLuxsiObj(X3DObject o)
{
    // Writes objects
    //
    CScriptErrorDescriptor status ;
    CValueArray fooArgs(1) ;
    fooArgs[0] = L"" ;
    CValue retVal=false ;
    bool vIsMeshLight=false;
    bool vIsSet=false;
    bool vText = false, vIsSubD = false;
    bool vIsMod=false;

    Geometry g(o.GetActivePrimitive().GetGeometry()) ;
    CRefArray mats(o.GetMaterials()); // Array of all materials of the object
    Material m = mats[0];
    CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
    Shader s(shad[0]);
    CGeometryAccessor ga;
    CString vUV=L"",vNormals=L"",vTris=L"",vMod=L"",vPoints=L"";

    LONG subdLevel = 0;
    Property geopr = o.GetProperties().GetItem(L"Geometry Approximation");
    if ((int)geopr.GetParameterValue(L"gapproxmordrsl") > 0 )
    {
        vIsSubD = true;
        subdLevel = (int)geopr.GetParameterValue(L"gapproxmordrsl"); //-- only render
    }
    //--
    CRefArray vImags=m.GetShaders();
    for (int i=0; i<vImags.GetCount(); i++)
    {
        CRefArray vImags2 = Shader(vImags[i]).GetShaders();
        for (int j=0; j<vImags2.GetCount(); j++)
        {
            CString vWhat((Shader(vImags2[j]).GetProgID()).Split(L".")[1]);
            if (vWhat==L"txt2d-image-explicit" || vWhat==L"Softimage.txt2d-image-explicit.1")
            {
                vText = true;
            }
        }
    }
    //--
    //CTransformation localTransformation = ga.GetTransform();
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_kinec_state.GetTransform();
    //CMatrix4 mat4(gt.GetMatrix4());
    //--
    if (int(g.GetTriangles().GetCount()) > 0 )
    {
        //--
        CTriangleRefArray triangles(g.GetTriangles()); // miga
        CLongArray indices( triangles.GetIndexArray() );

        CVector3Array allPoints(triangles.GetCount()*3);
        CVector3Array allUV(triangles.GetCount()*3);
        CVector3Array allNormals(triangles.GetCount()*3);

        long index=0;
        for (int i=0; i<triangles.GetCount(); i++)
        {
            Triangle triangle(triangles.GetItem(i));
            for (int j=0; j<triangle.GetPoints().GetCount(); j++)
            {
                TriangleVertex vertex0(triangle.GetPoints().GetItem(j));
                CVector3 pos(vertex0.GetPosition());
                CVector3 normal(vertex0.GetNormal());
                CUV uvs(vertex0.GetUV());
                //--
                long arrayPos = index++;
                allPoints[arrayPos] = pos;
                allNormals[arrayPos] = normal;
                allUV[arrayPos] = CVector3(uvs.u, uvs.v,0);
                vTris += L" "+ CString(arrayPos) + L" ";
            }
            vTris += L"\n";
        }

        //-- test to optimize processes
        if ( !vplymesh )
        {
            for (LONG j=0; j < allPoints.GetCount(); j++)
            {
                //-- create vector..
                CVector3 new_pos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);

                //- set transform..
                new_pos.MulByTransformationInPlace(global_trans);

                //- and write string array, in correct Lux format ( x, -z, y )
                vPoints += L" "+ CString(new_pos[0]) + L" "+  CString(-new_pos[2]) + L" "+ CString(new_pos[1]) + L"\n";

                //--
                CVector3 tr_normals(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
                tr_normals.MulByTransformationInPlace(global_trans);
                vNormals +=  L" "+ CString(tr_normals[0]) + L" "+ CString(tr_normals[2]) + L" "+ CString(tr_normals[1]) + L"\n";
                //-- UV need transpose?
                vUV +=  L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]) + L"\n";
            }
        }
        //--
        f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
        //--
        f << "\nNamedMaterial \""<< m.GetName().GetAsciiString() <<"\"\n";

        //-- Geometry associated to lights
        bool vIsPortal = false;
        string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
        if (loc != string::npos) vIsPortal = true;
        //-- meshlight
        if (float(s.GetParameterValue(L"inc_inten"))> 0 ) vIsMeshLight = true;
        CString type_mesh = L"mesh";
        if ( vplymesh ) type_mesh = L"plymesh";
        CString type_shape = L"Shape";
        if ( vIsPortal ) type_shape = L"PortalShape";

        //-----------------
        if ( vIsMeshLight )
            //-----------------
        {
            //--
            float red = 0.0f, green = 0.0f, blue = 0.0f, alpha = 0.0f;
            s.GetColorParameterValue(L"incandescence",red,green,blue,alpha );
            float inc_intensity(s.GetParameterValue(L"inc_inten"));
            //--
            CString lName = findInGroup(o.GetName());
            if (lName == L"") lName = o.GetName().GetAsciiString();
            //--
            f << " LightGroup \"" << lName.GetAsciiString() << "\"\n";
            f << "\nAreaLightSource \"area\" \n";
            f << "  \"float importance\" [1.00] \n";
            f << "  \"float gain\" ["<< inc_intensity <<"] \n";
            //f << "  \"float power\" [100.0]  \"float efficacy\" [17.0] \n";
            f << "  \"color L\" ["<< (red * inc_intensity) <<" "<< (green * inc_intensity) <<" "<<(blue * inc_intensity ) <<"]\n";
        }
        f << type_shape.GetAsciiString() <<" \""<< type_mesh.GetAsciiString() <<"\" \n";
        //-------------
        if ( vplymesh )
            //-------------
        {
            //--
            CString vInput_FileName = vFileObjects.GetAsciiString();
            int Loc = (int)vInput_FileName.ReverseFindString(".");
            vFilePLY = vInput_FileName.GetSubString(0,Loc) + L"_"+ o.GetName() + L".ply";
            f << "  \"string filename\" [\"" << replace(vFilePLY.GetAsciiString()) << "\"] \n";
        }
        //-- share
        f << "  \"integer nsubdivlevels\" [" << subdLevel  <<"] \"string subdivscheme\" [\"loop\"] \n";
        f << "  \"bool dmnormalsmooth\" [\""<< MtBool[vSmooth_mesh] <<"\"]";
        f << "  \"bool dmsharpboundary\" [\""<< MtBool[vSharp_bound] <<"\"] \n";
        //f << "  \"string displacementmap\" [\"none\"]\n";
        //f << "  \"float dmscale\" [\"0.0\"] \"float dmoffset\" [\" 0.0\"]\n";

        //--------------
        if ( !vplymesh )
            //--------------
        {
            f << "  \"string acceltype\" [\""<< MtAccel[vAccel] <<"\"] \"string tritype\" [\"wald\"] \n";//TODO
            f << "  \"integer triindices\" [\n" << vTris.GetAsciiString() << "\n ]";
            f << "  \"point P\" [\n" << vPoints.GetAsciiString() << "\n ]";
            if ( vSmooth_mesh && vNormals != L"" )
            {
                f << " \"normal N\" [\n" << vNormals.GetAsciiString() << "\n ]";
            }
            //--
            if ( vText && vUV != L"" )
            {
                f << " \"float uv\" [\n" << vUV.GetAsciiString() << "\n ]";
            }
        }
        //--
        f << "\nAttributeEnd #" << o.GetName().GetAsciiString() << "\n";
    }
    return 0;
}
