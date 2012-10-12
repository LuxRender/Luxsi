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

#ifndef LUXSI_GEOMETRY_H
#define LUXSI_GEOMETRY_H

#include "include\luxsi_main.h"
#include "plymesh\rply.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/* for animation
*/
extern double ftime;

/* ofstream pointer for write file export.
*/
extern ofstream f;

/* if mesh is a ply type
*/
extern bool vplymesh;

extern CString vFileExport;

/* The name of PLY file
*/
extern CString vFilePLY;

/* Define on/off exported smoooth objects
*/
extern bool vSmooth_mesh;

/**/
extern bool vSharp_bound;

/**/
extern int vAccel;

/**/
extern bool luxdebug;

/**/
extern Application app;

/**/
extern const char *MtBool[], *MtAccel[];

/**/
extern std::string luxsi_replace(string in_input);

/**/
extern CString findInGroup(CString s);

/**/
void new_ply(CString in_VertNormUv, CString in_faces, CString in_plyFile, int vcount, int pcount);

/**/
extern CString luxsi_normalize_path(CString vFile);

#endif // LUXSI_GEOMETRY_H