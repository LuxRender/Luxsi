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


#ifndef LUXSI_MATERIAL_H
#define LUXSI_MATERIAL_H

#include "luxsi_main.h"


//using namespace MATH;
using namespace std;
using namespace XSI;

extern float red, green, blue, alpha;
extern Application app;
extern  CStringArray aMatList;
extern ofstream f;
extern float vContrast;
//--
extern bool luxsi_find(CStringArray in_array, CString in_string);
//-
extern string replace(string input);
//-
extern bool luxdebug;

//- declare local variables
float b_red, b_green, b_blue, b_alpha;

float sp_red, sp_green, sp_blue, sp_alpha;

float refl_red, refl_green, refl_blue, refl_alpha;

float mRough;

//ImageClip2 vBumpFile;
bool vIsSet=false;

Material mat;
Texture vTexture;

//- generic string containers for shader definitions
CString 
	shaderStr,		//- general definitions
	vChanel,		//- chanel texture type
	shaderType;		//- shader type


/* Find texture connect to shader port */
/**/
Texture find_tex_used(Shader in_shader, CString in_string);

/* Find an specific shader port */
/**/
CString find_shader_used( Shader in_shader, CString in_string);

/* Write LuxRender Glass shader */
/**/
CString write_lux_glass(Shader in_shader, CString in_string, CString in_ID);

/* Write LuxRender Metal shader */
/**/
CString write_lux_metal(Shader in_shader, CString in_string);

/* Write LuxRender Car Paint shader */
/**/
CString write_lux_car_paint(Shader in_shader, CString in_string);

/* Write test to translate an generic mia_material_phen shader */
/**/
CString write_mia_material_phen(Shader in_shader, CString in_string);

/* Write LuxRender Shinymetal shader (deprecated?)*/
/**/
CString write_lux_shinymetal(Shader in_shader, CString in_string);

//--
CString write_lux_substrate(Shader in_shader, CString in_string);

/* Write LuxRender Matte and Matte Translucent shader */
/**/
CString write_lux_matte(Shader in_shader, CString in_string);


#endif //LUXSI_MATERIAL_H