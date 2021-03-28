/*****************************************************************************
  Purpose:     On Atari TOS palette modes on the output devices are supported.
               Conversions between RGB and palette colors are required.
               In high color mode (xhandle is RGB) RGB colors can be set
               and drawing is performed always on the same palette index 
               (DC_INDEX).
  Author:      Harald Rieder
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdlib.h>
#include <graphic.h>

void rgb_fstyle_fcolor(VirtualDevice * handle, int style, int color)
{
    int rgb[3] ;
    style_and_int16_to_rgb_array(style, color, rgb) ;
    vs_color (handle, DC_INDEX, rgb) ;
    color = DC_INDEX ;
    style = IP_SOLID ;
	if (!style) handle->fillInterior = FIS_HOLLOW ;
	else        handle->fillInterior = FIS_PATTERN ;
	handle->fillStyle = style ;
	vsf_color(handle, color) ;
}


void rgb_udsty_lcolor(VirtualDevice *  handle, int udsty, int color, int back)
{
    int bits = 0, maxbits = 16 ;
    int red   = get_RED  (color) ;
    int green = get_GREEN(color) ;
    int blue  = get_BLUE (color) ;
    int bred   = get_RED  (back) ;
    int bgreen = get_GREEN(back) ;
    int bblue  = get_BLUE (back) ;
    /* draw mixed color but solid */
    while (udsty)
    {
        if (udsty & 1)
            bits++ ; 
        udsty >>= 1 ;
    }
    red   = bred   - (bred   - red  ) * bits / maxbits ;
    green = bgreen - (bgreen - green) * bits / maxbits ;
    blue  = bblue  - (bblue  - blue ) * bits / maxbits ;
    color = make_RGB(red, green, blue) ;
    
    handle->lineType = SOLID ;
	rgb_lcolor(handle, color) ;
}


void rgb_3d_ltype_lcolor(VirtualDevice *  handle, int style, int color)
{
	int user = 0 ;
    int rgb[3] ;
    /* color diff from light border to dark border 
       shall be twice the color diff from interior to dark border */
    /* IP_SOLID corresponds to the dark border */
    style = IP_SOLID - 2 * (IP_SOLID - style) ;
    if (style < 0)
        style = 0 ; 
    style_and_int16_to_rgb_array(style, color, rgb) ;
    vs_color (handle, DC_INDEX, rgb) ;
    color = DC_INDEX ;
    handle->lineUserdefStyle = 0xFFFF ;
    handle->lineType = USERLINE ;
	vsl_color(handle, color) ;
}


void int16_to_rgb_array(int compact_rgb, int rgb[3]) 
{
	rgb[0] = (int)((long)get_RED  (compact_rgb) * 1000 / 0x1F) ;
	rgb[1] = (int)((long)get_GREEN(compact_rgb) * 1000 / 0x1F) ;
	rgb[2] = (int)((long)get_BLUE (compact_rgb) * 1000 / 0x1F) ;
}


void style_and_int16_to_rgb_array(int style, int compact_rgb, int rgb[3])
{
	int i, full = 1000 ;
	int16_to_rgb_array(compact_rgb, rgb) ;
	for (i = 0 ; i < 3 ; i++)
		rgb[i] = rgb[i] + (IP_SOLID - style) * (full - rgb[i]) / IP_SOLID ; 
}

void rgb_fcolor(VirtualDevice * handle, int color)
{
    int rgb[3] ;
    int16_to_rgb_array(color, rgb) ;
    vs_color (handle, DC_INDEX, rgb) ;
    color = DC_INDEX ;
	vsf_color(handle, color) ;
}


void rgb_lcolor(VirtualDevice * handle, int color)
{
    int rgb[3] ;
    int16_to_rgb_array(color, rgb) ;
    vs_color (handle, DC_INDEX, rgb) ;
    color = DC_INDEX ;
	vsl_color(handle, color) ;
}


void rgb_tcolor(VirtualDevice * handle, int color)
{
    int rgb[3] ;
    int16_to_rgb_array(color, rgb) ;
    vs_color (handle, DC_INDEX, rgb) ;
	handle->textColor = DC_INDEX ;
}
