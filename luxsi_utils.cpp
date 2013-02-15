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

#include "include\luxsi_main.h"

using namespace XSI;
using namespace MATH;
using namespace std;

/**/
extern Application app;
/**/
extern Model root;
/**/
extern bool luxdebug;

/**/
extern double ftime;

/**/
extern bool vIsHiddenLight, vIsHiddenObj, vIsHiddenCam, vIsHiddenClouds, vIsHiddenSurface;


//-
bool is_visible(X3DObject o, CString objType)
{
    //- vIsHiddenLight = exporta objetos que hayamos ocultado en el visor
    /* una parte controla las opciones del menu.
    *  La otra, que el objeto lo hayamos ocultado en el display.
    */  
    bool itemHidden;
    if ( objType == L"light")       itemHidden = vIsHiddenLight;
    if ( objType == L"polymsh")     itemHidden = vIsHiddenObj;
    if ( objType == L"camera")      itemHidden = vIsHiddenCam;
    if ( objType == L"pointcloud")  itemHidden = vIsHiddenClouds;
    if ( objType == L"surfmsh")     itemHidden = vIsHiddenSurface;   
    //-
    Property visi = o.GetProperties().GetItem(L"Visibility");
    bool view_visbl = (bool)visi.GetParameterValue(L"viewvis");
    bool rend_visbl = (bool)visi.GetParameterValue(L"rendvis");
    
    /*
    * add visible objects and not visibles if 'vIsHidden..' is True.
    */
    return ( itemHidden || ( !itemHidden && ( view_visbl && rend_visbl )));   
}
//-
CString find_texlayer(Shader s)
{
    CString
        textureLayerName = L"",     //- texturelayer name
        textureLayerPortName = L""; //- texturelayerports name
    
    //--
    CRefArray texLayerArray(s.GetTextureLayers());
    //->
    for ( long i = 0; i < texLayerArray.GetCount(); i++)
    {
        //->
        TextureLayer texLayer(texLayerArray[i]);
        textureLayerName = texLayer.GetName();
        //-
        if ( luxdebug ) app.LogMessage(L"TextureLayer name in use: "+ textureLayerName);
        //->
        if (textureLayerName != L"")
        {
            //->
            CRefArray t_layerPort(texLayer.GetTextureLayerPorts());
            for (long j = 0; j < t_layerPort.GetCount(); j++)
            {
                //->
                TextureLayerPort tlport(t_layerPort[j]);
                textureLayerPortName = tlport.GetName();
                //-
                if ( luxdebug ) app.LogMessage(L"TexLayerPort name in use: "+ textureLayerPortName);
            }
        }
        return textureLayerPortName;
    }
    return textureLayerName;
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
bool luxsi_find(CStringArray in_array, CString in_string)
{
    //
    // Returns true if String is in Array
    //
    for (int i = 0; i < in_array.GetCount(); i++)
    {
        if (in_array[i] == in_string)
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

        //if (luxdebug) app.LogMessage(L" Obj name is: "+ objname);

        //--
        if ( objname == used_shader )
        {
            //--
            CRefArray parent_needobj = obj.GetNestedObjects();
            for (long j = 0; j < parent_needobj.GetCount(); j++)
            {
                //--
                SIObject parent_obj = parent_needobj[j];
                used_shader = parent_obj.GetName();
                //-
                //if (luxdebug) app.LogMessage(L"Texture name: "+ used_shader);
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

//-
CString luxsi_normalize_path(CString vFile)
{
    //-- normalize path name
    CString normalized_path = luxsi_replace(vFile.GetAsciiString()).c_str();

    //- extract folder base for use 'relative path'
    int base = int(normalized_path.ReverseFindString("\\\\")); // .ext
    CString folder_base = normalized_path.GetSubString(0, base+2);

    //- extract filename
    CString file_path = normalized_path.GetSubString(base+2, normalized_path.Length());
    // file path is ==  file.ext
    //-
    if ( luxdebug )
    {
        app.LogMessage(L"Path normalized is: "+ folder_base + L" File name is: "+ file_path);
    }
    //- extract extension
    int ext = int(file_path.ReverseFindString("."));

    //- return only filename, without extension
    return file_path.GetSubString(0, ext);

}
//-
CString luxsiTransformMatrix(X3DObject o, CString pivotReference)
{
    /** 
    *   Status:
    *   Work in progress
    */
    /**
        Use objecState variable for expand function.\n
        Now, I can change to local or global pivot for transform.\n
        @param o class XSI::X3DObject
        @param pivotReference Global or Local reference for transform pivot

        \code
        KinematicState objectState;
        // by default, use global pivot
        objectState = o.GetKinematics().GetGlobal();
        if (pivotReference == L"local") objectState = o.GetKinematics().GetLocal();

        CTransformation goTransform = objectState.GetTransform(ftime);
        \endcode
    */
    CString transfData;
    KinematicState objectState;
    //-
    objectState = o.GetKinematics().GetGlobal();
    if (pivotReference == L"local") objectState = o.GetKinematics().GetLocal();

    CTransformation goTransform = objectState.GetTransform(ftime);

    //-
    CMatrix4 mTransf = goTransform.GetMatrix4();
    /**/
    transfData = CString(mTransf.GetValue(0,0)) + L" " // 0,0 -> scale X
        + CString(-mTransf.GetValue(0,2))    + L" "
        + CString(mTransf.GetValue(0,1))     + L" " // 0,2 -> swith YZ rot ( negative? )
        + CString(mTransf.GetValue(0,3))     + L" "
        + CString(-mTransf.GetValue(2,0))    + L" " // 1,0 -> swith YZ rot ( signe? )
        + CString(mTransf.GetValue(2,2))     + L" "  // 1,1 -> scale Y ( negative? )
        + CString(mTransf.GetValue(1,2))     + L" "
        + CString(mTransf.GetValue(1,3))     + L" "
        + CString(mTransf.GetValue(1,0))     + L" " // 2,0 -> swith YZ rot ( signo? )
        + CString(mTransf.GetValue(2,1))     + L" "
        + CString(mTransf.GetValue(1,1))     + L" " // 2,2 -> scale Z
        + CString(mTransf.GetValue(2,3))     + L" "
        + CString(mTransf.GetValue(3,0))     + L" " // translation, fixed, no touch
        + CString(-mTransf.GetValue(3,2))    + L" " //
        + CString(mTransf.GetValue(3,1))     + L" " //
        + CString(mTransf.GetValue(3,3));
    
    //-
    return transfData;
}
//--
CString luxsiTransformClasic(X3DObject o, CString pivotReference)
{
    /*!
    *   Result from testing... \n
    *   The better order for transform is; \n 
    *   - Translate \n    
    *   - Rotate    \n
    *   - Scale     \n
    *   Using this method. Is more precise to matrix.. atm.
    */
    CString transformData;
    KinematicState objectState;

    /* Extend fuction for use in local and global transform.
    */
    objectState = o.GetKinematics().GetGlobal();
    if (pivotReference == L"local") objectState = o.GetKinematics().GetLocal();
    //-
    CTransformation goTransform = objectState.GetTransform(ftime);
    //-
    transformData += L"Translate "
        + CString( goTransform.GetPosX()) + L" "
        + CString( -goTransform.GetPosZ()) + L" "
        + CString( goTransform.GetPosY()) + L"\n";

    //- rotation
    CVector3 axis;
    double rot = goTransform.GetRotationAxisAngle(axis);
    //-
    if (rot != 0)
    {
        transformData += L"Rotate "
            + CString( rot*180/PI ) + L" "
            + CString( axis[0] ) + L" "
            + CString( -axis[2] ) + L" "
            + CString( axis[1] ) + L"\n";
    }
    /** Scale.
    *   Add support for not uniform scale using "or". 
    */
    if (goTransform.GetSclX()!=1 || goTransform.GetSclY()!=1 || goTransform.GetSclZ()!=1)
    {
        /** WARNING! change 'Y' for 'Z', but not negative ( -z ).
        */
        transformData += L"Scale "
            + CString(goTransform.GetSclX()) + L" "
            + CString(goTransform.GetSclZ()) + L" "
            + CString(goTransform.GetSclY()) + L"\n";
    }
    return transformData;
}

//- material micro-functions
CString floatToString(Shader s, CString item, CString definition)
{
    CString strData = L"  \"float "+ item + L"\" ["+ CString(s.GetParameterValue(definition)) + L"] \n";
    //-
    return strData;
}
//-
CString floatToString(Shader s, CString item)
{
    CString strData = L"  \"float "+ item + L"\" ["+ CString(s.GetParameterValue(item)) + L"] \n";
    //-
    return strData;
}
//-
CString integerToString(Shader s, CString item, CString definition)
{
    CString strData = L"  \"integer "+ item + L"\" ["+ CString(s.GetParameterValue(definition)) + L"] \n";
    //-
    return strData;            
}
//-
CString integerToString(Shader s, CString item)
{
    CString strData = L"  \"integer "+ item + L"\" ["+ CString(s.GetParameterValue(item)) + L"] \n";
    //-
    return strData;            
}
float floatValue(Shader s, CString item)
{
    return float(s.GetParameterValue(item));
}
//-
CVector3 convertMatrix(CVector3 v)
{
    CMatrix3 m2(1.0,0.0,0.0, 0.0,0.0,1.0, 0.0,-1.0,0.0);
    return v.MulByMatrix3InPlace(m2);
}
