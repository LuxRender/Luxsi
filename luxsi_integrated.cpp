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

//--
#pragma warning (disable : 4244)
//--
#include "include\luxsi_main.h"

//--
using namespace XSI;
using namespace MATH;

// Rendering Engine Section

// Abort handling.

static bool   g_bAborted;
HANDLE           g_hAbort;
CRITICAL_SECTION g_barrierAbort;

void setAbort( bool in_bAbort )
{
   ::EnterCriticalSection( &g_barrierAbort );
   g_bAborted = in_bAbort;
   if( in_bAbort )
       ::SetEvent( g_hAbort );
   else
       ::ResetEvent( g_hAbort );
   ::LeaveCriticalSection( &g_barrierAbort );
}

//--
bool isAborted( )
{ 
   bool   bAbort;
   ::EnterCriticalSection( &g_barrierAbort );
   bAbort= g_bAborted;
   ::LeaveCriticalSection( &g_barrierAbort );

   return( bAbort );
}
//---- 5
//
//   Función de inicialización para el renderizador, llamado cuando el
//    plug-in se ha cargado.
//
//    Aquí es donde el motor de renderizado le dice al environment que
//    tipos de procesos puede realizar (renderizar, exportar archivos, etc),
//    que  propiedad va a utilizar para sus opciones y que  formatos de salida
//    apoya (y cómo se definen los formatos).

//    El procesador puede realizar cualquier otra inicialización de un tiempo aquí también.

XSIPLUGINCALLBACK CStatus LuxRenderRenderer_Init( CRef &in_ctxt )
{
   //------------------------------
   Context ctxt( in_ctxt );
   Renderer renderer = ctxt.GetSource();
	
   // Tell the render manager what render processes we support.
   CLongArray process;
   process.Add( siRenderProcessRender );
   renderer.PutProcessTypes( process );

   // Specify the custom property to use for the renderer options
   renderer.AddProperty( siRenderPropertyOptions, L"LuxRender Renderer.LuxRender Renderer" );

   // Add the Softimage PIC format as an output format.
   renderer.AddOutputImageFormat( L"Softimage PIC", L"pic" );
   renderer.AddOutputImageFormatSubType( siRenderChannelColorType, L"RGBA", siImageBitDepthInteger8 );

   // And some arbitrary image format.
   renderer.AddOutputImageFormat( L"Foo Format", L"foo" );
   renderer.AddOutputImageFormatSubType( siRenderChannelColorType, L"RGBA", siImageBitDepthInteger8 );
   renderer.AddOutputImageFormatSubType( siRenderChannelColorType, L"RGBA", siImageBitDepthInteger16 );
   renderer.AddOutputImageFormatSubType( siRenderChannelColorType, L"RGB", siImageBitDepthInteger8 );
   renderer.AddOutputImageFormatSubType( siRenderChannelGrayscaleType, L"Gray", siImageBitDepthInteger16 );

   // Create the handles for a thread-safe abort
   g_bAborted = false;
   ::InitializeCriticalSection( &g_barrierAbort );
   g_hAbort = ::CreateEvent( NULL, FALSE, FALSE, NULL );

   return( CStatus::OK );
}


//   Esto se llama cuando el plug-in se descarga. El motor de renderizado debe cerrar por completo y
//   limpiar los datos globales. Cualquier trabajo de representación con este motor
//   ya se ha terminado en este punto.


XSIPLUGINCALLBACK CStatus LuxRenderRenderer_Term( CRef &in_ctxt )
{
   ::DeleteObject( g_hAbort );
   ::DeleteCriticalSection( &g_barrierAbort );

   g_hAbort = NULL;
   ::ZeroMemory( &g_barrierAbort, sizeof( g_barrierAbort ) );

   return( CStatus::OK );
}

class MyFragment : public RendererImageFragment
{
public:
   MyFragment(
       unsigned int in_offX, unsigned int in_offY, unsigned int in_width, unsigned int in_height,
       double in_color[ 4 ] )
   {
       offX = in_offX;
       offY = in_offY;
       width = in_width;
       height = in_height;

       unsigned int r, g, b, a;

       r = (unsigned int)( in_color[ 0 ] * 255.0 );
       g = (unsigned int)( in_color[ 1 ] * 255.0 );
       b = (unsigned int)( in_color[ 2 ] * 255.0 );
       a = (unsigned int)( in_color[ 3 ] * 255.0 );

       color = ( a << 24 ) | ( b << 16 ) | ( g << 8 ) | ( r );
   }

   unsigned int GetOffsetX( ) const { return( offX ); }
   unsigned int GetOffsetY( ) const { return( offY ); }
   unsigned int GetWidth( ) const { return( width ); }
   unsigned int GetHeight( ) const { return( height ); }
   bool GetScanlineRGBA( unsigned int in_uiRow, siImageBitDepth in_eBitDepth, unsigned char *out_pScanline ) const
   {
       unsigned int *pScanline = (unsigned int *)out_pScanline;
       for( unsigned int i = 0; i < width; i++ )
          pScanline[ i ] = color;

       return( true );
   }

private:
   unsigned int offX, offY, width, height;
   unsigned int color;
};
//---- 4
//   ====================>
//   Esta es la función principal que es llamada por el gestor de render
//    cada vez que el motor de renderizado es solicitado para realizar un proceso
//    (renderizar un frame, exportar un archivo, etc.)

//    Se le llama con un objeto de contexto especializado, llamado RendererContext.
//   El RendererContext permite recuperar la propiedad de procesador de opciones,
//    obtener información de uso de este dispositivo y el envío de datos del azulejo de nuevo al render
//    manager.

