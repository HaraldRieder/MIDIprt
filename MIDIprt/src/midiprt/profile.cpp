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
#include <portable.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include "profile.h"
#include "common.h"

static const wxString on(_T("on"));
static const wxString off(_T("off"));
static const wxString yes(_T("yes"));
static const wxString no(_T("no"));

void default_doc_params(PROFILED_DOC_PARAMS & p)
{
	p.page = 1 ;
	p.zoom = MEAN_ZOOM ;
	p.has_toolbar = p.has_statusbar = p.is_open = TRUE ;
	p.left  = p.top    = 100 ; /* 0..1000 */
	p.right = p.bottom = 900 ; 
}

void profile_path(wxString & profile, const wxString & path)
	/* Constructs a profile path by substituting the last 
	   character in path with $ and writes it to profile. */
{
	profile = path;
    profile.Last() = wxUniChar('$');
}

int write_profile
	/* writes human readable profile */
	/* returns 0 if good */
(
	const wxString & filename,
	const wxString & apppath,	/* with slash at the end */
	const PARAMS_DB * params,
	const FILTER_DB * filter,
	const PROFILED_DOC_PARAMS & doc_params,
	const int dflt	/* write default profile */
) 
{
	unsigned i ;
	
	wxTextFile profile(filename);
    if (!profile.Exists()) 
        profile.Create();
	if (!profile.Open())
		return -1 ;
    // remove all lines
    profile.Clear();

	/* section "MidiPrt" */	
    profile.AddLine(_T("[ MidiPrt ]"));
    profile.AddLine(wxString(_T("font = ")).append(params->font));
    profile.AddLine(wxString(_T("titleHeight = ")).append(wxString::Format(_T("%d"),(int)params->points)));
    profile.AddLine(wxString(_T("barsPerSystem = ")).append(wxString::Format(_T("%d"),(int)params->bars_per_line)));
    if (!dflt) profile.AddLine(wxString(_T("barLength = ")).append(wxString::Format(_T("%d"),(int)params->bar_length)));
    if (!dflt) profile.AddLine(wxString(_T("subBarLevel = ")).append(wxString::Format(_T("%d"),(int)params->sub_bars -1)));
    profile.AddLine(wxString(_T("leftBorder = ")).append(wxString::Format(_T("%d"),(int)params->left_border)));
    profile.AddLine(wxString(_T("rightBorder = ")).append(wxString::Format(_T("%d"),(int)params->right_border)));
    profile.AddLine(wxString(_T("upperBorder = ")).append(wxString::Format(_T("%d"),(int)params->upper_border)));
    profile.AddLine(wxString(_T("lowerBorder = ")).append(wxString::Format(_T("%d"),(int)params->lower_border)));
    profile.AddLine(wxString(_T("systemDistance = ")).append(wxString::Format(_T("%d"),(int)params->system_distance)));
    profile.AddLine(wxString(_T("trackDistance = ")).append(wxString::Format(_T("%d"),(int)params->track_distance)));
    profile.AddLine(wxString(_T("noteDistance = ")).append(wxString::Format(_T("%d"),(int)params->note_distance)));
    profile.AddLine(wxString(_T("noteHeight = ")).append(wxString::Format(_T("%d"),(int)params->note_height)));
    profile.AddLine(wxString(_T("noteDynamic = ")).append(wxString::Format(_T("%d"),(int)params->note_dynscale)));
    profile.AddLine(wxString(_T("horizontalLines = ")).append(wxString::Format(_T("%d"),(int)params->note_height)));
    profile.AddLine(wxString(_T("lineWidthHorizontal = ")).append(params->line_width));
    profile.AddLine(wxString(_T("lineWidthBars = ")).append(params->bar_line_width));
    profile.AddLine(wxString(_T("lineWidthSubBars = ")).append(params->sub_bar_line_width));
    profile.AddLine(wxString(_T("lineWidthNotes = ")).append(params->note_line_width));
    profile.AddLine(wxString(_T("noteHeight = ")).append(wxString::Format(_T("%d"),(int)params->note_height)));
    if (!dflt) profile.AddLine(wxString(_T("transpose = ")).append(wxString::Format(_T("%d"),(int)params->transpose)));
    if (!dflt) profile.AddLine(wxString(_T("title = ")).append(params->title));

    wxString path;
    if ( !params->scheme_path.StartsWith(apppath, &path) )
        path = params->scheme_path; /* save with absolute path */
	/* else: save relative to application path */
    profile.AddLine(wxString(_T("scheme = ")).append(path));

	i = FALSE ;
	wxString line(_T("titleEffects = "));
	if (params->effects & TF_THICKENED)
	{
		i = TRUE ;
		line.append(_T("bold"));
	}
	if (params->effects & TF_SLANTED)
	{
		if (i) line.append(_T(","));
		else i = TRUE ;
		line.append(_T("italic"));
	}
	if (params->effects & TF_UNDERLINED)
	{
		if (i) line.append(_T(","));
		else i = TRUE ;
		line.append(_T("underlined"));
	}
    profile.AddLine(line);

    line = _T("mode = ");
	switch (params->mode)
	{
	case Beyreuther: line.append(_T("Beyreuther")) ; break ;
	case Rieder:     line.append(_T("Rieder"))     ; break ;
	case Mix:        line.append(_T("Mix"))        ; break ;
	}
    profile.AddLine(line);

    line = _T("noteHead = ");
	switch (params->note_type & HEAD_FLAGS)
	{
	case HEAD_LINE: line.append(_T("line"))     ; break ;
	case HEAD_TRI:  line.append(_T("triangle")) ; break ;
	case HEAD_DOT:  line.append(_T("circle"))   ; break ;
	}
    profile.AddLine(line);

    line = _T("noteBody = ");
	switch (params->note_type & BODY_FLAGS)
	{
	case BODY_RECT:  line.append(_T("rectangle")) ; break ;
	case BODY_TRI:   line.append(_T("triangle"))  ; break ;
	case BODY_ELLIP: line.append(_T("ellipse"))   ; break ;
	}
    profile.AddLine(line);

	line = _T("noteTail = ");
	switch (params->note_type & TAIL_FLAGS)
	{
	case TAIL_LINE: line.append(_T("line"))     ; break ;
	case TAIL_TRI:  line.append(_T("triangle")) ; break ;
	case TAIL_DOT:  line.append(_T("circle"))   ; break ;
	}
    profile.AddLine(line);

	line = _T("noteBorders = ");
	if (params->note_type & BORDERS_NONE)
		line.append(_T("no")) ;
	else if (params->note_type & BORDERS_3D)
		line.append(_T("3D")) ;
	else
		line.append(_T("yes")) ;	/* normal borders */
    profile.AddLine(line);
	
	if (!dflt)
	{
		/* track filters */
        line = _T("trackFilter = ");
		for (i = 0 ; i < filter->track.size() ; i++)
		{
            line.append(filter->track[i].filter & 1 ? on : off);
			if (i != filter->track.size() - 1)
				/* token separator */
				line.append(_T(","));
		}
        profile.AddLine(line);

		/* channel filters */
        line = _T("channelFilter = ");
		for (i = 0 ; i < filter->track.size() ; i++)
		{
            line.append(wxString::Format(_T("%x"),(int)filter->track[i].ch_filter));
			if (i != filter->track.size() - 1)
				/* token separator */
				line.append(_T(","));
		}
        profile.AddLine(line);
		
		/* track label selection */
        line = _T("trackLabel = ");
		for (i = 0 ; i < filter->track.size() ; i++)
		{
			switch (filter->track[i].select)
			{
			case TR_DEVICE: line.append(_T("device"))     ; break ;
			case TR_TEXT:   line.append(_T("text"))       ; break ;
			case TR_INSTR:  line.append(_T("instrument")) ; break ;
			default:        line.append(_T("name"))       ;
			}
			if (i != filter->track.size() - 1)
				/* token separator */
				line.append(_T(","));
		}
        profile.AddLine(line);
	}
	
	/* section "WindowMenu" */	
    profile.AddLine(_T(""));
	profile.AddLine(_T("[ WindowMenu ]"));
	profile.AddLine(wxString(_T("ToolBar   = ")).append(doc_params.has_toolbar  ? yes : no ));
	profile.AddLine(wxString(_T("StatusBar = ")).append(doc_params.has_statusbar  ? yes : no ));

	/* section "ToolBar" */	
    profile.AddLine(_T(""));
	profile.AddLine(_T("[ ToolBar ]"));
    profile.AddLine(wxString(_T("Zoom = ")).append(wxString::Format(_T("%d"),(int)doc_params.zoom)));
    if (!dflt) profile.AddLine(wxString(_T("Page = ")).append(wxString::Format(_T("%d"),(int)doc_params.page)));

	/* section "MainWindow" */	
    profile.AddLine(_T(""));
	profile.AddLine(_T("[ MainWindow ]"));
    profile.AddLine(wxString(_T("left   = ")).append(wxString::Format(_T("%d"),(int)doc_params.left)));
    profile.AddLine(wxString(_T("top    = ")).append(wxString::Format(_T("%d"),(int)doc_params.top)));
    profile.AddLine(wxString(_T("right  = ")).append(wxString::Format(_T("%d"),(int)doc_params.right)));
    profile.AddLine(wxString(_T("bottom = ")).append(wxString::Format(_T("%d"),(int)doc_params.bottom)));
	if (!dflt) profile.AddLine(wxString(_T("open   = ")).append(doc_params.is_open  ? yes : no ));
    
    return 0;
}


