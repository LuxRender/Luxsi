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

#include "luxsi_main.h"

using namespace std;
using namespace XSI;
using namespace MATH;

/* Diffuse reflect rejection 
*/
extern bool vdiff_reflect_reject;

/* Diffuse refract rejection.
*/
extern bool vdiff_refract_reject;

/**/
extern bool vglossy_reflect_reject;

/**/
extern bool vglossy_refract_reject;

/**/
extern CRefArray params;

/**/
extern int vSurfaceInt;

/**/
extern bool vsexpert;

/**/
extern bool vfinalgather;

/**/
extern int vSampler;

/**/
extern bool vExpert;

/**/
extern int vbasampler;

/**/
extern int vAccel;

/**/
extern bool vacexpert;

/**/
extern bool vExr;

/**/
extern bool vTga;

/**/
extern bool vPng;

/**/
extern int vfilter;

/**/
extern bool vfexpert;

/**/
extern CustomProperty prop;