/*****************************************************************************
  Purpose:     Functions for loading and saving of profiles.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: PROFILE.H,v 1.7 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_PROFILE
#define INC_PROFILE

#include "paramsdb.h"
#include "filterdb.h"

/* flags for immediate update */
#define IMMEDIATE_SLID 0x1
#define IMMEDIATE_BTN  0x2
#define IMMEDIATE_KEYB 0x4

typedef struct 
{
	enum { FORMAT_DIN_A4 = 0, FORMAT_US_LETTER = 1 } format ;
	enum { ONE_PER_PAGE = 0, ALL_INTO_ONE = 1 } meta_print_mode ;
	enum { LOWER_CASE_MASKS = 0, UPPER_CASE_MASKS = 1 } file_selector ;
	int immediate_update ;
	/* ACS does not remember icon arrangement mode, so we have to: */
	enum { ICONS_VER = 0, ICONS_HOR = 1} icon_arrangement ;
	/* use WDIALOG or avoid it because it crashes */
	int use_WDIALOG ;
}
PROFILED_OPTIONS ; 


#  define MIN_ZOOM   33   /* minimum zoom in % */
#  define MEAN_ZOOM 100
#  define MAX_ZOOM  250   /* maximum zoom in % */

typedef struct 
{
	int page ;			/* page currently displayed, 1..n */
	int zoom ;			/* current zoom in % */
	int has_toolbar, has_statusbar, is_open ;
	int left, top, right, bottom ;  /* in pixels */	
}
PROFILED_DOC_PARAMS ; /* these are profiled per document (= MIDI file) */

void default_doc_params(PROFILED_DOC_PARAMS & p);

void profile_path(wxString & profile_path, const wxString & path) ;
	/* Constructs a profile path by substituting the last 
	   character in path with $ and writes it to profile_path. */

int write_profile
	/* writes human readable profile */
	/* returns 0 if good */
(
	const wxString & filename,
	const wxString & apppath,	/* path of application for relative scheme path */
	const PARAMS_DB *,
	const FILTER_DB *,
	const PROFILED_DOC_PARAMS &,
	const int dflt	/* write default profile */
) ;
	
int read_profile
	/* reads profile to PARAMS_DB and FILTER_DB */
	/* returns 0 if good */
(
	const wxString & filename,
	const wxString & apppath,	/* path of application for relative scheme path */
	PARAMS_DB *,
	FILTER_DB *,
	PROFILED_DOC_PARAMS &,
	const int dflt	/* read default profile */
) ;

#endif // include blocker
