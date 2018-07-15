/*****************************************************************************
  Purpose:     Implements the parameters window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: paramsw.cpp,v 1.4 2008/09/20 20:06:51 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

// window position and size
static const int FRAME_W = 400, FRAME_H = 600 ; // in pels
static const int LABEL_OFFSET = 2 ;

#include  "common.h"
#include  "commonw.h"
#include  <wx/ownerdrw.h>
#include  <wx/menuitem.h>
#include  <wx/checklst.h>
#include  <wx/fontdlg.h>

#include <graphicf.h>
#include <wxVDI.h>
#include "scheme.h"
#include "paramsw.h"

MFPParamsWindow * params_window ; // singleton window


/** control IDs (menu items, buttons) */
enum 
{
    // line width menu items:
    Popup_First = 100,
    Popup_01,Popup_02,Popup_03,Popup_05,Popup_07,Popup_10,Popup_14,Popup_20,

    Control_First = 1000,
    Control_Title, Control_Font, Control_Bold, Control_Italic, Control_Underlined,
    Control_Borders, Slider_Left_border, Slider_Right_border,
    Control_Distances, Slider_Distance,
    Slider_Bars, Control_Bars_width, Slider_Bar_length, Slider_Subbars, Control_Subbars_width,
    Control_Inventor, Control_Horlines, Control_Horlines_width,
    Slider_Height, Slider_Dynamic,
    Widget_Head , Widget_Body , Widget_Tail, // note widgets
    Control_Head, Control_Body, Control_Tail,
    Control_Noteborders, Control_Notes_3D, Control_Noteborders_width,
    Control_Transpose, Control_Scheme
};

/** scheme owner drawn object */

BEGIN_EVENT_TABLE(SchemeWidget, wxWindow)
    EVT_PAINT(SchemeWidget::OnPaint)
    EVT_LEFT_UP(SchemeWidget::OnSchemeSelect)
END_EVENT_TABLE()

SchemeWidget::SchemeWidget(wxWindow *parent, const wxPoint& pos, const wxSize& size)
: wxWindow(parent, -1, pos, size), db(NULL) 
{
    m_text = new wxStaticText(this, -1, _T(""), wxPoint(size.x/11,3)) ; 
}

void SchemeWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxSize sz = GetSize() ;
    wxPaintDC dc(this);
    //dc.BeginDrawing() ;
    VirtualDevice vdi(&dc) ;
    if (db != NULL) 
    {
        int dots = -1 /* no dots */ ; 
        MARK_MODE mark_mode = behind ;
        if      (db->mode == Rieder) mark_mode = between ;
        else if (db->mode == Beyreuther) dots = 4 ;
        draw_scheme(&vdi, &(db->scheme), db->transpose,
            0, 0, sz.GetWidth(), sz.GetHeight(),
            5 /* dodecime index */,
            db->horizontal_lines, dots, mark_mode,
            db->note_type, TRUE) ;
    }
    //dc.EndDrawing() ;
}

void SchemeWidget::set_database(PARAMS_DB *_db)
{
    db = _db ; 

    const char *s = "(default)" ;
    if ( db && db->scheme_path[0] ) 
        s = strrchr( db->scheme_path, DIRSEP ) ;
    if (!s) 
      s = db->scheme_path ;
    m_text->SetLabel(wxString(_T(" "))+wxString::FromAscii(s+1)+_T(" ")) ;

    Refresh() ;
}

void SchemeWidget::OnSchemeSelect( wxMouseEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;

    wxString path ;
    if ( db->scheme_path[0] ) 
        path = wxString::FromAscii(db->scheme_path) ;
    else
        /* init path */
        path = apppath + _T(SCHEMES_DIRECTORY) + _T(DIRSEP) ;

    wxFileDialog dialog(this, _T("Select scheme"),_T(""),_T(""),_T("Color schemes (*.par)|*.par;*.PAR"));
    dialog.SetDirectory(path);
    if ( dialog.ShowModal() == wxID_OK )
    {
        do_load_scheme(db, dialog.GetPath().ToAscii()) ;
        set_database(db) ;
    }
}

/** note owner drawn object */

BEGIN_EVENT_TABLE(NoteWidget, wxWindow)
    EVT_PAINT(NoteWidget::OnPaint)
END_EVENT_TABLE()

NoteWidget::NoteWidget(wxWindow *parent, int id ,const wxPoint& pos, const wxSize& size)
: wxWindow(parent, id, pos, size), db(NULL) { }

void NoteWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxSize sz = GetSize() ;
    wxPaintDC dc(this);
    //dc.BeginDrawing() ;
    VirtualDevice vdi(&dc) ;
    //draw_note(&vdi, points, BODY_ELLIP, 8,BLACK,8,BLACK) ;

    int points[4] = { 0,0, sz.x-1,sz.y-1 } ;
    int type = 0 ;

    vsl_width(&vdi, 1) ;

    /* the note ends may be twice as high as the body */
    points[1] += (sz.y >> 2) ;    
    points[3] -= (sz.y >> 2) ;    

    /* draw corresponding note */
    if (!db)
        return ; /* and these need also the database */
    switch(GetId())
    {
    case Widget_Head: type = db->note_type & HEAD_FLAGS ; break ;
    case Widget_Body: type = db->note_type & BODY_FLAGS ; break ;
    case Widget_Tail: type = db->note_type & TAIL_FLAGS ; break ;
    }
    
    if (type & HEAD_FLAGS)
    {
        /* shift to the right */
        points[0] += (points[3]-points[1]) ;
        points[2] += (points[3]-points[1]) ;
    }
    else if (type & TAIL_FLAGS)
    {
        /* shift to the left */
        points[0] -= (points[3]-points[1]) ;
        points[2] -= (points[3]-points[1]) ;
    }
    else if (type & BODY_FLAGS)
    {
        points[0] += (points[3]-points[1]) ;
        points[2] -= (points[3]-points[1]) ;
    }
    /* solid black notes */
    draw_note(&vdi, points, type, 8,BLACK,8,BLACK) ;

    //dc.EndDrawing() ;
}

void NoteWidget::set_database(PARAMS_DB *_db)
{
    db = _db ; 
    Refresh() ;
}



BEGIN_EVENT_TABLE(MFPParamsWindow, wxDialog)
    EVT_CLOSE(MFPParamsWindow::OnCloseWindow)
    EVT_TEXT(Control_Title, MFPParamsWindow::OnTitleEdit)
    EVT_COMMAND(Control_Font            , wxEVT_COMMAND_BUTTON_CLICKED, MFPParamsWindow::OnFont)
    EVT_COMMAND(Control_Bold            , wxEVT_COMMAND_CHECKBOX_CLICKED, MFPParamsWindow::TextEffects)
    EVT_COMMAND(Control_Italic          , wxEVT_COMMAND_CHECKBOX_CLICKED, MFPParamsWindow::TextEffects)
    EVT_COMMAND(Control_Underlined      , wxEVT_COMMAND_CHECKBOX_CLICKED, MFPParamsWindow::TextEffects)

    EVT_SLIDER(Slider_Bars              , MFPParamsWindow::OnBarsPerLine    )
    EVT_COMMAND(Control_Bars_width      , wxEVT_COMMAND_BUTTON_CLICKED, MFPParamsWindow::ShowLineWidthMenu)
    EVT_SLIDER(Slider_Bar_length        , MFPParamsWindow::OnBarLength      )
    EVT_SLIDER(Slider_Subbars           , MFPParamsWindow::OnSubBars        )
    EVT_COMMAND(Control_Subbars_width   , wxEVT_COMMAND_BUTTON_CLICKED, MFPParamsWindow::ShowLineWidthMenu)

    EVT_COMMAND(Control_Borders   , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnBordersSelect)
    EVT_SLIDER(Slider_Left_border , MFPParamsWindow::OnLeftBorderSlider )
    EVT_SLIDER(Slider_Right_border, MFPParamsWindow::OnRightBorderSlider)

    EVT_COMMAND(Control_Distances , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnDistanceSelect)
    EVT_SLIDER(Slider_Distance    , MFPParamsWindow::OnDistanceSlider   )

    EVT_COMMAND(Control_Inventor      , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnInventorSelect)
    EVT_COMMAND(Control_Horlines      , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnHorlinesSelect)
    EVT_COMMAND(Control_Horlines_width, wxEVT_COMMAND_BUTTON_CLICKED , MFPParamsWindow::ShowLineWidthMenu)

    EVT_SLIDER(Slider_Height      , MFPParamsWindow::OnHeightSlider     )
    EVT_SLIDER(Slider_Dynamic     , MFPParamsWindow::OnDynamicSlider    )
    EVT_COMMAND(Control_Head     , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnHeadSelect     )
    EVT_COMMAND(Control_Body     , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnBodySelect     )
    EVT_COMMAND(Control_Tail     , wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnTailSelect     )
    EVT_COMMAND(Control_Transpose, wxEVT_COMMAND_CHOICE_SELECTED, MFPParamsWindow::OnTransposeSelect)
    EVT_COMMAND(Control_Noteborders      , wxEVT_COMMAND_CHECKBOX_CLICKED, MFPParamsWindow::OnNoteBorders)
    EVT_COMMAND(Control_Notes_3D         , wxEVT_COMMAND_CHECKBOX_CLICKED, MFPParamsWindow::OnNotes3D    )
    EVT_COMMAND(Control_Noteborders_width, wxEVT_COMMAND_BUTTON_CLICKED, MFPParamsWindow::ShowLineWidthMenu)

    EVT_MENU(Popup_01, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_02, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_03, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_05, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_07, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_10, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_14, MFPParamsWindow::OnLineWidthMenu)
    EVT_MENU(Popup_20, MFPParamsWindow::OnLineWidthMenu)

END_EVENT_TABLE()


