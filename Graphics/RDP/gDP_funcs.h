#ifndef _GDP_FUNCS_H
#define _GDP_FUNCS_H

#include <stdint.h>

enum gdp_plugin_type
{
   GDP_PLUGIN_GLIDE64 = 0,
   GDP_PLUGIN_GLN64
};

#define gDPSetScissor(mode, ulx, uly, lrx, lry) GDPSetScissor(GDP_PLUGIN_GLIDE64, mode, ulx, uly, lrx, lry)

void GDPSetScissor(enum gdp_plugin_type plug_type, uint32_t mode, float ulx, float uly, float lrx, float lry );

/* Glide64 prototypes */
extern "C" void glide64gDPSetScissor( uint32_t mode, float ulx, float uly, float lrx, float lry );

/* GLN64 prototypes */
#ifdef GLIDEN64
void gln64gDPSetScissor( uint32_t mode, float ulx, float uly, float lrx, float lry );
#else
extern "C" void gln64gDPSetScissor( uint32_t mode, float ulx, float uly, float lrx, float lry );
#endif

#endif