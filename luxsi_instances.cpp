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

#include "include\luxsi_instances.h"

//--
CString writeInstances(X3DObject oInstance)
{
    CString instancesData;
    //-
    Model vModel = Model(oInstance).GetInstanceMaster();
    //--
    instancesData += L"\nAttributeBegin # "+ vModel.GetName();    
    //-
    instancesData += L"\nTransformBegin\n";
    instancesData += luxsiTransformClasic(oInstance, L"global");    
    instancesData += L"ObjectInstance \""+ vModel.GetName() + L"\"\n";
    instancesData += L"TransformEnd\n";
    //-
    instancesData += L"AttributeEnd\n";
    //-
    return instancesData;
}
//--
CString writeModel(X3DObject oModel)
{
    CString modelData;
    //-
    CRefArray childModel = oModel.GetChildren();
    //-
    if ( !luxsi_find(aInstanceList, oModel.GetName() ) ) 
    {
        //- turn ON overrGeometry for not re-write geometry file in HD
        overrGeometry = true;
        //-
        modelData = L"\nObjectBegin \""+ oModel.GetName() + L"\"\n";
        //-
        for (int i=0; i < childModel.GetCount(); i++)
        {
            modelData += writeLuxsiObj(childModel[i]);
        }
        modelData += L"\nObjectEnd # "+ oModel.GetName() + L"\n";
        //- reset..
        overrGeometry = false;
        //-
        aInstanceList.Add(oModel.GetName());
    }   
    return modelData;
}
//-
CString writeLuxsiInstance(CRefArray models)
{
    //--
    CString instanceData;
    //-
    for ( int i=0; i < models.GetCount(); i++ )
    {
        X3DObject obj(models[i]);

        if ( Model(obj).GetModelKind() == 0 )// Model / master
        {
            instanceData += writeModel(obj);
        }
        else if ( Model(obj).GetModelKind() == 2 ) // Model / instance
        {
            instanceData += writeInstances(obj);
        }
        else //if ( Model(obj).GetModelKind() == 1 ) 
        {
            app.LogMessage(L"Referenced Model");
        }
    }
    return instanceData;
}
