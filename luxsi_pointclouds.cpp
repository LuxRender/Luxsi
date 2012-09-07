/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 2011 2012  Michael Gangolf 
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
#include "luxsi_main.h"

using namespace std;
using namespace XSI;
using namespace MATH;

//-
extern ofstream f; 

//-- work  in progress
int writeLuxsiCloud(X3DObject obj)
{     
    //
    // Write pointclouds
    //-
    /*
    All types are associated to specific ICEAttribute types. Therefore, you need to declare the right
	array object type that matches the ICEAttribute data type you want to access. Otherwise a runtime
	error will occur and the returned array will be empty.

	\sa ICEAttribute::GetDataArray, ICEAttribute::GetDataArrayChunk,
		\ref CICEAttributeDataArrayTypedefs "Type Definitions for CICEAttributeDataArray"
	\since 7.0

	\eg This example demonstrates how to iterate over the PointPosition attribute data on a grid primitive.
	
    //-code
		//using namespace XSI;
		//CValue CreatePrim( const CString& in_presetobj, const CString& in_geometrytype, const CString& in_name, const CString& in_parent );

		//X3DObject grid = CreatePrim( L"Grid", L"MeshSurface", L"", L"");

		ICEAttribute myattr = obj.GetActivePrimitive().GetGeometry().GetICEAttributeFromName( L"PointPosition" );

		CICEAttributeDataArrayVector3f points;
		myattr.GetDataArray( points );

		Application xsi;
		for( ULONG i=0; i<points.GetCount( ); i++ )
		{
            xsi.LogMessage(L"My prueba para MT 7.5: "+ CString( points[ i ] ) );
		}

		// Helper
		CValue CreatePrim( const CString& in_presetobj, const CString& in_geometrytype, const CString& in_name, const CString& in_parent )
		{
			CValueArray args(4);
			CValue retval;
			args[0]= in_presetobj;
			args[1]= in_geometrytype;
			args[2]= in_name;
			args[3]= in_parent;

			Application app;
			app.ExecuteCommand( L"CreatePrim", args, retval );
			return retval;
		}
        */
    //test for material
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
    CICEAttributeDataArrayFloat aSize;
    //--
    CICEAttributeDataArrayVector3f aVel;
    //- its work on xsi 2012
    CRefArray attrs = obj.GetActivePrimitive().GetGeometry().GetICEAttributes();
    
    //- asi esta en la demo de doc 7.5
    //ICEAttribute myattr = obj.GetActivePrimitive().GetGeometry().GetICEAttributeFromName( L"PointPosition" );
    //- more
    /* 
    case siICENodeDataFloat:
			{
				CICEAttributeDataArrayFloat dataArray;
				attr.GetDataArray(dataArray);
				return dataArray.GetCount() > 0;
			}
			break;
    */

    for( int i = 0; i<attrs.GetCount(); i++ ) 
    {
        //ICEAttribute attr = attrs[i];
        attr = attrs[i];
        //--
        if (attr.GetName() == L"PointPosition"){
            attr.GetDataArray(aPointPosition);
        }
        if (attr.GetName() == L"State_ID"){
            attr.GetDataArray(aID);
        }
        // error in 7.5
        if (attr.GetName() == L"Size"){
            attr.GetDataArray(aSize);
        }
        //- error in 7.5
        if (attr.GetName() == L"PointVelocity"){
            attr.GetDataArray(aVel);
        }
    }
    //- set transformations
    KinematicState start_state = obj.GetKinematics().GetGlobal();
    CTransformation cloud_transform = start_state.GetTransform();
    CVector3 cloud_pos;
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
        f << "Shape \"sphere\"\n"; //-TODO; use other shapes from GUI options?
        f << "  \"float radius\" [0.5]\n";
        //f << "  \"float zmin\" [ 0 ]\n"; //- test; semi-sphere up =  -90
        //f << "  \"float zmax\" [ 90 ]\n";//- test; semi-sphere down = 90
        f << "  \"float phimax\" [360]\n";
        f << "AttributeEnd #"<< obj.GetName().GetAsciiString() <<"\n"; // CString(i)...   
    }
    return 0;
   
}
//--