/** filter window constructor */
MFPParamsWindow::MFPParamsWindow(wxWindow *parent)
: wxDialog(parent, -1, _T(""), wxDefaultPosition, 
           wxSize(wxButton::GetDefaultSize().y * 23, wxButton::GetDefaultSize().y * 21) 
           /*, wxDEFAULT_DIALOG_STYLE, wxDialogNameStr*/)
{
    // set the icon
    SetIcon(wxIcon(_T("PARAMS")));

    // create the status line
//    CreateStatusBar(1/*fields*/,0/*without resizing grip*/); 

    // create panel for the controls
    int PANEL_W, PANEL_H ;
    DoGetClientSize(&PANEL_W, &PANEL_H) ;
    wxPanel *panel = new wxPanel(this, -1, wxPoint(0, 0), wxSize(PANEL_W, PANEL_H), wxTAB_TRAVERSAL);

    const int n_y = 100 ; 

    const int TITLE_H   = PANEL_H*17/n_y ;   // title label and text
    const int BORDERS_H = PANEL_H*21/n_y ;   // border controls box
    const int DIST_H    = PANEL_H*14/n_y ;   // distance controls box
    const int BARS_H    = PANEL_H*28/n_y ;   // bar controls box
    const int TYPE_H    = PANEL_H - BARS_H - DIST_H - BORDERS_H
                        - TITLE_H;// notation type box
    const int NOTES_H   = PANEL_H 
                        - TITLE_H;// note controls incl. scheme
    const int LEFT_W  = PANEL_W*6/11 ;         // borders, dist., bars, ...
    const int RIGHT_W = PANEL_W - LEFT_W ;  // channels: take the rest

    const int H = wxButton::GetDefaultSize().y ;

    wxSize sz ;
    int x,y ;

    // title and font controls
    sz.x = PANEL_W - 3*MFP_SPACING - wxButton::GetDefaultSize().x ;
    sz.y = TITLE_H - 2*MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("title"), wxPoint(MFP_SPACING,MFP_SPACING), sz);
    sz.x = PANEL_W*3/5 ;
    sz.y = H ;
    x = 2*MFP_SPACING; 
    int y0 = 1+2*MFP_SPACING;
    m_title = new wxTextCtrl(panel, Control_Title, _T(""), wxPoint(x,3*MFP_SPACING), sz) ;
    x = sz.x + 3*MFP_SPACING ;
    m_bold = new wxCheckBox(panel, Control_Bold, _T("&bold"), wxPoint(x,y0)) ;
    y = y0 + m_bold->GetSize().y ;
    m_italic = new wxCheckBox(panel, Control_Italic, _T("&italic"), wxPoint(x,y)) ;
    y = y0 + m_bold->GetSize().y * 2;
    m_underlined = new wxCheckBox(panel, Control_Underlined, _T("&underlined"), wxPoint(x,y)) ;
    x = PANEL_W    - MFP_SPACING - wxButton::GetDefaultSize().x ;
    y = (TITLE_H - H)/2 ;
    m_font = new wxButton(panel, Control_Font, _T("&Font..."), wxPoint(x,y)) ;

    // borders
    y = TITLE_H ;
    sz.x = LEFT_W - 2*MFP_SPACING ;
    sz.y = BORDERS_H - MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("borders in %"), wxPoint(MFP_SPACING,y), sz);
    sz.x -= 2*MFP_SPACING ;
    sz.y = H ;
    x = 2*MFP_SPACING;
    y = TITLE_H + 2*MFP_SPACING;
    const wxString border_choices[2] = {_T("Top / bottom:"),_T("Left / right:")} ;
    m_borders_choice = new wxChoice(panel, Control_Borders, wxPoint(x,y),sz, 2,border_choices) ;
    m_borders_choice->SetSelection(0) ;
    y = TITLE_H + MFP_SPACING*3 + H ;
    sz.x = sz.x / 3 ;
    m_left_border  = new Slider(panel, Slider_Left_border , 0, 0, 49, wxPoint(x,y),sz, wxSL_HORIZONTAL+wxSL_BOTTOM) ;
    m_left_border->SetPageSize(1) ;
    x = (LEFT_W - 2*MFP_SPACING) - sz.x ;
    m_right_border = new Slider(panel, Slider_Right_border, 0, 0, 49, wxPoint(x,y),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_INVERSE) ;
    m_right_border->SetPageSize(1) ;
    x = 3*MFP_SPACING + sz.x ;
    sz.x = (sz.x - MFP_SPACING)/2 - MFP_SPACING ;
    m_left_val  = new wxTextCtrl(panel, -1, _T("?"), wxPoint(x,y), sz, wxTE_READONLY|wxTE_CENTER) ;
    x += (MFP_SPACING + sz.x) ;
    m_right_val = new wxTextCtrl(panel, -1, _T("?"), wxPoint(x,y), sz, wxTE_READONLY|wxTE_CENTER) ;

    // distances
    y = TITLE_H + BORDERS_H ;
    sz.x = LEFT_W - 2*MFP_SPACING ;
    sz.y = DIST_H - MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("distances in %"), wxPoint(MFP_SPACING,y), sz);
    sz.x /= 3 ;
    sz.y = wxButton::GetDefaultSize().y ;
    const wxString dist_choices[3] = { _T("System:"), _T("Track:"), _T("Note:") } ;
    x = 2*MFP_SPACING;
    y += 2*MFP_SPACING;
    m_distances = new wxChoice(panel, Control_Distances, wxPoint(x,y), sz, 3, dist_choices) ;
    m_distances->SetSelection(0) ;
    x = 3*MFP_SPACING + sz.x ;
    sz.x = (LEFT_W - 2*MFP_SPACING)/6 ;
    m_distance_val = new wxTextCtrl(panel, -1, _T("?"), wxPoint(x,y), sz, wxTE_READONLY|wxTE_CENTER) ;
    x = LEFT_W - x + MFP_SPACING;
    sz.x = LEFT_W - 2*MFP_SPACING - x;
    m_distance = new Slider(panel, Slider_Distance, 0, 0, 10000, wxPoint(x,y), sz, wxSL_HORIZONTAL+wxSL_BOTTOM) ;

    // bars
    y = TITLE_H + BORDERS_H + DIST_H ;
    sz.x = LEFT_W - 2*MFP_SPACING ;
    sz.y = BARS_H - MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("bars"), wxPoint(MFP_SPACING,y), sz);
    x = 2*MFP_SPACING;
    y0 = y + 2*MFP_SPACING + LABEL_OFFSET ;
    new wxStaticText(panel, -1, _T("Per line:")    , wxPoint(x,y0)) ; 
    new wxStaticText(panel, -1, _T("Length [1/8]:"), wxPoint(x,y0+MFP_SPACING + H)) ; 
    new wxStaticText(panel, -1, _T("Sub-bars:")    , wxPoint(x,y0+2*MFP_SPACING + 2*H)) ; 
    x = MFP_SPACING + LEFT_W/3 ;
    y0 = y + 2*MFP_SPACING;
    sz.x = LEFT_W - x - 2*MFP_SPACING ;
    sz.y = H ;
    m_bars_per_line = new Slider(panel, Slider_Bars, 8, 1, 40, wxPoint(x,y0),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS) ;
    m_bars_per_line->SetPageSize(1) ;
    m_bar_length = new Slider(panel, Slider_Bar_length, 8, 1, 40, wxPoint(x,y0+(MFP_SPACING+H)),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS) ;
    m_bar_length->SetPageSize(1) ;
    m_sub_bars = new Slider(panel, Slider_Subbars, 2, 1, 12, wxPoint(x,y0+2*(MFP_SPACING+H)),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS+wxSL_AUTOTICKS) ;
    m_sub_bars->SetPageSize(1) ;
    sz.x = H/2 ;
    new wxButton(panel, Control_Bars_width   , _T("-"), wxPoint(x-H,y0)      , sz) ;
    new wxButton(panel, Control_Subbars_width, _T("-"), wxPoint(x-H,y0+2*(MFP_SPACING+H)), sz) ;

    // notation type
    y = TITLE_H + BORDERS_H + DIST_H + BARS_H ;
    sz.x = LEFT_W - 2*MFP_SPACING ;
    sz.y = TYPE_H - MFP_SPACING ;
    x = 2*MFP_SPACING;
    new wxStaticBox(panel, -1, _T("notation type"), wxPoint(MFP_SPACING,y), sz);
    y0 = y + 2*MFP_SPACING;
    new wxStaticText(panel, -1, _T("Inventor:")        , wxPoint(x,y0+LABEL_OFFSET)) ; 
    new wxStaticText(panel, -1, _T("Horizontal lines:"), wxPoint(x,y0+LABEL_OFFSET+MFP_SPACING+H)) ; 
    sz.x = LEFT_W/2 ;
    sz.y = H ;
    const wxString inv_choices[3] = { _T("Beyreuther"), _T("Rieder"), _T("Mix") } ;
    x = LEFT_W - 2*MFP_SPACING - sz.x ;
    m_inventor = new wxChoice(panel, Control_Inventor, wxPoint(x,y0),sz, 3,inv_choices) ;
    sz.x = sz.y/2 ; // narrow button
    new wxButton(panel, Control_Horlines_width, _T("-"), wxPoint(x,y0+MFP_SPACING+H), sz) ;
    sz.x = LEFT_W/4 ;
    const wxString lines_choices[5] = { _T("1"), _T("2"), _T("3"), _T("4"), _T("6") } ;
    x = LEFT_W - 2*MFP_SPACING - sz.x ;
    m_lines = new wxChoice(panel, Control_Horlines, wxPoint(x,y0+MFP_SPACING+H),sz, 5,lines_choices) ;

    // notes & color scheme
    y = TITLE_H ;
    sz.x = RIGHT_W - MFP_SPACING ;
    sz.y = NOTES_H - MFP_SPACING ;
    new wxStaticBox(panel, -1, _T("notes && colors"), wxPoint(LEFT_W,y), sz);
    const int x_lab = LEFT_W+MFP_SPACING;
    y0 = TITLE_H + 2*MFP_SPACING;
    new wxStaticText(panel, -1, _T("Av. height:"), wxPoint(x_lab,y0+LABEL_OFFSET)) ; 
    sz.x = RIGHT_W*3/5 ;
    sz.y = H ;
    x = PANEL_W - 2*MFP_SPACING - sz.x;
    m_height = new Slider(panel, Slider_Height, 5, 1, 9, wxPoint(x,y0),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS+wxSL_AUTOTICKS) ;
    m_height->SetPageSize(1) ;
    new wxStaticText(panel, -1, _T("Dynamic:"), wxPoint(x_lab,y0+LABEL_OFFSET+MFP_SPACING+H)) ; 
    m_dynamic = new Slider(panel, Slider_Dynamic, 0, 0, 15,    wxPoint(x,y0+MFP_SPACING+H),sz, wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS+wxSL_AUTOTICKS) ;
    m_dynamic->SetPageSize(1) ;
    sz.x = RIGHT_W/3 ;
    const wxString ends_choices[4] = { _T("(none)"), _T("line"), _T("triangle"), _T("dot")} ;
    const wxString body_choices[4] = { _T("(none)"), _T("bar"), _T("ellipse"), _T("triangle")} ;
    x = PANEL_W - 2*MFP_SPACING - sz.x ;
    y0 = TITLE_H + 4*MFP_SPACING;
    new wxStaticText(panel, -1, _T("Heads:") , wxPoint(x_lab,y0+LABEL_OFFSET+2*H)) ; 
    new wxStaticText(panel, -1, _T("Bodies:"), wxPoint(x_lab,y0+LABEL_OFFSET+3*H)) ; 
    new wxStaticText(panel, -1, _T("Tails:") , wxPoint(x_lab,y0+LABEL_OFFSET+4*H)) ; 
    m_head_choice = new wxChoice(panel, Control_Head, wxPoint(x,y0+2*H),sz,4,ends_choices) ;
    m_body_choice = new wxChoice(panel, Control_Body, wxPoint(x,y0+3*H),sz,4,body_choices) ;
    m_tail_choice = new wxChoice(panel, Control_Tail, wxPoint(x,y0+4*H),sz,4,ends_choices) ;

    x = LEFT_W + MFP_SPACING + RIGHT_W/4 ;
    y = 3*MFP_SPACING + 2*H + TITLE_H + MFP_SPACING ;
    sz.x = RIGHT_W / 5 ;
    sz.y = H/2 ;
    m_head = new NoteWidget(panel, Widget_Head, wxPoint(x,H/4+y), sz) ;
    m_body = new NoteWidget(panel, Widget_Body, wxPoint(x,H/4+y+H), sz) ;
    m_tail = new NoteWidget(panel, Widget_Tail, wxPoint(x,H/4+y+2*H), sz) ;

    y = TITLE_H + 4*MFP_SPACING + 5*H ;
    sz.x = H/2 ;
    sz.y = H ;
    x = LEFT_W + 2*MFP_SPACING;
    new wxButton(panel, Control_Noteborders_width, _T("-"), wxPoint(x,y), sz) ;
    sz.x = RIGHT_W/3 ;
    x += H;
    m_borders = new wxCheckBox(panel, Control_Noteborders, _T("Note borders"), wxPoint(x,y), sz) ;
    x = LEFT_W + H + (RIGHT_W - MFP_SPACING)/2 ;
    m_borders_3d = new wxCheckBox(panel, Control_Notes_3D, _T("3D notes"), wxPoint(x,y), sz) ;
    y = TITLE_H + 5*MFP_SPACING + 6*H ;
    new wxStaticText(panel, -1, _T("Transpose:"), wxPoint(x_lab,y+LABEL_OFFSET)) ; 
    const wxString transp_choices[12] = 
        { _T("0 (none)"),_T("+1 = -B"),_T("+2 = -A"),_T("+3 = -9"),_T("+4 = -8"),_T("+5 = -7"),
          _T("+6 = -6"),_T("+7 = -5"),_T("+8 = -4"),_T("+9 = -3"),_T("+A = -2"),_T("+B = -1") } ;
    sz.x = RIGHT_W/3 ;
    sz.y = H ;
    x = PANEL_W - 2*MFP_SPACING - sz.x ;
    m_transpose = new wxChoice(panel, Control_Transpose, wxPoint(x,y),sz, 12,transp_choices) ;

    // color scheme
    x = LEFT_W + MFP_SPACING/2 ;
    y = TITLE_H + 11*MFP_SPACING/2 + 7*H ;
    sz.x = RIGHT_W - 2*MFP_SPACING ;
    sz.y = PANEL_H - y - MFP_SPACING - MFP_SPACING/2 ;
    m_scheme = new SchemeWidget(panel, wxPoint(x,y), sz) ;

    set_database(NULL) ;
}


