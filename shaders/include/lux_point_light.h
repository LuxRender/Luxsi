// C Header File Generated by Softimage Shader Wizard

#ifndef LUX_POINT_LIGHT_H
#define LUX_POINT_LIGHT_H

#include <shader.h>

////////////////////////////////////////////////////////////////
// Type definition

typedef struct
{
	miColor				color;				// color
    miScalar            gain;
    miScalar            importance;
    miBoolean           flipz;
    miScalar            power;
} lux_point_light_t;

#endif // LUX_POINT_LIGHT_H
