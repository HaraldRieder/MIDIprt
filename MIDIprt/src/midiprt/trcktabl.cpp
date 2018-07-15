/*****************************************************************************
  Purpose:     Parse MIDI file and build hierarchical data structures from it.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: TRCKTABL.C,v 1.4 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include "trcktabl.h"


void free_track_table(TRACK table[])
{
	TRACK *current_track = table ;

	if (!table) return ; 

	/* free the note time tables */
	while (!current_track->last)
	{
		if (current_track->first_element)
			free(current_track->first_element) ;
		free_track_info(&current_track->track_info) ;
		current_track += 1 ;
	}
	/* also the very last */
	if (current_track->first_element)
		free(current_track->first_element) ; 
	free_track_info(&current_track->track_info) ;
	/* free track table */
	free(table) ;
}


int make_track_table(
	const unsigned char *data,   /* copy of file in RAM */
	long    size,             /* of file in bytes */
	int        number_of_tracks, /* in MIDI file */
	TRACK   *track_table[],
	long *total_transformed_size
)
{
	long length ;
	long transformed_size ;
	TRACK *current_track ;
	const unsigned char *address = data ;
	
	*total_transformed_size = 0 ;

	/* allocate track table */
	*track_table = current_track = (TRACK *)calloc(number_of_tracks, sizeof(TRACK)) ;
	if (!current_track) 
		return TRANSFORM_OUT_OF_MEMORY ;

	/* transform MIDI file */
	if (current_track) while (address < data + size)
	{
		/* get the length of the next chunk */
		length = read_int32(address + 4) ;
		if ( !memcmp("MTrk", address, 4) )
		{
			/* This is a track chunk: */
			/* init track data */
			init_track_info(&current_track->track_info) ;
	 		current_track->first_element = NULL ;
			current_track->elements_generated = 0 ; 
			current_track->last = 1 /* TRUE */ ;
			if (current_track != *track_table) 
			  (current_track - 1)->last = 0 /* FALSE */ ;

			/* allocate enough RAM for note time table */
			current_track->first_element = (NOTE_TIME_TABLE_ELEMENT *)
				calloc( length/3, sizeof(TRACK) )  ;
			if (!current_track->first_element) 
			{
				free_track_table(*track_table) ;
				*track_table = NULL ;
				return TRANSFORM_OUT_OF_MEMORY ;
			}

			/* fill note time table */
			if ( fill_note_time_table(address + 4 + 4, length, 
					current_track->first_element, 
			  		&current_track->elements_generated, 
			  		&current_track->track_info) != note_time_table_filled ) 
			{
				free_track_table(*track_table) ;
				*track_table = NULL ;
				return TRANSFORM_INVALID_FILE_FORMAT ;
			}

			/* shrink RAM to right size */
			transformed_size = current_track->elements_generated * sizeof(NOTE_TIME_TABLE_ELEMENT) ;
			if (transformed_size)
			{
				current_track->first_element = (NOTE_TIME_TABLE_ELEMENT *)
					realloc(current_track->first_element, transformed_size) ;
				if (!current_track->first_element)
				{
					free_track_table(*track_table) ;
					*track_table = NULL ;
					return TRANSFORM_OUT_OF_MEMORY ;
				}
				*total_transformed_size += transformed_size ;
			}
			else
			{
				free(current_track->first_element) ;
				current_track->first_element = NULL ;
			}
			/* address next track table element */
			current_track += 1 ; 
		}
		/* address next chunk */
		address = address + length + 4 + 4 ;	
	}
   return TRANSFORM_GOOD ;
}


int get_min_max
(
	const TRACK *t,		/* track data to search */
	const UINT16 channel_filter, /* bit mask: which channels to consider */
	TIME  min_time,		/* where to start search */
	TIME  max_time,		/* where to stop search */
	NOTE *min_note,		/* min found in search area */
	NOTE *max_note		/* max found in search area */
) 
{
	int found = 0 ;
	NOTE_TIME_TABLE_ELEMENT 
		*n, *last_element =
			t->first_element + t->elements_generated - 1 ; 


	n = get_element(t->first_element, t->elements_generated, min_time) ;
	if ( (!n) ||                    /* no element beyond min_time found */
	     (n->time >= max_time) )    /* element found, but its time is too high */
	{
		*max_note = *min_note = 0x00 ;
	}
	else
	{
		/* if max. time is higher than the highest time of the track reduce it */
		if ( last_element->time < max_time )
			max_time = (t->first_element + t->elements_generated - 1)->time ;

		/* search for minimum and maximum */
		*min_note = *max_note = n->note & ~_ON_ ;
		while ( (n->time <= max_time) && (n <= last_element) )
		{
			if ( channel_filter & (0x1 << n->channel) )
			{
				if ((n->note & ~_ON_) > *max_note) 
					*max_note = n->note & ~_ON_ ;
				if ((n->note & ~_ON_) < *min_note) 
					*min_note = n->note & ~_ON_ ;
				found = 1 ;
			}
			n += 1 ;
		}
	}
	return found ;
}


