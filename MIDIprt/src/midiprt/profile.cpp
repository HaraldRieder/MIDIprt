/*****************************************************************************
  Purpose:     Functions for loading and saving of profiles.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: PROFILE.C,v 1.13 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphic.h>
#include <graphicf.h>
#include <portable.h>
#include <diskfile.h>
#include "profile.h"
#include "common.h"

static char empty[] = "" ;
static char on[]    = "on" ;
static char off[]   = "off" ;

void default_doc_params(PROFILED_DOC_PARAMS *p)
{
	p->page = 1 ;
	p->zoom = MEAN_ZOOM ;
	p->has_toolbar = p->has_statusbar = p->is_open = TRUE ;
	p->left  = p->top    = 100 ; /* 0..1000 */
	p->right = p->bottom = 900 ; 
}

void profile_path(char *profile, const char *path)
	/* Constructs a profile path by substituting the last 
	   character in path with $ and writes it to profile. */
{
	strcpy(profile, path) ;
	profile[ strlen(profile) - 1 ] = '$' ;
}

int write_profile
	/* writes human readable profile */
	/* returns 0 if good */
(
	const char * filename,
	const char * apppath,	/* with slash at the end */
	const PARAMS_DB * params,
	const FILTER_DB * filter,
	const PROFILED_DOC_PARAMS * doc_params,
	const int dflt	/* write default profile */
) 
{
	unsigned i ;
	const char *s = 0 ;
	
	FILE *profile = fopen(filename, "w") ;
	if (!profile) 
		return -1 ;

	/* section "MidiPrt" */	
	fprintf(profile, "[ MidiPrt ]\n") ;
	fprintf(profile, "font = %s\n"           , params->font) ;
	fprintf(profile, "titleHeight = %d\n"    , (int)params->points) ;
	fprintf(profile, "barsPerSystem = %d\n"  , (int)params->bars_per_line) ;
	if (!dflt) fprintf(profile, "barLength = %d\n"  , (int)params->bar_length) ;
	if (!dflt) fprintf(profile, "subBarLevel = %d\n", (int)params->sub_bars -1) ;
	fprintf(profile, "leftBorder  = %d\n"    , (int)params->left_border) ;
	fprintf(profile, "rightBorder = %d\n"    , (int)params->right_border) ;
	fprintf(profile, "upperBorder = %d\n"    , (int)params->upper_border) ;
	fprintf(profile, "lowerBorder = %d\n"    , (int)params->lower_border) ;
	fprintf(profile, "systemDistance = %d\n" , (int)params->system_distance) ;
	fprintf(profile, "trackDistance  = %d\n" , (int)params->track_distance) ;
	fprintf(profile, "noteDistance   = %f\n" , (float)params->note_distance) ;
/*	fprintf(profile, "noteHeight =  %f\n"    , (float)params->note_height) ;*/
	fprintf(profile, "noteHeight =  %d\n"    , (int)params->note_height) ;
	fprintf(profile, "noteDynamic = %d\n"    , (int)params->note_dynscale) ;
	fprintf(profile, "horizontalLines = %d\n", (int)params->horizontal_lines) ;
	fprintf(profile, "lineWidthHorizontal = %s\n", params->line_width) ;
	fprintf(profile, "lineWidthBars       = %s\n", params->bar_line_width) ;
	fprintf(profile, "lineWidthSubBars    = %s\n", params->sub_bar_line_width) ;
	fprintf(profile, "lineWidthNotes      = %s\n", params->note_line_width) ;
	if (!dflt) fprintf(profile, "transpose = %d\n", (int)params->transpose) ;
	if (!dflt) fprintf(profile, "title = %s\n"    , params->title) ;

	i = (unsigned)strlen(apppath) ;
#if defined (_WINDOWS) 
	if ( strnicmp(apppath, params->scheme_path, i) )
#else
	if ( strncmp(apppath, params->scheme_path, i) )
#endif
		/* save with absolute path */
		i = 0 ;
	/* else: save relative to application path */
	fprintf(profile, "scheme = %s\n", params->scheme_path + i) ;

	i = FALSE ;
	fprintf(profile, "titleEffects = ") ;
	if (params->effects & TF_THICKENED)
	{
		i = TRUE ;
		fprintf(profile, "bold") ;
	}
	if (params->effects & TF_SLANTED)
	{
		if (i) fprintf(profile, ",") ;
		else i = TRUE ;
		fprintf(profile, "italic") ;
	}
	if (params->effects & TF_UNDERLINED)
	{
		if (i) fprintf(profile, ",") ;
		else i = TRUE ;
		fprintf(profile, "underlined") ;
	}
	fprintf(profile, "\n") ;

	switch (params->mode)
	{
	case Beyreuther: s = "Beyreuther" ; break ;
	case Rieder:     s = "Rieder"     ; break ;
	case Mix:        s = "Mix"        ; break ;
	}	
	fprintf(profile, "mode = %s\n", s) ;

	s = empty ;
	switch (params->note_type & HEAD_FLAGS)
	{
	case HEAD_LINE: s = "line"     ; break ;
	case HEAD_TRI:  s = "triangle" ; break ;
	case HEAD_DOT:  s = "circle"   ; break ;
	}
	fprintf(profile, "noteHead = %s\n", s) ;

	s = empty ;
	switch (params->note_type & BODY_FLAGS)
	{
	case BODY_RECT:  s = "rectangle" ; break ;
	case BODY_TRI:   s = "triangle"  ; break ;
	case BODY_ELLIP: s = "ellipse"   ; break ;
	}
	fprintf(profile, "noteBody = %s\n", s) ;

	s = empty ;
	switch (params->note_type & TAIL_FLAGS)
	{
	case TAIL_LINE: s = "line"     ; break ;
	case TAIL_TRI:  s = "triangle" ; break ;
	case TAIL_DOT:  s = "circle"   ; break ;
	}
	fprintf(profile, "noteTail = %s\n", s) ;

	s = empty ;
	if (params->note_type & BORDERS_NONE)
		s = "no" ;
	else if (params->note_type & BORDERS_3D)
		s = "3D" ;
	else
		s = "yes" ;	/* normal borders */
	fprintf(profile, "noteBorders = %s\n", s) ;
	
	if (!dflt)
	{
		/* track filters */
		fprintf(profile, "trackFilter = ") ;
		for (i = 0 ; i < filter->number_tracks ; i++)
		{
			if (filter->track[i].filter & 1) 
			     s = on  ;
			else s = off ;
			fprintf(profile, "%s", s) ;
			if (i != filter->number_tracks - 1)
				/* token separator */
				fprintf(profile, ",") ;
		}
		fprintf(profile, "\n") ;

		/* channel filters */
		fprintf(profile, "channelFilter = ") ;
		for (i = 0 ; i < filter->number_tracks ; i++)
		{
			fprintf(profile, "%x", (int)filter->track[i].ch_filter) ;
			if (i != filter->number_tracks - 1)
				/* token separator */
				fprintf(profile, ",") ;
		}
		fprintf(profile, "\n") ;
		
		/* track label selection */
		fprintf(profile, "trackLabel = ") ;
		for (i = 0 ; i < filter->number_tracks ; i++)
		{
			const char *s ;
			switch (filter->track[i].select)
			{
			case TR_DEVICE: s = "device"     ; break ;
			case TR_TEXT:   s = "text"       ; break ;
			case TR_INSTR:  s = "instrument" ; break ;
			default:        s = "name"       ;
			}
			fprintf(profile, "%s", s) ;
			if (i != filter->number_tracks - 1)
				/* token separator */
				fprintf(profile, ",") ;
		}
		fprintf(profile, "\n") ;
	}
	
	/* section "WindowMenu" */	
	fprintf(profile, "\n[ WindowMenu ]\n") ;
	fprintf(profile, "ToolBar   = %s\n", doc_params->has_toolbar  ?"yes":"no") ;
	fprintf(profile, "StatusBar = %s\n", doc_params->has_statusbar?"yes":"no") ;

	/* section "ToolBar" */	
	fprintf(profile, "\n[ ToolBar ]\n") ;
	fprintf(profile, "Zoom = %d\n", (int)doc_params->zoom) ;
	if (!dflt) fprintf(profile, "Page = %d\n", (int)doc_params->page) ;

	/* section "MainWindow" */	
	fprintf(profile, "\n[ MainWindow ]\n") ;
	fprintf(profile, "left   = %d\n", (int)doc_params->left  ) ;
	fprintf(profile, "top    = %d\n", (int)doc_params->top   ) ;
	fprintf(profile, "right  = %d\n", (int)doc_params->right ) ;
	fprintf(profile, "bottom = %d\n", (int)doc_params->bottom) ;
	if (!dflt) fprintf(profile, "open   = %s\n", doc_params->is_open ? "yes" : "no") ;
	
	return fclose(profile) ;
}


