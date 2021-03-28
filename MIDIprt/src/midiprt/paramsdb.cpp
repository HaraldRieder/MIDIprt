/*****************************************************************************
  Purpose:     Functions operating on the data model of the parameters 
               window.
  Author:      Harald Rieder
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <diskfile.h>
#include "string.h"
#include "paramsdb.h"

using namespace std;

void default_params(PARAMS_DB *db)
{
	db->title[0] = 0 ;
	db->left_border  = db->right_border = 
	db->lower_border = db->upper_border = 0 ;

	db->system_distance = 5 ;
	db->track_distance  = 4 ;
	db->note_distance = 0.3f ;
	
	db->horizontal_lines = 2 ;
	db->mode = Rieder ;
	
/*	db->note_height  = 1.0f ;*/
	db->note_height  = 5 ;
	db->note_dynscale = (MAX_DYNSCALE + MIN_DYNSCALE)/2 ;
	db->transpose = 0 ;
	
	db->bars_per_line = 12 ;
	db->bar_length    =  8 ; /* 8/8 = 4/4 */
	
	db->note_type = BODY_RECT ;

	db->font[0] = 0 ;
	db->points  = 18; /* 18 points for the title */
	db->effects = 0 ; /* normal */

	strcpy(db->line_width        , DEFAULT_LINE_WIDTH) ;
	strcpy(db->bar_line_width    , DEFAULT_LINE_WIDTH) ;
	strcpy(db->sub_bar_line_width, DEFAULT_LINE_WIDTH) ;
	strcpy(db->note_line_width   , DEFAULT_LINE_WIDTH) ;
		
	default_scheme(&(db->scheme)) ;
}


void validate(PARAMS_DB *db)
{
	db->left_border     = min(max(db->left_border    , MIN_BORDER)         , MAX_BORDER) ;
	db->right_border    = min(max(db->right_border   , MIN_BORDER)         , MAX_BORDER) ;
	db->lower_border    = min(max(db->lower_border   , MIN_BORDER)         , MAX_BORDER) ;
	db->upper_border    = min(max(db->upper_border   , MIN_BORDER)         , MAX_BORDER) ;

	db->bars_per_line   = min(max(db->bars_per_line  , MIN_BARS_PER_LINE)  , MAX_BARS_PER_LINE) ;
	db->bar_length      = min(max(db->bar_length     , MIN_BAR_LENGTH)     , MAX_BAR_LENGTH) ;
	db->sub_bars        = min(max(db->sub_bars       , MIN_SUB_BARS)       , MAX_SUB_BARS) ;

	db->system_distance = min(max(db->system_distance, MIN_SYSTEM_DISTANCE), MAX_SYSTEM_DISTANCE) ;
	db->track_distance  = min(max(db->track_distance , MIN_TRACK_DISTANCE) , MAX_TRACK_DISTANCE) ;
	/* backward compatibility to the time when we measured this in 1/1000 (now in 1/100): */
	while (db->note_distance > MAX_NOTE_DISTANCE)
		db->note_distance /= 10 ;
	db->note_distance   = max(db->note_distance, MIN_NOTE_DISTANCE) ;

	db->note_height     = min(max(db->note_height    , MIN_AVERAGE_HEIGHT) , MAX_AVERAGE_HEIGHT) ;
	db->note_dynscale   = min(max(db->note_dynscale  , MIN_DYNSCALE)       , MAX_DYNSCALE) ;
	
	db->transpose       = min(max(db->transpose      , MIN_TRANSPOSE)      , MAX_TRANSPOSE) ; 
}


void do_load_scheme(PARAMS_DB *db, const char *path) 
{
  	if ( load_scheme(&(db->scheme), path) == 0 )
  	{
   		strcpy(db->scheme_path, path) ;
   	}
   	else 
   	{
   		default_scheme(&(db->scheme)) ;
   		db->scheme_path[0] = 0 ;
   	}
}
