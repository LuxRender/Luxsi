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

#ifndef LUXSI_INSTANCES_H
#define LUXSI_INSTANCES_H

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

/**/
extern CString luxsiTransformClasic(X3DObject o, CString pivotReference);

#endif //LUXSI_INSTANCES_H