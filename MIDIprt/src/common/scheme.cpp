/*****************************************************************************
  Purpose:     Functions for loading and saving of color scheme files.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: SCHEME.C,v 1.9 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <portable.h> 
#include <servimem.h> /* because of swap_int16(), ... */
#include <graphic.h>
#include <graphicf.h>
#include "scheme.h"

#include <wxVDI.h>
#pragma pack(1)

typedef struct
{
	char reserved[94] ;
	/* offset 94 */ INT16 dodecime_style[12], dodecime_color[12] ;
	INT16 reserved1[2] ;
	/* offset 146 */ INT16 note_style[12], note_color[12] ;
	/* offset 194 */ INT16 text_color ;
	char reserved12, reserved13;
	/* offset 198 */ INT16 back_style, back_color ;
	/* offset 202 */ INT16 note_style_ends[12], note_color_ends[12] ;
} 
SCHEME_FILE ;

#pragma pack()

void default_scheme(COLOR_SCHEME *s)
	/* inits scheme with some reasonable values */
{
	int i ;
	int black = 0x0000 ; /* 15 bit RGB */
	int white = 0x7fff ;
	for (i = 0 ; i < 11 ; i++)
	{
		s->dodecime_style[i] = IP_1PATT ;
		s->dodecime_color[i] = black ;
	}
	for (i = 0 ; i < 12 ; i++)
	{
		if (i % 2) s->note_style[i] = IP_SOLID  ;
		else       s->note_style[i] = IP_HOLLOW ;
		s->note_style_ends[i] = IP_SOLID ;
		s->note_color[i] = s->note_color_ends[i] = black ;
	}
	s->back_style = IP_SOLID ;
	s->back_color = white ;
	s->text_color = black ;
}	

static void copy_to_scheme(COLOR_SCHEME *s, const SCHEME_FILE *sf)
{
	int i ;
	for (i = 0 ; i < 12 ; i++)
	{
		// Motorola to Intel format (little endian<=>big endian conversions)
		s->note_color [i]	  = swap_int16( sf->note_color[i] ) ;
		s->note_style [i]	  = swap_int16( sf->note_style[i] ) ; 
		s->note_color_ends[i] = swap_int16( sf->note_color_ends[i] ) ;
		s->note_style_ends[i] = swap_int16( sf->note_style_ends[i] ) ; 
	}
	for (i = 0 ; i < 11 ; i++)
	{
		s->dodecime_color[i]  = swap_int16( sf->dodecime_color[i] ) ;
		s->dodecime_style[i]  = swap_int16( sf->dodecime_style[i] ) ;			
	}
	s->back_color = swap_int16( sf->back_color ) ;
	s->back_style = swap_int16( sf->back_style ) ;			
	s->text_color = swap_int16( sf->text_color ) ;
}


static void copy_to_scheme_file(SCHEME_FILE *sf, const COLOR_SCHEME *_s)
{
	int i ;
	for (i = 0 ; i < 12 ; i++)
	{
		// Intel to Motorola format
		sf->note_color [i]	   = swap_int16( _s->note_color[i] ) ;
		sf->note_style [i]	   = swap_int16( _s->note_style[i] ) ; 
		sf->note_color_ends[i] = swap_int16( _s->note_color_ends[i] ) ;
		sf->note_style_ends[i] = swap_int16( _s->note_style_ends[i] ) ; 
	}
	for (i = 0 ; i < 11 ; i++)
	{
		sf->dodecime_color[i]  = swap_int16( _s->dodecime_color[i] ) ;
		sf->dodecime_style[i]  = swap_int16( _s->dodecime_style[i] ) ;			
	}
	sf->back_color = swap_int16( _s->back_color ) ;
	sf->back_style = swap_int16( _s->back_style ) ;			
	sf->text_color = swap_int16( _s->text_color ) ;
}


int load_scheme(COLOR_SCHEME *scheme, const char *scheme_path)
{
	size_t read_bytes ;
	char buff[sizeof(SCHEME_FILE) + 1] ;
	FILE *file ;

	if (!scheme_path || !*scheme_path)
		return -1 ;

	if ( file = fopen(scheme_path, "rb"), file )
	{
		read_bytes = fread(buff,1,sizeof(buff),file) ;
		if ( read_bytes == sizeof(SCHEME_FILE) )
		{
			buff[sizeof(buff)-1] = 0 ; /* allow strcpy() of last struct member */
			copy_to_scheme(scheme, (SCHEME_FILE *)buff) ;
		}
		else if ( ferror(file) )
		     return -1 ;
		else return 1 ; /* wrong format */ ;
		return fclose(file) ;
	}
	return -1 ;
}

int save_scheme(const COLOR_SCHEME *scheme, const char *scheme_path)
{
	SCHEME_FILE sf ;
	FILE *file = fopen(scheme_path, "wb") ;
	
	if ( file )
	{
		copy_to_scheme_file(&sf, scheme) ;
		if ( fwrite(&sf, sizeof(sf), 1, file) != 1 ) /* 1 scheme written ? */
		{
			fclose(file) ;
			return -1 ;
		}
		return fclose(file) ;
	}
	return -1 ;
}

