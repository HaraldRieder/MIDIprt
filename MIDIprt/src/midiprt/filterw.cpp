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
    Control_Track_on, Control_Track_off,
    Control_All_tracks_on, Control_All_tracks_off,
    Control_All_channels_on,
    Control_All_channels_off,
    Control_Track_name, Control_Device, Control_Instrument, Control_Text,
    Control_Channel_toggle = 1500 // insert new IDs before this one!
};


BEGIN_EVENT_TABLE(MFPFilterWindow, wxDialog)
    EVT_CLOSE(MFPFilterWindow::OnCloseWindow)
    EVT_SLIDER(Slider_Track, MFPFilterWindow::OnTrackSlider)
    EVT_COMMAND(Control_Track_on , wxEVT_COMMAND_RADIOBUTTON_SELECTED, MFPFilterWindow::OnTrackOn)
    EVT_COMMAND(Control_Track_off, wxEVT_COMMAND_RADIOBUTTON_SELECTED, MFPFilterWindow::OnTrackOff)
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
: wxDialog(parent, -1, _T(""), wxDefaultPosition, wxSize(wxButton::GetDefaultSize().y * 20, wxButton::GetDefaultSize().y * 12))
, m_changed(false)
{
    // set the icon
    SetIcon(wxIcon(apppath + _T(DIRSEP) + _T("filter.ico")));

    // create the status line
//    CreateStatusBar(1/*fields*/,0/*without resizing grip*/); 

    // create panel for the controls
    int PANEL_W, PANEL_H ;
    DoGetClientSize(&PANEL_W, &PANEL_H) ;
    wxPanel *panel = new wxPanel(this, -1, wxPoint(0, 0), wxSize(PANEL_W, PANEL_H), wxTAB_TRAVERSAL);

    const int UPPER_H = PANEL_H/2 ;   // tracks and channels
    const int LEFT_W = PANEL_W/2 ;    // tracks

    const int LOWER_H = PANEL_H - UPPER_H ;  // textes: take the rest
    const int RIGHT_W = PANEL_W - LEFT_W ;   // channels: take the rest


    wxSize sz ;

    // track filter controls
    const int upper_box_h = UPPER_H-MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("tracks"), 
        wxPoint(MFP_SPACING,MFP_SPACING),wxSize(LEFT_W-2*MFP_SPACING,upper_box_h));
    sz.x = LEFT_W - 4*MFP_SPACING ;
    sz.y = wxButton::GetDefaultSize().y;
    m_track_slider = new Slider(panel, Slider_Track, 0, 0, 10, 
        wxPoint(MFP_SPACING*2,MFP_SPACING*3),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_AUTOTICKS+wxSL_LABELS) ;
    sz.x /= 2 ;
    sz.y = (upper_box_h - 4*MFP_SPACING)/3 ;
    m_track_on   = new wxRadioButton(panel,Control_Track_on, _T("o&n"), 
        wxPoint(5*MFP_SPACING, MFP_SPACING*4 + sz.y)) ;
    m_track_off  = new wxRadioButton(panel,Control_Track_off, _T("o&ff"), 
        wxPoint(5*MFP_SPACING + sz.x, MFP_SPACING*4 + sz.y)) ;
    m_tracks_on  = new wxButton(panel,Control_All_tracks_on,_T("all on"), 
        wxPoint(2*MFP_SPACING, MFP_SPACING*4 + 2*sz.y), sz) ;
    m_tracks_off = new wxButton(panel,Control_All_tracks_off,_T("all off"), 
        wxPoint(2*MFP_SPACING + sz.x, MFP_SPACING*4 + 2*sz.y), sz) ;

    // channel filter controls
    new wxStaticBox(panel, -1, _T("channels"), 
        wxPoint(RIGHT_W+MFP_SPACING,MFP_SPACING), wxSize(RIGHT_W-2*MFP_SPACING,upper_box_h));
    sz.x = (RIGHT_W-4*MFP_SPACING) * 2 / MIDI_CHANNELS ;
    for (unsigned i = 0 ; i < 2 ; i++)
        for (unsigned j = 0 ; j < 8 ; j++)
        {
            char buf[3] ;
            unsigned index = i*8+j ;
            sprintf(buf, "%X", index) ;
            m_channels[index] = new wxToggleButton(panel, Control_Channel_toggle + index, 
                wxString::FromAscii(buf), 
                wxPoint(RIGHT_W+MFP_SPACING*2+j*sz.x,MFP_SPACING*3+i*sz.y), sz) ;
        }
    sz.x = (RIGHT_W - 4*MFP_SPACING)/2 ;
    m_channels_on = new wxButton(panel, Control_All_channels_on,_T("all on"), 
        wxPoint(RIGHT_W+MFP_SPACING*2, MFP_SPACING*4 + 2*sz.y), sz) ;
    m_channels_off = new wxButton(panel, Control_All_channels_off,_T("all off"), 
        wxPoint(RIGHT_W+MFP_SPACING*2 + sz.x, MFP_SPACING*4 + 2*sz.y), sz) ;

    // track name controls
    const int show_box_h = LOWER_H-2*MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("show"), 
        wxPoint(MFP_SPACING, UPPER_H+MFP_SPACING), wxSize(PANEL_W-2*MFP_SPACING, show_box_h));
    sz.y = (show_box_h - MFP_SPACING*5/2)/4 ;
    sz.x = sz.y * 4 ; // should be broad enough for "Track name"
    int x = MFP_SPACING*2;
    int y0 = UPPER_H+MFP_SPACING*3;
    m_trackname  = new wxRadioButton(panel, Control_Track_name, _T("&Track name"), wxPoint(x, y0)       , sz) ;
    m_instrument = new wxRadioButton(panel, Control_Instrument, _T("&Instrument"), wxPoint(x, y0+sz.y)  , sz) ;
    m_device     = new wxRadioButton(panel, Control_Device    , _T("&Device")    , wxPoint(x, y0+sz.y*2), sz) ;
    m_text       = new wxRadioButton(panel, Control_Text      , _T("Te&xt")      , wxPoint(x, y0+sz.y*3), sz) ;
    const long style = wxTE_READONLY+wxTE_LEFT ;
    x = MFP_SPACING*3+sz.x ;
    sz.x = PANEL_W-5*MFP_SPACING-sz.x ;
    m_trackname_txt  = new wxTextCtrl(panel, -1, _T(""), wxPoint(x, y0)       , sz, style) ;
    m_instrument_txt = new wxTextCtrl(panel, -1, _T(""), wxPoint(x, y0+sz.y)  , sz, style) ;
    m_device_txt     = new wxTextCtrl(panel, -1, _T(""), wxPoint(x, y0+sz.y*2), sz, style) ;
    m_text_txt       = new wxTextCtrl(panel, -1, _T(""), wxPoint(x, y0+sz.y*3), sz, style) ;

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


void MFPFilterWindow::OnTrackOn( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    db->track[db->current_track].filter = TRUE ; 
    update_channel_buttons() ;
    update_string_buttons () ;
    m_changed = true ;
}

void MFPFilterWindow::OnTrackOff( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    db->track[db->current_track].filter = FALSE ; 
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
            m_track_on ->SetValue(true) ;
            m_track_off->SetValue(false) ;
            m_track_on ->Enable() ;
            m_track_off->Enable() ;
            break ;
        case FALSE:
            m_track_off->SetValue(true) ;
            m_track_off->SetValue(false) ;
            m_track_on ->Enable() ;
            m_track_off->Enable() ;
            break ;
        default: // additional flag in filter indicates "disabled"
            m_track_on ->Disable() ;
            m_track_off->Disable() ;
        }
    }
    else
    {
        m_track_on    ->Disable() ;
        m_track_off   ->Disable() ;
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
    }
    else
    {
        m_trackname->Enable() ;
        m_device->Enable() ;
        m_instrument->Enable() ;
        m_text->Enable() ;
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

