/*****************************************************************************
  Purpose:     Parse MIDI file and build hierarchical data structures from it.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: TRCKTABL.H,v 1.5 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_TRCKTABL
#define INC_TRCKTABL

#include <portable.h>
#include <notimtab.h>

typedef struct 
{
	TRACK_INFO track_info ;
	NOTE_TIME_TABLE_ELEMENT *first_element ;
	INT32 elements_generated ;
	int last ;   /* TRUE, if last element in table */
} 
TRACK ;


/* return codes: */

#define TRANSFORM_GOOD                 0
#define TRANSFORM_OUT_OF_MEMORY       -1
#define TRANSFORM_INVALID_FILE_FORMAT -2

int make_track_table(
	const unsigned char *data,   /* copy of file in RAM */
	long  size,             /* of file in bytes */
	int   number_tracks,    /* in MIDI file */
	TRACK *track_table[],   /* out: array of TRACK */
	long *transformed_size /* out: size of track table */
) ;

void free_track_table(TRACK table[]) ;

int get_min_max
/*** find lowest and highest notes in specified time interval
     of the specified track t ***/
(
	const TRACK *t,	    /* track data to search */
	const UINT16 channel_filter, /* bit mask: which channels to consider */
	TIME  min_time,		/* where to start search */
	TIME  max_time,		/* where to stop search */
	NOTE *min_note,		/* min found in search area */
	NOTE *max_note		/* max found in search area */
) ;
/*
*	return: != 0 if notes were found in the searched time 
*	        If returns with 0, min and max note are set to 0.
*/


#endif /* include blocker */
