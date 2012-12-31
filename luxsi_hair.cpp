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

/**/
extern Application app;
/**/
extern double ftime;
/**/
extern bool luxdebug;
/**/
extern Model root;
/**/
extern bool overrGeometry;
/**/
extern CString writeModel(X3DObject in_object);
/**/
extern CString writeLuxsiObj(X3DObject in_object);

/**/
CString luxNativeHair(X3DObject o, CFloatArray posVals)
{
    Material m = o.GetMaterial();
    CString hairData;
    
    //- write object 'base'
    hairData = L"\nObjectBegin \""+ o.GetName() + L"\"\n";
    //-
    hairData += L"NamedMaterial \""+ m.GetName() + L"\"\n";
    hairData += L"Shape \"sphere\"\n";
    hairData += L"  \"float radius\" [ 0.2 ]\n";
    hairData += L"\nObjectEnd \n";
      
    //- instantiate object to each 'hair segment'
    for(int j = 0; j < posVals.GetCount(); j +=3)
    {
        hairData += L"\nAttributeBegin \n";
        hairData += L"Translate "
            + CString(posVals[j])   + L" "
            + CString(-posVals[j+2])+ L" "
            + CString(posVals[j+1]) + L"\n";
        hairData += L"ObjectInstance \""+ o.GetName() + L"\"\n";
        hairData += L"AttributeEnd #"+ o.GetName() + L"\n"; 
    }
    return hairData;
}
//-
CString instantiateHair(CFloatArray posVals, CString objName, long jump)
{
    CString hairData;
    //-
    for(int j = 0; j < posVals.GetCount(); j += jump)
    {
        /* Use 'jump' for moved each object instanced to 'root' hair position
        */
        hairData += L"\nAttributeBegin # "+ objName;  
        hairData += L"\nTransformBegin\n";                 
        hairData += L"Translate "
            + CString(posVals[j]) + L" "
            + CString(-posVals[j+2]) + L" "
            + CString(posVals[j+1]) + L"\n";   
        //-
        hairData += L"ObjectInstance \""+ objName + L"\"\n";
        hairData += L"TransformEnd\n";
        hairData += L"AttributeEnd\n";
    }
    return hairData;
}
//--
CString writeLuxsiHair(X3DObject o)
{
    //--
    CString hairData;
   
    Material m = o.GetMaterial();
    //-
    HairPrimitive myHairPrim( o.GetActivePrimitive());
    //-- add transform
    KinematicState  global_state = o.GetKinematics().GetGlobal();
    CTransformation global_trans = global_state.GetTransform(ftime);        
    /**
    *   Use 'RenderPercentage' value for 'requesthaircount' ( is float, change to integer ?)\n
    *   Use 'TotalHairs' for 'chunksize.
    */
    int percentaje = o.GetParameterValue(L"RenderPercentage");
    int totalHairs = o.GetParameterValue(L"TotalHairs");
    LONG request = (totalHairs /100) * percentaje;
    /**
    *   Notes: XSI generate...                      \n 
    *   1 vector xyz por each 'hair root'.          \n
    *   1 vector xyz for each 'hair section'.       \n
    *   1 float for each radius of 'hair section'.
    */
    int segment = o.GetParameterValue(L"Segments");
    if (luxdebug) app.LogMessage(L"Segments: "+ CString(segment));
    //
    long jump = (segment + 1)*3;
    if (luxdebug) app.LogMessage(L"Jump: "+ CString(jump));
                        
    //-
    bool is_instanced = o.GetParameterValue(L"InstanceEnabled");
    //-
    CRenderHairAccessor rha;
    rha = myHairPrim.GetRenderHairAccessor(request); 
    LONG nReqChunkSize = rha.GetRequestedChunkSize();
    if (luxdebug) app.LogMessage( L"nReqChunkSize: " + CValue(nReqChunkSize).GetAsText() );

    LONG nReqHairCount = rha.GetRequestedHairCount();
    if (luxdebug) app.LogMessage( L"nReqHairCount: " + CValue(nReqHairCount).GetAsText() );

    // get the values
    while( rha.Next() )
    {
        // get the render hair positions
        CFloatArray posVals;
        rha.GetVertexPositions(posVals);
        //-
        if ( !is_instanced )
        {
            //- write object 'base'
            hairData = L"\nObjectBegin \""+ o.GetName() + L"\"\n";
            hairData += L"Shape \"sphere\"\n";
            hairData += L"  \"float radius\" [ 0.02 ]\n";
            hairData += L"\nObjectEnd \n";
            
            //- instantiate object to each 'hair segment'
            for(int j = 0; j < posVals.GetCount(); j +=3)
            {
                hairData += L"\nAttributeBegin \n";
                hairData += L"Translate "
                    + CString(posVals[j])   + L" "
                    + CString(-posVals[j+2])+ L" "
                    + CString(posVals[j+1]) + L"\n";
                //-
                hairData += L"NamedMaterial \""+ m.GetName() + L"\"\n";
                hairData += L"ObjectInstance \""+ o.GetName() + L"\"\n";
                hairData += L"AttributeEnd #"+ o.GetName() + L"\n"; 
            }
            
        }
        else //-- use instanced object
        {
            X3DObject objInstance;            
            //--
            CRefArray grps = root.GetGroups();
            //-
            for (int i=0; i < grps.GetCount(); i++)
            {
                if (luxdebug) app.LogMessage(L"Group name: "+ Group(grps[i]).GetName());
                CString gName = Group(grps[i]).GetName();
                //
                string::size_type gpname = string(CString(gName).GetAsciiString()).find( "HAIR", 0 );
                //--
                if ( gpname != string::npos )
                {               
                    CRefArray groupMembers = Group(grps[i]).GetMembers();
                    //-
                    for (int j = 0; j < groupMembers.GetCount(); j++)
                    {
                        objInstance = groupMembers[j];
                        CString oName = objInstance.GetName();
                        //-
                        if ( objInstance.GetType() == L"polymsh" ) 
                        {
                            overrGeometry = true;
                            //-
                            hairData += L"\nObjectBegin \""+ oName + L"\"\n";
                            hairData += writeLuxsiObj(objInstance);
                            hairData += L"\nObjectEnd # "+ oName + L"\n";
                            //-
                            overrGeometry = false;
                            //-
                            hairData += instantiateHair(posVals, oName, jump);                        
                        }
                        else if ( objInstance.GetType() == L"#model" ) 
                        {
                            hairData += writeModel(objInstance);
                            //-
                            hairData += instantiateHair(posVals, oName, jump);
                        }
                        else 
                        {
                            app.LogMessage(L"Object type not valid for hair instances", siErrorMsg);
                        }
                    }
                }
                else
                {
                    app.LogMessage(L"Not Group object with 'HAIR' string in your name", siErrorMsg);
                }
            }
        }           
    }
    //--
    return hairData;
}
