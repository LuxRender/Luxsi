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

#ifndef LUXSI_GEOMETRY_H
#define LUXSI_GEOMETRY_H

#include "luxsi_main.h"
//#include "plymesh\rply.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/**/
long vertCount;
long triCount;

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
extern CString vFileGeo;

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
extern CString luxsi_normalize_path(CString in_fileNamePath);

/**/
void write_plyFile(CString in_plyData, CString in_faces, CString in_plyFile, int vertexCount, int facesCount);

/**/
void write_lxoFile(CString in_lxoData, CString in_lxoFile);

/**/
extern CString luxsiTransformMatrix(X3DObject o);

#endif // LUXSI_GEOMETRY_H