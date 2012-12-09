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

/**/
extern bool is_visible(X3DObject in_obj, CString in_type);

/**/
extern bool luxdebug;

/**/
extern Model root;

/* for test MsgBox
*/
extern UIToolkit kit;

/**/
extern bool vIsHiddenLight;

/**/
extern ofstream f;

/* convert  0 and 1 value bool to string
*/
extern const char *MtBool[];

/* float color values 
*/
extern float red, green, blue, alpha;

/* search name of light nodes
*/
CString Light_Shader_ID;

/**/
float vIntensity;

/* IES file path
*/
extern CString ies_file;

/* generic XSI Application 
*/
extern XSI::Application app;

/* replace  '/' and  '\' chars into path files
*/
extern std::string luxsi_replace(string input);

/* find name object into a group 
*/
extern CString findInGroup(CString in_name);

/* for math transform to 'light area' oobject 
*/
CString luxsi_area_light_transform(X3DObject o, float in_sizeX, float in_sizeY);

/* Search IBL and HDRI files used
*/
CString find_XSI_env(CString env_file);

/* Write specific 'point light' type
*/
void luxsi_point_light(X3DObject in_obj, Shader in_shader, CVector3 in_vector);
