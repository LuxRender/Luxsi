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
//extern bool luxdebug;

//-- work in progress
CString writeLuxsiSurface(X3DObject o)
{
    //-- WIP: lack a lots of updates..
    //- or use other methode (like Yafxsi :)
    CString surfaceData;

    Geometry g(o.GetActivePrimitive().GetGeometry(ftime)) ; // add time
    CRefArray mats(o.GetMaterials()); // Array of all materials of the object
    Material m = mats[0];
    CRefArray shad(m.GetShaders()); // Array of all shaders attached to the material [e.g. phong]
    Shader s(shad[0]);
    //--
    KinematicState  gs = o.GetKinematics().GetLocal();
    CTransformation gt = gs.GetTransform(ftime); // add time

    //--
    float vradius = o.GetParameterValue(L"radius");
    //-
    float start_u = o.GetParameterValue(L"startuangle");
    float end_u = o.GetParameterValue(L"enduangle");
    //
    CVector3 axis(0.0f, 0.0f, 0.0f);
    float vphimax = 360;
    float rotar = 0.0;
    //-
    // el objeto a dibujar es la diferencia entre el mayor y el menor valor
    if (start_u > end_u)
    {
        if (start_u == 180)
        {
            vphimax = start_u + end_u;
        }
        else //if (start_u != 180)
        {
            vphimax = (360 - start_u) + end_u;
            rotar = end_u - start_u; // result negative
            axis.Set(0,0,1);
        }
    }
    else
    {
        vphimax = end_u - start_u;
        rotar = -vphimax;
        axis.Set(0,0,1);
    }
    //surfaceData.Clear();


    //-
    surfaceData = L"\nAttributeBegin\n";
    //-
    surfaceData += L"\nTransformBegin\n";

    //surfaceData += luxsiTransformClasic(o, L"global");

    surfaceData += L"\nTranslate "
                   + CString( gt.GetPosX() ) + L" "
                   + CString( gt.GetPosY() ) + L" "
                   + CString( gt.GetPosZ()) + L"\n";
    //-
    if (rotar != 0)
    {
        surfaceData += L"Rotate "
                       + CString( rotar )/*(rot*180/PI)*/ + L" "
                       + CString( axis[0]) + L" "
                       + CString( axis[1]) + L" "
                       + CString( axis[2]) + L"\n";
    }
    if (gt.GetSclX()!=1 || gt.GetSclY()!=1 || gt.GetSclZ()!=1)
    {
        surfaceData += L"Scale "
                       + CString( gt.GetSclX() ) + L" "
                       + CString( gt.GetSclY() ) + L" "
                       + CString( gt.GetSclZ() ) + L"\n";
    }
    //-
    float end_v = o.GetParameterValue(L"endvangle");
    float start_v = o.GetParameterValue(L"startvangle");
    //--
    surfaceData += L" Shape  \"sphere\" \n";
    surfaceData += L"  \"float radius\" ["+ CString( vradius ) + L"]\n";
    surfaceData += L"  \"float zmin\" [ -90 ]\n";
    surfaceData += L"  \"float zmax\" [ 90 ]\n";
    surfaceData += L"  \"float phimax\" ["+ CString( vphimax ) + L"]\n";
    //--
    surfaceData += L"TransformEnd\n";
    surfaceData += L"\nAttributeEnd\n";
    //--
    return surfaceData;
}
