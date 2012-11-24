// C++ Source Code Generated by Softimage Shader Wizard

////////////////////////////////////////////////////////////////
// Includes

#include <shader.h>
#include "include\lux_shinymetal.h"

////////////////////////////////////////////////////////////////
// Implementation

extern "C" DLLEXPORT miBoolean
lux_shinymetal
(
	miColor				*result,
	miState				*state,
	lux_shinymetal_t			*params
	)
{
	// TODO: Shader main code goes here
	*result = *mi_eval_color(&params->kr);
	*result = *mi_eval_color(&params->ks);
	return( miTRUE );
}


extern "C" DLLEXPORT void
lux_shinymetal_init
(
	miState				*state,
	lux_shinymetal_t			*params,
	miBoolean			*inst_init_req
	)
{
	if( params == NULL )
	{
		// TODO: Shader global initialization code goes here (if needed)
		
		// Request a per-instance shader initialization as well (set to miFALSE if not needed)
		*inst_init_req = miTRUE;
	}
	else
	{
		// TODO: Shader instance-specific initialization code goes here (if needed)
	}
}


extern "C" DLLEXPORT void
lux_shinymetal_exit
(
	miState				*state,
	lux_shinymetal_t			*params
	)
{
	if( params == NULL )
	{
		// TODO: Shader global cleanup code goes here (if needed)
	}
	else
	{
		// TODO: Shader instance-specific cleanup code goes here (if needed)
	}
}


extern "C" DLLEXPORT int
lux_shinymetal_version( )
{
	return( 1 );
}
