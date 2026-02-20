/*****************************************************************************
  Purpose:     Declares functions common to the platforms (Atari TOS and
               wxWidgets) that don't belong to any other of the common 
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: COMMON.H,v 1.12 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INCL_COMMON
#define INCL_COMMON

#include <vector>
#include <notimtab.h>
#include "draw.h"
#include "trcktabl.h"
#include "infodb.h"
#include "paramsdb.h"
#include "filterdb.h"
#include "profile.h"

#define UP_DOWN_SCROLL_PIXELS 20
#define HELP_DIRECTORY "HELP"
#define SCHEMES_DIRECTORY "SCHEMES"

#if defined (_WINDOWS) 
#  define DIRSEP '\\'
#else
#  define DIRSEP '/'
#endif

/************* DATA PROTOTYPE ***********************/

typedef struct 
{
	wxString filename, pathname, profile;
	void *RAM_file ;
	long filesize ;
	int has_profile ;

	std::vector<TRACK> track_table;
	LAYOUT layout ;
	PROFILED_DOC_PARAMS opts ;

	INFO_DB    info  ; /* database of the info window */
	FILTER_DB  filter; /* etc. */
	PARAMS_DB  params;

/* unused ?	int all_tracks_off ;*/ /* all track filters are off => nothing to draw */
} 
DOCUMENT ;
 

int mm_to_pixel(const wxString & mm, int pixel_size) ;
	/* convert number of millimeters to number of pixels */
	/* pixel_size must be in micrometers */
	/* input must be a 3-char. string like "0.5" or "1.2" */

const wxString make_title(
	const wxString & title,		/* as loaded from profile */
	const wxString & path		/* of MIDI file */
	) ;
	/* returns title, if title is not NULL and not empty,
	   returns the filename else */

TIME time_per_system(int ticks_per_quarter, int eighths_per_bar, int bars_per_line) ;
	/* returns time in ticks per system (= per line) */

void init_info_from_tracks(INFO_DB *db, const std::vector<TRACK> & track_table);

void init_params_from_tracks(PARAMS_DB *db, const std::vector<TRACK> & track_table);

void init_filter_from_tracks(FILTER_DB *db, const std::vector<TRACK> & track_table);
	/* 1. sets track filter of non-empty tracks to 1, the other ones to DISABLED. */
	/* 2. sets channel filter according to non-empty channels in track. */

TIME get_max_time( 
	const std::vector<TRACK> & track_table, /* in: track table */ 
	const std::vector<FILTERED_TRACK> & ft  /* in: only take into account if filter is on */
) ;
	/* search the highest time of all events of all enabled tracks */

void relpath(wxString & absolute, const wxString & path, const wxString & dir) ;
    /* remove dir from path if contained therein and store into relative */
	   
void abspath(wxString & absolute, const wxString & path, const wxString & dir) ;
    /* prefix path with dir, if path is not absolute, and
       store into absolute. dir must end with a slash. */ 
	   

#endif // include blocker