MFPParamsWindow::~MFPParamsWindow()
{
    params_window = NULL ;
}

void MFPParamsWindow::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    //if (event.CanVeto())
        Hide() ;
    //else
    //    Destroy() ;
}


void MFPParamsWindow::ShowLineWidthMenu(wxCommandEvent& event)
{
    if (db == NULL) return ;

    wxPoint pos ;
    wxMenu menu(_T("Line width:"));

    menu.AppendRadioItem(Popup_01, _T("0.1 mm"));
    menu.AppendRadioItem(Popup_02, _T("0.2 mm"));
    menu.AppendRadioItem(Popup_03, _T("0.3 mm"));
    menu.AppendRadioItem(Popup_05, _T("0.5 mm"));
    menu.AppendRadioItem(Popup_07, _T("0.7 mm"));
    menu.AppendRadioItem(Popup_10, _T("1.0 mm"));
    menu.AppendRadioItem(Popup_14, _T("1.4 mm"));
    menu.AppendRadioItem(Popup_20, _T("2.0 mm"));
//    for (int i = Popup_01 ; i <= Popup_20 ; i++)
//        menu.Enable(i, true) ;

    char *s = NULL ;

    switch (event.GetId())
    {
    case Control_Bars_width:        s = db->bar_line_width     ; break ;
    case Control_Subbars_width:     s = db->sub_bar_line_width ; break ;
    case Control_Noteborders_width: s = db->note_line_width    ; break ;
    default:                        s = db->line_width         ;
    }

    // pre-select current line width
    int current ;
    for (current = Popup_01 ; current <= Popup_20 ; current++)
    {
        if (strstr(menu.GetLabel(current).ToAscii(), s) != NULL)
        {
            menu.Check(current, true) ;
            break ;
        }
    }
    // ...take default here if not found ?

    pos.x = pos.y = 140 ;
    m_popup_id = -1 ;
    PopupMenu(&menu, pos.x, pos.y) ; // sets m_popup_id
    if (m_popup_id == -1)
        return ;

    strncpy(s, menu.GetLabel(m_popup_id).ToAscii(), LINE_WIDTH_STRLEN) ;
    s[LINE_WIDTH_STRLEN] = 0 ; 
}