XSIPLUGINCALLBACK CStatus LuxRenderRenderer_Process( CRef &in_ctxt )
{
   setAbort( false );

   RendererContext ctxt( in_ctxt );
   Renderer renderer = ctxt.GetSource();
 
	// El método * * LockSceneData debe ser llamado antes de acceder a cualquier potencial
	// datos de la escena. Esto es para asegurar que varios subprocesos no al mismo tiempo el acceso
    // y / o modificar los datos de la escena. También es importante que el procesador de * no *
    // modificar los datos de la escena en absoluto. Puede modificar sus propios datos privados, pero nada
    // que es una parte de la escena o el estado de aplicación actual, a menos que explícitamente
    // permitido.
   
   if( renderer.LockSceneData() != CStatus::OK )
       return( CStatus::Abort );

   Primitive  camera_prim = ctxt.GetAttribute( L"Camera" );
   X3DObject  camera_obj = camera_prim.GetOwners( )[ 0 ];
   Camera        camera  = camera_obj;
   CString       camera_name = camera_obj.GetName();
   const wchar_t *wcsCameraName = camera_name.GetWideString();

   // Get the size of the image to render (in pixels). The origin is defiend as the
   // bottom-left corner of the image.
   unsigned int width, height;
   width = (ULONG)ctxt.GetAttribute( L"ImageWidth" );
   height = (ULONG)ctxt.GetAttribute( L"ImageHeight" );

	// Comprobar si hay un área de recorte definida. Si el desplazamiento es 0,0 y el crop
    // ancho / alto es el mismo que el ancho alto de la imagen, entonces no debería tener crops
    // lugar. La ventana crop es siempre completamente dentro de la imagen representada.

   unsigned int cropOffsetX, cropOffsetY;
   unsigned int cropWidth, cropHeight;

   cropOffsetX = (ULONG)ctxt.GetAttribute( L"CropLeft" );
   cropOffsetY = (ULONG)ctxt.GetAttribute( L"CropBottom" );
   cropWidth = (ULONG)ctxt.GetAttribute( L"CropWidth" );
   cropHeight = (ULONG)ctxt.GetAttribute( L"CropHeight" );

   // Obtener nuestra propiedad hacen evaluado en el momento correcto. Si los campos de representación, cualquier
    // parámetro que está animado, tiene que ser evaluada en la media-marco entre el
    // marco actual y el siguiente fotograma después. Lo mismo ocurre potencialmente para el desenfoque de movimiento, a menos que
    // el procesador no es capaz de interpolar los datos, en cuyo caso se debe utilizar
    // el marco actual como base.

   CTime  evalTime = ctxt.GetTime();

   Property myProp = ctxt.GetRendererProperty( evalTime );

   double     color[ 4 ];

   color[ 0 ] = myProp.GetParameterValue( L"Color_R", evalTime );
   color[ 1 ] = myProp.GetParameterValue( L"Color_G", evalTime );
   color[ 2 ] = myProp.GetParameterValue( L"Color_B", evalTime );
   color[ 3 ] = myProp.GetParameterValue( L"Color_A", evalTime );

   // Unlock the scene data *before* we start rendering and sending tile data back.
   renderer.UnlockSceneData();

   // Check after the scene data has been evaluted whether the abort flag is set.
   if( isAborted() )
       return( CStatus::Abort );

   // Notify the renderer manager that a new frame is about to begin. This is necessary so
   // that any recipient tile sink can re-adjust its own size to accommodate.
   
   ctxt.NewFrame( width, height );

   unsigned int  tileSize = 32;

   for( unsigned y = 0; y <= ( cropHeight / tileSize ); y++ )
   {
       for( unsigned x = 0; x <= ( cropWidth / tileSize ); x++ )
       {
          unsigned int ox, oy, sx, sy;

          ox = x * tileSize;
          oy = y * tileSize;
          sx = tileSize;
          sy = tileSize;

          if( ( ox + tileSize ) > cropWidth )
              sx = width - ox;
          else
              sx = tileSize;

          if( ( oy + tileSize ) > cropHeight )
              sy = height - oy;
          else
              sy = tileSize;

          MyFragment fragment(
              ox + cropOffsetX, oy + cropOffsetY, sx, sy, color );

          // Send back a new tile.
          ctxt.NewFragment( fragment );

          DWORD  dwResult = ::WaitForSingleObject( g_hAbort, 40 );
          if( dwResult != WAIT_TIMEOUT )
              return( CStatus::Abort );
       }
   }

   return( CStatus::OK );
}

 
//    Llamado por el administrador de render cuando el procesador debe realizar una completa
//    limpieza de los datos que se creó por la función del proceso.
//    Esto se llama normalmente cuando la escena actual está siendo destruido,
//    o si el proceso de render específico (región, render pass, exportación)
//    peticiones que los datos se limpian después de que el proceso ha terminado.

//---- 3

XSIPLUGINCALLBACK CStatus LuxRenderRenderer_Cleanup( CRef &in_ctxt )
{
   Context ctxt( in_ctxt );
   Renderer renderer = ctxt.GetSource();

   return( CStatus::OK );
}

XSIPLUGINCALLBACK CStatus LuxRenderRenderer_Abort( CRef &in_ctxt )
{
   Context ctxt( in_ctxt );
   Renderer renderer = ctxt.GetSource();

   setAbort( true );

   return( CStatus::OK );
}
