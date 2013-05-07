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

using namespace XSI;
using namespace MATH;

extern int vXRes;
extern int vYRes;
extern int vDis;
extern float vContrast;
//-
extern ProgressBar pb;
/**/
extern Model root;
/**/
extern bool exportNormals;
/**/
extern int vmaxdepth;

//- external functions...
extern CString luxsi_normalize_path(CString in_path);
/**/
extern CVector3 cameraLookAt(Camera in_cam, CString in_date);
/**/
extern CRefArray sceneCollectionsLights();
/**/
extern void sceneCollectionsObjects();
/**/
extern CRefArray sceneCollectionMaterials();
/**/
extern CRefArray sceneCollectionsCameras();
/**/
extern CString writeLuxsiObj(X3DObject in_object);
/**/
extern bool is_visible(X3DObject in_obj, CString in_type);