void MFPParamsWindow::OnLineWidthMenu(wxCommandEvent& event)
{
    // workaround: radio buttons do not work in popup menu ?
    // => we need this own event handler
    m_popup_id = event.GetId() ;
}

// title/text controls

void MFPParamsWindow::OnTitleEdit(wxCommandEvent& WXUNUSED(event))
{
    if (db == NULL) return ;
    strncpy(db->title, m_title->GetValue().ToAscii(), sizeof(db->title)-1) ;
    db->title[sizeof(db->title)-1] = 0 ;
}

void MFPParamsWindow::OnFont( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;

    wxNativeFontInfo info ;
    info.FromString(wxString::FromAscii(db->font)) ;
    wxFont font;
    font.SetNativeFontInfo(info) ;
    wxFontData data;
    data.EnableEffects(false) ;
    data.SetShowHelp(false) ;
    data.SetInitialFont(font);
//    data.SetColour(wxGetApp().m_canvasTextColour);

    // you might also do this:
    //
    //  wxFontDialog dialog;
    //  if ( !dialog.Create(this, data) { ... error ... }
    //
    wxFontDialog dialog(this, data);

    if (dialog.ShowModal() == wxID_OK)
    {
        wxFontData retData = dialog.GetFontData();
        strncpy(db->font, retData.GetChosenFont().GetNativeFontInfoDesc().ToAscii(), sizeof(db->font)-1) ;
        db->font[sizeof(db->font)-1] = 0 ;
        db->points = retData.GetChosenFont().GetPointSize() ;
    }
    //else: cancelled by the user, don't change the font
}