static int parse_mode(const wxChar *s)
	/* parses string s for "Rieder", "Beyreuther", "Mix"
	   and returns corresponding mode as int */
{
  if      ( wxStrpbrk(s, _T("yY")) ) return Beyreuther ;
  else if ( wxStrpbrk(s, _T("xX")) ) return Mix ;
  return Rieder ;
}


static void parse_note_head(const wxChar *s, int *note_type)
	/* parses string s for "line", "triangle", "circle"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~HEAD_FLAGS ;

  if      ( wxStrpbrk(s, _T("cC")) ) *note_type |= HEAD_DOT ;
  else if ( wxStrpbrk(s, _T("tT")) ) *note_type |= HEAD_TRI ;
  else if ( wxStrpbrk(s, _T("nN")) ) *note_type |= HEAD_LINE ;
}


static void parse_note_body(const wxChar *s, int *note_type)
	/* parses string s for "line", "triangle", "circle"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~BODY_FLAGS ;

  if      ( wxStrpbrk(s, _T("cC")) ) *note_type |= BODY_RECT ;
  else if ( wxStrpbrk(s, _T("tT")) ) *note_type |= BODY_TRI ;
  else if ( wxStrpbrk(s, _T("pP")) ) *note_type |= BODY_ELLIP ;
}


static void parse_note_tail(const wxChar *s, int *note_type)
	/* parses string s for "line", "triangle", "circle"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~TAIL_FLAGS ;

  if      ( wxStrpbrk(s, _T("cC")) ) *note_type |= TAIL_DOT ;
  else if ( wxStrpbrk(s, _T("tT")) ) *note_type |= TAIL_TRI ;
  else if ( wxStrpbrk(s, _T("nN")) ) *note_type |= TAIL_LINE ;
}


static void parse_note_borders(const wxChar *s, int *note_type)
	/* parses string s for "no", "yes", "3D"
	   and sets/resets the corresponding flags in note_type */
{
  /* first clear all head flags */
  *note_type &= ~BORDER_FLAGS ;	/* normal borders ("yes") */

  if      ( wxStrpbrk(s, _T("3"))  ) *note_type |= BORDERS_3D ;
  else if ( wxStrpbrk(s, _T("nN")) ) *note_type |= BORDERS_NONE ;
}


