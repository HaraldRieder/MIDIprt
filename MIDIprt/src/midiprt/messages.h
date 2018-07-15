/*****************************************************************************
  Purpose:     Messages between ACS windows (Atari TOS only).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: MESSAGES.H,v 1.3 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

/*** messages, 25000..25010 reserved for MIDI FILE PRINTER ***/

#define SET_DATABASE        25000
#define INVALIDATE_DATABASE 25001
#define DATA_CHANGED        25002
#define DATA_CHANGED_BUTTON 25003
#define DATA_CHANGED_SLIDER 25004
#define DATA_CHANGED_KEY    25005
#define IS_FILE_OPEN        25006
#define FILE_WAS_SAVED      25007

#define DO_REDRAW      25028
#define KEY_PRESS      25029
#define FORMAT_CHANGED 25032	/* paper format change => window format change */

/* message format for key pressure events */
typedef struct 
{ 
	Awindow *sender ; 
	int kstate ; 
	int key ; 
} KEY_MESSAGE ;

/* message format for IS_FILE_OPEN query */
typedef struct 
{ 
	char *query ; 
	int answer ; 
} QUERY_MESSAGE ;

static KEY_MESSAGE key_message ;