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

#include "include\luxsi_geometry.h"

using namespace XSI;
using namespace MATH;
using namespace std;

//--
int writeLuxsiObj(X3DObject o)
{
    // Writes objects
    //
    bool vIsMeshLight=false;
    //bool vIsSet=false;
    bool vText = false, vIsSubD = false;

    Geometry g(o.GetActivePrimitive().GetGeometry(ftime));


    CRefArray mats(o.GetMaterials());
    Material m = mats[0];

    CRefArray shaderArray(m.GetShaders());
    Shader s(shaderArray[0]);

    CRefArray vtexture(s.GetShaders());
    Texture tex(vtexture[0]);
    CString vTexID(tex.GetProgID().Split(L".")[1]);
    if (vTexID == L"txt2d-image-explicit" || vTexID == L"Softimage.txt2d-image-explicit.1") vText = true;

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
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_kinec_state.GetTransform(ftime); // add time
    
    //-- 
    bool have_UV = false; ga = PolygonMesh(g).GetGeometryAccessor();
    //int uvdata = ga.GetUVs().GetCount();
    if ( ga.GetUVs().GetCount() > 0 && vText) have_UV = true;

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
                //-- triangles or faces
                vTris += L" "+ CString(arrayPos) + L" ";
            }
            vTris += L"\n";
        }

        //-- test to optimize process
        if ( !vplymesh )
        {
            for (LONG j=0; j < allPoints.GetCount(); j++)
            {
                //-- create vector..
                CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);

                //- set transform..
                vPos.MulByTransformationInPlace(global_trans);

                //- and write string array, in correct Lux format ( x, -z, y )
                vPoints += L" "+ CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]) + L"\n";

                //-- same for normals
                if ( vSmooth_mesh )
                {
                    CVector3 vNorm(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
                    //-
                    vNorm.MulByTransformationInPlace(global_trans);
                    //-- for 'normals', change 'y' for 'z'(x, z, y), but not negative value (-z)
                    //- need more testing..
                    vNormals += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]) + L"\n";
                }

                //-- UV need transpose?
                if ( have_UV )
                {
                    vUV += L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]) + L"\n";
                }
            }
        }
        //--
        f << "\nAttributeBegin #" << o.GetName().GetAsciiString();
        //--
        f << "\nNamedMaterial \""<< m.GetName().GetAsciiString() <<"\"\n";

        //-- Geometry associated to lights
        //- portal lights. Use a 'trick'..
        //- TODO; find better mode
        bool vIsPortal = false;
        string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
        if (loc != string::npos) vIsPortal = true;

        //-- meshlight
        //- this way is only for XSI incandescence mode
        //- find better way for Lux
        if (float(s.GetParameterValue(L"inc_inten"))> 0 ) vIsMeshLight = true;
        //-
        CString type_mesh = L"mesh";
        //-
        if ( vplymesh ) type_mesh = L"plymesh";
        //--
        CString type_shape = L"Shape";
        //-
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
        f <<"\n"<< type_shape.GetAsciiString() <<" \""<< type_mesh.GetAsciiString() <<"\" \n";
        //-------------
        if ( vplymesh )
            //-------------
        {
            //-- make link to .ply file
            //-- vFilePLY has the number of frame, but not the extension .ply
            app.LogMessage(L"File ply is: "+ vFilePLY);
            //-
            CString ply_frame(vFilePLY + L"_"+ o.GetName() + L".ply");
            //-
            app.LogMessage(L"Frame ply is: "+ ply_frame);

            f <<"  \"string filename\" [\""<< luxsi_replace(ply_frame.GetAsciiString()) <<"\"] \n";
        }
        //-- share
        //f << "  \"integer nsubdivlevels\" [" << subdLevel  <<"]\n";
        //f << "  \"string subdivscheme\" [\"loop\"] \n";
        //f << "  \"bool dmnormalsmooth\" [\""<< MtBool[vSmooth_mesh] <<"\"]";
        //f << "  \"bool dmsharpboundary\" [\""<< MtBool[vSharp_bound] <<"\"] \n";
        //f << "  \"string displacementmap\" [\"none\"]\n";
        //f << "  \"float dmscale\" [\"0.0\"] \"float dmoffset\" [\" 0.0\"]\n";

        //--------------
        if ( !vplymesh )
        //--------------
        {
            f << "  \"string acceltype\" [\""<< MtAccel[vAccel] <<"\"]\n";
            f << "  \"string tritype\" [\"auto\"] \n";//TODO
            f << "  \"integer triindices\" [\n"<< vTris.GetAsciiString() <<"\n ]";
            f << "  \"point P\" [\n"<< vPoints.GetAsciiString() <<"\n ]"; // 
            if ( vSmooth_mesh && vNormals != L"" )
            {
                f <<" \"normal N\" [\n"<< vNormals.GetAsciiString() <<"\n ]";
            }
            //--
            if (have_UV)
            {
                f <<" \"float uv\" [\n"<< vUV.GetAsciiString() <<"\n ]";
            }
        }
        //--
        f << "\nAttributeEnd #"<< o.GetName().GetAsciiString() <<"\n";
    }
    return 0;
}