static const wxChar delimiter[] = _T(",") ;

static void parse_title_effects(const wxChar *s, int *effects)
{
  *effects = 0 ;
  if ( wxStrpbrk(s, _T("bB")) ) *effects |= TF_THICKENED ;  /* bold */
  if ( wxStrpbrk(s, _T("cC")) ) *effects |= TF_SLANTED ;    /* italic */
  if ( wxStrpbrk(s, _T("uU")) ) *effects |= TF_UNDERLINED ; /* underlined */
}

static void parse_track_filter(wxChar *s, FILTER_DB *db)
	/* parses string s for comma separated "on" and "off"
	   and sets .filter correspondingly */
	/* if the number of hex values is smaller than the
	   the track size, the remaining elements remain unchanged */
{
	unsigned int i ;
    wxStringTokenizer t(s, delimiter);
	for (i = 0 ; i < db->track.size() && t.HasMoreTokens() ; i++)
	{
		const wxChar * token = t.GetNextToken().wc_str();
		/* parse token */
		if ( wxStrpbrk(token, _T("nN")) )
		     db->track[i].filter |= 1  ;
		else db->track[i].filter &= ~1 ; /* leave all other bits untouched */
	}
}


static void parse_channel_filter(wxChar *s, FILTER_DB *db)
	/* parses string s for comma separated "$XXXX" hex values
	   and sets .ch_filter correspondingly */
	/* if the number of hex values is smaller than the
	   the track size, the remaining elements remain unchanged */
{
    wxStringTokenizer t(s, delimiter);
	for (unsigned i = 0 ; i < db->track.size() && t.HasMoreTokens() ; i++)
	{
		const wxChar * token = t.GetNextToken().wc_str();
		/* parse token */
		wxSscanf(token, _T("%x"), &(db->track[i].ch_filter)) ;
	}
}


