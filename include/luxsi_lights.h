/*
LuXSI - Autodesk(c) Softimage(c) XSI Export addon for the LuxRender  Renderer
(http://www.luxrender.org)

Copyright (C) 2010 2012  Michael Gangolf 
Code contributor; Pedro Alcaide (aka povmaniaco)

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

// extern
extern ofstream f;

/* for use IES file data */
/**/
extern bool vUse_IES;

/* IES file path */
/**/
extern CString ies_file;

extern XSI::Application app;

extern std::string replace(string input);

extern CString findInGroup(CString name);

/* for math transform to 'light area' oobject */
/**/
CString luxsi_area_transf(X3DObject o, float size_X, float size_Y);

/* Search IBL and HDRI files used */
/**/
CString find_XSI_env(CString env_file);