void MFPParamsWindow::TextEffects(wxCommandEvent &event)
{
    if (!db) return ;
    switch (event.GetId())
    {
        case Control_Bold:       db->effects ^= TF_THICKENED  ; break ;
        case Control_Italic:     db->effects ^= TF_SLANTED    ; break ;
        case Control_Underlined: db->effects ^= TF_UNDERLINED ; break ;
    }
}


// border controls

void MFPParamsWindow::OnLeftBorderSlider( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    int value = m_left_border->GetValue() ;
    if (m_borders_choice->GetSelection() == 0)
        db->upper_border = value ;
    else
        db->left_border = value ;
    char buf[10] ;
    sprintf(buf, "%d", value) ;
    m_left_val->SetValue(wxString::FromAscii(buf)) ;
}

void MFPParamsWindow::OnRightBorderSlider( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    int value = m_right_border->GetValue() ;
    if (m_borders_choice->GetSelection() == 0)
        db->lower_border = value ;
    else
        db->right_border = value ;
    char buf[10] ;
    sprintf(buf, "%d", value) ;
    m_right_val->SetValue(wxString::FromAscii(buf)) ;
}

void MFPParamsWindow::OnBordersSelect( wxCommandEvent &WXUNUSED(event) )
{
    update_border_sliders() ;
}

void MFPParamsWindow::update_border_sliders()
{
    int pos1, pos2 ;
    
    if (!db) 
    {
        m_left_border->Disable() ;
        m_right_border->Disable() ;
        pos1 = 0 ;
        pos2 = 0 ;
    }
    else
    {
        m_left_border->Enable() ;
        m_right_border->Enable() ;
            
        /* set slider positions and live data */
        if (m_borders_choice->GetSelection() == 0)
        {
            /* top and bottom */
            pos1 = db->upper_border ;
            pos2 = db->lower_border ;
        }
        else 
        {
            /* left and right */
            pos1 = db->left_border ;
            pos2 = db->right_border ;
        }
    }
    /* set positions */
    m_left_border ->SetValue(pos1) ;
    m_right_border->SetValue(pos2) ;

    /* refresh text */
    wxCommandEvent dummy ;
    OnLeftBorderSlider(dummy) ;
    OnRightBorderSlider(dummy) ;
}


// distance controls

void MFPParamsWindow::OnDistanceSlider( wxCommandEvent &WXUNUSED(event) )
{
    if (!db) return ;
    
    char buf[10] ;
    int value = m_distance->GetValue() ;
    switch (m_distances->GetSelection())
    {
    case 0:
        db->system_distance = value ;
        sprintf(buf, "%d", value) ;
        break ;
    case 1:
        db->track_distance = value ;
        sprintf(buf, "%d", value) ;
        break ;
    case 2:
        {
            float distance = (float)(value+1) / 20 ;
            db->note_distance = distance ;
            sprintf(buf, "%.2f", distance) ;
        }
        break ;
    }
    m_distance_val->SetValue(wxString::FromAscii(buf)) ;
}

