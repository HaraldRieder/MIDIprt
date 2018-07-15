/*****************************************************************************
  Purpose:     Data model for the parameters window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: PARAMSDB.H,v 1.7 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_PARAMSDB
#define INC_PARAMSDB 1

#include <diskfile.h>
#include "scheme.h"

#define Beyreuther 0x20    /* notation mode Beyreuther */
#define Rieder     0x40    /* notation mode Rieder     */
#define Mix        0x30    /* notation mode Mix        */

#define MAX_BORDER          49
#define MIN_BORDER           0
#define MAX_BARS_PER_LINE   40 
#define MIN_BARS_PER_LINE    1 
#define MAX_BAR_LENGTH      40 
#define MIN_BAR_LENGTH       1 
#define MAX_SUB_BARS        12
#define MIN_SUB_BARS         1
#define MAX_SYSTEM_DISTANCE 15 
#define MIN_SYSTEM_DISTANCE  1
#define MAX_TRACK_DISTANCE  15 
#define MIN_TRACK_DISTANCE   1
#define MAX_NOTE_DISTANCE    0.8f
#define MIN_NOTE_DISTANCE    0.05f
/*#define MAX_AVERAGE_HEIGHT   2.0f
#define MIN_AVERAGE_HEIGHT   0.1f*/
#define MAX_AVERAGE_HEIGHT   9
#define MIN_AVERAGE_HEIGHT   1
#define MAX_DYNSCALE        15
#define MIN_DYNSCALE         0
#define MAX_TRANSPOSE       11
#define MIN_TRANSPOSE        0

#define LINE_WIDTH_STRLEN    3  /* e.g. "0.7" millimeters */ 
#define DEFAULT_LINE_WIDTH "0.2" /* millimeters */

typedef struct
{
	char *filename ;
	
	char title[FILENAME_LENGTH] ; 
	char font[300] ; /**< wxString native font info description */
	int points, effects ;

	int left_border, right_border, upper_border, lower_border ;

	int system_distance, track_distance ;
	float note_distance ;	/* all distances in % */
	
	int bars_per_line, bar_length /* in 1/8 */, sub_bars ;
	char bar_line_width    [LINE_WIDTH_STRLEN+1] ; 	
	char sub_bar_line_width[LINE_WIDTH_STRLEN+1] ; 	
	
	int horizontal_lines ;
	int mode ;
	char line_width[LINE_WIDTH_STRLEN+1] ; 	
	
	int   note_height ;		/* relative units, normal = 5..6 */
	int   note_dynscale ;	/* 0 .. n (relative units) */
	int   note_type ;       /* see draw_note() */
	char  note_line_width[LINE_WIDTH_STRLEN+1] ; 	
	
	char scheme_path[PATHNAME_LENGTH] ;
	COLOR_SCHEME scheme ;
	int   transpose ;
}
PARAMS_DB ;

/** sets the parameters to reasonable default values:
   a black and white notation with mode Rieder */
void default_params(PARAMS_DB *db) ;

/** corrects forbidden values in db */
void validate(PARAMS_DB *db) ;

	
/** tries to load scheme from file into db,
   sets init values if not possible */
void do_load_scheme(PARAMS_DB *db, const char *path) ;
 
#endif /* include blocker */