static int parse_mode(const char *s)
	/* parses string s for "Rieder", "Beyreuther", "Mix"
	   and returns corresponding mode as int */
{
  if      ( strpbrk(s, "yY") ) return Beyreuther ;
  else if ( strpbrk(s, "xX") ) return Mix ;
  return Rieder ;
}


static void parse_note_head(const char *s, int *note_type)
	/* parses string s for "line", "triangle", "circle"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~HEAD_FLAGS ;

  if      ( strpbrk(s, "cC") ) *note_type |= HEAD_DOT ;
  else if ( strpbrk(s, "tT") ) *note_type |= HEAD_TRI ;
  else if ( strpbrk(s, "nN") ) *note_type |= HEAD_LINE ;
}


static void parse_note_body(const char *s, int *note_type)
	/* parses string s for "line", "triangle", "circle"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~BODY_FLAGS ;

  if      ( strpbrk(s, "cC") ) *note_type |= BODY_RECT ;
  else if ( strpbrk(s, "tT") ) *note_type |= BODY_TRI ;
  else if ( strpbrk(s, "pP") ) *note_type |= BODY_ELLIP ;
}


static void parse_note_tail(const char *s, int *note_type)
	/* parses string s for "line", "triangle", "circle"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~TAIL_FLAGS ;

  if      ( strpbrk(s, "cC") ) *note_type |= TAIL_DOT ;
  else if ( strpbrk(s, "tT") ) *note_type |= TAIL_TRI ;
  else if ( strpbrk(s, "nN") ) *note_type |= TAIL_LINE ;
}


static void parse_note_borders(const char *s, int *note_type)
	/* parses string s for "no", "yes", "3D"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~BORDER_FLAGS ;	/* normal borders ("yes") */

  if      ( strchr (s, '3' ) ) *note_type |= BORDERS_3D ;
  else if ( strpbrk(s, "nN") ) *note_type |= BORDERS_NONE ;
}


