/*****************************************************************************
  Purpose:     Implements track and channel filter window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: filterw.cpp,v 1.4 2008/09/20 20:06:51 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

// window position and size
static const int FRAME_W = 400, FRAME_H = 600 ; // in pels

#include  "common.h"
#include  "commonw.h"
#include  <wx/ownerdrw.h>
#include  <wx/menuitem.h>
#include  <wx/checklst.h>

#include <graphicf.h>
#include <wxVDI.h>
#include "filterw.h"

MFPFilterWindow * filter_window ; // singleton window


/** control IDs (sliders, buttons) */
enum 
{
    Control_First = 1000,
    Slider_Track ,
	Control_Show_track,
    Control_All_tracks_on, Control_All_tracks_off,
    Control_All_channels_on,
    Control_All_channels_off,
    Control_Track_name, Control_Device, Control_Instrument, Control_Text,
    Control_Channel_toggle = 1500 // insert new IDs before this one!
};


BEGIN_EVENT_TABLE(MFPFilterWindow, wxDialog)
    EVT_CLOSE(MFPFilterWindow::OnCloseWindow)
    EVT_SLIDER(Slider_Track, MFPFilterWindow::OnTrackSlider)
	EVT_COMMAND(Control_Show_track, wxEVT_COMMAND_CHECKBOX_CLICKED, MFPFilterWindow::OnShowTrack)
    EVT_COMMAND(Control_All_tracks_on   , wxEVT_COMMAND_BUTTON_CLICKED, MFPFilterWindow::OnAllTracksOn)
    EVT_COMMAND(Control_All_tracks_off  , wxEVT_COMMAND_BUTTON_CLICKED, MFPFilterWindow::OnAllTracksOff)
    EVT_COMMAND(Control_All_channels_on , wxEVT_COMMAND_BUTTON_CLICKED, MFPFilterWindow::OnAllChannelsOn)
    EVT_COMMAND(Control_All_channels_off, wxEVT_COMMAND_BUTTON_CLICKED, MFPFilterWindow::OnAllChannelsOff)
    EVT_COMMAND(Control_Track_name, wxEVT_COMMAND_RADIOBUTTON_SELECTED, MFPFilterWindow::OnTrackName)
    EVT_COMMAND(Control_Instrument, wxEVT_COMMAND_RADIOBUTTON_SELECTED, MFPFilterWindow::OnInstrument)
    EVT_COMMAND(Control_Device    , wxEVT_COMMAND_RADIOBUTTON_SELECTED, MFPFilterWindow::OnDevice)
    EVT_COMMAND(Control_Text      , wxEVT_COMMAND_RADIOBUTTON_SELECTED, MFPFilterWindow::OnText)
    EVT_COMMAND(Control_Channel_toggle   , wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 1, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 2, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 3, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 4, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 5, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 6, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 7, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 8, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+ 9, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+10, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+11, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+12, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+13, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+14, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
    EVT_COMMAND(Control_Channel_toggle+15, wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, MFPFilterWindow::OnChannelToggle )
END_EVENT_TABLE()


/** filter window constructor */
MFPFilterWindow::MFPFilterWindow(wxWindow *parent)
: wxDialog(parent, -1, _T(""), wxDefaultPosition, wxDefaultSize), m_changed(false)
{
    // set the icon
    SetIcon(wxIcon(apppath + _T(DIRSEP) + _T("filter.ico")));

    // create the status line
//    CreateStatusBar(1/*fields*/,0/*without resizing grip*/); 

    wxSize sz;
    const wxSizerFlags flags = wxSizerFlags(1).Center().Border(wxLEFT|wxRIGHT|wxDOWN, MFP_TEXT_SPACING);
    const wxSizerFlags buttonFlags = wxSizerFlags(0).Center().Border(wxALL, MFP_SPACING);

	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *upper = new wxBoxSizer(wxHORIZONTAL);

    // track filter controls
  	wxStaticBoxSizer *box = new wxStaticBoxSizer(wxVERTICAL, this, _T("Tracks"));
    m_track_slider = new wxSlider(box->GetStaticBox(), Slider_Track, 0,0,10, 
        wxDefaultPosition, wxDefaultSize, 
        wxSL_HORIZONTAL+wxSL_LABELS);
    box->Add(m_track_slider, wxSizerFlags().Center().Border(wxLEFT+wxRIGHT, MFP_SPACING).Expand());
    m_show_track = new wxCheckBox(box->GetStaticBox(), Control_Show_track, _T("&show track"));
    box->Add(m_show_track, flags);
    wxBoxSizer *buttons = new wxBoxSizer(wxHORIZONTAL);
    m_tracks_on  = new wxButton(box->GetStaticBox(),Control_All_tracks_on,_T("all on"));
    m_tracks_off = new wxButton(box->GetStaticBox(),Control_All_tracks_off,_T("all off"));
    buttons->Add(m_tracks_on, buttonFlags);
    buttons->Add(m_tracks_off, buttonFlags);
    box->Add(buttons, wxSizerFlags().Center());
    upper->Add(box, wxSizerFlags(1).Expand());

    // channel filter controls
  	box = new wxStaticBoxSizer(wxVERTICAL, this, _T("Channels"));
    wxFlexGridSizer *grid = new wxFlexGridSizer(8);
    sz = wxButton::GetDefaultSize();
    sz.x = sz.y; // quadratic buttons
    for (unsigned i = 0 ; i < 16 ; i++)
    {
        char buf[3] ;
        sprintf(buf, "%X", i) ;
        m_channels[i] = new wxToggleButton(box->GetStaticBox(), 
                Control_Channel_toggle + i, wxString::FromAscii(buf), wxDefaultPosition, sz);
        // compact block of quadratic buttons, spacing around the block but not between buttons
        int direction = i < 8 ? wxTOP : wxBOTTOM;
        switch (i) {
            case 0: case 8: direction += wxLEFT; break;
            case 7: case 15: direction += wxRIGHT; break;
        }
        grid->Add(m_channels[i], wxSizerFlags(0).Center().Border(direction, MFP_SPACING));
    }
    box->Add(grid, flags);
    buttons = new wxBoxSizer(wxHORIZONTAL);
    m_channels_on = new wxButton(box->GetStaticBox(), Control_All_channels_on,_T("all on"));
    m_channels_off = new wxButton(box->GetStaticBox(), Control_All_channels_off,_T("all off"));
    buttons->Add(m_channels_on, buttonFlags);
    buttons->Add(m_channels_off, buttonFlags);
    box->Add(buttons, wxSizerFlags().Center());
    upper->Add(box, wxSizerFlags(1).Border(wxLEFT, MFP_SPACING).Expand());

    // track name controls
    // workaround TextCtrl not expanding automatically?
    sz = wxButton::GetDefaultSize();
    sz.x *= 5;
  	box = new wxStaticBoxSizer(wxVERTICAL, this, _T("show"));
    grid = new wxFlexGridSizer(2);
    grid->Add(m_trackname = new wxRadioButton(box->GetStaticBox(), Control_Track_name, _T("&Track name")));
    grid->Add(m_trackname_txt = new wxTextCtrl(box->GetStaticBox(), -1, _T(""), wxDefaultPosition, sz), flags);
    grid->Add(m_instrument = new wxRadioButton(box->GetStaticBox(), Control_Instrument, _T("&Instrument")));
    grid->Add(m_instrument_txt = new wxTextCtrl(box->GetStaticBox(), -1, _T(""), wxDefaultPosition, sz), flags);
    grid->Add(m_device = new wxRadioButton(box->GetStaticBox(), Control_Device, _T("&Device")));
    grid->Add(m_device_txt = new wxTextCtrl(box->GetStaticBox(), -1, _T(""), wxDefaultPosition, sz), flags);
    grid->Add(m_text = new wxRadioButton(box->GetStaticBox(), Control_Text, _T("Te&xt")));
    grid->Add(m_text_txt = new wxTextCtrl(box->GetStaticBox(), -1, _T(""), wxDefaultPosition, sz), flags);
    box->Add(grid, wxSizerFlags(1).Center().Expand());    

    topsizer->Add(upper, wxSizerFlags().Border(wxALL, MFP_SPACING).Expand());
    topsizer->Add(box, wxSizerFlags().Border(wxALL, MFP_SPACING).Expand());

    SetSizerAndFit(topsizer);

    set_database(NULL) ;
}


MFPFilterWindow::~MFPFilterWindow()
{
    filter_window = NULL ;
}


void MFPFilterWindow::OnCloseWindow( wxCloseEvent& WXUNUSED(event) )
{
    //if (event.CanVeto())
        Hide() ;
    //else
    //    Destroy() ;
}


void MFPFilterWindow::OnTrackSlider( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    db->current_track = m_track_slider->GetValue() ;
    {
        m_trackname_txt ->SetValue(wxString::FromAscii(db->track[db->current_track].name      )) ;
        m_device_txt    ->SetValue(wxString::FromAscii(db->track[db->current_track].device    )) ;
        m_instrument_txt->SetValue(wxString::FromAscii(db->track[db->current_track].instrument)) ;
        m_text_txt      ->SetValue(wxString::FromAscii(db->track[db->current_track].text      )) ;
    }
    update_channel_buttons() ;
    update_on_off_buttons () ;
    update_string_buttons () ;
}

void MFPFilterWindow::OnShowTrack( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    db->track[db->current_track].filter = m_show_track->IsChecked(); 
    update_channel_buttons() ;
    update_string_buttons () ;
    m_changed = true ;	
}

void MFPFilterWindow::OnAllTracksOn( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    for (unsigned i = 0 ; i < db->number_tracks ; i++)
        if ( !(db->track[i].filter & DISABLED) )
            db->track[i].filter = TRUE ;
    update_on_off_buttons() ;
    m_changed = true ;
}

void MFPFilterWindow::OnAllTracksOff( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    for (unsigned i = 0 ; i < db->number_tracks ; i++)
        if ( !(db->track[i].filter & DISABLED) )
            db->track[i].filter = FALSE ;
    update_on_off_buttons() ;
    m_changed = true ;
}

void MFPFilterWindow::OnChannelToggle(wxCommandEvent &event)
{
    if (!db) return ;
    int i = event.GetId() - Control_Channel_toggle ;
    db->track[db->current_track].ch_filter ^= 1<<i ;
    m_changed = true ;
}

void MFPFilterWindow::OnAllChannelsOn( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    db->track[db->current_track].ch_filter = 0xFFFF ;
    update_channel_buttons() ; 
    m_changed = true ;
}

void MFPFilterWindow::OnAllChannelsOff( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    db->track[db->current_track].ch_filter = 0x0000 ; 
    update_channel_buttons() ;
    m_changed = true ;
}

void MFPFilterWindow::OnTrackName( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    db->track[db->current_track].select = TR_NAME ; 
    m_changed = true ;
}

void MFPFilterWindow::OnInstrument( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    db->track[db->current_track].select = TR_DEVICE ; 
    m_changed = true ;
}

void MFPFilterWindow::OnDevice( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    db->track[db->current_track].select = TR_INSTR ; 
    m_changed = true ;
}

void MFPFilterWindow::OnText( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    db->track[db->current_track].select = TR_TEXT ; 
    m_changed = true ;
}

void MFPFilterWindow::update_channel_buttons()
{
    for (unsigned i = 0 ; i < 16 ; i++)
    {
        wxToggleButton *b = m_channels[i] ;
        if ( !db ||
             db->track[db->current_track].filter & DISABLED ||
             !(db->track[db->current_track].channels & (0x1 << i)) )
        {
            b->Disable() ;
            b->SetValue(false) ;
        }
        else 
        {
            b->Enable() ;
            b->SetValue((db->track[db->current_track].ch_filter & (1<<i)) != 0) ;
        }
    }
}

void MFPFilterWindow::update_on_off_buttons()
{
    if (db)
    {
        m_tracks_on   ->Enable() ;
        m_tracks_off  ->Enable() ;
        m_channels_on ->Enable() ;
        m_channels_off->Enable() ;
        switch ( db->track[db->current_track].filter )
        {
        case TRUE:
			m_show_track->Enable();
			m_show_track->SetValue(true);
            break ;
        case FALSE:
			m_show_track->Enable();
			m_show_track->SetValue(false);
            break ;
        default: // additional flag in filter indicates "disabled"
			m_show_track->Disable();
        }
    }
    else
    {
		m_show_track  ->Disable() ;
        m_tracks_on   ->Disable() ;
        m_tracks_off  ->Disable() ;
        m_channels_on ->Disable() ;
        m_channels_off->Disable() ;
    }
}

void MFPFilterWindow::update_string_buttons()
{
    if (!db || db->track[db->current_track].filter != 1/*TRUE*/) // "disabled" flag in filter!
    {
        m_trackname->Disable() ;
        m_device->Disable() ;
        m_instrument->Disable() ;
        m_text->Disable() ;
        m_trackname_txt->Disable() ;
        m_device_txt->Disable() ;
        m_instrument_txt->Disable() ;
        m_text_txt->Disable() ;
    }
    else
    {
        m_trackname->Enable() ;
        m_device->Enable() ;
        m_instrument->Enable() ;
        m_text->Enable() ;
        m_trackname_txt->Enable() ;
        m_device_txt->Enable() ;
        m_instrument_txt->Enable() ;
        m_text_txt->Enable() ;
    }
    
    if (db) switch ( db->track[db->current_track].select )
    {
    case TR_NAME  : m_trackname ->SetValue(true) ; break ;
    case TR_DEVICE: m_device    ->SetValue(true) ; break ;
    case TR_INSTR : m_instrument->SetValue(true) ; break ;
    case TR_TEXT  : m_text      ->SetValue(true) ; break ;
    }
}


bool MFPFilterWindow::has_changed()
{
    if (!m_changed)
        return false ;
    m_changed = false ;
    return true ;
}

void MFPFilterWindow::set_database(FILTER_DB *_db)
{
    db = _db ; 
    redisplay() ;
}

void MFPFilterWindow::redisplay()
{
    /* enter filename into window info line */
    wxString title(_T("Filter")) ;
    if (db)
        title.append(_T(" - ")).append(wxString::FromAscii(db->filename)) ;
    SetTitle(title) ;

    if (!db)
    {
        m_track_slider->Disable() ;

        m_trackname ->Disable() ; m_trackname_txt ->SetValue(_T("")) ;
        m_device    ->Disable() ; m_device_txt    ->SetValue(_T("")) ;
        m_instrument->Disable() ; m_instrument_txt->SetValue(_T("")) ;
        m_text      ->Disable() ; m_text_txt      ->SetValue(_T("")) ;
    }
    else
    {
        /* set slider positions */
        m_track_slider->SetRange(0, db->number_tracks - 1) ;
        m_track_slider->SetValue(db->current_track) ;
        m_track_slider->Enable() ;

        m_trackname ->Enable() ; m_trackname_txt ->SetValue(wxString::FromAscii(db->track[db->current_track].name      )) ;
        m_device    ->Enable() ; m_device_txt    ->SetValue(wxString::FromAscii(db->track[db->current_track].device    )) ;
        m_instrument->Enable() ; m_instrument_txt->SetValue(wxString::FromAscii(db->track[db->current_track].instrument)) ;
        m_text      ->Enable() ; m_text_txt      ->SetValue(wxString::FromAscii(db->track[db->current_track].text      )) ;
    }
    update_channel_buttons() ;
    update_on_off_buttons () ;
    update_string_buttons () ;
}

