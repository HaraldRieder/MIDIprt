/*****************************************************************************
  Purpose:     Drawing functions (floating point part).
               Conversions between 15 and 24 bit RGB colors.
               Use of this module requires linking of floating point lib on
               Atari TOS.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: GRAPHICF.H,v 1.8 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef GRAPHICF_H
#define GRAPHICF_H

#include "portable.h"
#include "graphic.h"

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

#ifndef __PUREC__
INT32 RGB_15to24(UINT16 in) ;
INT16 RGB_24to15(UINT32 in) ;
#endif

#endif /* include blocker */
