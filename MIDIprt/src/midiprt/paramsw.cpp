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

#include <graphic.h>
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

    const char *s = " (default)" ;
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
MFPParamsWindow::MFPParamsWindow(wxWindow *parent) : wxDialog(parent, -1, _T(""), wxDefaultPosition, wxDefaultSize)
{
    // set the icon
    SetIcon(wxIcon(apppath + _T(DIRSEP) + _T("params.ico")));

    // create the status line
//    CreateStatusBar(1/*fields*/,0/*without resizing grip*/); 

    // create panel for the controls
    const int sliderFlags = wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS;
    wxFlexGridSizer *grid;
   	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *upper = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *lower = new wxBoxSizer(wxHORIZONTAL);
    
    // title and font controls
    wxStaticBoxSizer *titlebox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Title"));
    wxSize sz = wxButton::GetDefaultSize();
    sz.x *= 4;
    m_title = new wxTextCtrl(titlebox->GetStaticBox(), Control_Title, _T(""), wxDefaultPosition, sz);
    wxBoxSizer *styles = new wxBoxSizer(wxVERTICAL);
    styles->Add(m_bold = new wxCheckBox(titlebox->GetStaticBox(), Control_Bold, _T("&bold")));
    styles->Add(m_italic = new wxCheckBox(titlebox->GetStaticBox(), Control_Italic, _T("&italic")));
    styles->Add(m_underlined = new wxCheckBox(titlebox->GetStaticBox(), Control_Underlined, _T("&underlined")),
        wxSizerFlags().Border(wxBOTTOM+wxRIGHT, MFP_TEXT_SPACING));
    titlebox->Add(m_title, wxSizerFlags().Border(wxLEFT,MFP_SPACING).Center());
    titlebox->Add(styles);
    upper->Add(titlebox, wxSizerFlags().Border(wxALL, MFP_SPACING));
    upper->Add(m_font = new wxButton(this, Control_Font, _T("&Font...")),
        wxSizerFlags().Border(wxRIGHT, MFP_SPACING).Center());

    // left boxes
    sz = wxButton::GetDefaultSize();
    sz.x = sz.y; // quadratic buttons
    wxFlexGridSizer *left = new wxFlexGridSizer(1);
    // borders
    wxStaticBoxSizer *borderbox = new wxStaticBoxSizer(wxVERTICAL, this, _T("Borders in %"));
    const wxString border_choices[2] = {_T("Top / bottom:"),_T("Left / right:")} ;
    m_borders_choice = new wxChoice(borderbox->GetStaticBox(), Control_Borders, wxDefaultPosition,wxDefaultSize, 2,border_choices) ;
    m_borders_choice->SetSelection(0) ;
    wxBoxSizer *sliderbox = new wxBoxSizer(wxHORIZONTAL);
    wxSize slidersz = wxDefaultSize;
    slidersz.x = wxButton::GetDefaultSize().x * 3/2;
    m_left_border = new wxSlider(borderbox->GetStaticBox(), Slider_Left_border , 0, 0, 25, wxDefaultPosition,slidersz, sliderFlags) ;
    m_left_border->SetPageSize(1) ;
    m_right_border = new wxSlider(borderbox->GetStaticBox(), Slider_Right_border, 0, 0, 25, wxDefaultPosition,slidersz, sliderFlags+wxSL_INVERSE) ;
    m_right_border->SetPageSize(1) ;
    sliderbox->Add(m_left_border, wxSizerFlags(1).Expand().Border(wxRIGHT,MFP_SPACING));
    sliderbox->Add(m_right_border, wxSizerFlags(1).Expand().Border(wxLEFT,MFP_SPACING));
    borderbox->Add(m_borders_choice, wxSizerFlags().Border(wxLEFT+wxRIGHT+wxTOP,MFP_TEXT_SPACING).Center());
    borderbox->Add(sliderbox, wxSizerFlags(1).Expand().Border(wxALL,MFP_TEXT_SPACING));
    left->Add(borderbox, wxSizerFlags(1).Expand().Border(wxLEFT+wxBOTTOM,MFP_SPACING));
    // distances
    wxStaticBoxSizer *distbox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Distances in %"));
    const wxString dist_choices[3] = { _T("System:"), _T("Track:"), _T("Note:") } ;
    m_distances = new wxChoice(distbox->GetStaticBox(), Control_Distances, wxDefaultPosition,wxDefaultSize, 3, dist_choices) ;
    m_distances->SetSelection(0) ;
    m_distance = new wxSlider(distbox->GetStaticBox(), Slider_Distance, 0, 0, 10000, wxDefaultPosition,wxDefaultSize, sliderFlags) ;
    distbox->Add(m_distances, wxSizerFlags().Border(wxALL,MFP_TEXT_SPACING).Center());
    distbox->Add(m_distance, wxSizerFlags(1).Expand().Border(wxLEFT+wxRIGHT+wxBOTTOM,MFP_TEXT_SPACING));
    left->Add(distbox, wxSizerFlags(1).Expand().Border(wxLEFT+wxBOTTOM,MFP_SPACING));
    // bars
    wxStaticBoxSizer *barsbox = new wxStaticBoxSizer(wxVERTICAL, this, _T("Bars"));
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(barsbox->GetStaticBox(), -1, _T("Per line:")), wxSizerFlags().Border(wxALL,MFP_TEXT_SPACING).Center()); 
    hbox->Add(new wxButton(barsbox->GetStaticBox(), Control_Bars_width, _T("-"), wxDefaultPosition, sz), wxSizerFlags().Center());
    m_bars_per_line = new wxSlider(barsbox->GetStaticBox(), Slider_Bars, 8, 4, 32, wxDefaultPosition,wxDefaultSize, sliderFlags) ;
    m_bars_per_line->SetPageSize(1) ;
    hbox->Add(m_bars_per_line, wxSizerFlags(1).Expand().Border(wxLEFT+wxRIGHT,MFP_TEXT_SPACING));
    barsbox->Add(hbox, wxSizerFlags(0).Expand());
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(barsbox->GetStaticBox(), -1, _T("Length [1/8]:")), wxSizerFlags().Border(wxALL,MFP_TEXT_SPACING).Center()); 
    m_bar_length = new wxSlider(barsbox->GetStaticBox(), Slider_Bar_length, 8, 4, 32, wxDefaultPosition,wxDefaultSize, sliderFlags) ;
    m_bar_length->SetPageSize(1) ;
    hbox->Add(m_bar_length, wxSizerFlags(1).Expand().Border(wxLEFT+wxRIGHT,MFP_TEXT_SPACING));
    barsbox->Add(hbox, wxSizerFlags(0).Expand());
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(barsbox->GetStaticBox(), -1, _T("Sub-bars:")), wxSizerFlags().Border(wxALL,MFP_TEXT_SPACING).Center());
    hbox->Add(new wxButton(barsbox->GetStaticBox(), Control_Subbars_width, _T("-"), wxDefaultPosition, sz), wxSizerFlags().Center());
    m_sub_bars = new wxSlider(barsbox->GetStaticBox(), Slider_Subbars, 2, 1, 12, wxDefaultPosition,wxDefaultSize, sliderFlags) ;
    m_sub_bars->SetPageSize(1) ;
    hbox->Add(m_sub_bars, wxSizerFlags(1).Expand().Border(wxLEFT+wxRIGHT+wxBOTTOM,MFP_TEXT_SPACING));
    barsbox->Add(hbox, wxSizerFlags(0).Expand());
    left->Add(barsbox, wxSizerFlags(1).Border(wxLEFT+wxBOTTOM,MFP_SPACING).Expand());
    // notation type
    wxStaticBoxSizer *ntypebox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Notation type"));
    grid = new wxFlexGridSizer(2);
    grid->Add(new wxStaticText(ntypebox->GetStaticBox(), -1, _T("Inventor:")), wxSizerFlags().Border(wxTOP+wxRIGHT,MFP_TEXT_SPACING));
    const wxString inv_choices[3] = { _T("Beyreuther"), _T("Rieder"), _T("Mix") } ;
    m_inventor = new wxChoice(ntypebox->GetStaticBox(), Control_Inventor, wxDefaultPosition,wxDefaultSize, 3,inv_choices) ;
    grid->Add(m_inventor, wxSizerFlags().Border(wxBOTTOM,MFP_TEXT_SPACING));
    grid->Add(new wxStaticText(ntypebox->GetStaticBox(), -1, _T("Horizontal lines:")), wxSizerFlags().Border(wxTOP+wxRIGHT,MFP_TEXT_SPACING)); 
    wxBoxSizer *hlinesbox = new wxBoxSizer(wxHORIZONTAL);
    hlinesbox->Add(new wxButton(ntypebox->GetStaticBox(), Control_Horlines_width, _T("-"), wxDefaultPosition,sz));
    const wxString lines_choices[5] = { _T("1"), _T("2"), _T("3"), _T("4"), _T("6") } ;
    m_lines = new wxChoice(ntypebox->GetStaticBox(), Control_Horlines, wxDefaultPosition,wxDefaultSize, 5,lines_choices) ;
    hlinesbox->Add(m_lines, wxSizerFlags().Border(wxLEFT+wxRIGHT,MFP_SPACING));
    grid->Add(hlinesbox, wxSizerFlags(1).Expand());
    ntypebox->Add(grid, wxSizerFlags(1).Border(wxALL,MFP_TEXT_SPACING).Expand());
    left->Add(ntypebox, wxSizerFlags(0).Border(wxLEFT+wxBOTTOM,MFP_SPACING).Expand());

    // notes & color scheme
    wxStaticBoxSizer *right = new wxStaticBoxSizer(wxVERTICAL, this, _T("Notes && Colors"));
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(right->GetStaticBox(), -1, _T("Average height:")), wxSizerFlags().Border(wxLEFT+wxRIGHT,MFP_TEXT_SPACING).Center());
    m_height = new wxSlider(right->GetStaticBox(), Slider_Height, 5, 1, 9, wxDefaultPosition,wxDefaultSize, sliderFlags) ;
    m_height->SetPageSize(1) ;
    hbox->Add(m_height, wxSizerFlags(1).Expand().Border(wxRIGHT,MFP_TEXT_SPACING));
    right->Add(hbox, wxSizerFlags(0).Expand());
    hbox = new wxBoxSizer(wxHORIZONTAL);
    hbox->Add(new wxStaticText(right->GetStaticBox(), -1, _T("Dynamic:")), wxSizerFlags().Border(wxLEFT+wxRIGHT,MFP_TEXT_SPACING).Center());
    m_dynamic = new wxSlider(right->GetStaticBox(), Slider_Dynamic, 0, 0, 15, wxDefaultPosition,wxDefaultSize, sliderFlags) ;
    m_dynamic->SetPageSize(1) ;
    hbox->Add(m_dynamic, wxSizerFlags(1).Expand().Border(wxRIGHT,MFP_TEXT_SPACING));
    right->Add(hbox, wxSizerFlags(0).Expand());
    const wxString ends_choices[4] = { _T("(none)"), _T("line"), _T("triangle"), _T("dot")} ;
    const wxString body_choices[4] = { _T("(none)"), _T("bar"), _T("ellipse"), _T("triangle")} ;
    grid = new wxFlexGridSizer(3);
    grid->Add(new wxStaticText(right->GetStaticBox(), -1, _T("Heads:")));
    grid->Add(m_head = new NoteWidget(right->GetStaticBox(), Widget_Head ,wxDefaultPosition,wxButton::GetDefaultSize()));
    grid->Add(m_head_choice = new wxChoice(right->GetStaticBox(), Control_Head, wxDefaultPosition,wxDefaultSize,4,ends_choices));
    grid->Add(new wxStaticText(right->GetStaticBox(), -1, _T("Bodies:")));
    grid->Add(m_body = new NoteWidget(right->GetStaticBox(), Widget_Body, wxDefaultPosition,wxButton::GetDefaultSize())); 
    grid->Add(m_body_choice = new wxChoice(right->GetStaticBox(), Control_Body, wxDefaultPosition,wxDefaultSize,4,body_choices));
    grid->Add(new wxStaticText(right->GetStaticBox(), -1, _T("Tails:"))); 
    grid->Add(m_tail = new NoteWidget(right->GetStaticBox(), Widget_Tail, wxDefaultPosition,wxButton::GetDefaultSize()));
    grid->Add(m_tail_choice = new wxChoice(right->GetStaticBox(), Control_Tail, wxDefaultPosition,wxDefaultSize,4,ends_choices));
    right->Add(grid, wxSizerFlags().Border(wxALL,MFP_TEXT_SPACING));
    wxBoxSizer *noteborderbox = new wxBoxSizer(wxHORIZONTAL);
    noteborderbox->Add(new wxButton(right->GetStaticBox(), Control_Noteborders_width, _T("-"), wxDefaultPosition, sz), wxSizerFlags().Border(wxLEFT+wxRIGHT,MFP_TEXT_SPACING));
    noteborderbox->Add(m_borders = new wxCheckBox(right->GetStaticBox(), Control_Noteborders, _T("Note borders")), wxSizerFlags().Center());
    noteborderbox->Add(m_borders_3d = new wxCheckBox(right->GetStaticBox(), Control_Notes_3D, _T("3D notes")), wxSizerFlags().Center()) ;
    right->Add(noteborderbox, wxSizerFlags().Center().Border(wxTOP,MFP_TEXT_SPACING));
    wxBoxSizer *transpbox = new wxBoxSizer(wxHORIZONTAL);
    transpbox->Add(new wxStaticText(right->GetStaticBox(), -1, _T("Transpose:")), wxSizerFlags().Border(wxALL,MFP_SPACING).Center()); 
    const wxString transp_choices[12] = 
        { _T("0 (none)"),_T("+1 = -B"),_T("+2 = -A"),_T("+3 = -9"),_T("+4 = -8"),_T("+5 = -7"),
          _T("+6 = -6"),_T("+7 = -5"),_T("+8 = -4"),_T("+9 = -3"),_T("+A = -2"),_T("+B = -1") } ;
    m_transpose = new wxChoice(right->GetStaticBox(), Control_Transpose, wxDefaultPosition,wxDefaultSize, 12,transp_choices);
    transpbox->Add(m_transpose, wxSizerFlags().Border(wxALL,MFP_SPACING).Center());
    right->Add(transpbox, wxSizerFlags().Center().Border(wxALL,MFP_TEXT_SPACING));
    // color scheme
    sz.x = wxButton::GetDefaultSize().x*3;
    sz.y = wxButton::GetDefaultSize().x*2;
    right->Add(m_scheme = new SchemeWidget(right->GetStaticBox(), wxDefaultPosition, sz),
        wxSizerFlags(1).Border(wxALL, MFP_TEXT_SPACING).Expand());

    lower->Add(left);
    lower->Add(right, wxSizerFlags(1).Border(wxLEFT+wxBOTTOM+wxRIGHT,MFP_SPACING).Expand());
    topsizer->Add(upper);
    topsizer->Add(lower);
    SetSizerAndFit(topsizer);

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
    //sprintf(buf, "%d", value) ;
    //m_left_val->SetValue(wxString::FromAscii(buf)) ;
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
    //sprintf(buf, "%d", value) ;
    //m_right_val->SetValue(wxString::FromAscii(buf)) ;
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
    
    //char buf[10] ;
    int value = m_distance->GetValue() ;
    switch (m_distances->GetSelection())
    {
    case 0:
        db->system_distance = value ;
        //sprintf(buf, "%d", value) ;
        break ;
    case 1:
        db->track_distance = value ;
        //sprintf(buf, "%d", value) ;
        break ;
    case 2:
        {
            float distance = (float)(value+1) / 20 ;
            db->note_distance = distance ;
            //sprintf(buf, "%.2f", distance) ;
        }
        break ;
    }
    //m_distance_val->SetValue(wxString::FromAscii(buf)) ;
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
    wxSlider * normal_sliders[] = {
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