void MFPParamsWindow::OnDistanceSelect( wxCommandEvent &WXUNUSED(event) )
{
    update_distance_slider() ;
}

void MFPParamsWindow::update_distance_slider()
{
    if (!db) 
    {
        m_distance->Disable() ;
    }
    else
    {
        m_distance->Enable() ;

        /* set slider position and data */
        switch (m_distances->GetSelection())
        {
        case 0:    
            m_distance->SetRange(1,MAX_SYSTEM_DISTANCE) ;
            m_distance->SetPageSize(1) ;
            m_distance->SetValue(db->system_distance) ;
            break ;
        case 1:    
            m_distance->SetRange(1,MAX_TRACK_DISTANCE) ;
            m_distance->SetPageSize(1) ;
            m_distance->SetValue(db->track_distance) ;
            break ;
        case 2:    
            m_distance->SetRange(0,(MAX_NOTE_DISTANCE*20+0.5)-1) ;
            m_distance->SetPageSize(1) ;
            m_distance->SetValue((int)(db->note_distance*20+0.5) - 1) ;
            break ;
        }
    }
    /* refresh text */
    wxCommandEvent dummy ;
    OnDistanceSlider(dummy) ;
}


// bar controls

void MFPParamsWindow::OnBarsPerLine( wxCommandEvent &WXUNUSED(event) )
{
    if (db != NULL) db->bars_per_line = m_bars_per_line->GetValue() ;
}

void MFPParamsWindow::OnBarLength( wxCommandEvent &WXUNUSED(event) )
{
    if (db != NULL) db->bar_length = m_bar_length->GetValue() ;
}

void MFPParamsWindow::OnSubBars( wxCommandEvent &WXUNUSED(event) )
{
    if (db != NULL) db->sub_bars = m_sub_bars->GetValue() ;
}


// inventor and horizontal lines

void MFPParamsWindow::OnInventorSelect( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    
    wxString s = m_inventor->GetStringSelection() ;
    if      (s[0] == 'B') db->mode = Beyreuther ; 
    else if (s[0] == 'M') db->mode = Mix        ;
    else                  db->mode = Rieder     ;

    m_scheme->Refresh() ;
}

void MFPParamsWindow::OnHorlinesSelect( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    
    wxString s = m_lines->GetStringSelection() ;
    db->horizontal_lines = atoi(s.ToAscii()) ;

    m_scheme->Refresh() ;
}

// notes & colors

void MFPParamsWindow::OnHeightSlider( wxCommandEvent &WXUNUSED(event) )
{
    if (db != NULL) db->note_height = m_height->GetValue() ;
}

void MFPParamsWindow::OnDynamicSlider( wxCommandEvent &WXUNUSED(event) )
{
    if (db != NULL) db->note_dynscale = m_dynamic->GetValue() ;
}

void MFPParamsWindow::OnHeadSelect( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    
    db->note_type &= ~HEAD_FLAGS ;
    
    switch (m_head_choice->GetSelection())
    {
    case 0: break ;
    case 1: db->note_type |= HEAD_LINE ; break ;
    case 2: db->note_type |= HEAD_TRI  ; break ;
    case 3: db->note_type |= HEAD_DOT  ; break ;
    }
    m_head  ->Refresh() ;
    m_scheme->Refresh() ;
}

void MFPParamsWindow::OnBodySelect( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    
    db->note_type &= ~BODY_FLAGS ;
    
    switch (m_body_choice->GetSelection())
    {
    case 0: break ;
    case 1: db->note_type |= BODY_RECT  ; break ;
    case 2: db->note_type |= BODY_ELLIP ; break ;
    case 3: db->note_type |= BODY_TRI   ; break ;
    }
    m_body  ->Refresh() ;
    m_scheme->Refresh() ;
}

void MFPParamsWindow::OnTailSelect( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    
    db->note_type &= ~TAIL_FLAGS ;
    
    switch (m_tail_choice->GetSelection())
    {
    case 0: break ;
    case 1: db->note_type |= TAIL_LINE ; break ;
    case 2: db->note_type |= TAIL_TRI  ; break ;
    case 3: db->note_type |= TAIL_DOT  ; break ;
    }
    m_tail  ->Refresh() ;
    m_scheme->Refresh() ;
}

void MFPParamsWindow::OnTransposeSelect( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    db->transpose = m_transpose->GetSelection() ;
    m_scheme->Refresh() ;
}

void MFPParamsWindow::OnNoteBorders( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    if (m_borders->IsChecked())
        db->note_type &= ~BORDERS_NONE ;
    else
        db->note_type |= BORDERS_NONE ;
    update_3d_checkbox() ;
    m_scheme->Refresh() ;
}

void MFPParamsWindow::OnNotes3D( wxCommandEvent &WXUNUSED(event) )
{
    if (db == NULL) return ;
    if (m_borders_3d->IsChecked())
        db->note_type |= BORDERS_3D ;
    else
        db->note_type &= ~BORDERS_3D ;
    m_scheme->Refresh() ;
}


// UI updaters

