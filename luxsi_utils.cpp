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


#include "include/luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/*
*/
extern Application app;
/*
*/
extern Model root;
/*
*/
extern bool luxdebug;


//-
CString find_texlayer(Shader s)
{
    //- texturelayer name
    CString tl_name = L"";
    //- texturelayerports name
    CString tlportName = L"";

    //--
    CRefArray texlayer(s.GetTextureLayers());
    //->
    for ( long i = 0; i < texlayer.GetCount(); i++)
    {
        //->
        TextureLayer t_layer(texlayer[i]);
        tl_name = t_layer.GetName();
        //-
        if ( luxdebug )
        {
            app.LogMessage(L"TextureLayer name in use: "+ tl_name);
        }
        //->
        if (tl_name != L"")
        {
            //->
            CRefArray t_layerPort(t_layer.GetTextureLayerPorts());
            for (long j = 0; j < t_layerPort.GetCount(); j++)
            {
                //->
                TextureLayerPort tlport(t_layerPort[j]);
                tlportName = tlport.GetName();
                //-
                if ( luxdebug ) app.LogMessage(L"TexLayerPort name in use: "+ tlportName);
            }
        }
        return tlportName;
    }
    return tl_name;
}
//--
CString findInGroup(CString s)
{
    //--
    CRefArray grps = root.GetGroups();
    //--
    for (int i=0; i<grps.GetCount(); i++)
    {
        CRefArray a=Group(grps[i]).GetMembers();
        for (int j=0; j<a.GetCount(); j++)
        {
            if (X3DObject(a[j]).GetName()==s)
            {
                //app.LogMessage(L"Group: " + Group(grps[i]).GetName() + L"Childname: "+X3DObject(a[j]).GetName());
                return Group(grps[i]).GetName();
            }
        }
    }
    return L"";
}
//--
string luxsi_replace(string input)
{
    //-
    int len = input.length();
    //->
    for (int i=0; i<len; i++)
    {
        if (input[i]=='\\' || input[i]=='/')
        {
            input.replace(i, 1, "\\\\");
            i++;
        }
    }
    return input;
}
//--
bool luxsi_find(CStringArray a, CString s)
{
    //
    // Returns true if String s is in Array a
    //
    for (int i = 0; i < a.GetCount(); i++)
    {
        if (a[i]==s)
        {
            return true;
        }
    }
    return false;
}
//--
Texture find_tex_used(Shader s, CString tname)
{
    Texture tex_shader;
    //--
    CRefArray t_shader(s.GetShaders());
    //--
    for ( long ts = 0; ts < t_shader.GetCount(); ts++)
    {
        if ( Shader(t_shader[ts]).GetName()== tname)
        {
            tex_shader = t_shader[ts];
        }
    }
    return tex_shader;
}
//-
CString find_shader_used( Shader s, CString used_shader)
{
    //-- search texture connect to port shader in used
    CRefArray needobj = s.GetNestedObjects();
    //--
    for ( long i = 0; i < needobj.GetCount(); i++)
    {
        //--
        SIObject obj = needobj[i];
        //--
        CString objname = obj.GetName();

        if (luxdebug) app.LogMessage(L" Obj name is: "+ objname);

        //--
        if ( objname == used_shader )
        {
            //--
            CRefArray parent_needobj = obj.GetNestedObjects();
            for (long j = 0; j < parent_needobj.GetCount(); j++)
            {
                //--
                SIObject parent_obj = parent_needobj[j];
                used_shader = parent_obj.GetName(); // GetAsciiString() ??
                //-
                if (luxdebug) app.LogMessage(L"Texture name: "+ used_shader);
            }
        }
    }
    return used_shader;
}
//--
bool find_crefarray_object( CRefArray in_ref, CString in_name )
{
    //- test helper for find object in CRefArray

    for(long i=0; i < in_ref.GetCount(); i++ )
    {
        if( SIObject(in_ref[i]).GetName() == in_name )
        {
            return true;
        }
    }
    return false;
}