void draw_scheme(
    VirtualDevice * handle,  // wxWindows device context
	COLOR_SCHEME *scheme, int tr /*transpose*/,
    int x, int y, int w, int h,
    int dodecime_index,
    int number_of_lines, int dots, MARK_MODE mark_mode,
    int note_type,
    int dodecime_samples) 
{
	int i,j, points[4];

	float dx_notes, dy_notes ;
	int dy_dodec = h / 11 ;
		  
	int x_notes = x ;
	int y_notes = y + w - 1 ;

	if (mark_mode == between)
	{
		dy_notes = ((float)h) / 12 ;
	}
	else
	{
		/* we need more space at the bottom */
		/* because a pattern is drawn behind the lowest line */
		dy_notes = ((float)h) / 12.5 ;
		y_notes -= (int)dy_notes;
	}
	y_notes -= (dy_notes / 4) ; /* HEAD_TRI and TAIL_TRI need more space */
	
	if (dodecime_samples)
	{
		dx_notes = ((float)w) / 14 , /* 1 note width resevere space for dot ends */
		x_notes = (int)(dx_notes + x + 0.5) ;
	}
	else
	{
		dx_notes = ((float)w) / 13 , /* 1 note width resevere space for dot ends */
		x_notes = x ;
	}

	/* draw background */
    points[0] = x ;
	points[1] = y ;	
	points[2] = x + w - 1 ;
	points[3] = y + h - 1 ;	
	draw_background(handle, points, scheme->back_style, scheme->back_color) ;
	
	/* draw hor. lines */
	points[0]++ ;	/* space for bold Beyreuther lines */
	points[2]-- ;
	points[1] = y + (int)(dy_notes * 11.5) ;
	draw_lines(handle, number_of_lines, 0 /*first_shadow*/, 
	           number_of_lines, mark_mode, 
	           scheme->dodecime_style, scheme->dodecime_color, dodecime_index, 
	           dots, INVALID_DX,INVALID_DY, /*no text*/
	           x_notes, points[1], x + w - 1, points[1],
	           0.0f, - dy_notes * (12/number_of_lines), 
	           1 /* line width */) ;

	if (dodecime_samples)
	{	           
		/* draw 11 (128 MIDI notes < 11 * 12) dodecime colors & styles */
		points[3] -= 3 ; /* 3 pixels space */
		for (i = 0 ; i < 11 ; i++)
		{
			draw_lines(handle, 2 /*number_of_lines*/, 0 /*first_shadow*/, 
	           4 /*hor_lines*/,
	           between /*mark_mode*/, 
	           scheme->dodecime_style, scheme->dodecime_color, i, 
	           -1 /*no dots*/, 
	           INVALID_DX,INVALID_DY, /*no text*/
	           x, 
	           points[3] - (int)(dy_dodec * i), 
	           x_notes - 3,	/* 3 pixels space */ 
	           points[3] - (int)(dy_dodec * i),
	           0.0f, -(dy_dodec - 3) /* 3 pixels space */,
	           1 /* line width */ ) ;
		}
	}
	
	/* draw notes */
	/* 1/2 of the 1 note width reserve space for dot ends on the left: */
	vsl_width(handle, 1) ;
	for (i = 0 ; i < 12 ; i++)
	{
		points[0] = x_notes + dx_notes / 2 + (int)(dx_notes * i + 0.5) ;
		points[3] = y + (int)(dy_notes * (11.75f - i) + 0.5) ;	
		points[2] = points[0] + (int)(dx_notes) ;
		points[1] = points[3] - (int)(dy_notes/2 + 0.5) ;
		j = (i + 12 - tr) % 12 ;
		draw_note(handle, points, note_type, 
			scheme->note_style[j]     , scheme->note_color[j],
			scheme->note_style_ends[j], scheme->note_color_ends[j] ) ;
	}

	/* draw ver. lines */
	rgb_lcolor(handle, scheme->text_color) ;
    handle->lineType = SOLID ;
	draw_lines(handle, 2, -1, -1,
		behind /*any mark_mode*/, NULL, NULL, 0, -1, 0, 0,
		x_notes, y,	x_notes, y+h-1, x+w-1-x_notes, 0.0f, 1) ;
	/* draw dashed vertical lines (sub-bars) */
	rgb_udsty_lcolor(handle, /*0x2222*/ 0x4924, 
		scheme->text_color, scheme->back_color) ;
	draw_lines(handle, 1, -1, -1,
		behind /*any mark_mode*/, NULL, NULL, 0, -1, 0, 0,
		(x_notes + x+w-1)/2, y,	(x_notes + x+w-1)/2, y+h-1, 0.0f, 0.0f, 1) ;
}