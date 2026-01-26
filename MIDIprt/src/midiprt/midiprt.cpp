/*****************************************************************************
  Purpose:     Implements Mad Harry's MIDI File Printer application.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: midiprt.cpp,v 1.4 2008/09/20 20:06:51 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <wx/display.h>
#include <wx/html/htmlwin.h>

#include "version.h"
#include "mainw.h"
#include "paramsw.h"
#include "filterw.h"
#include "infow.h"

#if !defined (_WINDOWS) && !defined (__PUREC__)
#  define _strdup strdup
#endif

wxString apppath ;
wxApp * app ;

PROFILED_OPTIONS options ;

/* currently keeps list of open files only */
FILE * config_file = NULL ;
static wxString config_filepath ;

/* This dummy is a workaround. Ununderstood, why IsShown() 
   always deliveres false for info_window, whereas 
   it works correctly for filter_window and params_window.
   The 3 windows are implemented nearly equal. 
   Suspicion: IsShown() does not work correctly 
   in first window created ?!? */
wxDialog * dummy ;

static void open_window_on_pos(wxWindow *wi, const wxPoint & pos, bool open) 
	/* opens window top = x, left = y */
	/* relative units 0..1000 are transformed back to pixels */
	/* if open is false, the window is not opened */
{
	if (wi == NULL)
		return ;
	wxDisplay display;
	wxRect drect = display.GetGeometry();
	wxRect rect ;
	rect.x = (int)((float)(pos.x) / 1000 * drect.width + 0.5) ;
	rect.y = (int)((float)(pos.y) / 1000 * drect.height + 0.5) ;
	rect.x = min(drect.width - 32, max(-32, rect.x)) ;
	rect.y = min(drect.height - 32, max(  0, rect.y)) ;
	rect.width = rect.height = -1 ;
	wi->SetSize(rect) ;
	if (open)
		wi->Show() ;
}

static void fwrite_window_pos(FILE *file, wxTopLevelWindow *wi, const char *name)
	/* write left and top of wi in relative units 0..1000
	   to file, 1000 corresponds to desktop width/height */
{
	if (wi == NULL)
		return ;
	wxDisplay display;
	wxRect drect = display.GetGeometry();
	wxPoint pos = wi->GetPosition() ;
	float left   = (float)pos.x * 1000 / drect.width + 0.5 ;
	float top    = (float)pos.y * 1000 / drect.height + 0.5 ;
	fprintf(file, "left%s = %i\n", name, (int)left) ;
	fprintf(file, "top%s  = %i\n", name, (int)top ) ;
	fprintf(file, "open%s = %s\n", name, wi->IsShown() ? "yes" : "no") ;
}



/** The MIDI File Printer application. */
class MidiFilePrinterApplication: public wxApp
{
public:
	~MidiFilePrinterApplication() ;
	/** init application: create frame and dialog windows */
    bool OnInit();
	/** exit application: save application settings */
    virtual int OnExit();
private:
	/** 
	* Treats some key events handled commonly for all
	* windows together.
	*/
	virtual int FilterEvent(wxEvent& event) ;
};


IMPLEMENT_APP(MidiFilePrinterApplication);


int MidiFilePrinterApplication::FilterEvent(wxEvent& event)
{
	if (event.GetEventType() == wxEVT_KEY_DOWN)
	{
		wxKeyEvent * key_evt = //dynamic_cast<wxKeyEvent *>(&event) ;
			(wxKeyEvent *)(&event) ;
		int code = key_evt->GetKeyCode() ;
		switch(code)
		{
		case WXK_HELP:
		case WXK_F1:
		case WXK_F11: // Atari Help on ST Emulator
			{
				wxFrame * frame = new wxFrame(NULL, -1, _T(CAPTION)) ;
				wxHtmlWindow * help = new wxHtmlWindow(frame) ;
				wxString helppath = apppath + _T(HELP_DIRECTORY)+_T(DIRSEP)+_T("INDEX.HTM") ;
				if (!help->LoadFile(helppath))
				{
					wxString msg ;
					msg.append(_T("Could not load\n")).append(helppath).append(_T(" !")) ;
					wxMessageDialog err(NULL, msg.c_str(),
                           _T("Error"), wxICON_ERROR | wxCANCEL);
					err.ShowModal();
				}
				else
					frame->Show(true) ;
			}			
			return true ; // event processed
		case WXK_F2:
			info_window->Show() ;
			info_window->Raise() ;
			return true ; // event processed
		case WXK_F3:
			params_window->Show() ;
			params_window->Raise() ;
			return true ; // event processed
		case WXK_F4:
			// Alt+F4 closes top window!
			if (!key_evt->AltDown())
			{
				filter_window->Show() ;
				filter_window->Raise() ;
				return true ; // event processed
			}
			return false ;
		case 'w': 
		case 'W':
			if (key_evt->ControlDown())
			{
				MFPMainFrame::RaiseNext() ;
				return true ;
			}
			break ;
		case 'q':
		case 'Q':
			if (key_evt->ControlDown())
			{
				MFPMainFrame::quit() ;
				return true ;
			}
		}
	}
	return wxApp::FilterEvent(event) ;
}


