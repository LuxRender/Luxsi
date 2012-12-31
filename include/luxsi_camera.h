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

#ifndef LUXSI_CAMERA_H
#define LUXSI_CAMERA_H


#include "luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/**/
extern Application app;
/**/
extern double ftime;
/**/
extern bool luxdebug;
/**/
extern CString floatToString(Shader in_shader, CString in_item);
/**/
extern CString floatToString(Shader in_shader, CString in_item, CString in_definition);
/**/
extern CString integerToString(Shader s, CString item);

#endif // LUXSI_CAMERA_H