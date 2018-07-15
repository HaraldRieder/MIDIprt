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
	char *filename;	/* for window info line */
	long filesize;
	long transformed_size;
	MIDI_FILE_HEADER midi_header ;
	char key  [6] ;	/* e.g. "0 maj" or "1 min" */
	char time [6] ;	/* e.g. "17/16" */
	char tempo[6] ; /* milliseconds */
	char *copyright ;
} 
INFO_DB ;

#endif /* include blocker */
