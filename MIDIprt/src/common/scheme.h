/*****************************************************************************
  Purpose:     Functions for loading and saving of color scheme files.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: SCHEME.H,v 1.6 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INCL_SCHEME
#define INCL_SCHEME

#include <portable.h>   // from my libs
#include <graphic.h>

typedef struct
{
	int note_style[12]     , note_color[12] ;
	int note_style_ends[12], note_color_ends[12] ;
	int dodecime_style[11] , dodecime_color[11] ;
	int back_style         , back_color ;
	int text_color ;
} 
COLOR_SCHEME ;
	/* On TOS the colors are indices or 15 bit RGBs, on Windows 24 bits. */

void default_scheme(COLOR_SCHEME *s) ;
	/* inits scheme with some reasonable values */
	
//void rgb_to_index_scheme(COLOR_SCHEME *s) ;
//	/* convert the scheme's colors to indices (color palette) */
//	/* if the screen mode is direct color, this function does nothing, 
//	   i.e. colors are always 15 bit RGB values */
	   
//void index_to_rgb_scheme(COLOR_SCHEME *s) ;
//	/* convert the scheme's colors to 15 bit RGBs */
//	/* if the screen mode is direct color, this function does nothing, 
//	   i.e. colors are always 15 bit RGB values */
	   
int save_scheme(const COLOR_SCHEME *scheme, const char *scheme_path) ;
	/* save scheme to file "scheme_path" */
	/* Intel to Motorola conversion and 24 bits to 15 bits RGB
	   conversion are performed when necessary. */
	/* return: 0 = good, -1 = err */
	
int load_scheme(COLOR_SCHEME *scheme, const char *scheme_path) ;
	/* load scheme from disk */
	/* Conversions are performed inverse save_scheme(), when necessary. */
	/* return: 0 = good, -1 = err, 1 = file OK but its length is wrong */

void draw_scheme(
    VirtualDevice * handle,  // wxWindows device context
	COLOR_SCHEME *scheme, 
	int transpose,
    int x, int y, int w, int h,
    int dodecime_index,
    int horizontal_lines, int dots, MARK_MODE mark_mode,
    int note_type,
    int dodecime_samples) ;
	/* draws scheme example on VDI handle */
	/* if scheme is NULL, the function will crash */
	/* note_type is used in draw_note() calls */
	/* all 11 dodecime styles and colors are drawn if dodecime_samples is TRUE */

#endif /* include blocker */
