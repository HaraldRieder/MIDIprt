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
#include <cstdint>

/* paper formats in micrometers */
#define DIN_A4_WIDTH     210000.0f
#define DIN_A4_HEIGHT    296984.8f	/* = width * sqrt(2) */
#define US_LETTER_WIDTH  216000.0f
#define US_LETTER_HEIGHT 279000.0f

/* 1 point = 1/72 inch, 1 inch = 25400 micrometers */
#define MICROMETER_PER_POINT   353 
#define MICROMETER_PER_INCH  25400


#include "wxVDI.h"

/* Win32 RGB macros */
#ifndef RGB
#define RGB(r, g ,b)  ((unsigned int) (((unsigned char) (r) | ((unsigned short) (g) << 8)) | (((unsigned int) (unsigned char) (b)) << 16))) 
#define GetRValue(rgb)   ((unsigned char) (rgb))
#define GetGValue(rgb)   ((unsigned char) ((rgb) >> 8)) 
#define GetBValue(rgb)   ((unsigned char) ((rgb) >> 16)) 
#endif

void rgb_fcolor(VirtualDevice * handle, int color) ;	/* fill color */
void rgb_lcolor(VirtualDevice * handle, int color) ;	/* line color */
void rgb_tcolor(VirtualDevice * handle, int color) ;	/* text color */
	/* call vsf_color() / vsl_color() / vst_color() */
	/* palette mode: color is interpreted as index */
	/* direct color: color is interpreted as 15-bit RGB,
	   drawing is done with DC_INDEX, vs_color() is called */
void rgb_udsty_lcolor(VirtualDevice * handle, int udsty, int color, int back_color) ;
	/* palette mode: vsl_udsty() is used to simulate mixed color */
	/* direct color: color is interpreted as RGB and the red, green
	   and blue components are mixed with the background color separately */
	/* back_color is needed only in DC mode, in palette mode MD_TRANS
	   will "mix" the line color with the background */
	/* in MD_REPLACE mode back_color should be 0x7fff (white) */
void rgb_fstyle_fcolor(VirtualDevice * handle, int style, int color) ;
	/* palette mode: vsf_style() is used to simulate whiter color */
	/* direct color: color is interpreted as RGB and the red, green
	   and blue components are lightened separately */
void rgb_3d_ltype_lcolor(VirtualDevice * handle, int style, int color) ;
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
	VirtualDevice * from_handle, int from_ncolors, 
	VirtualDevice * to_handle  , int to_ncolors  ) ;
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
#define direct_color 1 // other platforms must have at least high color
#define DC_INDEX 255   // always 256 palette colors in emulation mode

void draw_background 
(
	VirtualDevice * handle, 	/* VDI workstation handle */
	int *points,	/* points[0]:     x-value of 1. corner
   					   points[1]:     y-value of 1. corner
	   				   points[2]:     x-value of 2. corner
		   			   points[3]:     y-value of 2. corner */
	int style, 		/* fill style */
	int color		/* fill	color */
) ;

void draw_note 
(
	VirtualDevice * handle, 	/* VDI workstation handle */
	int *points,	/* points[0]:     x-value of 1. corner
					   points[1]:     y-value of 1. corner
					   points[2]:     x-value of 2. corner
					   points[3]:     y-value of 2. corner */
	int type,       /* see below */
	int body_style, 	/* fill style body */
	int body_color,		/* fill	color body */
	int end_style, 		/* fill style ends */
	int end_color		/* fill	color ends */
) ;

/******** note type is a set of the following attributes *****/

#define HEAD_LINE      0x0001   /* note head marked with a line */
#define TAIL_LINE      0x0002   /* note tail marked with a line */
#define HEAD_TRI       0x0004   /* note head marked with a triangle */
#define TAIL_TRI       0x0008   /* note tail marked with a triangle */
#define BODY_RECT      0x0010   /* note body is a rectangle */
#define BODY_ELLIP     0x0020   /* note body is an ellipse */
#define HEAD_DOT       0x0040   /* note head marked with something round */
#define TAIL_DOT       0x0080   /* note tail marked with something round */
#define BODY_TRI       0x0100   /* note body is a triangle */
#define HEAD_CUT       0x1000   /* points[0] in middle of note body */
#define TAIL_CUT       0x2000   /* points[2] in middle of note body */

#define HEAD_FLAGS     (HEAD_LINE+HEAD_TRI+HEAD_DOT+HEAD_CUT)
#define TAIL_FLAGS     (TAIL_LINE+TAIL_TRI+TAIL_DOT+TAIL_CUT)
#define BODY_FLAGS     (BODY_RECT+BODY_ELLIP+BODY_TRI)

#define BORDERS_NONE   0x4000   /* perimeter off */
#define BORDERS_3D     0x8000   /* 3D style */

#define BORDER_FLAGS   (BORDERS_NONE|BORDERS_3D)

/* other attributes may follow ... */


typedef enum {behind, between} MARK_MODE ;
#define INVALID_DX 0x7FFF	/* would be outside screen either */
#define INVALID_DY 0x7FFF

void draw_lines 
(
	VirtualDevice * handle, 	/* VDI workstation handle */
	int  number,			/* number of lines to be drawn */
	char first_marked, 		/* first line with special marking */
	char d_marked, 			/* special marking each d_marked lines */
	MARK_MODE mode, 		/* where to place rectangle marks */
	int styles[11], 		/* styles to be used after a special line, or NULL */
							/* set to NULL when no filled areas shall be drawn */
	int colors[11], 		/* colors to be used after a special line, or NULL */
	char first_number,      /* number (text) for first special line */
	int number_dots,		/* of special line, no dots if -1 */
	int dx_text, 			/* text (number) offset from x_start on */
	                        /* if INVALID_DX, no text is drawn      */
	int dy_text,			/* text (number) offset from y_start on */
	                        /* if INVALID_DY, no text is drawn      */
	int x_start, 			/* of first line */
	int y_start,			/* of first line */
	int x_end, 				/* of first line */
	int y_end,				/* of first_line */
	float dx, 				/* distance of succeeding lines */
	float dy,				/* distance of succeeding lines */
	int line_width			/* in pixels */
) ; 


/*** macros for converting 8 bits and 5 bits  ***/
/*** color intensities with correct rounding. ***/
#define eight_to_five_bits(x) ( (int)((float)(x) * 0x1F/0xFF + 0.5) )
#define five_to_eight_bits(x) ( (int)((float)(x) * 0xFF/0x1F + 0.5) )

/* 5 bits red, 5 bits green, 5 bits blue */
/* least significant is red */

int32_t RGB_15to24(uint16_t in) ;
int16_t RGB_24to15(uint32_t in) ;

#endif /* include blocker */
