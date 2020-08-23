/*****************************************************************************
  Purpose:     On Atari TOS palette modes on the output devices are supported.
               Conversions between RGB and palette colors are required.
               In high color mode (xhandle is RGB) RGB colors can be set
               and drawing is performed always on the same palette index 
               (DC_INDEX).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: GRAPHIC.C,v 1.5 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdlib.h>
#ifdef __PUREC__
#  include <acs.h>
#else
# ifndef min 
#  define min __min
#  define max __max
# endif
#endif
#include <graphic.h>

void rgb_fstyle_fcolor(X_HANDLE xhandle, int style, int color)
{
	VDI_HANDLE handle = (VDI_HANDLE)xhandle ;
	if ( is_rgb_handle(xhandle) )
	{
		int rgb[3] ;
		style_and_int16_to_rgb_array(style, color, rgb) ;
		vs_color (handle, DC_INDEX, rgb) ;
		color = DC_INDEX ;
		style = IP_SOLID ;
	}
	if (!style) vsf_interior(handle, FIS_HOLLOW) ;
	else        vsf_interior(handle, FIS_PATTERN) ;
	vsf_style(handle, style) ;
	vsf_color(handle, color) ;
}


void rgb_udsty_lcolor(X_HANDLE xhandle, int udsty, int color, int back)
{
	VDI_HANDLE handle = (VDI_HANDLE)xhandle ;
	if ( is_rgb_handle(xhandle) )
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
		
		vsl_type (handle, SOLID) ;
	}
	else
	{
		vsl_udsty(handle, udsty) ;
		vsl_type (handle, USERLINE) ;
	}
	rgb_lcolor(xhandle, color) ;
}


void rgb_3d_ltype_lcolor(X_HANDLE xhandle, int style, int color)
{
	VDI_HANDLE handle = (VDI_HANDLE)xhandle ;
	int user = 0 ;
	if ( is_rgb_handle(xhandle) )
	{
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
		user  = 0xFFFF ;
	}
	else switch (style)
	{
	case 8: user = 0xFFFF ; break ;
	case 7: user = 0xEEEE ; break ;
	case 6: user = 0x5555 ; break ;
	case 5: user = 0x4444 ; break ;
	}
	vsl_udsty(handle, user) ;
	vsl_type (handle, USERLINE) ;
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

void rgb_fcolor(X_HANDLE xhandle, int color)
{
	VDI_HANDLE handle = (VDI_HANDLE)xhandle ;
	if ( is_rgb_handle(xhandle) )
	{
		int rgb[3] ;
		int16_to_rgb_array(color, rgb) ;
		vs_color (handle, DC_INDEX, rgb) ;
		color = DC_INDEX ;
	}
	vsf_color(handle, color) ;
}


void rgb_lcolor(X_HANDLE xhandle, int color)
{
	VDI_HANDLE handle = (VDI_HANDLE)xhandle ;
	if ( is_rgb_handle(xhandle) )
	{
		int rgb[3] ;
		int16_to_rgb_array(color, rgb) ;
		vs_color (handle, DC_INDEX, rgb) ;
		color = DC_INDEX ;
	}
	vsl_color(handle, color) ;
}


void rgb_tcolor(X_HANDLE xhandle, int color)
{
	VDI_HANDLE handle = (VDI_HANDLE)xhandle ;
	if ( is_rgb_handle(xhandle) )
	{
		int rgb[3] ;
		int16_to_rgb_array(color, rgb) ;
		vs_color (handle, DC_INDEX, rgb) ;
		color = DC_INDEX ;
	}
	vst_color(handle, color) ;
}


#if defined (__PUREC__)

void copy_color_palette(
	VDI_HANDLE from_handle, INT16 from_ncolors, 
	VDI_HANDLE to_handle  , INT16 to_ncolors  ) 
{
	INT16 i, ncolors = min(from_ncolors, to_ncolors) ;
	for (i = 0 ; i < ncolors ; i++)
	{
		int rgb[3] ;
		vq_color(from_handle, i, 0, rgb) ;
		vs_color(to_handle  , i, rgb) ;
	}
}

/******** auxiliary procedures *********/

void vs_clip_from_parmblk(VDI_HANDLE handle, const PARMBLK* parmblk)
{
	int points[4] ;
	
	points[0] = parmblk->pb_xc ;	
	points[1] = parmblk->pb_yc ;	
	points[2] = parmblk->pb_xc + parmblk->pb_wc - 1 ;
	points[3] = parmblk->pb_yc + parmblk->pb_hc - 1 ;	
	
	vs_clip(handle, TRUE, points) ;

	/* ACS 3 VDI library Axywh ????: */
/*	Axywh clip ;
	clip.x = parmblk->pb_xc ;
	clip.y = parmblk->pb_yc ;
	clip.w = parmblk->pb_wc ;
	clip.h = parmblk->pb_hc ;
	
	vs_clip(handle, TRUE, &clip) ;*/
}

void te_color(OBJECT *obj, int color)
{
	if (obj)
	{
		obj->ob_spec.tedinfo->te_color &= 0xF0FF ;
		obj->ob_spec.tedinfo->te_color |= (color & 0xF) << 8 ;
	}
}


/*** the RGB table ***/

static int rgb_colors[MAX_COLORS] ;

void init_rgb_table (VDI_HANDLE handle)
{
	INT16 i, rgb[3] ;
	
	for (i = 0 ; i < MAX_COLORS && i < ACSblk->ncolors ; i++)
	{
		vq_color(handle, i, 1, rgb);
		rgb_colors[i] = 
			 thousand_to_5bit(rgb[0])      +
			(thousand_to_5bit(rgb[1])<<5 ) +
			(thousand_to_5bit(rgb[2])<<10) ;
	}
}

int rgb_to_index (int rgb)
{
	int i, diff, best_i = 0, smallest_diff = 0x7FFF ;
	
	for (i = 0 ; i < MAX_COLORS && i < ACSblk->ncolors ; i++)
	{
		diff = 
			 abs((rgb_colors[i] & RGB_RED  ) - (rgb & RGB_RED  )) +
			(abs((rgb_colors[i] & RGB_GREEN) - (rgb & RGB_GREEN))>>5) +
			(abs((rgb_colors[i] & RGB_BLUE ) - (rgb & RGB_BLUE ))>>10) ;
		if (diff < smallest_diff)
		{
			best_i = i ; 
			smallest_diff = diff ;
		}
	}
	return best_i ;
}

int index_to_rgb (int index)
{
	if (index > ACSblk->ncolors - 1 ||
	    index > MAX_COLORS - 1)
		index = BLACK ;
	return rgb_colors[index] ;
}
#endif /* __PUREC__ */
