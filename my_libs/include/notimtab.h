/*****************************************************************************
  Purpose:     Models note-on and note-off events and time 
               (for 1 MIDI channel).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: NOTIMTAB.H,v 1.3 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_NOTIMTAB
#define INC_NOTIMTAB

#include "servimem.h"
#include "midi.h"

/******* access to note time table via the following structure *************/

typedef UBYTE NOTE ;		/* 8 Bits */
#define _ON_	0x80		/* internal representation of "note on" */
typedef INT32 TIME ;


/*---------------------------------------------- 
The following structure must be packed.
The note time table memory can be allocated 
as estimated in advance from the MIDI file
size multiplied by a minimum fixed number. 
For this easy approach to work, the note 
time table element size 
must be packed to its minimum possible size. 
------------------------------------------------*/

#pragma pack(1)

typedef struct 
{
	TIME time ;              /* absolute time of event (element) */
	INT16 reference ;        /* pointer to corresponding note on/off event */
	unsigned char dynamic ;  /* note on dynamic or note off dynamic */
	NOTE note ;              /* bit7 = 0 note off; bit7 = 1 note on */
	unsigned char channel ;  /* origination channel of event */ 
} 
NOTE_TIME_TABLE_ELEMENT ;

#pragma pack()

typedef struct
{
	UINT16 channels ;	/* mask indicating non-empty channels */
	NOTE min_note, max_note ;	/* lowest and highest note of the track  */

	/*** MIDI meta events: ***/
	
	int sequence_number ;	/* of format 0 or 1 file, number of sequence in format 2 file, or -1 */
	
	char *text ;			/* only the 1. text event found */
	char *copyright ;		/* should occur in 1. track only */
	char *track_name ;		/* or sequence name if in format 0 file or in the 1. track */
	char *instrument_name ;	/* per track, possible at different times ? */
	char *lyric ;			/* only the 1. lyric event found */
	char *marker ;			/* only the 1. text marker found */
	char *cue_point ;		/* only the 1. description found */
	char *program_name ;	/* only the 1. program name found */
	char *device_name ;		/* device to be used for track */

	long tempo ;			/* in microseconds, only the 1. found, or -1 */
	/* SMPTE offset not supported here */

	/* time signature: */
	int numerator ;			/* e.g. 6 for 6/8, or -1 */
	int denominator ;		/* power of 2 already expanded, e.g. 3 in file => 2^3 = 8 here, or -1 */
	/* metronome clicks and ??? not supported */

	/* key signature */
	int key ;		/* already chromatic: 0..11, 0 is C, or -1 */
	int minor ;	/* not zero if minor, or -1 */
}
TRACK_INFO ;


void init_track_info(TRACK_INFO *track_info) ;
	/* init nearly all values to unknow (NULL or -1) */

void free_track_info(TRACK_INFO *track_info) ;
	/* frees the allocated char strings */

unsigned int fill_note_time_table
(
	const unsigned char     *track_address,      /* IN: pointer to a track 
                                                                   in a MIDI file */
	INT32                   track_length,        /* IN: in bytes */
	NOTE_TIME_TABLE_ELEMENT *first_element,      /* OUT: note time table */
	INT32                   *elements_generated, /* OUT: elems. filled into note time table */
	TRACK_INFO              *track_info          /* OUT: see above */
) ;
	/*  Transforms the data of a MIDI standard file into an array for 
	    faster processing. */
	/*  RETURN: */
	#define note_time_table_filled         10
	#define wrong_event_found              11
	#define data_without_status_given      12
	#define next_data_overflow             13
	#define no_var_length_number           14


NOTE_TIME_TABLE_ELEMENT *get_element
(
	NOTE_TIME_TABLE_ELEMENT *first_element, /* IN: start position */
	UINT32 number,         /* IN: number of elements after
	                          first element to inspect */
	TIME   time            /* IN: nominal time of element */
) ;
	/* Searches the one element in a note time table
	   which possesses a time that is as near as possible 
	   to the input time and higher than the input time.
	   The first element with this fewest possible or equal time
	   is returned if more elements have the same time.

	RETURN:  pointer to the element found, or NULL (if only 
	         elements with lower times exist in the specified range)
	*/	

#endif // include blocker