static void parse_track_label(wxChar *s, FILTER_DB *db)
	/* parses string s for comma separated string values
	   and sets .select correspondingly */
	/* if the number of hex values is smaller than the
	   the track size, the remaining elements remain unchanged */
{
    wxStringTokenizer t(s, delimiter);
	for (unsigned i = 0 ; i < db->track.size() && t.HasMoreTokens() ; i++)
	{
		const wxChar * token = t.GetNextToken().wc_str();
		/* parse token */
		if      ( wxStrpbrk(token, _T("iI")) ) db->track[i].select = TR_INSTR   ;
		else if ( wxStrpbrk(token, _T("xX")) ) db->track[i].select = TR_TEXT    ;
		else if ( wxStrpbrk(token, _T("vV")) ) db->track[i].select = TR_DEVICE  ;
		else                                   db->track[i].select = TR_NAME    ;
	}
}

#define _STRING_(var) #var /* 1234 => "1234" */

int read_profile
	/* reads profile to PARAMS_DB and FILTER_DB */
	/* returns 0 if good */
(
	const wxString & filename,
	const wxString & apppath, 	/* with slash at the end */
	PARAMS_DB * params,
	FILTER_DB * filter,
	PROFILED_DOC_PARAMS & doc_params,
	const int dflt	/* read default profile */
) 
{
#define VAR_LEN  30  /* length of a variable name incl. " = " */
#define VAL_LEN  700 /* lenght of a variable value (longest case is a path) */

	wxChar var [VAR_LEN ] ;
	wxChar val [VAL_LEN ] ;
	wxString sect;
	
  	wxTextFile profile(filename);
	if (!profile.Open())
		return -1 ;
		
	params->note_type = 0 ;  /* 3-part variable with flags for head, body, tail */

    wxString line;
    for ( line = profile.GetFirstLine(); !profile.Eof(); line = profile.GetNextLine() )
    {
        if ( wxSscanf(line, _T("%s = %s"), var, val) == 2 )
		{
			/* this is a variable = value line */
			if ( sect.Contains(_T("MidiPrt")) )
			{
				if      (wxStrstr(var, _T("font")           )) params->font = val;
				else if (wxStrstr(var, _T("titleHeight")    )) params->points           = wxAtoi(val) ;
				else if (wxStrstr(var, _T("titleEffects")   )) parse_title_effects(val, &(params->effects)) ;
				else if (wxStrstr(var, _T("barsPerSystem")  )) params->bars_per_line    = wxAtoi(val) ;
				else if (!dflt && wxStrstr(var, _T("barLength")  )) params->bar_length  = wxAtoi(val) ;
				else if (!dflt && wxStrstr(var, _T("subBarLevel"))) params->sub_bars    = wxAtoi(val)+1 ;
				else if (wxStrstr(var, _T("leftBorder")     )) params->left_border      = wxAtoi(val) ;
				else if (wxStrstr(var, _T("rightBorder")    )) params->right_border     = wxAtoi(val) ;
				else if (wxStrstr(var, _T("upperBorder")    )) params->upper_border     = wxAtoi(val) ;
				else if (wxStrstr(var, _T("lowerBorder")    )) params->lower_border     = wxAtoi(val) ;
				else if (wxStrstr(var, _T("systemDistance") )) params->system_distance  = wxAtoi(val) ;
				else if (wxStrstr(var, _T("trackDistance")  )) params->track_distance   = wxAtoi(val) ;
				else if (wxStrstr(var, _T("noteDistance")   )) params->note_distance    = wxAtoi(val) ;
				else if (wxStrstr(var, _T("noteHeight")     )) params->note_height      = wxAtoi(val) ;
				else if (wxStrstr(var, _T("noteDynamic")    )) params->note_dynscale    = wxAtoi(val) ;
				else if (wxStrstr(var, _T("horizontalLines"))) params->horizontal_lines = wxAtoi(val) ;
				else if (!dflt && wxStrstr(var, _T("transpose"))) params->transpose  = wxAtoi(val) ;
				else if (!dflt && wxStrstr(var, _T("title")    )) params->title = val;
				else if (wxStrstr(var, _T("scheme")))
				{
					/* make path absolute */
					abspath(params->scheme_path, val, apppath) ;
				}
				else if (wxStrstr(var, _T("mode")           )) params->mode = parse_mode(val) ;
				else if (wxStrstr(var, _T("noteHead")       )) parse_note_head   (val, &(params->note_type)) ;
				else if (wxStrstr(var, _T("noteBody")       )) parse_note_body   (val, &(params->note_type)) ;
				else if (wxStrstr(var, _T("noteTail")       )) parse_note_tail   (val, &(params->note_type)) ;
				else if (wxStrstr(var, _T("noteBorders")    )) parse_note_borders(val, &(params->note_type)) ;
				else if (wxStrstr(var, _T("lineWidthHorizontal"))) params->line_width = val;
				else if (wxStrstr(var, _T("lineWidthBars")      )) params->bar_line_width = val;
				else if (wxStrstr(var, _T("lineWidthSubBars")   )) params->sub_bar_line_width = val;
				else if (wxStrstr(var, _T("lineWidthNotes")     )) params->note_line_width = val;
				else if (!dflt && wxStrstr(var, _T("trackFilter")  )) parse_track_filter  (val, filter) ;
				else if (!dflt && wxStrstr(var, _T("channelFilter"))) parse_channel_filter(val, filter) ;
				else if (!dflt && wxStrstr(var, _T("trackLabel")   )) parse_track_label   (val, filter) ;
			}
			else if ( sect.Contains(_T("WindowMenu")) )
			{
				if      (wxStrstr(var, _T("ToolBar")   )) doc_params.has_toolbar      = (wxStrpbrk(val, _T("yY")) != NULL) ;
				else if (wxStrstr(var, _T("StatusBar") )) doc_params.has_statusbar    = (wxStrpbrk(val, _T("yY")) != NULL) ;
			}
			else if ( sect.Contains(_T("ToolBar")) )
			{
				if      (wxStrstr(var, _T("Zoom"))) doc_params.zoom = wxAtoi(val) ;
				else if (!dflt && wxStrstr(var, _T("Page"))) doc_params.page = wxAtoi(val) ;
			}
			else if ( sect.Contains(_T("MainWindow")) )
			{
				if      (wxStrstr(var, _T("left")  )) doc_params.left   = wxAtoi(val) ;
				else if (wxStrstr(var, _T("right") )) doc_params.right  = wxAtoi(val) ;
				else if (wxStrstr(var, _T("top")   )) doc_params.top    = wxAtoi(val) ;
				else if (wxStrstr(var, _T("bottom"))) doc_params.bottom = wxAtoi(val) ;
				else if (!dflt && wxStrstr(var, _T("open")  )) doc_params.is_open = (wxStrpbrk(val, _T("yY")) != NULL) ;
			}
		}
		else if ( wxSscanf(line, "[ %s ]", val) == 1 )
		{
			/* this is a section header */
			sect = val;
		}
	}
    return 0;
}