static const char delimiter[] = "," ;

static void parse_title_effects(const char *s, int *effects)
{
  *effects = 0 ;
  if ( strpbrk(s, "bB") ) *effects |= TF_THICKENED ;  /* bold */
  if ( strpbrk(s, "cC") ) *effects |= TF_SLANTED ;    /* italic */
  if ( strpbrk(s, "uU") ) *effects |= TF_UNDERLINED ; /* underlined */
}

static void parse_track_filter(char *s, FILTER_DB *db)
	/* parses string s for comma separated "on" and "off"
	   and sets .filter correspondingly */
	/* if the number of hex values is smaller than the
	   the track size, the remaining elements remain unchanged */
{
	unsigned int i ;
	char *token = NULL ;
	
	for (i = 0 ; i < db->number_tracks ; i++, s = NULL)
	{
		token = strtok(s, delimiter) ;
		if ( !token )
			return ;
		/* parse token */
		if ( strpbrk(token, "nN") )
		     db->track[i].filter |= 1  ;
		else db->track[i].filter &= ~1 ; /* leave all other bits untouched */
	}
	/* the token list may be longer then the array size,
	   we have to read it till the end to clear any '\0'
	   inserted by strtok() */
	while (token)
		token = strtok(NULL, delimiter) ; 
}


