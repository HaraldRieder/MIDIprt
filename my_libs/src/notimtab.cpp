/*****************************************************************************
  Purpose:     Models note-on and note-off events and time 
               (for 1 MIDI channel).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: NOTIMTAB.C,v 1.3 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#if defined (__PUREC__)
#	include <portable.h>
#	include <acs.h>        /* because of NULL, Ax_malloc(), ... */
#else 
#   include <malloc.h>
#	define TRUE  1
#	define FALSE 0
#endif
#include <string.h>     /* because of memcmp(), ... */

#include <servimem.h>
#include <notimtab.h>


/********************************************************************
*/
	int delta(
		NOTE_TIME_TABLE_ELEMENT *border,  /* search till border is reached */
		NOTE_TIME_TABLE_ELEMENT *position /* search from position onwards  */
    )    
/* Assumes "border" and "position" to be pointers
* into a note_time_table. "delta" searches - starting from "position" 
* in direction to "border" - for the first note in the table which ma- 
* tches to the one "position" is pointing to. Two notes match - in 
* the sense used here - if note number, channel and source chunk are 
* equal and if the reference of the found note is still initialized.
*
* RETURN:  d: = 0 , if no match was found or if position == border.
*             = matching position - position , else.
* 
********************************************************************/
{
	int d = 0;
	NOTE_TIME_TABLE_ELEMENT *current ;

	current = position ;
	if (border > position)
	{
		while ( (current < border) && !d )
		{
			current += 1 ;
			if (!current->reference &&
			     (current->note == position->note) &&
			     (current->channel == position->channel) )
				d = (int)(current - position) ;	
		}
	}
	else if (border < position)
	{
		while ( (current > border) && !d )
		{
			current -= 1 ;
			if (!current->reference &&
			     (current->note == position->note) &&
			     (current->channel == position->channel) )
				d = (int)(current - position) ;	
		}
	}
	return(d) ;
}


static char *create_text(const unsigned char* data, size_t len)
{
	char *text = (char*) malloc (len+1) ;
	if (text) 
	{
		memcpy (text, data, len) ;
		text[len] = 0 ;
	}
	return text ;
}


/******************************************************************/
/***********  PUBLISHED INTERFACE (PIF)  **************************/
/******************************************************************/

void init_track_info(TRACK_INFO *ti) 
{
	ti->sequence_number = -1 ;

	ti->text = ti->copyright = ti->track_name =	ti->instrument_name =
	ti->lyric =	ti->marker = ti->cue_point = ti->program_name =
	ti->device_name = NULL ;

	ti->tempo =	ti->numerator =	ti->denominator = ti->key =	ti->minor = -1 ;
}

void free_track_info(TRACK_INFO *ti) 
{
	if (ti->text)            free(ti->text) ;
	if (ti->copyright)       free(ti->copyright) ;
	if (ti->track_name)      free(ti->track_name) ;
	if (ti->instrument_name) free(ti->instrument_name) ;
	if (ti->lyric)           free(ti->lyric) ;
	if (ti->marker)          free(ti->marker) ;
	if (ti->cue_point)       free(ti->cue_point) ;
	if (ti->program_name)    free(ti->program_name) ;
	if (ti->device_name)     free(ti->device_name) ;
}

