/***********************************************************************
This file is part of LuXSI;
LuXSI is a LuxRender Exporter for Autodesk(C) Softimage(C) ( ex-XSI )
http://www.luxrender.net

Copyright(C) 2007 - 2013  of all Authors:
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

***********************************************************************/

#include "include\luxsi_geometry.h"
//#include "include\rply.h"

//-
CString writeGeometryLXO(CVector3Array allPoints, CVector3Array allNormals, CVector3Array allUV,
                         CLongArray indices, CTransformation global_trans)
{
    /** Specialized fuction. 
    *   The questions, always before the loop.
    *   First of all, the most small case.
    */
    CString lxoData, vTris, vPoints, vNormals, vUV;
    //-
    if (!exportNormals && !have_UV )
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            vPoints += L" "+ CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]) + L"\n";
        }
    }
    //- only UV, without normals
    else if( !exportNormals && have_UV )
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            vPoints += L" "+ CString(vPos[0]) + L" "+ CString(-vPos[2]) + L" "+ CString(vPos[1]) + L"\n";
            //-- UVs
            vUV += L" "+ CString(allUV[j][0]) + L" "+ CString(allUV[j][1]) + L"\n";
        }
    }
    //-- only normals, not UV
    else if ( exportNormals && !have_UV )
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            vPoints += L" "+ CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]) + L"\n";
            //- normals        
            CVector3 vNorm(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
            vNorm.MulByTransformationInPlace(global_trans);
            vNormals += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]) + L"\n";                        
        }
    }
    //- all data
    else // ( exportNormals && have_UV )
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            vPoints += L" "+ CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]) + L"\n";
            //- normals        
            CVector3 vNorm(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
            vNorm.MulByTransformationInPlace(global_trans);
            vNormals += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]) + L"\n";                        
            //-- UVs
            vUV += L" "+ CString(allUV[j][0]) + L" "+ CString(allUV[j][1]) + L"\n";
        }

    }
    //- faces
    for ( int k = 0; k < indices.GetCount(); k += 3)
    {
        vTris += L" "+ CString(int(k)) + L" "+ CString(int(k+1)) + L" "+ CString(int(k+2)) + L"\n";
    }
    //-
    lxoData =  L"  \"integer triindices\" [\n"+ vTris + L"\n ]";
    lxoData += L"  \"point P\" [\n"+ vPoints + L"\n ]";
    if ( exportNormals )
    {
        lxoData += L" \"normal N\" [\n"+ vNormals + L"\n ]";
    }
    //--
    if (have_UV)
    {
        lxoData += L" \"float uv\" [\n"+ vUV + L"\n ]";
    }
    //-
    return lxoData;
}
//--
CString writeGeometryPLY(CVector3Array allPoints, CVector3Array allNormals, CVector3Array allUV,
                         CLongArray indices, CTransformation global_trans)
{
    /** Specialized fuction. 
    *   The questions, always before the loop.
    *   First of all, the most small case.
    */
    CString plyData;
    //- only points
    if ( !exportNormals && !have_UV )
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            plyData += CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]);
            //-
            plyData += L" 255 255 255";
            plyData += L"\n"; 
        }
    }
    //- points + UVs, not normals
    else if( !exportNormals && have_UV ) 
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            plyData += CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]);
            //-- UVs
            plyData += L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]);
            //-
            plyData += L" 255 255 255";
            plyData += L"\n";
        }
    }
    //-- points + normals, not UV
    else if ( exportNormals && !have_UV )
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points..
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            plyData += CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]);
            //- normals
            CVector3 vNorm(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
            vNorm.MulByTransformationInPlace(global_trans);
            plyData += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]);                   
            //- close line
            plyData += L" 255 255 255";
            plyData += L"\n";
        }
    }
    //- all data; points + normals + UVs.
    else
    {
        for (LONG j=0; j < allPoints.GetCount(); j++)
        {
            //-- points
            CVector3 vPos(allPoints[j][0], allPoints[j][1], allPoints[j][2]);
            vPos.MulByTransformationInPlace(global_trans);
            //- and write string array, in correct Lux format ( x, -z, y )
            plyData += CString(vPos[0]) + L" "+  CString(-vPos[2]) + L" "+ CString(vPos[1]);
            //- normals   
            CVector3 vNorm(allNormals[j][0], allNormals[j][1], allNormals[j][2]);
            vNorm.MulByTransformationInPlace(global_trans);
            // for 'normals', change 'y' for 'z'(x, z, y), but not negative value (-z). TODO: more testing..
            plyData += L" "+ CString(vNorm[0]) + L" "+ CString(vNorm[2]) + L" "+ CString(vNorm[1]);        
            //-- UVs
            plyData += L" "+ CString(allUV[j][0]) + L" "+  CString(allUV[j][1]);              
            //-
            plyData += L" 255 255 255";
            plyData += L"\n";
        }

    }
    //- faces
    for ( int k = 0; k < indices.GetCount(); k += 3)
    {
        plyData += L"3 "+ CString(int(k)) + L" "+ CString(int(k+1)) + L" "+ CString(int(k+2)) + L"\n";
    }
    //-
    return plyData;
}
//-- 
CString writeLuxsiObj(X3DObject o)
{
    // Writes objects
    vUV.Clear();
    vNormals.Clear();
    vTris.Clear();
    vPoints.Clear();
    lxoGeometry.Clear();
    lxoData.Clear();
    plyData.Clear();
    //
    bool vText = false, vIsSubD = false;
    long vertCount, triCount;

    Geometry g(o.GetActivePrimitive().GetGeometry(ftime));

    /** use an 'obus' for kill flyes?
    *   nah nah... find better way!
    */
    CRefArray mats(o.GetMaterials());
    Material m = mats[0];

    CRefArray shaderArray(m.GetShaders());
    Shader s(shaderArray[0]);
    // test
    CString vMatID(s.GetProgID().Split(L".")[1]);

    CRefArray vtexture(s.GetShaders());
    Texture tex(vtexture[0]);
    CString vTexID(tex.GetProgID().Split(L".")[1]);
    if (vTexID == L"txt2d-image-explicit") vText = true;

    CGeometryAccessor ga;
    
    //-
    LONG subdLevel = 0;
    Property geopr = o.GetProperties().GetItem(L"Geometry Approximation");
    if (int(geopr.GetParameterValue(L"gapproxmordrsl")) > 0 )
    {
        vIsSubD = true; //-- only render
        subdLevel = int(geopr.GetParameterValue(L"gapproxmordrsl"));
        //- TODO; create advice message, for used this option ?
    }
    //--
    KinematicState  global_kinec_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_kinec_state.GetTransform(ftime); // add time

    //--
    ga = PolygonMesh(g).GetGeometryAccessor();
    //-
    if ( ga.GetUVs().GetCount() > 0 && vText) have_UV = true;    
    
    //{
    if (int(g.GetTriangles().GetCount()) > 0 )
    {
        //-- new pos for override
        if (!overrGeometry)
        {
            //--
            CTriangleRefArray triangles(g.GetTriangles());
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

            /** Process geometry. 
            */
            vertCount = allPoints.GetCount();
            triCount = g.GetTriangles().GetCount();
        
            if ( vplymesh ) 
            {
                plyData = writeGeometryPLY(allPoints, allNormals, allUV, indices, global_trans);
            }
            else
            {
                lxoGeometry = writeGeometryLXO( allPoints, allNormals, allUV, indices, global_trans);
            }
        } //- end override
        //--
        lxoData = L"\nAttributeBegin #"+ o.GetName();
        lxoData += L"\nNamedMaterial \""+ m.GetName() + L"\"\n";

        /** Geometry associated to lights.
        *   Portal lights use a 'trick'..
        *   TODO; find better mode
        */
        bool vIsPortal = false;        
        //-
        string::size_type loc = string( CString( o.GetName()).GetAsciiString()).find( "PORTAL", 0 );
        if (loc != string::npos) vIsPortal = true;

        //- mesh..
        CString type_mesh = L"mesh";
        if ( vplymesh ) type_mesh = L"plymesh";
        //- shape..
        CString type_shape = L"Shape";
        if ( vIsPortal ) type_shape = L"PortalShape";

        //-----------------
        if ( vMatID == L"lux_emitter_mat" )
        //-----------------
        {
            //--
            float red, green, blue, alpha;
            s.GetColorParameterValue(L"color", red, green, blue, alpha );
            float emitt(s.GetParameterValue(L"power"));
            //--
            CString lName = findInGroup(o.GetName());
            if (lName == L"") lName = o.GetName();
            //--
            lxoData += L"LightGroup \""+ lName + L"\"\n";
            //-
            lxoData += L"\nAreaLightSource \"area\" \n"
                + floatToString(s, L"importance")
                + floatToString(s, L"gain")
                + floatToString(s, L"power")
                + floatToString(s, L"efficacy")
                + integerToString(s, L"nsamples");
            lxoData += L"  \"color L\" ["
                + CString(red * emitt) + L" "
                + CString(green * emitt) + L" "
                + CString(blue * emitt ) + L"]\n";
        }        

        //-- share for all types; PLY and LXO
        lxoData += L"\n"+ type_shape + L" \""+ type_mesh + L"\" \n";        
        lxoData += L"  \"integer nsubdivlevels\" ["+ CString( subdLevel ) + L"]\n";
        lxoData += L"  \"string subdivscheme\" [\"loop\"] \n";
        lxoData += L"  \"bool dmnormalsmooth\" [\""+ CString( MtBool[exportNormals] ) + L"\"]\n";
        lxoData += L"  \"bool dmsharpboundary\" [\""+ CString( MtBool[vSharp_bound] ) + L"\"]\n";
        //f << "  \"string displacementmap\" [\"none\"]\n"; // here, place normalmap texture
        //f << "  \"float dmscale\" [\"0.0\"] \"float dmoffset\" [\"0.0\"]\n";
        
        //--------------
        if ( !vplymesh )
        //--------------
        {            
            lxoData += L"  \"string acceltype\" [\""+ CString( MtAccel[vAccel] ) + L"\"]\n";
            lxoData += L"  \"string tritype\" [\"auto\"] \n";//TODO
            //-
            lxoData += lxoGeometry;
            //-
            lxoData += L"\nAttributeEnd #"+ o.GetName() + L"\n";
            //- set extension lxo
            CString lxo_extension = L"_"+ o.GetName() + L".lxo";
        
            //- set name for include, use path relative to export.
            CString include_lxo_filename = luxsi_normalize_path(vFileGeo) + lxo_extension;

            //- set path for write file
            if ( !overrGeometry )
            {
                int next = vFileGeo.ReverseFindString(".");
                CString write_lxo_filename = vFileGeo.GetSubString(0,next) + lxo_extension;
                //-
                write_lxoFile(lxoData, write_lxo_filename);
            }
            //- 
            lxoData.Clear();
            lxoData = L"\nInclude \""+ include_lxo_filename + L"\"\n";
            
        }
        else
        {
            /** Make link to .ply file \n
            *   The vFileGeo value = full path + filename + framenumber + LXS extension. \n 
            *   luxsi_normalize_path(), returns alone the filename with the framenumber. \n
            *   Here we add the name of the object and the expension PLY.
            */
            CString ply_ext = L"_"+ o.GetName() + L".ply";
            CString include_ply_filename = luxsi_normalize_path(vFileGeo) + ply_ext;
            //-
            if (luxdebug ) app.LogMessage(L"File ply: "+ vFileGeo + L" Include ply: "+ include_ply_filename);
            //-
            lxoData += L"  \"string filename\" [\""+ include_ply_filename + L"\"]\n";
            //-
            lxoData += L"\nAttributeEnd #"+ o.GetName() + L"\n";

            /* if not override geometry, go to write ply file 
            */
            if ( !overrGeometry )
            {
                int next = vFileGeo.ReverseFindString(".");
                CString write_ply_filename = vFileGeo.GetSubString(0,next) + ply_ext;
                //-
                write_plyFile(plyData, write_ply_filename, vertCount, triCount);
            }
        }        
    }
    return lxoData;
}
//-
void write_lxoFile(CString lxoData, CString lxoFile)
{
    std::ofstream filelxo;
    filelxo.open(lxoFile.GetAsciiString(),'w');
    //-
    filelxo << lxoData.GetAsciiString();
    filelxo.close();
    
}
//-
void write_plyFile(CString plyGeometryData, CString vfile, int vCount, int tCount)
{
    //-
    CString plyStr;
    //-
    plyStr  = L"ply\n"; 
    plyStr += L"format ascii 1.0\n";
    plyStr += L"comment created with LuXSI; LuxRender Exporter for Autodesk Softimage\n";
    plyStr += L"element vertex "+ CString( vCount ) + L"\n";
    plyStr += L"property float x\n";
    plyStr += L"property float y\n";
    plyStr += L"property float z\n";
    
    if ( exportNormals )
    {
        plyStr += L"property float nx\n";
        plyStr += L"property float ny\n";
        plyStr += L"property float nz\n";
    }
    if ( have_UV )
    {
        plyStr += L"property float u\n";
        plyStr += L"property float v\n";
    }
    // for vertex colors
    plyStr += L"property uchar red\n";
    plyStr += L"property uchar green\n";
    plyStr += L"property uchar blue\n";
    //-
    plyStr += L"element face "+ CString( tCount ) + L"\n";
    plyStr += L"property list uchar uint vertex_indices\n";
    plyStr += L"end_header\n";

    //-- write file
    std::ofstream fileply;
    fileply.open(vfile.GetAsciiString(), ios::binary); // add 'binary' for compat to SLG
    //-
    fileply << plyStr.GetAsciiString();
    fileply << plyGeometryData.GetAsciiString();
    
    fileply.close();
}
//-