#define VAR_LEN  20  /* length of a variable name incl. " = " */
#define VAL_LEN  230 /* lenght of a variable value (longest case is a path) */

bool MidiFilePrinterApplication::OnInit(void)
{
	app = this ;
	// init application path from argv:
	// cut away application file name at the end
	apppath = argv[0] ;
	int pos = apppath.Find('\\', true) ;
	if (pos >= 0)
		apppath = apppath.SubString(0, pos) ;
	else
	{
		pos = apppath.Find('/', true) ;
		if (pos >= 0)
			apppath = apppath.SubString(0, pos) ;
	}
	/* This dummy is a workaround. Ununderstood, why IsShown() 
	   always deliveres false for info_window, whereas 
	   it works correctly for filter_window and params_window.
	   The 3 windows are implemented nearly equal. 
	   Suspicion: IsShown() does not work correctly 
	   for first window created ?!? (wxWindows 2.4.2) */
	dummy = new wxDialog(NULL, -1, wxString(_T(""))) ;

	info_window   = new MFPInfoWindow  (NULL) ;
	filter_window = new MFPFilterWindow(NULL) ;
	params_window = new MFPParamsWindow(NULL) ;

	/* construct config file path (might be in user's $HOME) */
	config_filepath = wxGetHomeDir() + _T(DIRSEP) + _T(".MIDI_PRT.CFG") ;

	/* parse config file */
	bool have_main_frame = false ;
	config_file = fopen(config_filepath.ToAscii(), "r") ;
	if (config_file)
	{
		char var [VAR_LEN] ;
		char val [VAL_LEN] ;
		wxPoint info_pos, filter_pos, params_pos ;
		bool info_open = false, filter_open = false, params_open = false ;
		char buffer [VAR_LEN*2 + VAL_LEN*2] ;
		
		while( fgets(buffer, (int)(sizeof(buffer)-1), config_file) ) 
		{
			char *c ;
			/* Not terminated with \n, because sscanf() would read over it 
			   into foreign memory because it is a white space char. */
			if (c = strchr(buffer, '\n'))
				*c = '#' ;
			char *dupbuffer = _strdup(buffer); // because of sscanf() error in Visual C++ 2008!
			char format[VAR_LEN*2 + VAL_LEN*2] ;
			sprintf(format, "%%%ds = %%%d[^#]", VAR_LEN, VAL_LEN) ;
			if ( sscanf(buffer, format, var, val) == 2 ) // destroys buffer content if not true!
			{
				if (strstr(var, "Format"         )) 
				{
					options.format = (strchr (val, '4' ) == NULL) ? PROFILED_OPTIONS::FORMAT_US_LETTER : PROFILED_OPTIONS::FORMAT_DIN_A4 ;
					// this option affects main frames already created
					MFPMainFrame::update_options_all() ;
				}
				//else if (strstr(var, "MetaPrintMode"  )) options.meta_print_mode = (strpbrk(val, "pP") == NULL) ;
				//else if (strstr(var, "FileSelectMasks")) options.file_selectoddr   = (strpbrk(val, "lL") == NULL) ;
				//else if (strstr(var, "FontPrintDialog")) options.use_WDIALOG     = (strpbrk(val, "nN") == NULL) ;
				//else if (strstr(var, "Icons"          )) options.icon_arrangement= (strpbrk(val, "vV") == NULL) ;
				//else if (strstr(var, "ImmediateUpdate")) parse_update(val, &(options.immediate_update)) ;
				else if (strstr(var, "leftInfo"  )) info_pos.x   = atoi(val) ;
				else if (strstr(var, "topInfo"   )) info_pos.y   = atoi(val) ;
				else if (strstr(var, "openInfo"  )) info_open    = (strpbrk(val, "yY") != NULL) ;
				else if (strstr(var, "leftFilter")) filter_pos.x = atoi(val) ;
				else if (strstr(var, "topFilter" )) filter_pos.y = atoi(val) ;
				else if (strstr(var, "openFilter")) filter_open  = (strpbrk(val, "yY") != NULL) ;
				else if (strstr(var, "leftParams")) params_pos.x = atoi(val) ;
				else if (strstr(var, "topParams" )) params_pos.y = atoi(val) ;
				else if (strstr(var, "openParams")) params_open  = (strpbrk(val, "yY") != NULL) ;
			}
			else if (argc <= 1)
			{
				/* interpret as filename ***/
				if (c = strchr(dupbuffer, '#'))
					*c = 0 ; /* normal string */
				new MFPMainFrame(dupbuffer) ; 
				have_main_frame = true ;
			}
			free(dupbuffer);
		}
		fclose(config_file) ;
		
		//if (options.icon_arrangement == ICONS_HOR)
			/* not the ACS default, tell it to ACS: */
		//	Awd_hor() ;

		open_window_on_pos(info_window  , info_pos  , info_open  ) ;
		open_window_on_pos(filter_window, filter_pos, filter_open) ;
		open_window_on_pos(params_window, params_pos, params_open) ;
	}

	if (argc <= 1)
	{
		if (!have_main_frame)
			// guarantee that there is at least 1 document window
			new MFPMainFrame(NULL);
	}
	else for (int i = 1 ; i < argc ; i++)
		new MFPMainFrame(wxString(argv[i]).ToAscii());

	/* now open for writing (see main frame window) */
	config_file = fopen(config_filepath.ToAscii(), "w") ;

	return true;
}

