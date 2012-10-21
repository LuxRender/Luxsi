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

#include "include\luxsi_geometry.h"

using namespace XSI;
using namespace MATH;
using namespace std;

//-- 
CString writeLuxsiObj(X3DObject o)
{
    // Writes objects
    //
    bool vIsMeshLight=false;
    bool vText = false, vIsSubD = false;

    Geometry g(o.GetActivePrimitive().GetGeometry(ftime));

    /** use an 'obus' for kill flyes?
    *   nah nah... find better way!
    */
    CRefArray mats(o.GetMaterials());
    Material m = mats[0];

    CRefArray shaderArray(m.GetShaders());
    Shader s(shaderArray[0]);

    CRefArray vtexture(s.GetShaders());
    Texture tex(vtexture[0]);
    CString vTexID(tex.GetProgID().Split(L".")[1]);
    if (vTexID == L"txt2d-image-explicit" || vTexID == L"Softimage.txt2d-image-explicit.1") vText = true;

    CGeometryAccessor ga;
    CString 
        vUV = L"",      //- for UV data.
        vNormals = L"", //- for normals data.
        vTris = L"",    //- for faces.
        vPoints = L"";  //- for point poditions.
       
    //- test for ply ofrmat
    CString 
        lxoData,    //! for LuxRender native geometry format.
        plyData,    //! for vertex, normal and UV data in PLY format.
        plyFaces;   //! for faces data in PLY format.

    LONG subdLevel = 0;
    Property geopr = o.GetProperties().GetItem(L"Geometry Approximation");
    if ((int)geopr.GetParameterValue(L"gapproxmordrsl") > 0 )
    {
        vIsSubD = true; //-- only render
        subdLevel = (int)geopr.GetParameterValue(L"gapproxmordrsl");
        //- TODO; create advice message, for used this option ?
    }
    //--
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_kinec_state.GetTransform(ftime); // add time

    //--
    ga = PolygonMesh(g).GetGeometryAccessor();
    //-
    if ( ga.GetUVs().GetCount() > 0 && vText) have_UV = true;
    

   //--
    if (int(g.GetTriangles().GetCount()) > 0 )
    {
        //--
        CTriangleRefArray triangles(g.GetTriangles()); // miga
        CLongArray indices( triangles.GetIndexArray() );
        
        CVector3Array allPoints(triangles.GetPositionArray().GetCount());
        CVector3Array allUV(triangles.GetUVArray().GetCount());
        CVector3Array allNormals(triangles.GetPolygonNodeNormalArray().GetCount());

        long index=0;
        for (int i=0; i<triangles.GetCount(); i++)
        {
            Triangle triangle(triangles.GetItem(i));
            for (int j=0; j < triangle.GetPoints().GetCount(); j++)
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
            }
        }

        /** Process geometry
        */
        plyData.Clear();
        //-
        if (!overrGeometry)
        {
            for (LONG j=0; j < allPoints.GetCount(); j++)
            {
                //-- create vector..
                CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);

                //- set transform..
                vPos.MulByTransformationInPlace(global_trans);

                //- and write string array, in correct Lux format ( x, -z, y )
                if ( !vplymesh )
                {
                    vPoints += L" "+ CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]) + L"\n";
                } 
                else
                {
                    plyData += CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]);
                }
                //-- same for normals
                if ( vSmooth_mesh )
                {
                    CVector3 vNorm(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
                    //-
                    vNorm.MulByTransformationInPlace(global_trans);
                    /** for 'normals', change 'y' for 'z'(x, z, y), but not negative value (-z). TODO: more testing..
                    */
                    if ( !vplymesh )
                    {
                        vNormals += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]) + L"\n";
                    }
                    else
                    {
                        plyData += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]);
                    }
                }
                //-- UV need transpose?
                if ( have_UV ) 
                {
                    if ( !vplymesh ) 
                    {
                        vUV += L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]) + L"\n";
                    }
                    else
                    {
                        plyData += L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]);
                    }
                }
                //-
                if ( vplymesh ) plyData += L"\n";
            }
            //-- tes ply
            plyFaces.Clear();
            CString _dat = L"";
            for ( int k = 0; k < indices.GetCount(); k += 3)
            {
                _dat = CString(int(k)) + L" "+ CString(int(k+1)) + L" "+ CString(int(k+2)) + L"\n";

                /*  if use this method, verify boolean two times.. ¿is better, use 'if else' method ?   
                */
                if ( !vplymesh ) vTris += L" "+ _dat;
                //--
                if ( vplymesh ) plyFaces += L"3 "+ _dat;
            }
        }
        //--
        lxoData = L"\nAttributeBegin #"+ o.GetName();
        lxoData += L"\nNamedMaterial \""+ m.GetName() + L"\"\n";

        /** Geometry associated to light objects.
        *   Portal lights use a 'trick'..
        *   TODO; find better mode
        */
        bool vIsPortal = false;
        
        //- test
        // Returns 1 if "12" is found
		//npos = str.FindString( CString("12") );
        CString objName = o.GetName();
        ULONG spos;
        spos = objName.FindString(L"PORTAL");
        if ( spos == 1) app.LogMessage(L"Object is type Portal");

        string::size_type loc = string(CString(o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
        if (loc != string::npos) vIsPortal = true;

        /** Meshlight
        *   this way is only for XSI incandescence mode
        *   search better way for Luxsi.. or create light material, like YafXSI
        */
        if (float(s.GetParameterValue(L"inc_inten"))> 0 ) vIsMeshLight = true;
        //- mesh..
        CString type_mesh = L"mesh";
        if ( vplymesh ) type_mesh = L"plymesh";
        //- shape..
        CString type_shape = L"Shape";
        if ( vIsPortal ) type_shape = L"PortalShape";

        //-----------------
        if ( vIsMeshLight )
        //-----------------
        {
            //--
            float red, green, blue, alpha;
            s.GetColorParameterValue(L"incandescence", red, green, blue, alpha );
            float inc_inten(s.GetParameterValue(L"inc_inten"));
            //--
            CString lName = findInGroup(o.GetName());
            if (lName == L"") lName = o.GetName();
            //--
            lxoData += L" LightGroup \""+ lName + L"\"\n";
            lxoData += L"\nAreaLightSource \"area\" \n";
            lxoData += L"  \"float importance\" [1.00] \n";
            lxoData += L"  \"float gain\" ["+ CString(inc_inten) + L"] \n";
            //f << "  \"float power\" [100.0]  \"float efficacy\" [17.0] \n";
            lxoData += L"  \"color L\" ["
                + CString(red * inc_inten) + L" "
                + CString(green * inc_inten) + L" "
                + CString(blue * inc_inten ) + L"]\n";
        }
        lxoData += L"\n"+ type_shape + L" \""+ type_mesh + L"\" \n";
        
        //-- share
        lxoData += L"  \"integer nsubdivlevels\" ["+ CString( subdLevel ) + L"]\n";
        lxoData += L"  \"string subdivscheme\" [\"loop\"]\n";
        lxoData += L"  \"bool dmnormalsmooth\" [\""+ CString( MtBool[vSmooth_mesh] ) + L"\"]\n";
        lxoData += L"  \"bool dmsharpboundary\" [\""+ CString( MtBool[vSharp_bound] ) + L"\"]\n";
        //f << "  \"string displacementmap\" [\"none\"]\n"; // here, place normalmap texture
        //f << "  \"float dmscale\" [\"0.0\"] \"float dmoffset\" [\"0.0\"]\n";
        //-

        long vertCount(allPoints.GetCount());
        long triCount(g.GetTriangles().GetCount());
        //--------------
        if ( !vplymesh )
        //--------------
        {
            lxoData += L"  \"string acceltype\" [\""+ CString( MtAccel[vAccel] ) + L"\"]\n";
            lxoData += L"  \"string tritype\" [\"auto\"] \n";//TODO
            lxoData += L"  \"integer triindices\" [\n"+ vTris + L"\n ]";
            lxoData += L"  \"point P\" [\n"+ vPoints + L"\n ]"; //
            if ( vSmooth_mesh && vNormals != L"" )
            {
                lxoData += L" \"normal N\" [\n"+ vNormals + L"\n ]";
            }
            //--
            if (have_UV)
            {
                lxoData += L" \"float uv\" [\n"+ vUV + L"\n ]";
            }
        }
        else
        {
            /** make link to .ply file
            *   The vFilePLY value = full path + filename + framenumber + LXS extension. 
            *   luxsi_normalize_path(), returns alone the filename with the framenumber.
            *   Here we add the name of the object and the expension PLY.
            */
            CString ply_ext = L"_"+ o.GetName() + L".ply";
            CString include_ply_filename = luxsi_normalize_path(vFilePLY) + ply_ext;
            //-
            if (luxdebug ) app.LogMessage(L"File ply: "+ vFilePLY + L" Include ply: "+ include_ply_filename);
            //-
            lxoData += L"  \"string filename\" [\""+ include_ply_filename + L"\"]\n";

            /* if not override geometry, go to write ply file 
            */
            if ( !overrGeometry )
            {
                int next = vFilePLY.ReverseFindString(".");
                CString write_ply_filename = vFilePLY.GetSubString(0,next) + ply_ext;
                //-
                write_plyFile(plyData, plyFaces, write_ply_filename, vertCount, triCount);
            }
        }
        //--
        lxoData += L"\nAttributeEnd #"+ o.GetName() + L"\n";
    }
    return lxoData;
}
//-
void write_plyFile(CString in_plyData, CString in_faceData, CString vfile, int vertCount, int triCount)
{
    //-
    f.open(vfile.GetAsciiString());
    f << "ply\n";
    f << "format ascii 1.0\n";
    f << "comment created with LuXSI; LuxRender Exporter for Autodesk Softimage\n";
    f << "element vertex "<< vertCount <<"\n";
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
    if ( have_UV )
    {
        f << "property float u\n";
        f << "property float v\n";
    }

    f << "element face "<< triCount <<"\n";
    f << "property list uchar uint vertex_indices\n";
    f << "end_header\n";

    //-- vertex, normals and UV, if exist
    f << in_plyData.GetAsciiString();
    //-- faces
    f << in_faceData.GetAsciiString();
    //->
    f.close();
    //--
}
