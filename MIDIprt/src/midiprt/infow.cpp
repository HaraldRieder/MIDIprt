/*****************************************************************************
  Purpose:     Implements MIDI file information window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: infow.cpp,v 1.3 2008/09/20 20:06:51 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include  "commonw.h"
#include  <wx/ownerdrw.h>
#include  <wx/menuitem.h>
#include  <wx/checklst.h>

#include <graphicf.h>
#include <wxVDI.h>
#include "infow.h"

MFPInfoWindow * info_window ; // singleton window


BEGIN_EVENT_TABLE(MFPInfoWindow, wxDialog)
	EVT_CLOSE(MFPInfoWindow::OnCloseWindow)
END_EVENT_TABLE()


MFPInfoWindow::MFPInfoWindow(wxWindow *parent)
: wxDialog(parent, -1, _T(""), wxDefaultPosition, 
		   wxSize(wxButton::GetDefaultSize().y * 10, wxButton::GetDefaultSize().y * 15)
		   /*, wxDEFAULT_DIALOG_STYLE, wxDialogNameStr*/)
{
	db = NULL ;

    // set the icon
    SetIcon(wxIcon(_T("INFO")));

    // create panel for the controls
    int PANEL_W, PANEL_H ;
    DoGetClientSize(&PANEL_W, &PANEL_H) ;
    wxPanel *panel = new wxPanel(this, -1, wxPoint(0, 0), wxSize(PANEL_W, PANEL_H), wxTAB_TRAVERSAL);

	const int UPPER_H = PANEL_H * 3 / 15 ;
	const int LOWER_H = PANEL_H * 9 / 15 ;

	wxSize sz ; 
	const int box_w = PANEL_W - 2*MFP_SPACING ;

	// size 
	const int size_box_h = UPPER_H ;
    new wxStaticBox(panel,-1,_T(""), wxPoint(MFP_SPACING,MFP_SPACING),wxSize(box_w,size_box_h));
	sz.y = (size_box_h - 2*MFP_SPACING) / 2/*lines*/ ;
	sz.x = (PANEL_W/2) ;
	int x  = MFP_SPACING*2;
	int y0 = MFP_SPACING*5/2;
	new wxStaticText(panel, -1, _T("File size:")     , wxPoint(x,y0)     , sz) ; 
	new wxStaticText(panel, -1, _T("Size in memory:"), wxPoint(x,y0+sz.y), sz) ; 
	int x_k = MFP_SPACING + box_w * 4/5 ;
	sz.x = box_w/5 - MFP_SPACING ;
	new wxStaticText(panel, -1, _T("Byte"), wxPoint(x_k,y0)     , sz) ; 
	new wxStaticText(panel, -1, _T("Byte"), wxPoint(x_k,y0+sz.y), sz) ; 
	int x_val = MFP_SPACING + box_w / 2 ;
	m_file_size        = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0)     , sz) ; 
	m_transformed_size = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y), sz) ; 

	// infos from inside the MIDI file
	const int midi_box_h = LOWER_H - MFP_SPACING ;
    new wxStaticBox(panel,-1,_T(""), wxPoint(MFP_SPACING,UPPER_H+MFP_SPACING),wxSize(box_w, midi_box_h));
	sz.y = (midi_box_h - 2*MFP_SPACING) / 8/*lines*/ ;
	sz.x = (PANEL_W/2) ;
	y0 = UPPER_H+MFP_SPACING*5/2;
	new wxStaticText(panel, -1, _T("MIDI file type:") , wxPoint(x,y0)       , sz) ; 
	new wxStaticText(panel, -1, _T("No. of tracks:")  , wxPoint(x,y0+sz.y)  , sz) ; 
	new wxStaticText(panel, -1, _T("Ticks per beat:") , wxPoint(x,y0+sz.y*2), sz) ; 
	new wxStaticText(panel, -1, _T("SMPTE:")          , wxPoint(x,y0+sz.y*3), sz) ; 
	new wxStaticText(panel, -1, _T("Ticks per frame:"), wxPoint(x,y0+sz.y*4), sz) ; 
	new wxStaticText(panel, -1, _T("Initial tempo:")  , wxPoint(x,y0+sz.y*5), sz) ; 
	new wxStaticText(panel, -1, _T("Time signature:") , wxPoint(x,y0+sz.y*6), sz) ; 
	new wxStaticText(panel, -1, _T("Key signature:")  , wxPoint(x,y0+sz.y*7), sz) ; 
	x_val = MFP_SPACING + box_w * 3/4 ;
	sz.x = box_w/5 - MFP_SPACING ;
	m_midi_file_type   = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0), sz) ; 
	m_number_of_tracks = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y), sz) ; 
	m_ticks_per_beat   = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y*2), sz) ; 
	m_smpte            = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y*3), sz) ; 
	m_ticks_per_frame  = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y*4), sz) ; 
	m_tempo            = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y*5), sz) ; 
	m_time             = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y*6), sz) ; 
	m_key              = new wxStaticText(panel, -1, _T("?"), wxPoint(x_val,y0+sz.y*7), sz) ; 

	// text control displaying the copyright notice found in the MIDI file
	sz.x = box_w ;
	sz.y = wxButton::GetDefaultSize().y * 2 ;
	m_copyright_notice = 
		new wxTextCtrl(panel, -1, _T(""), wxPoint(MFP_SPACING,MFP_SPACING+UPPER_H+LOWER_H), sz, 
		               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH) ;

    set_database(NULL) ;
}