static void parse_channel_filter(char *s, FILTER_DB *db)
	/* parses string s for comma separated "$XXXX" hex values
	   and sets .ch_filter correspondingly */
	/* if the number of hex values is smaller than the
	   the track size, the remaining elements remain unchanged */
{
	unsigned int i ;
	char *token = NULL ;
	
	for (i = 0 ; i < db->number_tracks ; i++, s = NULL)
	{
		token = strtok(s, delimiter) ;
		if ( !token )
			return ;
		/* parse token */
		sscanf(token, "%x", &(db->track[i].ch_filter)) ;
	}
	/* the token list may be longer then the array size,
	   we have to read it till the end to clear any '\0'
	   inserted by strtok() */
	while (token)
		token = strtok(NULL, delimiter) ; 
}


static void parse_track_label(char *s, FILTER_DB *db)
	/* parses string s for comma separated strings values
	   and sets .select correspondingly */
	/* if the number of hex values is smaller than the
	   the track size, the remaining elements remain unchanged */
{
	unsigned int i ;
	char *token = NULL ;
	
	for (i = 0 ; i < db->number_tracks ; i++, s = NULL)
	{
		token = strtok(s, delimiter) ;
		if ( !token )
			return ;
		/* parse token */
		if      ( strpbrk(token, "iI") ) db->track[i].select = TR_INSTR   ;
		else if ( strpbrk(token, "xX") ) db->track[i].select = TR_TEXT    ;
		else if ( strpbrk(token, "vV") ) db->track[i].select = TR_DEVICE  ;
		else                             db->track[i].select = TR_NAME    ;
	}
	/* the token list may be longer then the array size,
	   we have to read it till the end to clear any '\0'
	   inserted by strtok() */
	while (token)
		token = strtok(NULL, delimiter) ; 
}

static void parse_width(char *s, char *parsed)
{
	strncpy(parsed, s, LINE_WIDTH_STRLEN) ;
	parsed[LINE_WIDTH_STRLEN] = 0 ;
}

#define _STRING_(var) #var /* 1234 => "1234" */

int read_profile
	/* reads profile to PARAMS_DB and FILTER_DB */
	/* returns 0 if good */
