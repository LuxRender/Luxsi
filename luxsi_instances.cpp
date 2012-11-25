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

//-
using namespace XSI;
using namespace MATH;
using namespace std;

//-
extern CStringArray aInstanceList;

/**/
extern bool luxdebug;

/**/
extern double ftime;

/**/
extern Application app;

/**/
extern bool luxsi_find(CStringArray in_array, CString in_string);

/**/
extern CString writeLuxsiObj(X3DObject o);

/**/
extern CString luxsiTransformMatrix(X3DObject o);

//--
CString luxsiGlobalTransformClasic(X3DObject o)
{
    //-
    CString transformData;
    //--
    KinematicState global_state = o.GetKinematics().GetGlobal();
    CTransformation global_transf = global_state.GetTransform(ftime);
    //-
    transformData += L"Translate "
        + CString( global_transf.GetPosX())
        + L" "+ CString( -global_transf.GetPosZ())
        + L" "+ CString( global_transf.GetPosY()) + L"\n";

    //- rotation
    CVector3 axis;
    double rot = global_transf.GetRotationAxisAngle(axis);
    //-
    if (rot != 0)
    {
        transformData += L"Rotate "
            + CString( rot*180/PI )
            + L" "+ CString( axis[0] ) 
            + L" "+ CString( -axis[2] ) 
            + L" "+ CString( axis[1] ) + L"\n";
    }
    /** Scale.
    *   Add support for not uniform scale using "or". 
    */
    if (global_transf.GetSclX()!=1 || global_transf.GetSclY()!=1 || global_transf.GetSclZ()!=1)
    {
        /** WARNING! change 'Y' for 'Z', but not negative ( -z ).
        */
        transformData += L"Scale "
            + CString(global_transf.GetSclX())
            + L" "+ CString(global_transf.GetSclZ()) 
            + L" "+ CString(global_transf.GetSclY()) + L"\n";
    }
    return transformData;
}

//-
CString writeLuxsiInstance(X3DObject o)
{
    //--------------------
    // status: in progress
    //--------------------
    //-- instance
    CString instanceData = L"";
    //- write source object [won't be displayed]
    Model vModel = Model(o).GetInstanceMaster();
    CRefArray vGroup = X3DObject(vModel).GetChildren();
    //-
    if ( luxsi_find(aInstanceList, vModel.GetName() ) ) 
    {
        if ( luxdebug ) app.LogMessage(L"The instance already exists");
    }
    else
    {
        CString obj_instance = L"";
        //-
        instanceData = L"\nObjectBegin \""+ vModel.GetName() + L"\"\n";
        //-
        for (int i=0; i < vGroup.GetCount(); i++)
        {
            obj_instance = writeLuxsiObj(X3DObject(vGroup[i]));
        }
        //- test
        instanceData += obj_instance;
        //- end
        instanceData += L"\nObjectEnd # "+ o.GetName() + L"\n";
        aInstanceList.Add(vModel.GetName());// is correct?
    }
    //-
    instanceData += L"\nAttributeBegin # "+ o.GetName();
    
    //
    instanceData += L"\nTransformBegin\n";
    //-
    //CString transfMatrix = luxsiTransformMatrix(o); 
    //instanceData += L"\nTransform ["+ transfMatrix + L"]\n";
  
    /** Result from testing:
    *   The better order for transform is; Translate, Rotate and finally Scale.
    *   Usinng this method. Is more precise to matrix.. atm.
    */
    instanceData += luxsiGlobalTransformClasic(o);
    
    //-
    instanceData += L"ObjectInstance \""+ Model(o).GetInstanceMaster().GetName() + L"\"\n";
    //-
    instanceData += L"TransformEnd\n";
    //-
    instanceData += L"AttributeEnd\n";
    //-
    return instanceData;
}