MFPInfoWindow::~MFPInfoWindow() 
{
	info_window = NULL ;
}

void MFPInfoWindow::OnCloseWindow( wxCloseEvent & WXUNUSED(event) )
{
	//if (event.CanVeto())
		Hide() ;
	//else
	//	Destroy() ;
}


void MFPInfoWindow::set_database(INFO_DB *_db)
{
	db = _db ; 
	redisplay() ;
}

void MFPInfoWindow::redisplay()
{
	/* enter filename into window info line */
	wxString title(_T("File Info")) ;
	if (db)
		title.append(_T(" - ")).append(wxString::FromAscii(db->filename)) ;
	SetTitle(title) ;

	if (!db)
	{
		m_file_size->Disable() ;
		m_transformed_size->Disable() ;
		m_midi_file_type->Disable() ;
		m_number_of_tracks->Disable() ;
		m_smpte->Disable() ;
		m_ticks_per_frame->Disable() ;
		m_ticks_per_beat->Disable() ;
		m_tempo->Disable() ;
		m_key->Disable() ;
		m_time->Disable() ;
		m_copyright_notice->Disable() ;
	}
	else
	{
		m_file_size->Enable() ;
		m_transformed_size->Enable() ;
		m_midi_file_type->Enable() ;
		m_number_of_tracks->Enable() ;

		char buf[20] ;

		sprintf(buf, "%li", db->filesize /* in units of Byte */) ;
		m_file_size->SetLabel(wxString::FromAscii(buf)) ;
		sprintf(buf, "%li", db->transformed_size /* in units of Byte */) ;
		m_transformed_size->SetLabel(wxString::FromAscii(buf)) ;
		sprintf(buf, "%i", db->midi_header.midi_file_type) ;
		m_midi_file_type->SetLabel(wxString::FromAscii(buf)) ;
		sprintf(buf, "%i", db->midi_header.number_tracks) ;
		m_number_of_tracks->SetLabel(wxString::FromAscii(buf)) ;

		if (db->midi_header.smpte > 0)
		{
			/* SMPTE format */
			m_smpte->Enable() ;
			m_ticks_per_frame->Enable() ;
			m_ticks_per_beat->Disable() ;
			sprintf(buf, "%i", db->midi_header.smpte) ;
			m_smpte->SetLabel(wxString::FromAscii(buf)) ;
			sprintf(buf, "%i", db->midi_header.ticks_per_frame) ;
			m_ticks_per_frame->SetLabel(wxString::FromAscii(buf)) ;
		}
		else 
		{
			/* non-SMPTE format */
			m_smpte->Disable() ;
			m_ticks_per_frame->Disable() ;
			m_ticks_per_beat->Enable() ;
			sprintf(buf, "%i", db->midi_header.ticks_per_beat) ;
			m_ticks_per_beat->SetLabel(wxString::FromAscii(buf)) ;
		}
		if (db->tempo[0] == 0)
		     m_tempo->Disable() ;
		else m_tempo->Enable() ;
		if (db->key[0] == 0)
		     m_key->Disable() ;
		else m_key->Enable() ;
		if (db->time[0] == 0)
		     m_time->Disable() ;
		else m_time->Enable() ;
		
		m_tempo->SetLabel(wxString::FromAscii(db->tempo)) ;
		m_key  ->SetLabel(wxString::FromAscii(db->key  )) ;
		m_time ->SetLabel(wxString::FromAscii(db->time )) ;

		if (true)
		{
			if (db->copyright == NULL)
			{
				m_copyright_notice->SetValue(_T("no copyright notice")) ;
				m_copyright_notice->Disable() ;
			}
			else
			{
				m_copyright_notice->SetValue(wxString::FromAscii(db->copyright)) ;
				m_copyright_notice->Enable() ;
			}
		}
		else
		{
			// test copyright control
			m_copyright_notice->SetValue(_T("Bla fasel. Diese Datei ist meine und nur meine.\nD.h. Fingas!\nSonst gibt's eins auf die Fingas!!!")) ;
			m_copyright_notice->Enable() ;
		}
	}
}