int MidiFilePrinterApplication::OnExit()
{
	if ( config_file )
	{
		/* save options */
		fprintf(config_file, "Format          = %s\n", (options.format          == PROFILED_OPTIONS::FORMAT_DIN_A4)?"DIN A4":"US letter") ;
		//fprintf(config_file, "MetaPrintMode   = %s\n", (options.meta_print_mode == ONE_PER_PAGE)?"one per page":"all into one") ;
		//fprintf(config_file, "FileSelectMasks = %s\n", (options.file_selector   == UPPER_CASE_MASKS)?"upper case":"lower case") ;
		//fprintf(config_file, "FontPrintDialog = %s\n", (options.use_WDIALOG)?"OS":"own") ;
		//fprintf(config_file, "Icons           = %s\n", (options.icon_arrangement== ICONS_VER)?"vertical":"horizontal") ;
		//fprintf(config_file, "ImmediateUpdate = ") ;
		//if (options.immediate_update & IMMEDIATE_SLID) fprintf(config_file, "sliders ") ;
		//if (options.immediate_update & IMMEDIATE_BTN ) fprintf(config_file, "buttons ") ;
		//if (options.immediate_update & IMMEDIATE_KEYB) fprintf(config_file, "keyboard ") ;
		//fprintf(config_file, "\n") ;
		fwrite_window_pos(config_file, info_window  , "Info") ;
		fwrite_window_pos(config_file, params_window, "Params") ;
		fwrite_window_pos(config_file, filter_window, "Filter") ;
		fclose(config_file) ;
	}
	if (info_window)   info_window->Destroy() ;
	if (params_window) params_window->Destroy() ;
	if (filter_window) filter_window->Destroy() ;

	if (dummy) dummy->Destroy() ;

	return 0 ;
}


MidiFilePrinterApplication::~MidiFilePrinterApplication()
{
/*	delete info_window ;
	delete params_window ;
	delete filter_window ;*/
}
