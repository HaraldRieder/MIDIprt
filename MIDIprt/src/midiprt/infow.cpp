/*****************************************************************************
  Purpose:     Implements MIDI file information window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: infow.cpp,v 1.3 2008/09/20 20:06:51 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include  "common.h"
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


MFPInfoWindow::MFPInfoWindow(wxWindow *parent) : wxDialog(parent, -1, _T(""), wxDefaultPosition, wxDefaultSize)
{
    db = NULL;
    
    // set the icon
    SetIcon(wxIcon(apppath + _T(DIRSEP) + _T("info.ico")));
	
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
    
    wxSizerFlags rightFlags = wxSizerFlags().Right().Border(wxLEFT|wxRIGHT|wxDOWN, MFP_TEXT_SPACING);
    wxSizerFlags leftFlags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxDOWN, MFP_TEXT_SPACING);
	
	// flexible grid with 3 columnns and a static box around it
	wxStaticBoxSizer *box = new wxStaticBoxSizer(wxVERTICAL, this, _T("Size"));
	wxFlexGridSizer *grid = new wxFlexGridSizer(3);
	grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("File size:")), rightFlags); 
	grid->Add(m_file_size = new wxStaticText(box->GetStaticBox(), -1, _T("?")), rightFlags); 
	grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Byte")), leftFlags); 
	grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Size in memory:")), rightFlags); 
	grid->Add(m_transformed_size = new wxStaticText(box->GetStaticBox(), -1, _T("?")), rightFlags); 
	grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Byte")), leftFlags); 
	box->Add(grid);
    topsizer->Add(box, wxSizerFlags().Border(wxALL, MFP_SPACING).Expand());

	// flexible grid with 2 columnns and a static box around it
	box = new wxStaticBoxSizer(wxVERTICAL, this, _T("MIDI"));
	grid = new wxFlexGridSizer(2);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("MIDI file type:")), rightFlags);
    grid->Add(m_midi_file_type = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("No. of tracks:")), rightFlags);
    grid->Add(m_number_of_tracks = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Ticks per beat:")), rightFlags);
    grid->Add(m_ticks_per_beat = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("SMPTE:")), rightFlags);
    grid->Add(m_smpte = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Ticks per frame:")), rightFlags);
    grid->Add(m_ticks_per_frame = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Initial tempo:")), rightFlags);
    grid->Add(m_tempo = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Time signature:")), rightFlags);
    grid->Add(m_time = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
    grid->Add(new wxStaticText(box->GetStaticBox(), -1, _T("Key signature:")), rightFlags);
    grid->Add(m_key = new wxStaticText(box->GetStaticBox(), -1, _T("?")), leftFlags);
	box->Add(grid);
    topsizer->Add(box, wxSizerFlags().Border(wxALL, MFP_SPACING).Expand());
    
   	// text control displaying the copyright notice found in the MIDI file
	wxSize sz ; 
	sz.x = wxButton::GetDefaultSize().y * 10;
	sz.y = wxButton::GetDefaultSize().y * 3;
	m_copyright_notice = new wxTextCtrl(this, -1, _T(""), wxDefaultPosition, sz, 
		               wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH) ;
    topsizer->Add(m_copyright_notice, wxSizerFlags(0).Border(wxALL, MFP_SPACING).Expand());
    
    SetSizerAndFit(topsizer);
    
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

