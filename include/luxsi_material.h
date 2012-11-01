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

#ifndef LUXSI_MATERIAL_H
#define LUXSI_MATERIAL_H

#include "luxsi_main.h"

using namespace std;
using namespace XSI;

/* for preview materials
*/
extern bool is_preview;

/**/
extern const char *MtBool[];

/* generic float for color
*/
extern float red, green, blue, alpha;

extern Application app;

extern  CStringArray aMatList;

extern ofstream f;

extern float vContrast;
//--
extern bool luxsi_find(CStringArray in_array, CString in_string);
//-
extern std::string luxsi_replace(string input);
//-
extern bool luxdebug;

extern CString luxsi_texture( Material in_mat, Shader in_shader);

//- declare local variables
float b_red, b_green, b_blue, b_alpha;

float sp_red, sp_green, sp_blue, sp_alpha;

float refl_red, refl_green, refl_blue, refl_alpha;

/* for roughness value
*/
float mRough;

/* shader is defined
*/
bool vIsSet = false;

/* instance of Material Class
*/
Material mat;

/* instance of Texture class
*/
Texture vTexture;

/* generic string containers for shader definitions
*/
CString
    shaderStr,      //- container string for Shader data.
    materialData,   //- for material data.
    texStr,         //- container string for Texture data.
    vChanel,        //- chanel texture type.
    shaderType;     //- shader type.

/* Find texture connect to shader port
*/
Texture find_tex_used(Shader in_shader, CString in_string);

/* Find an specific shader port
*/
CString find_shader_used( Shader in_shader, CString in_string);

/* Write LuxRender Glass shader
*/
CString write_lux_glass(Shader in_shader, CString in_ID);

/* Write LuxRender Metal shader
*/
CString write_lux_metal(Shader in_shader);

/* Write LuxRender Car Paint shader
*/
CString write_lux_car_paint(Shader in_shader);

/* Write test to translate an generic mia_material_phen shader
*/
CString write_mia_material_phen(Shader in_shader);

/* Write LuxRender Shinymetal shader (deprecated?)
*/
CString write_lux_shinymetal(Shader in_shader);

/**/
CString write_lux_substrate(Shader in_shader);

/* Write LuxRender Matte and Matte Translucent shader
*/
CString write_lux_matte(Shader in_shader, CString in_string);

/**/
CString write_lux_glossy(Shader in_shader);

/** Helper for search color of texture connect to each node port
* @function mat_value,
* @param in_shader,      //- specific Shader Class
* @param in_texture,     //- Lux texture component ( Kd, Ks, Kr...)
* @param in_shader_port  //- Nodeport name ( diffuse, specular, Kd,,)
*/
CString mat_value( Shader in_shader, CString in_texture, CString in_shader_port);

#endif //LUXSI_MATERIAL_H
