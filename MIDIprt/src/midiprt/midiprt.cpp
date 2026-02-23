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
	rect.x = std::min(drect.width  - 32, std::max(-32, rect.x)) ;
	rect.y = std::min(drect.height - 32, std::max(  0, rect.y)) ;
	rect.width = rect.height = -1 ;
	wi->SetSize(rect) ;
	if (open)
		wi->Show() ;
}

static void save_window_pos(wxTopLevelWindow *wi, const wxString & name)
	/* write left and top of wi in relative units 0..1000
	   to config, 1000 corresponds to desktop width/height */
{
	if (wi == NULL)
		return ;
	wxDisplay display;
	wxRect drect = display.GetGeometry();
	wxPoint pos = wi->GetPosition() ;
	float left   = (float)pos.x * 1000 / drect.width + 0.5 ;
	float top    = (float)pos.y * 1000 / drect.height + 0.5 ;
    wxConfig::Get()->Write(wxString(_T("left")).append(name), (long)left) ;
    wxConfig::Get()->Write(wxString(_T("top")).append(name), (long)top) ;
    wxConfig::Get()->Write(wxString(_T("open")).append(name), wi->IsShown() ? _T("yes") : _T("no")) ;
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
					wxMessageDialog err(NULL, msg, _T("Error"), wxICON_ERROR | wxCANCEL);
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
    wxConfig::Set(new wxConfig("MIDI_PRT.CFG"));
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

	/* read config */
    wxConfigBase * config = wxConfig::Get();
    bool info_open = config->ReadBool(_T("openInfo"), false);
    bool filter_open = config->ReadBool(_T("openFilter"), false);
    bool params_open = config->ReadBool(_T("openParams"), false);
    wxPoint info_pos, filter_pos, params_pos ;
    info_pos.x = config->ReadLong(_T("leftInfo"), 0);
    info_pos.y = config->ReadLong(_T("topInfo"), 0);
    filter_pos.x = config->ReadLong(_T("leftFilter"), 0);
    filter_pos.y = config->ReadLong(_T("topFilter"), 0);
    params_pos.x = config->ReadLong(_T("leftParams"), 0);
    params_pos.y = config->ReadLong(_T("topParams"), 0);
    options.format = config->Read(_T("Format"), wxString()).Contains(_T("4")) ? 
        PROFILED_OPTIONS::FORMAT_DIN_A4 : PROFILED_OPTIONS::FORMAT_US_LETTER;
	// this option affects main frames already created
	MFPMainFrame::update_options_all() ;

	bool have_main_frame = false ;
    wxString lastfile = config->Read(_T("LastFile"), wxString());
    if ( !lastfile.empty() ) {
        new MFPMainFrame(lastfile) ; 
		have_main_frame = true ;
    } 
		
	open_window_on_pos(info_window  , info_pos  , info_open  ) ;
	open_window_on_pos(filter_window, filter_pos, filter_open) ;
	open_window_on_pos(params_window, params_pos, params_open) ;

	if (argc <= 1)
	{
		if (!have_main_frame)
			// guarantee that there is at least 1 document window
			new MFPMainFrame(wxString());
	}
	else for (int i = 1 ; i < argc ; i++)
		new MFPMainFrame(wxString(argv[i]));

	return true;
}

int MidiFilePrinterApplication::OnExit()
{
    /* save options */
    wxConfig::Get()->Write(_T("Format"), options.format == PROFILED_OPTIONS::FORMAT_DIN_A4 ? _T("DIN A4") : _T("US letter"));
    save_window_pos(info_window  , _T("Info")) ;
    save_window_pos(params_window, _T("Params")) ;
    save_window_pos(filter_window, _T("Filter")) ;
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
