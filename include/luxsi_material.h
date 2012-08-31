/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 2011 2012  Michael Gangolf and Pedro Alcaide

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
#include "luxsi_main.h"

using namespace std;
using namespace XSI;
//using namespace MATH;

extern float red, green, blue, alpha;
extern CString shaderType;
extern Application app;
extern CStringArray aMatList;
extern ofstream f;
extern float vContrast;
//--
extern bool luxsi_find(CStringArray a, CString s);
extern string replace(string input);

//--
CString write_lux_glass(Shader s, CString shStr);
//--
CString write_lux_roughglass(Shader s, CString shStr);
//--
CString write_lux_metal(Shader s, CString shStr);
//--
CString write_lux_car_paint(Shader s, CString shStr);
//--
CString write_mia_material_phen(Shader s, CString shStr);
//--
CString write_lux_shinymetal(Shader s, CString shStr);
//--
CString write_lux_substrate( Shader s, CString shStr);
//--
CString write_lux_mattetranslucent(Shader s, CString shStr);
