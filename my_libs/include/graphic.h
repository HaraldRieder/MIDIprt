/*****************************************************************************
  Purpose:     On Atari TOS palette modes on the output devices are supported.
               Conversions between RGB and palette colors are required.
               In high color mode (xhandle is RGB) RGB colors can be set
               and drawing is performed always on the same palette index 
               (DC_INDEX).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: GRAPHIC.H,v 1.8 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef GRAPHIC_H
#define GRAPHIC_H 

/* paper formats in micrometers */
#define DIN_A4_WIDTH     210000.0f
#define DIN_A4_HEIGHT    296984.8f	/* = width * sqrt(2) */
#define US_LETTER_WIDTH  216000.0f
#define US_LETTER_HEIGHT 279000.0f

/* 1 point = 1/72 inch, 1 inch = 25400 micrometers */
#define MICROMETER_PER_POINT   353 
#define MICROMETER_PER_INCH  25400

#include "my_vdi.h"
//typedef short INT16 ;
typedef void * X_HANDLE ; // always RGB, extended-bit not used on other platforms

/* Win32 RGB macros */
#ifndef RGB
#define RGB(r, g ,b)  ((unsigned int) (((unsigned char) (r) | ((unsigned short) (g) << 8)) | (((unsigned int) (unsigned char) (b)) << 16))) 
#define GetRValue(rgb)   ((unsigned char) (rgb))
#define GetGValue(rgb)   ((unsigned char) ((rgb) >> 8)) 
#define GetBValue(rgb)   ((unsigned char) ((rgb) >> 16)) 
#endif

/* on wxWindows we always use RGBs */
//#define is_rgb_handle(xhandle) 1

void rgb_fcolor(X_HANDLE handle, int color) ;	/* fill color */
void rgb_lcolor(X_HANDLE handle, int color) ;	/* line color */
void rgb_tcolor(X_HANDLE handle, int color) ;	/* text color */
	/* call vsf_color() / vsl_color() / vst_color() */
	/* palette mode: color is interpreted as index */
	/* direct color: color is interpreted as 15-bit RGB,
	   drawing is done with DC_INDEX, vs_color() is called */
void rgb_udsty_lcolor(X_HANDLE handle, int udsty, int color, int back_color) ;
	/* palette mode: vsl_udsty() is used to simulate mixed color */
	/* direct color: color is interpreted as RGB and the red, green
	   and blue components are mixed with the background color separately */
	/* back_color is needed only in DC mode, in palette mode MD_TRANS
	   will "mix" the line color with the background */
	/* in MD_REPLACE mode back_color should be 0x7fff (white) */
void rgb_fstyle_fcolor(X_HANDLE handle, int style, int color) ;
	/* calls vsf_color(), vsf_interior() and vsf_style() */
	/* palette mode: vsf_style() is used to simulate whiter color */
	/* direct color: color is interpreted as RGB and the red, green
	   and blue components are lightened separately */
void rgb_3d_ltype_lcolor(X_HANDLE handle, int style, int color) ;
	/* calls vsl_color() and vsl_type() */
	/* calculates color for light 3D border line from dark border line (color)
	   and the fill style (which can mix white into the color) */
	/* palette mode: vsl_udsty() is used to simulate whiter color */
	/* direct color: color is interpreted as RGB and the red, green
	   and blue components are lightened separately */
void int16_to_rgb_array(int compact_rgb, int rgb[3]) ;
	/* accepts a 15-bit RGB color and splits it into the rgb array */
	/* red[0], green[1], blue[2] output values reach from 0..1000 */
void style_and_int16_to_rgb_array(int style, int compact_rgb, int rgb[3]) ;
	/* accepts a 15-bit RGB color and splits it into the rgb array */
	/* then it applies style to mix white color into the rgb array */
	/* style goes from 0 .. IP_SOLID */
	/* red[0], green[1], blue[2] output values reach from 0..1000 */
void copy_color_palette(
	VDI_HANDLE from_handle, int from_ncolors, 
	VDI_HANDLE to_handle  , int to_ncolors  ) ;
	/* copy color palette of one workstation to another */
	/* only the minimum of the number of colors from both workstations is copied */


/* 5 bits red, 5 bits green, 5 bits blue */
/* least significant is red */

#define RGB_RED   0x001F
#define RGB_GREEN 0x03E0
#define RGB_BLUE  0x7C00

/* x should be INT16 */
#define get_RED(x)   (x & RGB_RED  )
#define get_GREEN(x)((x & RGB_GREEN) >> 5)
#define get_BLUE(x) ((x & RGB_BLUE ) >> 10)
#define make_RGB(red, green, blue) (red | (green << 5) | (blue << 10))

/* VDI delivers 0..1000, I use 5 bit (0..255) color intensities */ 
/* +500 guarantees correct rounding */
#define thousand_to_5bit(x) ( (((int)(x) * 0x1f + 500)/1000) )

/* non-zero in direct color mode: */
//#define direct_color 1 // other platforms must have at least high color
#define DC_INDEX 255   // always 256 palette colors in emulation mode

/* max palette handled by RGB table */
#define MAX_COLORS 256

/* must be called before index_to_rgb and rgb_to_index! */
void init_rgb_table (VDI_HANDLE handle) ; /* normally the screen handle */

/* 15-bit (5+5+5) rgb value to VDI color index */
int rgb_to_index (int rgb) ;

/* VDI color index to 15-bit (5+5+5) rgb value */
int index_to_rgb (int index) ;

#endif /* include blocker */
