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

#ifndef LUXSI_H
#define LUXSI_H
//--
#pragma warning (disable : 4244) 
#pragma warning (disable : 4996) 

#include "luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/** CString containers 
*   vFileLxs        //- lxs file name
*   vFileQueue      //- lxq file name
*   luxsiShaderData //- shader data
*   queue_list      //- queue list data
*/
CString vFileLxs, vFileQueue, luxsiShaderData;

/* for load lxs file from GUI
*/
CString vblxs_file = L"";

/**/
CString queue_list;

/* for launch LuxRender with -L "queue.lxq" param 
*/
bool lqueue = false;

/* time 
*/
double ftime = DBL_MAX;

/**/
int vframestep = 1;



#endif //LUXSI_H