(
	const char *filename,
	const char *apppath, 	/* with slash at the end */
	PARAMS_DB * params,
	FILTER_DB * filter,
	PROFILED_DOC_PARAMS * doc_params,
	const int dflt	/* read default profile */
) 
{
#define VAR_LEN  30  /* length of a variable name incl. " = " */
#define VAL_LEN  700 /* lenght of a variable value (longest case is a path) */
#define LINE_LEN 730 /* >= VAR_LEN + VAL_LEN */

	char line[LINE_LEN] ;
	char var [VAR_LEN ] ;
	char sect[VAR_LEN ] = "" ;
	char val [VAL_LEN ] ;
	
	FILE *profile = fopen(filename, "r") ;
	if (!profile)
		return -1 ;
		
	params->note_type = 0 ;  /* 3-part variable with flags for head, body, tail */
		
	while ( fgets(line, LINE_LEN-1, profile) )
	{
		char *c ;
		/* Not terminated with \n, because sscanf() would read over it 
		   into foreign memory because it is a white space char. */
		if (c = strchr(line, '\n'))
			*c = '#' ;
		/*line[strlen(line)-1] = '#' ;*/
		// _STRING_ macro does not work with MSVC++ 6.0
		char format[PATHNAME_LENGTH*2 + VAR_LEN*2] ;
		sprintf(format, "%%%ds = %%%d[^#]", VAR_LEN, PATHNAME_LENGTH) ;
		if ( sscanf(line, format, var, val) == 2 )
		{
			/* this is a variable = value line */
			if ( strstr(sect, "MidiPrt") )
			{
				if      (strstr(var, "font"           )) strcpy(params->font, val) ;
				else if (strstr(var, "titleHeight"    )) params->points           = atoi(val) ;
				else if (strstr(var, "titleEffects"   )) parse_title_effects(val, &(params->effects)) ;
				else if (strstr(var, "barsPerSystem"  )) params->bars_per_line    = atoi(val) ;
				else if (!dflt && strstr(var, "barLength"  )) params->bar_length  = atoi(val) ;
				else if (!dflt && strstr(var, "subBarLevel")) params->sub_bars    = atoi(val)+1 ;
				else if (strstr(var, "leftBorder"     )) params->left_border      = atoi(val) ;
				else if (strstr(var, "rightBorder"    )) params->right_border     = atoi(val) ;
				else if (strstr(var, "upperBorder"    )) params->upper_border     = atoi(val) ;
				else if (strstr(var, "lowerBorder"    )) params->lower_border     = atoi(val) ;
				else if (strstr(var, "systemDistance" )) params->system_distance  = atoi(val) ;
				else if (strstr(var, "trackDistance"  )) params->track_distance   = atoi(val) ;
				else if (strstr(var, "noteDistance"   )) params->note_distance    = atof(val) ;
/*				else if (strstr(var, "noteHeight"     )) params->note_height      = atof(val) ;*/
				else if (strstr(var, "noteHeight"     )) params->note_height      = atoi(val) ;
				else if (strstr(var, "noteDynamic"    )) params->note_dynscale    = atoi(val) ;
				else if (strstr(var, "horizontalLines")) params->horizontal_lines = atoi(val) ;
				else if (!dflt && strstr(var, "transpose")) params->transpose  = atoi(val) ;
				else if (!dflt && strstr(var, "title"    )) strcpy(params->title, val) ;
				else if (strstr(var, "scheme")) 
				{
					/* make path absolute */
/*					if (val[1] == ':' || val[0] == '\\')*/
						/* is aleady absolute */
/*						strcpy(params->scheme_path, val) ;
					else 
						sprintf(params->scheme_path, "%s%s", apppath, val) ;*/
					abspath(params->scheme_path, val, apppath) ;
						
				}
				else if (strstr(var, "mode"           )) params->mode = parse_mode(val) ;
				else if (strstr(var, "noteHead"       )) parse_note_head   (val, &(params->note_type)) ;
				else if (strstr(var, "noteBody"       )) parse_note_body   (val, &(params->note_type)) ;
				else if (strstr(var, "noteTail"       )) parse_note_tail   (val, &(params->note_type)) ;
				else if (strstr(var, "noteBorders"    )) parse_note_borders(val, &(params->note_type)) ;
				else if (strstr(var, "lineWidthHorizontal")) parse_width(val, params->line_width) ;
				else if (strstr(var, "lineWidthBars"      )) parse_width(val, params->bar_line_width) ;
				else if (strstr(var, "lineWidthSubBars"   )) parse_width(val, params->sub_bar_line_width) ;
				else if (strstr(var, "lineWidthNotes"     )) parse_width(val, params->note_line_width) ;
				else if (!dflt && strstr(var, "trackFilter"  )) parse_track_filter  (val, filter) ;
				else if (!dflt && strstr(var, "channelFilter")) parse_channel_filter(val, filter) ;
				else if (!dflt && strstr(var, "trackLabel"   )) parse_track_label   (val, filter) ;
			}
			else if ( strstr(sect, "WindowMenu") )
			{
				if      (strstr(var, "ToolBar"   )) doc_params->has_toolbar      = (strpbrk(val, "yY") != NULL) ;
				else if (strstr(var, "StatusBar" )) doc_params->has_statusbar    = (strpbrk(val, "yY") != NULL) ;
			}
			else if ( strstr(sect, "ToolBar") )
			{
				if      (strstr(var, "Zoom")) doc_params->zoom = atoi(val) ;
				else if (!dflt && strstr(var, "Page")) doc_params->page = atoi(val) ;
			}
			else if ( strstr(sect, "MainWindow") )
			{
				if      (strstr(var, "left"  )) doc_params->left   = atoi(val) ;
				else if (strstr(var, "right" )) doc_params->right  = atoi(val) ;
				else if (strstr(var, "top"   )) doc_params->top    = atoi(val) ;
				else if (strstr(var, "bottom")) doc_params->bottom = atoi(val) ;
				else if (!dflt && strstr(var, "open"  )) doc_params->is_open = (strpbrk(val, "yY") != NULL) ;
			}
		}
		else if ( sscanf(line, "[ %s ]", val) == 1 )
		{
			/* this is a section header */
			strcpy(sect, val) ;
		}
	}
	return fclose(profile) ;
}

