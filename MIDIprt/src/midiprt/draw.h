/*****************************************************************************
  Purpose:     Page layout and drawing of music notation systems.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: DRAW.H,v 1.8 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_DRAW
#define INC_DRAW

#include <notimtab.h> 
#include "trcktabl.h"
#include "scheme.h"
 
/* title height = syst.dist. * TILTE_FACTOR */
#define TITLE_FACTOR 0.33f 

/* small text height =  note dist. * SMALL_TEXT_FACTOR */
#define SMALL_TEXT_FACTOR 3.1f

#define FOOTLINE_HEIGHT 10     /* [points] */

/*** possible values for system type ***/

#define NO_TYPE    0x00  /* no valid type */
#define PRECEEDING 0x01  /* the tail of the system is on a succeeding page */
#define SUCCEEDING 0x02  /* the head of the system is on a preceeding page */


typedef struct
{
	float y_min, y_max ;					/* horizontal lines & notes */
	NOTE min_note, max_note, note_of_y ;
	char number_of_lines, first_modulo_0 ;
	int track ;								/* index into track table */
} 
LINE_DESCRIPTOR ;	/* 1 per track and line */

typedef struct
{
	float y_min, y_max ;			/* for the background */
	float y_min_lines, y_max_lines ;/* for the vertical bar lines */
	TIME min_time ;                 /* in ticks */
	short first_line, last_line  ;	/* indeces into line table */
	short type ;					/* see above */
}
SYSTEM_DESCRIPTOR ;

typedef struct 
{
	short first_system, last_system ; /* indeces into system table */
} 
PAGE_DESCRIPTOR ;

#define MAX_PAGES    200
#define MAX_LINES    800
#define MAX_SYSTEMS  400

typedef struct 
{
	short npgs ;                       /* number of layouted pages */
	PAGE_DESCRIPTOR page[MAX_PAGES] ;  /* 1 per page */

	short nsts ;
	SYSTEM_DESCRIPTOR syst[MAX_SYSTEMS];/* 1 per system or system-part of split systems */
	
	short nlns ;						/* number of layouted track lines */
	LINE_DESCRIPTOR line[MAX_LINES] ;   /* 1 per track and line */

	/* The system data must be constructed from page and line data. */

	float x_min, x_max, y_min, y_max ; /* maximum area allowed for whole drawing */

	float y_min_systems_0,	           /* (title only on page 0) */
	      y_min_systems, y_max_systems ;

	float dy_title ;					/* space for headline (cell height) */
	float dy_small_text ;               /* space to reserve for small text (cell height) */
	float dy_system, dy_track ; 
	float dy_note, dy_line ;            /* y-distances */
	float dy_extent ;					/* background is higher than track lines together */

	char hor_lines ;					/* per dodecime, influences track line height */
	TIME max_time ;
	TIME time_per_system ;              /* in ticks per system */
} 
LAYOUT ;

typedef enum {TR_NAME, TR_DEVICE, TR_INSTR, TR_TEXT} TR_INFOTYPE ;

typedef struct
{
	/* which of the corresponding strings to draw */
	TR_INFOTYPE select ;

	/* all strings are only read by filter window */
	char *name ;  
	char *device ;
	char *instrument ;
	char *text ;

	int filter ;   /* bit mask: (0 or DISABLED) + (0 = off or 1 = on) */
	unsigned int channels ;  /* 16 bits for MIDI channels: which contain events ? */
	unsigned int ch_filter ; /* 16 bits for MIDI channels: per channel filter */
}
FILTERED_TRACK ;


/*******************************************************************
*/
  void draw_page(
	VirtualDevice * handle,
	int x0, 
	int y0, 
	int width,			/* already zoomed */
	int height,
	int *points,		/* clipping */ 
	int pixel_height,	/* in micrometers */
	const TRACK track[],	/* the data to draw... */
	const FILTERED_TRACK ft[],	
	const LAYOUT *lt,			/* with this layout... */
	int p,					    /* index of page to draw */
	VDI_FONT_ID font_id,	/* font for all textes */
	int pts,		/* font height in points */  
	int effects,	/* text effects for title only */
	int foot_pts,	/* font height of footline in points */ 
	const char *title,
	const char *filename,	/* for footline ... */
	long filesize,
	const char *caption,
	const char *version,
	const char *platform,
    int av_height,		/* relative, 5..6 is normal */
	int maxdynscale,    /* n, max of the following parameter: */
	int dynscale,		/* 0 .. n */        
	int mode,			/* Rieder, Beyreuther, Mix */
	int type,			/* of note (e.g. with tail, etc.) */
	COLOR_SCHEME *scheme,
	int transpose,      /* 0 .. 11 of note styles and colors in COLOR_SCHEME */
	char bars_per_system,
	char sub_bars,
	int line_width,     /* in pixel */
	int line_width_bars,
	int line_width_sub_bars,
	int line_width_notes
	) ;
/*
 *	Possible notation modes:
 */
	#define Beyreuther 0x20 
	#define Rieder     0x40
	#define Mix        0x30 
/******************************************************************/


/*******************************************************************
*/
	int page_layouter(
		LAYOUT *lt,			/* OUT: the layout, all other params are IN */
		float left_border,  /* 0..1 */
		float right_border, 
		float upper_border, 
		float lower_border,
		float system_distance,
		float track_distance,
		float note_distance,
		char  hor_lines,			/* per dodecime */
		int   ntrks,				/* dimension of the 3 following arrays */
		const FILTERED_TRACK ft[],
		const TRACK track_table[],	/* from MIDI file */
		TIME  max_time,				/* occuring in all tracks */
		TIME  time_per_system		/* in ticks */
	) ;
	
/* Calculates the layout of all pages. The number
*  of pages depends on the current parameters.
*  The layouter only needs to be called when parameters have
*  changed, thus CPU time can be saved (i.e. the layouter 
*  should not always be called before drawing).
* 
* OUTPUT:  (lt->x_min/lt->y_min) and (lt->x_max/lt->y_max) are border
*          points of the allowed drawing area (allowed by the 
*          parameters object).
*          (lt->x_min_systems/lt->y_min_systems) and 
*          (lt->x_max_systems/lt->y_max_systems) are border
*          points of the drawing area for note systems 
*          (without title, indeces).
*          lt->npgs:             number of pages layouted, = 0 in error cases
*          lt->beats_per_system: beats_per_system
*          lt->time_per_system:  time per system in [ticks] 
*          lt->bars_per_system:  bars_per_system
*          lt->bar_divider:      sub-bars per bar
*          lt->dx_bars:          x-distance between bars
*          lt->dy_system:        y-distance between 2 systems (staves)
*          lt->dy_track:         y-distance between 2 track lines
*          lt->dy_line:          y-distance between note lines
*          lt->dy_note:          y-distance between notes
*          lt->page[0..n]:       correctly updated
* 
* RETURN:  
*/
			#define LAYOUT_ERR_TOO_MANY_PAGES   -1
			#define LAYOUT_ERR_TOO_MANY_SYSTEMS -2
			#define LAYOUT_ERR_TOO_MANY_LINES   -3
			#define LAYOUT_ERR_TRACK_TOO_HIGH   -4
			#define LAYOUT_ERR_ALL_TRACKS_OFF   -5
			#define LAYOUT_ERR_INVALID_BORDERS  -6
			#define LAYOUT_GOOD 0  
/* 
*******************************************************************/


#endif /* include blocker */

