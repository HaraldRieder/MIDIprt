/*****************************************************************************
  Purpose:     Functions operating on the data model of the filter window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: FILTERDB.C,v 1.4 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <malloc.h>
#include "filterdb.h"

unsigned set_number_tracks(FILTER_DB *db, unsigned n) 
{
	db->current_track = 0 ;
	db->number_tracks = n ;
	
	if (db->track) 
		free(db->track) ;
	if (!n)
		return 1 ;	/* good */

	db->track = (FILTERED_TRACK *)calloc(n, sizeof(FILTERED_TRACK)) ;
	if ( !db->track )
	{
		set_number_tracks(db, 0) ;
		return 0 ;
	}
	return 1 ;	
}

