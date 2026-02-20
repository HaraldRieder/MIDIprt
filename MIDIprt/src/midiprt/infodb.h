/*****************************************************************************
  Purpose:     Data model for the info window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: INFODB.H,v 1.5 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_INFODB
#define INC_INFODB 1

#include <midi.h>

typedef struct
{
	wxString filename;	/* for window info line */
	long filesize;
	long transformed_size;
	MIDI_FILE_HEADER midi_header ;
	wxString key;	/* e.g. "0 maj" or "1 min" */
	wxString time;	/* e.g. "17/16" */
	wxString tempo; /* milliseconds */
	wxString copyright;
} 
INFO_DB ;

#endif /* include blocker */