unsigned int fill_note_time_table
(
	const unsigned char *track_address,
	INT32 track_length,
	NOTE_TIME_TABLE_ELEMENT *first_element,
	INT32 *elements_generated,
	TRACK_INFO *ti 
)
{
	const unsigned char *next_data = track_address ;
	unsigned char status = 0 ;
	NOTE_TIME_TABLE_ELEMENT *current_element = first_element;
	UINT32 length = 0 ; /* length of sysex or meta events */
	TIME time = 0, 
	     delta_time = 0 ;
	unsigned char read_bytes = 0 ;
	INT16 d = 0 ;   /* reference(note on) = - reference(note off) = d */
	int EOT_reached = FALSE ;

	ti->channels = 0 ; /* up to now, all are empty */
	ti->min_note = 0x7f ; 
	ti->max_note = 0x00 ; 

	/*** while not end of track reached ***/
	while (!EOT_reached)
	{
		/*** get next time value ***/
		if ( read_var_len(next_data, (UINT32 *)&delta_time, &read_bytes) )
			return no_var_length_number ;
		next_data += read_bytes ; 
		time += delta_time ;

		/*** get next event out of this chunk ***/
		if ( (*next_data & MSN) == SYS_EX )
		{ 
			/*** sysex or meta event or error ***/
			status = 0 ;
			switch (*next_data)
			{
			case SYS_EX: /*** sysex event transmit "f0": ignore ***/
				if ( read_var_len(next_data + 1, &length, &read_bytes) 
					== no_var_length_number ) return no_var_length_number ;
				next_data += (1 + read_bytes + length) ; break ;
			case SYS_EX_NT: /*** sysex event not transmit "f0": ignore ***/
				if ( read_var_len(next_data + 1, &length, &read_bytes) 
					== no_var_length_number ) return no_var_length_number ;
				next_data += (1 + read_bytes + length) ; break ;
			case META_EVENT: /*** meta event ***/
				next_data += 1 ; /* point to type of meta event */
				if ( read_var_len(next_data + 1, &length, &read_bytes) 
					== no_var_length_number ) return no_var_length_number ;
				switch (*next_data)
				{
				case END_OF_TRACK: EOT_reached = TRUE ; break ;
				case SEQUENCE_NUM:
					if (ti->sequence_number < 0)
						ti->sequence_number = 
							(next_data[1+read_bytes] << 8) + 
							 next_data[2+read_bytes] ;
					break ;
				case GENERAL_TEXT:
					if (!ti->text)
						ti->text = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case COPYR_NOTICE:
					if (!ti->copyright)
						ti->copyright = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case SQ_TRCK_NAME: 
					if (!ti->track_name)
						ti->track_name = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case INSTRUM_NAME:
					if (!ti->instrument_name)
						ti->instrument_name = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case LYRIC:
					if (!ti->lyric)
						ti->lyric = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case TEXT_MARKER:
					if (!ti->marker)
						ti->marker = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case CUE_POINT:
					if (!ti->cue_point)
						ti->cue_point = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case PROGRAM_NAME:
					if (!ti->program_name)
						ti->program_name = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case DEVICE_NAME:
					if (!ti->device_name)
						ti->device_name = create_text(next_data + 1 + read_bytes, length) ;
					break ;
				case SET_TEMPO:
					if (ti->tempo < 0)
						ti->tempo = 
							(next_data[1+read_bytes] << 16) +
							(next_data[2+read_bytes] << 8) + 
							 next_data[3+read_bytes] ;
					break ;
				case SMPTE_OFFSET: break ; /* not supported */
				case TIME_SIGNATURE:
					if (ti->numerator < 0)
					{
						ti->numerator   = next_data[1+read_bytes] ;
						ti->denominator = 1 << next_data[2+read_bytes] ;
						/* the rest is not supported */
					}
					break ;
				case KEY_SIGNATURE:
					if (ti->key < 0)
					{
						ti->key = (24 + next_data[1+read_bytes]) % 12 ; /* => 0..11, 0 is C */
						ti->minor = next_data[2+read_bytes] ;
					}
					break ;
				}
				next_data += (1 + read_bytes + length) ;
				break ;
			default: 
				return wrong_event_found ;
			}
		}
		else 
		{
			/*** midi data or channel message status byte ***/
			if ( !(*next_data & STATUS) ) 
			{ 
				/*** data, running status must be given ***/
				if (!status) return data_without_status_given ;
			}
			else	
			{ 
				/*** channel message status byte ***/
				status = *next_data ;
				next_data += 1 ;
			}
			switch (status & MSN)
			{
			case NOTE_OFF: /* for delta: | _ON_ */ 
				current_element->note = *next_data | _ON_ ; 
				current_element->channel = status & LSN ;
				next_data += 1 ;
				current_element->dynamic = *next_data ;
				next_data += 1 ;
				current_element->time = time ;

				/*** link note on and note off ***/
				d = delta(first_element, current_element) ; 

				/*** reset: note off ***/
				current_element->note &= ~_ON_ ; 
				current_element->reference = d ;   
				(current_element + d)->reference = - d ;
				current_element += 1 ; 
				break ;
			case NOTE_ON: 
				if (*next_data > ti->max_note) ti->max_note = *next_data ;
				if (*next_data < ti->min_note) ti->min_note = *next_data ;
				current_element->note = *next_data | _ON_ ; 
				current_element->channel = status & LSN ;
				ti->channels |= 0x1<<(current_element->channel) ; 
				next_data += 1 ;
				current_element->dynamic = *next_data ;
				next_data += 1 ;
				current_element->time = time ;

				/*** note on with dynamic == 0 is note off ***/
				if (!current_element->dynamic)
				{ 
					/*** link note on and note off ***/
					d = delta(first_element, current_element) ;

					/*** switch note to off ! ***/
					current_element->note &= ~_ON_ ; 
					current_element->reference = d ;   
					(current_element + d)->reference = - d ;
					current_element += 1 ; 
				} 
				else  
				{ 
					/*** normal note on:                   ***/
					/*** initialize reference and continue ***/
					current_element->reference = 0 ; 
					current_element += 1 ;
				}
				break ;
			case POLY_KEY_PRESS: 
			case CONTROL_CHANGE: 
			case PITCH_BEND: 
				next_data += 2 ; break ; 
			case PROGRAM_CHANGE: 
			case CHANNEL_PRESS: 
				next_data += 1 ; break ; 
			}
		}
		if (next_data - track_address > track_length)
			return next_data_overflow ;
	} /*** <-- end while (!EOT_reached) ***/

	*elements_generated = (current_element - first_element) / sizeof *first_element ;
	/*** return success message ***/
	return note_time_table_filled ;
}



NOTE_TIME_TABLE_ELEMENT *get_element(
	NOTE_TIME_TABLE_ELEMENT *first_element, UINT32 number , TIME time)
{
NOTE_TIME_TABLE_ELEMENT *current_element, *previous_element ;
unsigned long low = 0, mid, high = number ;
int found = FALSE ;

	/*** Has already the first element a higher time ? ***/
	if (first_element->time >= time) return first_element ;	

	/*** Has the last element a lower time ? ***/
	current_element = first_element + number - 1 ;
	if (current_element->time < time) return NULL ; /* failed */

	/*** Now, number shurely is greater or equal 2 ! ***/
	while (!found)
		{
		mid = (low + high) / 2 ;
		current_element = first_element + mid ;
		previous_element = current_element - 1 ;
		if ((current_element->time >= time) && (previous_element->time >= time)) 
			 high = mid ;
		else if (current_element->time < time) low = mid ;
			 else found = TRUE ;
		if (!high) { current_element = first_element ; found = TRUE ; } ;
		}
	return current_element ;
}