//--
void write_ply_object(X3DObject o, CString vFilePLY)
{
    Geometry g(o.GetActivePrimitive().GetGeometry(ftime)); // add time
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_kinec_state.GetTransform(ftime); // add time value

    //--
    CTriangleRefArray triangles(g.GetTriangles());
    CLongArray indices(triangles.GetIndexArray());
    CVector3Array allPoints(triangles.GetPositionArray().GetCount());
    CVector3Array allUV(triangles.GetUVArray().GetCount());
    CVector3Array allNormals(triangles.GetPolygonNodeNormalArray().GetCount());

    //-- test for UV
    app.LogMessage(L"[DEBUG]: UV count: "+ CString(triangles.GetUVArray().GetCount()));

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
            allUV[arrayPos] = CVector3(uvs.u, uvs.v, 0);
        }
    }
    //----------------------------------------------------------
    //-- vertex
    long vCount(triangles.GetCount()*3);
    long pCount(g.GetTriangles().GetCount());

    //-- vertex
    CString sPos;
    for (LONG j=0; j < allPoints.GetCount(); j++)
    {
        //-- vertex
        CVector3 point_pos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
        point_pos.MulByTransformationInPlace(global_trans);
        //-- normals
        CVector3 norm_idx(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
        norm_idx.MulByTransformationInPlace(global_trans);

        //-- ply write points..
        sPos += CString(point_pos[0]) + L" "+ CString(-point_pos[2]) + L" "+ CString(point_pos[1]);

        //- and normals if 'smooth_mesh' is ON
        //- !!WARNING!!, POSSIBLE ERROR IN NORMALS
        if ( vSmooth_mesh )
        {
            sPos += L" "+ CString(norm_idx[0]) + L" "+ CString(norm_idx[2]) + L" "+ CString(norm_idx[1]);
        }
        //-- UV need transpose?
        //-- if ( UV ) {
        sPos += L" "+ CString(allUV[j][0]) + L" "+ CString(allUV[j][1]);
        //-- }
        sPos += L"\n";
    }
    //-- triangles
    CString vFaces;
    LONG nIndices = indices.GetCount();
    for ( LONG k=0; k < nIndices; k += 3 )
    {
        //--
        vFaces += L"3 "+ CString(int(k)) + L" "+ CString(int(k+1)) + L" "+ CString(int(k+2)) + L"\n";
        //vFaces += L"\n";
    }
    //--

    vFilePLY += L"_"+ o.GetName() + L".ply";
    //FILE * file=fopen(vFilePLY.GetAsciiString(), "wb"); // from kies project
    f.open(vFilePLY.GetAsciiString(), ios::out | ios::binary);
    f << "ply\n";
    f << "format ascii 1.0\n";
    f << "comment LuXSI; LuxRender Exporter for Autodesk Softimage\n";
    f << "element vertex "<< vCount <<"\n";
    f << "property float x\n";
    f << "property float y\n";
    f << "property float z\n";
    /* for vertex colors?
    f << "property uchar red\n";
    f << "property uchar green\n";
    f << "property uchar blue\n";
    */
    if ( vSmooth_mesh )
    {
        f << "property float nx\n";
        f << "property float ny\n";
        f << "property float nz\n";
    }
    f << "property float u\n";
    f << "property float v\n";

    f << "element face "<< pCount <<"\n";
    f << "property list uchar uint vertex_indices\n";
    f << "end_header\n";

    //-- vertex, normals and UV, if exist
    f << sPos.GetAsciiString();
    //-- faces
    f << vFaces.GetAsciiString();
    //->
    f.close();
    //--

}
