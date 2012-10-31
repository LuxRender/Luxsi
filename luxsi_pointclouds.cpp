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

using namespace std;
using namespace XSI;
using namespace MATH;

/**/
extern double ftime;

/**/
extern ofstream f;

/**/
extern Application app;

/**/
extern bool luxdebug;

/**/
CString strCloud;

CString writeLuxsiCloud(X3DObject obj)
{     
    //
    // Write pointclouds

    //- test for material
    CRefArray cloud_mats(obj.GetMaterials()); // Array of all materials
    //- for one mat.. atm
    Material m = cloud_mats[0];
    //CRefArray shad(m.GetShaders()); // Array of all shaders
    //Shader s(shad[0]);

    ICEAttribute attr;
    CICEAttributeDataArrayVector3f aPointPosition; 
    CICEAttributeDataArrayLong aID;
    //- if use aSize, 'ERROR Runtime library c++'
    //- TODO; revise
    //
    CICEAttributeDataArrayFloat aSize;
    //--
    CICEAttributeDataArrayVector3f aVel;
    //-
    CICEAttributeDataArrayShape ashape;
    //-
    int particle_shape = 0;
    
    //-
    CRefArray attrs = obj.GetActivePrimitive().GetGeometry(ftime).GetICEAttributes();
    
    //-
    for( int i = 0; i<attrs.GetCount(); i++ ) 
    {
        ICEAttribute attr = attrs[i]; 
        //attr = attrs[i];
        //if ( luxdebug ) 
        //app.LogMessage(L"Particle attributes name ..: "+ CString(attr.GetName()));
        //--
        if (attr.GetName() == L"PointPosition"){
            attr.GetDataArray(aPointPosition);
        }
        if (attr.GetName() == L"State_ID"){
            attr.GetDataArray(aID);
        }
        //-
        if (attr.GetName() == L"Size")
        {
            attr.GetDataArray(aSize);
        }
        //-
        if (attr.GetName() == L"PointVelocity")
        {
            attr.GetDataArray(aVel);
        }
        //- test shape
       // CString _shape=L"";
       // if (attr.GetName() == L"Shape")
       // {
           
       // }
    }
    //- set transform
    KinematicState start_state = obj.GetKinematics().GetGlobal();
    CTransformation cloud_transform = start_state.GetTransform(ftime); //-- time
    CVector3 cloud_pos;

    /** More tests for particle support.
    *   Only for test; not better way
    *   Float not work, use a 'trick'
    */
    CString _size = L"0.2"; 
    //- TODO; shape options into 'emmiter' UI
    //->
    if (aSize.GetCount() > 0)
    {
        for (unsigned int sz = 0; sz < aSize.GetCount(); sz++)
        {
            _size = CString(aSize[sz]).GetAsciiString();
            app.LogMessage(L"Size value: "+ CString(aSize[sz]));
        }
    }
    else
    {
        _size = CString(aSize[0]);
    }
       
    //---------------
    strCloud.Clear();
    //---------------
    for (unsigned int i = 0; i < aPointPosition.GetCount(); i++)
    {
        //-
        cloud_pos.Set(double(aPointPosition[i][0]), double(aPointPosition[i][1]), double(aPointPosition[i][2]));
        cloud_pos.MulByTransformationInPlace(cloud_transform);

        //- get all points
        strCloud += L"\nAttributeBegin \n";
        strCloud += L"\nNamedMaterial \""+ m.GetName() + L"\"\n";
        strCloud += L"Translate "+ CString(cloud_pos[0]) + L" "+ CString(-cloud_pos[2]) + L" "+ CString(cloud_pos[1]) + L"\n";
        //- TODO; use a a instance object
        strCloud += L"Shape \"sphere\"\n";                      //- TODO; use other shapes from GUI options?
        strCloud += L"  \"float radius\" ["+ _size + L"]\n";    //- if use float 'aSize', make a error; revised
        strCloud += L"  \"float zmin\" [ -90 ]\n";              //- test; semi-sphere up =  -90
        strCloud += L"  \"float zmax\" [ 90 ]\n";               //- test; semi-sphere down = 90
        strCloud += L"  \"float phimax\" [360]\n";
        strCloud += L"AttributeEnd #"+ obj.GetName() + L"\n";   //- CString(i)...     
    }
    return strCloud;
}