void MFPParamsWindow::update_3d_checkbox()
{
    if (!db || (db->note_type & BORDERS_NONE))
        m_borders_3d->Disable() ;
    else
        m_borders_3d->Enable() ;
}


void MFPParamsWindow::update_head_body_tail()
{
    int type = db ? db->note_type : 0 ;

    switch (type & HEAD_FLAGS)
    {
    case 0:         m_head_choice->SetSelection(0) ; break ;
    case HEAD_LINE: m_head_choice->SetSelection(1) ; break ;
    case HEAD_TRI:  m_head_choice->SetSelection(2) ; break ;
    case HEAD_DOT:  m_head_choice->SetSelection(3) ; break ;
    }
    switch (type & BODY_FLAGS)
    {
    case 0:          m_body_choice->SetSelection(0) ; break ;
    case BODY_RECT:  m_body_choice->SetSelection(1) ; break ;
    case BODY_ELLIP: m_body_choice->SetSelection(2) ; break ;
    case BODY_TRI:   m_body_choice->SetSelection(3) ; break ;
    }
    switch (type & TAIL_FLAGS)
    {
    case 0:         m_tail_choice->SetSelection(0) ; break ;
    case TAIL_LINE: m_tail_choice->SetSelection(1) ; break ;
    case TAIL_TRI:  m_tail_choice->SetSelection(2) ; break ;
    case TAIL_DOT:  m_tail_choice->SetSelection(3) ; break ;
    }
}


void MFPParamsWindow::set_database(PARAMS_DB *_db)
{
    db = _db ; 
    m_scheme->set_database(_db) ;
    m_head  ->set_database(_db) ;
    m_body  ->set_database(_db) ;
    m_tail  ->set_database(_db) ;
    redisplay() ;
}


void MFPParamsWindow::redisplay()
{
    //long pos ;
    /* not multiply used = "normal" */
    Slider * normal_sliders[] = {
        m_bars_per_line, m_bar_length, m_sub_bars, 
        m_height, m_dynamic, NULL /* the end */ } ;
    wxWindow * widgets[] = { 
        m_head, m_body, m_tail,
        m_head_choice, m_body_choice, m_tail_choice,
        m_bold, m_italic, m_underlined, m_font,
        m_borders, /*AUTO_BACK,*/
        m_scheme, /*BARS_ATTR, SUB_BARS_ATTR, NOTES_ATTR, LINES_ATTR,*/
        NULL /* the end */ } ;

    /* enter filename into window info line */
    wxString title(_T("Parameters")) ;
    if (db)
        title.append(_T(" - ")).append(wxString::FromAscii(db->filename)) ;
    SetTitle(title) ;

    if (!db)
    {
        for (unsigned i = 0 ; normal_sliders[i] ; i++) 
            normal_sliders[i]->Disable() ;
        m_title->SetValue(_T("?")) ;
    }
    else
    {
        char *s ;
        unsigned i = 0;
        validate(db) ;
    
        for (i = 0 ; normal_sliders[i] ; i++) 
            normal_sliders[i]->Enable() ;
        if ( !db->title[0] )
        {
            /* init with file name */
            strcpy(db->title, db->filename) ;
            s = strrchr(db->title, '.') ;
            if (s) *s = 0 ;        /* cut away extension */
        }
        m_title->SetValue(wxString::FromAscii(db->title)) ;
        
        /* set slider positions */
        m_bars_per_line->SetValue(db->bars_per_line) ;
        m_bar_length->SetValue(db->bar_length) ;
        m_sub_bars->SetValue(db->sub_bars) ;
        m_height->SetValue(db->note_height) ;
        m_dynamic->SetValue(db->note_dynscale) ;

        /* horizontal lines */
        switch (db->horizontal_lines)
        {
        case 1: case 2: 
        case 3:    case 4:    
            i = db->horizontal_lines - 1 ; 
            break ;
        case 5:    
            db->horizontal_lines = 6 ; /* 5 is not allowed */
        case 6:  
            i = 4 ;
            break ;
        default: 
            db->horizontal_lines = i = 1 ; /* not allowed */
        }
        m_lines->SetSelection(i) ;
        
        /* transpose */
        i = db->transpose ;
        m_transpose->SetSelection(i) ;
        
        /* mode */
        switch (db->mode)
        {
            case Beyreuther: i = 0 ; break ;
            case Mix:        i = 2 ; break ;
            default:         i = 1 ; /* Rieder */
        }
        m_inventor->SetSelection(i) ;
        
        /* text effects */
        m_bold      ->SetValue((db->effects & TF_THICKENED ) != 0) ;
        m_italic    ->SetValue((db->effects & TF_SLANTED   ) != 0) ;
        m_underlined->SetValue((db->effects & TF_UNDERLINED) != 0) ;
        
        /* borders */
        m_borders   ->SetValue((db->note_type & BORDERS_NONE) == 0) ;
        m_borders_3d->SetValue((db->note_type & BORDERS_3D  ) != 0) ;
    }
    update_border_sliders() ;
    update_distance_slider() ;
    update_3d_checkbox() ;
    update_head_body_tail() ;

    for (unsigned i = 0 ; widgets[i] ; i++) 
    {
        if (db)
             widgets[i]->Enable()  ;
        else widgets[i]->Disable() ;
    }
}
