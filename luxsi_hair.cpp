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

/**/
#include "include\luxsi_main.h"

using namespace std;
using namespace XSI;
using namespace MATH;

extern Application app;

extern double ftime;

/**/
extern Model root;

/**/
extern CString writeModel(X3DObject in_object);


//--
CString writeLuxsiHair(X3DObject o)
{
    //--
    CString hairData;
    CVector3 hair_pos; 
    Material m = o.GetMaterial();
    //-
    HairPrimitive myHairPrim( o.GetActivePrimitive());
    //-- add transform
    KinematicState  global_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_state.GetTransform(ftime);        
    /**
    *   Use 'RenderPercentage' value for 'requesthaircount' ( is float, change to integer ?)
    *   Use 'TotalHairs' for 'chunksize.
    */
    int percentaje = o.GetParameterValue(L"RenderPercentage");
    int totalHairs = o.GetParameterValue(L"TotalHairs");
    LONG request = (totalHairs /100) * percentaje;
    /**
    *   Notes: XSI generate... 
    *   1 vector xyz por each 'hair root'.
    *   1 vector xyz for each 'hair section'.
    *   1 float for each radius of 'hair section'.
    */
    int segment = o.GetParameterValue(L"Segments");
    app.LogMessage(L"Segments: "+ CString(segment));
    //
    long jump = (segment + 1)*3;
    app.LogMessage(L"Jump: "+ CString(jump));
                        
    //-
    bool is_instanced = o.GetParameterValue(L"InstanceEnabled");
    //-
    CRenderHairAccessor rha;
    rha = myHairPrim.GetRenderHairAccessor(request); 
    LONG nReqChunkSize = rha.GetRequestedChunkSize();
    app.LogMessage( L"nReqChunkSize: " + CValue(nReqChunkSize).GetAsText() );

    LONG nReqHairCount = rha.GetRequestedHairCount();
    app.LogMessage( L"nReqHairCount: " + CValue(nReqHairCount).GetAsText() );

    LONG nUVs = rha.GetUVCount();
    app.LogMessage( L"nUVs: " + CValue(nUVs).GetAsText() ); 
   
    // get the values in chunks
    LONG nChunk = 0;
    LONG i=0;
    while( rha.Next() )
    {
        app.LogMessage( L"Chunk: " + CValue(nChunk++).GetAsText() );
        /**
        *   Get the number of vertices for each render hair
        *    note: this array is used for iterating over the render hair position
        *    and radius values
        */
        CLongArray verticesCountArray;
        rha.GetVerticesCount(verticesCountArray);

        // get the render hair positions
        CFloatArray posVals;
        rha.GetVertexPositions(posVals);

        LONG k = 0;
        //-
        if ( !is_instanced )
        {
            //- write object 'base'
            hairData = L"\nObjectBegin \""+ o.GetName() + L"\"\n";
            //-
            hairData += L"\nAttributeBegin \n";
            hairData += L"NamedMaterial \""+ m.GetName() + L"\"\n";
            hairData += L"Shape \"sphere\"\n";
            hairData += L"  \"float radius\" [ 0.2 ]\n";
            hairData += L"  \"float zmin\" [ -90 ]\n";
            hairData += L"  \"float zmax\" [ 90 ]\n";
            hairData += L"  \"float phimax\" [360]\n";
            hairData += L"AttributeEnd \n";
            //-
            hairData += L"\nObjectEnd \n";

            //- instantiate object to each 'hair root'
            for(int j = 0; j < posVals.GetCount(); j +=3)
            {
                //- get all points
                hairData += L"\nAttributeBegin \n";
                //-
                hairData += L"TransformBegin\n";                 
                hairData += L"Translate "
                    + CString(posVals[j])   + L" "
                    + CString(-posVals[j+2])+ L" "
                    + CString(posVals[j+1]) + L"\n";
                //-
                hairData += L"ObjectInstance \""+ o.GetName() + L"\"\n";
                hairData += L"TransformEnd\n";
                hairData += L"AttributeEnd #"+ o.GetName() + L"\n"; 
            }
        }
        else
        {
            //-- use instanced object
            CString memberName;
            X3DObject objInstance;
            CRefArray gMembers;
            //--
            CRefArray grps = root.GetGroups();
            //-
            for (int i=0; i < grps.GetCount(); i++)
            {
                app.LogMessage(L"Group name: "+ Group(grps[i]).GetName());
                //-
                gMembers = Group(grps[i]).GetMembers();
                //-
                for (int j = 0; j < gMembers.GetCount(); j++)
                {
                    objInstance = gMembers[j];
                    memberName = objInstance.GetName();
                    //--
                    app.LogMessage(L"Member object name: "+ memberName);
                }           
            }            
            //-
            if (!memberName.IsEmpty())
            {
                /** */
                hairData += writeModel(objInstance);
                //-
                for(int j = 0; j < posVals.GetCount(); j += jump)
                {
                    /* Use 'jump' for moved each object instanced to 'root' hair position
                    */
                    hairData += L"\nAttributeBegin # "+ o.GetName();  
                    hairData += L"\nTransformBegin\n";                 
                    hairData += L"Translate "
                        + CString(posVals[j]) + L" "
                        + CString(-posVals[j+2]) + L" "
                        + CString(posVals[j+1]) + L"\n";   
                    //-
                    hairData += L"ObjectInstance \""+ objInstance.GetName() + L"\"\n";
                    hairData += L"TransformEnd\n";
                    hairData += L"AttributeEnd\n";
                }
            }
            else
            {
                app.LogMessage(L"Not object instanced", siErrorMsg);
            }
        }           
    }
    //--
    return hairData;
}
