/*****************************************************************************
  Purpose:     Data model for the filter window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: FILTERDB.H,v 1.6 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_FILTERDB_H
#define INC_FILTERDB_H 1

#include "draw.h"

#ifndef DISABLED
#	define DISABLED 0x8 /* e.g. visualize that track is empty */
#endif


typedef struct
{
	char *filename ;

	unsigned number_tracks ;   /* dimension for the following array */
	FILTERED_TRACK *track ;
	
	/* private: */	
	int current_track ;	
}
FILTER_DB ;

unsigned set_number_tracks(FILTER_DB *db, unsigned n) ;

#endif /* include blocker */
