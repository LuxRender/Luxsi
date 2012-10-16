/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 - 2012  Michael Gangolf, 'miga'
Code contributor ; Pedro Alcaide, 'povmaniaco'

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
//#include "plymesh\rply.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/**/
bool have_UV = false;

/* for animation
*/
extern double ftime;

/* ofstream pointer for write file export.
*/
extern ofstream f;

/* define use format PLY for mesh export.
*/
extern bool vplymesh;

/**/
extern CString vFileExport;

/* The name of PLY file
*/
extern CString vFilePLY;

/* Define on/off exported smoooth objects
*/
extern bool vSmooth_mesh;

/* Preserve hard edges 
*/
extern bool vSharp_bound;

/* Mode expert for Accelerator GUI
*/
extern int vAccel;

/**/
extern bool luxdebug;

/* Override export of geometry using PLY format 
*/
extern bool overrGeometry;

/**/
extern Application app;

/**/
extern const char *MtBool[];

/**/
extern const char *MtAccel[];

/**/
extern std::string luxsi_replace(string in_strInput);

/**/
extern CString findInGroup(CString in_items);

/**/
void write_plyFile(CString in_plyData, CString in_faces, CString in_plyFile, int vertexCount, int facesCount);

/**/
extern CString luxsi_normalize_path(CString in_fileNamePath);

#endif // LUXSI_GEOMETRY_H