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

using namespace std;
using namespace XSI;
using namespace MATH;

//-
extern double ftime;
extern ofstream f;

extern Application app;

extern bool luxdebug;

int writeLuxsiCloud(X3DObject obj)
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
    //- si se usa el siguiente comando, se produce un 'ERROR Runtime library c++'
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
        if ( luxdebug ) app.LogMessage(L"Particle attributes..: "+ CString(attr.GetName()));
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
        if (attr.GetName() == L"Shape")
        {
            attr.GetDataArray(ashape);
           // particle_shape = ashape[0];
        }
    }
    //- set transform
    KinematicState start_state = obj.GetKinematics().GetGlobal();
    CTransformation cloud_transform = start_state.GetTransform(ftime); //-- time
    CVector3 cloud_pos;

    //--- more  test´s for particle support
    //-- only for test; not better way
    //- float not work, use a 'trick'
    //-
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
       
    //-
    for (unsigned int i=0;i<aPointPosition.GetCount();i++)
    {
        //-
        cloud_pos.Set(double(aPointPosition[i][0]), double(aPointPosition[i][1]), double(aPointPosition[i][2]));
        cloud_pos.MulByTransformationInPlace(cloud_transform);

        //- get all points
        f << "\nAttributeBegin \n";
        f << "\nNamedMaterial \""<< m.GetName().GetAsciiString() <<"\"\n";
        f << "Translate "<< cloud_pos[0] <<" "<< -cloud_pos[2] <<" "<< cloud_pos[1] <<"\n";
        //-
        f << "Shape \"sphere\"\n";          //- TODO; use other shapes from GUI options?
        f << "  \"float radius\" ["<< CString(_size).GetAsciiString() <<"]\n";  //- if use float 'aSize', make a error; revised
        f << "  \"float zmin\" [ -90 ]\n";  //- test; semi-sphere up =  -90
        f << "  \"float zmax\" [ 90 ]\n";   //- test; semi-sphere down = 90
        f << "  \"float phimax\" [360]\n";
        f << "AttributeEnd #"<< obj.GetName().GetAsciiString() <<"\n"; // CString(i)...   
    }
    return 0;
   
}
//--