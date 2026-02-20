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

void set_number_tracks(FILTER_DB *db, unsigned n) 
{
	db->current_track = 0 ;
	db->track = std::vector<FILTERED_TRACK>(n, FILTERED_TRACK());
}

