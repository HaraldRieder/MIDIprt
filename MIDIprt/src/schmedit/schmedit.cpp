/*****************************************************************************
  Purpose:     Scheme Editor for Mad Harry's MIDI File Printer color schemes
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: schmedit.cpp,v 1.5 2008/09/20 20:07:52 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#define TITLE "Mad Harry's Scheme Editor"

// window position and size
static const int FRAME_W = 480, FRAME_H = 720 ; // in pels

// widths and heights in percent of the panel width and heights
// (will be transformed to pels later)
static int DODEC_W = 21 ;
static int DODEC_H = 66 ;

// distance between control and frame(s) in pels
static const int SPACING = 10 ;   

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
  #include "wx/wx.h"
#endif

#include  "wx/ownerdrw.h"
#include  "wx/menuitem.h"
#include  "wx/checklst.h"

#pragma warning( disable : 4786 ) // otherwise too many warnings because of <set> compiliation
#include <set>
#include <graphicf.h>
#include <colcube.h>
#include <wxVDI.h>
#include "scheme.h"
//#include "ICON.XPM"

/** database of the application */
class SchemeEditorData
{
public:
  SchemeEditorData() ;
  wxString dir ;
  wxString path ;
  wxString file ;   /* file part of path (basename) */
  COLOR_SCHEME scheme ;
  int current_dodecime ;
} ;

SchemeEditorData::SchemeEditorData()
{
  default_scheme(&scheme) ;
  current_dodecime = 0 ;
}

/** selectable base object */
class SelectableWidget: public wxWindow
{
public:
  bool isSelected() { return selected ; }
  void select(bool _select = true) ;
  virtual void setColor(int color) = 0 ;
  virtual void setStyle(int style) = 0 ;
  static std::set<SelectableWidget *> selectionSet ;
  void OnToggle(wxMouseEvent& WXUNUSED(event)) { select(!selected) ; }
protected:
  SelectableWidget(SchemeEditorData *db, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  void draw(int style, int color, bool border, char *text = NULL) ;
  SchemeEditorData *db ;
  bool selected ;
};

/** selected SelectableWidget objects are contained in this set */
std::set<SelectableWidget *> SelectableWidget::selectionSet ;

SelectableWidget::SelectableWidget(SchemeEditorData *_db, wxWindow *parent, const wxPoint& pos, const wxSize& size)
 : wxWindow(parent, -1, pos, size), db(_db), selected(false) {  }

void SelectableWidget::select(bool _select)
{
  selected = _select  ; 
  if (selected)
    selectionSet.insert(this) ;
  else
    selectionSet.erase(this) ;
  Refresh() ;
}

void SelectableWidget::draw(int style, int color, bool border, char *text)
{
  wxPaintDC dc(this);
  //dc.BeginDrawing() ;
  VirtualDevice vdi(&dc) ;

  int points[10], type = BODY_RECT ;
  
  points[0] = points[2] = points[8] = 0 ; 
  points[1] = points[7] = points[9] = 0 ; 
  points[3] = points[5] = GetSize().GetHeight() - 1 ;
  points[4] = points[6] = GetSize().GetWidth() - 1 ; 

  if (selected)
  {
    vsl_color(&vdi, BLACK) ;
    vsl_type(&vdi, DOT) ;
    v_pline(&vdi, 5, points) ;
    points[0] = 1 ; 
    points[1] = 1 ; 
    points[2] = GetSize().GetWidth() - 2 ; 
    points[3] = GetSize().GetHeight() - 2 ;
  }
  else
  {
    // fill all with draw_note
    points[0] = 0 ; 
    points[1] = 0 ; 
    points[2] = GetSize().GetWidth() - 1 ; 
    points[3] = GetSize().GetHeight() - 1 ;
  }
  if (!border)
    type |= BORDERS_NONE ;
  draw_note(&vdi, points, type, style, color, 0,0) ;

  // draw text
  if (text != NULL)
  {
    int dummy ;
    vswr_mode(&vdi, MD_TRANS) ;
    vst_point(&vdi, 8, &dummy, &dummy, &dummy, &dummy ) ;
    vst_font(&vdi, (char *)"") ;   
    rgb_tcolor(&vdi, db->scheme.text_color) ;
    vst_alignment(&vdi, 1, 2, &dummy, &dummy) ;
    v_gtext(&vdi, GetSize().GetWidth()/2, GetSize().GetHeight()/2, text) ;
  }
  //dc.EndDrawing() ;
}

/** dodecime owner drawn objects */
class DodecimeWidget: public SelectableWidget
{
public:
  DodecimeWidget(SchemeEditorData *db, int index, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  virtual void setColor(int color) { db->scheme.dodecime_color[index] = color ; Refresh() ; }
  virtual void setStyle(int style) { db->scheme.dodecime_style[index] = style ; Refresh() ; }
private:
  void OnPaint(wxPaintEvent& event);
  int index ; // dodecime index 0..10
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(DodecimeWidget, SelectableWidget)
  EVT_PAINT(DodecimeWidget::OnPaint)
  EVT_LEFT_UP(SelectableWidget::OnToggle)
END_EVENT_TABLE()

DodecimeWidget::DodecimeWidget(SchemeEditorData *_db, int _index, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: SelectableWidget(_db, parent, pos, size), index(_index) { }

void DodecimeWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  /* find dodecime index */
  int style, color ;
  if (index < 0)
  {
    style = db->scheme.dodecime_style[db->current_dodecime] ; 
    color = db->scheme.dodecime_color[db->current_dodecime] ;
    //::draw(this, style, color, false) ;
    draw(style, color, false) ;
  }
  style = db->scheme.dodecime_style[index] ; 
  color = db->scheme.dodecime_color[index] ;
  //::draw(this, style, color, false) ;
  draw(style, color, false) ;
}

/** background owner drawn object */
class BackgroundWidget: public SelectableWidget
{
public:
    BackgroundWidget(SchemeEditorData *db, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  virtual void setColor(int color) ;
  virtual void setStyle(int style) ;
  void setSchemeBars(wxWindow *scheme, wxWindow *bars) { schemeWidget = scheme ; barsWidget = bars ; }
private:
  wxWindow * schemeWidget ;
  wxWindow * barsWidget ;
  void OnPaint(wxPaintEvent& event);
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BackgroundWidget, wxWindow)
    EVT_PAINT(BackgroundWidget::OnPaint)
  EVT_LEFT_UP(SelectableWidget::OnToggle)
END_EVENT_TABLE()

BackgroundWidget::BackgroundWidget(SchemeEditorData *_db, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: SelectableWidget(_db, parent, pos, size) { }

void BackgroundWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{ 
  draw(db->scheme.back_style, db->scheme.back_color, false) ; 
}

void BackgroundWidget::setColor(int color)
{ 
  db->scheme.back_color = color ; 
  Refresh() ; 
  if (schemeWidget)
    schemeWidget->Refresh() ;
  if (barsWidget)
    barsWidget->Refresh() ;
}

void BackgroundWidget::setStyle(int style) 
{ 
  db->scheme.back_style = style ; 
  Refresh() ; 
  if (schemeWidget)
    schemeWidget->Refresh() ;
  if (barsWidget)
    barsWidget->Refresh() ;
}

/** bars owner drawn object */
class BarsWidget: public SelectableWidget
{
public:
    BarsWidget(wxWindow *scheme, SchemeEditorData *db, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  virtual void setColor(int color) ;
  virtual void setStyle(int WXUNUSED(style)) { } /**< there is no text style up to now */
private:
  wxWindow * schemeWidget ;
    void OnPaint(wxPaintEvent& event);
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BarsWidget, wxWindow)
  EVT_PAINT(BarsWidget::OnPaint)
  EVT_LEFT_UP(SelectableWidget::OnToggle)
END_EVENT_TABLE()

BarsWidget::BarsWidget(wxWindow *scheme, SchemeEditorData *_db, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: SelectableWidget(_db, parent, pos, size), schemeWidget(scheme) { }

void BarsWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{ draw(db->scheme.back_style, db->scheme.back_color, false, (char *)"Bars"/*, db->scheme.text_color*/) ; }

void BarsWidget::setColor(int color)
{
  db->scheme.text_color = color ; 
  Refresh() ;
  if (schemeWidget)
    schemeWidget->Refresh() ;
}

/** scheme owner drawn object */
class SchemeWidget: public wxWindow
{
public:
    SchemeWidget(SchemeEditorData *db, wxWindow *parent, const wxPoint& pos, const wxSize& size);
private:
    void OnPaint(wxPaintEvent& event);
    SchemeEditorData *db ;
    DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(SchemeWidget, wxWindow)
  EVT_PAINT(SchemeWidget::OnPaint)
END_EVENT_TABLE()

SchemeWidget::SchemeWidget(SchemeEditorData *_db, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: wxWindow(parent, -1, pos, size), db(_db) { }

void SchemeWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  wxSize sz = GetSize() ;
  wxPaintDC dc(this);
  //dc.BeginDrawing() ;
  VirtualDevice vdi(&dc) ;
  draw_scheme(&vdi, &(db->scheme), 0 /*no transpose*/,
    0, 0, sz.GetWidth(), sz.GetHeight(),
    db->current_dodecime, 2 /* lines */, -1 /* dots */, between, 
    0 /* no notes */, FALSE) ;
  //dc.EndDrawing() ;
}

/** note ends owner drawn objects */
class EndsWidget: public SelectableWidget
{
public:
  EndsWidget(SchemeEditorData *_db, int _index, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  virtual void setColor(int color) ; 
  virtual void setStyle(int style) ; 
  /** if this is a head, other points to the tail, and vice versa */
  void setOther(EndsWidget * _other) { other = _other ; }
private:
  void OnPaint(wxPaintEvent& event);
  void OnToggle(wxMouseEvent& event);
  void doToggle(wxMouseEvent& event) { SelectableWidget::OnToggle(event) ; }
  int index ; /**< note index 0..11 */
  EndsWidget * other ; 
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(EndsWidget, wxWindow)
  EVT_PAINT(EndsWidget::OnPaint)
  EVT_LEFT_UP(EndsWidget::OnToggle)
END_EVENT_TABLE()

EndsWidget::EndsWidget(SchemeEditorData *_db, int _index, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: SelectableWidget(_db, parent, pos, size), index(_index), other(NULL) { }

void EndsWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{ draw(db->scheme.note_style_ends[index], db->scheme.note_color_ends[index], true) ; }

void EndsWidget::OnToggle(wxMouseEvent& event)
{ 
  doToggle(event) ;
  if (other)
    other->doToggle(event) ;
}

void EndsWidget::setColor(int color)
{
  if (color == db->scheme.note_color_ends[index])
    return ;
  db->scheme.note_color_ends[index] = color ;
  Refresh() ;
  if (other)
    other->Refresh() ;
}

void EndsWidget::setStyle(int style)
{
  if (style == db->scheme.note_style_ends[index])
    return ;
  db->scheme.note_style_ends[index] = style ;
  Refresh() ;
  if (other)
    other->Refresh() ;
}

/** note body owner drawn objects */
class BodyWidget: public SelectableWidget
{
public:
  BodyWidget(SchemeEditorData *_db, int _index, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  virtual void setColor(int color) { db->scheme.note_color[index] = color ; Refresh() ; }
  virtual void setStyle(int style) { db->scheme.note_style[index] = style ; Refresh() ; }
private:
  void OnPaint(wxPaintEvent& event);
  int index ; // note index 0..11 
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(BodyWidget, wxWindow)
  EVT_PAINT(BodyWidget::OnPaint)
  EVT_LEFT_UP(SelectableWidget::OnToggle)
END_EVENT_TABLE()

BodyWidget::BodyWidget(SchemeEditorData *_db, int _index, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: SelectableWidget(_db, parent, pos, size), index(_index) { }

void BodyWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{ draw(db->scheme.note_style[index], db->scheme.note_color[index], true) ; }


/** control IDs (menu items, buttons) */
enum 
{
  Menu_First = 100,
  Control_First = 1000,
  Control_Dodec_select,
  Control_Dodec_all, Control_Dodec_rot, Control_Dodec_swap,
  Control_Body_all , Control_Body_rot , Control_Body_swap ,
  Control_Ends_all , Control_Ends_rot , Control_Ends_swap ,
  Control_Note_all , Control_Note_rot , Control_Note_swap , Control_Note_66, Control_Note_75,
  Control_Style /* 9 styles reserved, should be the last entry! */
};

/** style owner drawn controls */
class StyleWidget: public SelectableWidget
{
public:
  StyleWidget(int _style, SchemeWidget * scheme, wxWindow *parent, const wxPoint& pos, const wxSize& size);
  virtual void setColor(int WXUNUSED(color)) { } /**< here a dummy */
  virtual void setStyle(int WXUNUSED(style)) { } /**< here a dummy */
  void setCurrentDodec(DodecimeWidget *dodecWidget) { currentDodec = dodecWidget ; }
private:
  DodecimeWidget * currentDodec ;
  SchemeWidget * schemeWidget ;
  void OnPaint(wxPaintEvent& event);
  void OnClick(wxMouseEvent& event);
  int style ; /**< fill style index */
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(StyleWidget, wxWindow)
  EVT_PAINT(StyleWidget::OnPaint)
  EVT_LEFT_UP(StyleWidget::OnClick)
END_EVENT_TABLE()

StyleWidget::StyleWidget(int _style, SchemeWidget * scheme, wxWindow *parent, const wxPoint& pos, const wxSize& size)
: SelectableWidget(NULL, parent, pos, size), schemeWidget(scheme), currentDodec(NULL), style(_style) { }

void StyleWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{ draw(style, 0x0/*black*/, true) ; }

void StyleWidget::OnClick(wxMouseEvent& WXUNUSED(event))
{
  std::set<SelectableWidget *>::iterator it ;
  for (it = selectionSet.begin() ; it != selectionSet.end() ; ++it)
  {
    (*it)->setStyle(style) ;
    if (*it == currentDodec)
      schemeWidget->Refresh() ;
  }
}

/** color cube owner drawn object */
class CubeWidget: public wxWindow
{
public:
  CubeWidget(SchemeWidget * scheme, wxWindow *parent, const wxPoint& pos, const wxSize& size, bool black=false);
  void setCurrentDodec(DodecimeWidget *dodecWidget) { currentDodec = dodecWidget ; }
private:
  COLOR_CUBE cube ;
  SchemeWidget * schemeWidget ;
  DodecimeWidget * currentDodec ;
  void OnPaint(wxPaintEvent& event);
  void OnClick(wxMouseEvent& event);
  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(CubeWidget, wxWindow)
  EVT_PAINT(CubeWidget::OnPaint)
  EVT_LEFT_UP(CubeWidget::OnClick)
END_EVENT_TABLE()

CubeWidget::CubeWidget(SchemeWidget * scheme, wxWindow *parent, const wxPoint& pos, const wxSize& size, bool black)
 : wxWindow(parent, -1, pos, size), schemeWidget(scheme), currentDodec(NULL)
{
  init_color_cube(&cube, size.GetWidth(), size.GetHeight(), black) ;
}

void CubeWidget::OnPaint(wxPaintEvent& WXUNUSED(event))
{
  wxPaintDC dc(this);
  //dc.BeginDrawing() ;
  VirtualDevice vdi(&dc) ;
  draw_cube(&vdi, &cube, 0,0) ;
  //dc.EndDrawing() ;
}

void CubeWidget::OnClick(wxMouseEvent& event)
{
  int color = get_color_from_xy(&cube, event.GetX(), event.GetY()) ;
  if (color < 0)
    return ; // user clicked outside the cube
  std::set<SelectableWidget *>::iterator it ;
  for (it = SelectableWidget::selectionSet.begin() ; it != SelectableWidget::selectionSet.end() ; ++it)
  {
    (*it)->setColor(color) ;
    if (*it == currentDodec)
      schemeWidget->Refresh() ;
  }
}

/** The scheme editor application. */
class SchemeEditorApplication: public wxApp
{
public:
  bool OnInit();
private:
  SchemeEditorData db ;  // application database
};


#define N_DODECIMES 11 // 11 * 12 > 128 MIDI note numbers
#define N_NOTES     12 // 12 notes per dodecime

/** The scheme editor frame window. */
class SchemeEditorFrame : public wxFrame
{
public:
  // ctor & dtor
  SchemeEditorFrame(SchemeEditorData *db, wxFrame *frame, const wxString &title, const wxPoint & pos, const wxSize & size);
  ~SchemeEditorFrame() { unload_scheme() ; }

  // notifications
  void OnQuit             (wxCommandEvent& event);
  void OnOpen             (wxCommandEvent& event);
  void OnSave             (wxCommandEvent& event);
  void OnSaveAs           (wxCommandEvent& event);
  void OnRevertToSaved    (wxCommandEvent& event);
  void OnAbout            (wxCommandEvent& event);
  bool OnClose            () ;
  void OnDodecSelect      (wxCommandEvent& event);
  void OnDodecAll         (wxCommandEvent& event);
  void OnDodecRot         (wxCommandEvent& event);
  void OnDodecSwap        (wxCommandEvent& event);
  void OnEndsAll          (wxCommandEvent& event);
  void OnEndsRot          (wxCommandEvent& event);
  void OnEndsSwap         (wxCommandEvent& event);
  void OnBodyAll          (wxCommandEvent& event);
  void OnBodyRot          (wxCommandEvent& event);
  void OnBodySwap         (wxCommandEvent& event);
  void OnNoteAll          (wxCommandEvent& event);
  void OnNoteRot          (wxCommandEvent& event);
  void OnNoteSwap         (wxCommandEvent& event);
  void OnNote66           (wxCommandEvent& event);
  void OnNote75           (wxCommandEvent& event);

  DECLARE_EVENT_TABLE()

private:
  void InitMenu();

  /** 
  * Loads scheme file and shows error messages if necessary.
  * @return true if loaded successfully 
  */
  bool load_scheme(const char * path) ;

  /**
  * Called to guarantee that the user does not loose changes.
  */
  void unload_scheme() ;

  /**
  * Redraws all dodecime widgets in the frame window.
  */
  void redisplay_dodecimes() ;

  void redisplay_heads () ;
  void redisplay_bodies() ;
  void redisplay_tails () ;

  /** consistently updates current dodecime index to i */
  void currentDodec(int i) ;

  SchemeWidget     * m_scheme ;             /**< the scheme on screen */
  BarsWidget       * m_bars ;               /**< bars text */
  BackgroundWidget * m_background ;         /**< staves background */
  DodecimeWidget   * m_dodec[N_DODECIMES] ; /**< dodecime background on screen */
  EndsWidget       * m_head[N_NOTES] ;      /**< note heads */
  SelectableWidget * m_body[N_NOTES] ;      /**< note bodies */
  EndsWidget       * m_tail[N_NOTES] ;      /**< note tails */
  /** color and style controls */
  CubeWidget       * m_lightCube ;
  CubeWidget       * m_darkCube ;
  StyleWidget      * m_style[IP_SOLID - IP_HOLLOW + 1] ;
  wxMenu *m_file_menu ;            /**< the file menu */
  SchemeEditorData *db ;            /**< application database */
};

BEGIN_EVENT_TABLE(SchemeEditorFrame, wxFrame)
    EVT_MENU(wxID_OPEN  , SchemeEditorFrame::OnOpen         )
    EVT_MENU(wxID_EXIT  , SchemeEditorFrame::OnQuit         )
    EVT_MENU(wxID_SAVE  , SchemeEditorFrame::OnSave         )
    EVT_MENU(wxID_SAVEAS, SchemeEditorFrame::OnSaveAs       )
    EVT_MENU(wxID_REVERT, SchemeEditorFrame::OnRevertToSaved)
    EVT_COMMAND(Control_Dodec_select, wxEVT_COMMAND_CHOICE_SELECTED, SchemeEditorFrame::OnDodecSelect)
    EVT_COMMAND(Control_Dodec_all , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnDodecAll )
    EVT_COMMAND(Control_Dodec_rot , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnDodecRot )
    EVT_COMMAND(Control_Dodec_swap, wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnDodecSwap)
    EVT_COMMAND(Control_Ends_all  , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnEndsAll  )
    EVT_COMMAND(Control_Ends_rot  , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnEndsRot  )
    EVT_COMMAND(Control_Ends_swap , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnEndsSwap )
    EVT_COMMAND(Control_Body_all  , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnBodyAll  )
    EVT_COMMAND(Control_Body_rot  , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnBodyRot  )
    EVT_COMMAND(Control_Body_swap , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnBodySwap )
    EVT_COMMAND(Control_Note_all  , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnNoteAll  )
    EVT_COMMAND(Control_Note_rot  , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnNoteRot  )
    EVT_COMMAND(Control_Note_swap , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnNoteSwap )
    EVT_COMMAND(Control_Note_66   , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnNote66   )
    EVT_COMMAND(Control_Note_75   , wxEVT_COMMAND_BUTTON_CLICKED, SchemeEditorFrame::OnNote75   )
END_EVENT_TABLE()

IMPLEMENT_APP(SchemeEditorApplication);


/** init application: create frame window */
bool SchemeEditorApplication::OnInit(void)
{
    SchemeEditorFrame *pFrame
      = new SchemeEditorFrame(&db, NULL, _T(TITLE),
          wxDefaultPosition, wxSize(FRAME_W, FRAME_H));
    SetTopWindow(pFrame);
    return true;
}

/** create the menu bar for the main frame */
void SchemeEditorFrame::InitMenu()
{
    // construct menu
    m_file_menu = new wxMenu;
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_OPEN, _T("&Open..."), _T("Open color scheme file (*.PAR) for modification"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_SAVE, _T("&Save"), _T("Save changes to color scheme file"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_SAVEAS, _T("Save &as..."), _T("Save scheme to another color scheme file"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_REVERT, _T("&Revert to saved..."), _T("Reload from disk and loose changes"), wxITEM_NORMAL));
    m_file_menu->AppendSeparator();
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_EXIT, _T("&Quit"), _T("Quit "TITLE), wxITEM_NORMAL));
    m_file_menu->Enable(wxID_SAVE, false);
    m_file_menu->Enable(wxID_REVERT, false);

    // make a menubar
    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(m_file_menu, _T("&File"));
    SetMenuBar(menu_bar);
}

/** main frame constructor */
SchemeEditorFrame::SchemeEditorFrame(SchemeEditorData *_db, 
     wxFrame *frame, const wxString & title, const wxPoint & pos, const wxSize & size)
: wxFrame(frame, -1, title, pos, size, wxMINIMIZE_BOX|wxCAPTION|wxSYSTEM_MENU), db(_db)
{
    // set the icon
//#ifdef _WINDOWS    
    SetIcon(wxIcon(_T("SCHEME")));
//#else
//    SetIcon(wxIcon(_T("ICON")));
//#endif

    // create the menu
    InitMenu();

    // create the status line
    CreateStatusBar(1/*fields*/,0/*without resizing grip*/); 

    // create panel for the controls
    int PANEL_W, PANEL_H ;
    DoGetClientSize(&PANEL_W, &PANEL_H) ;

    wxPanel *panel = new wxPanel(this, -1, wxPoint(0, 0), 
                                  wxSize(PANEL_W, PANEL_H), wxTAB_TRAVERSAL);

    // make all relative widths and heights absolute
    DODEC_W = DODEC_W * PANEL_W / 100 ;
    DODEC_H = DODEC_H * PANEL_H / 100 ;

    // create controls for dodecimes
    DODEC_W = DODEC_W - SPACING ;
    new wxStaticBox(panel, -1, _T("dodec"), wxPoint(SPACING/2, SPACING/2), wxSize(DODEC_W-SPACING, DODEC_H-SPACING));
    int w = DODEC_W - 3 * SPACING ;
    int lines = 17 ;
    int h = (DODEC_H - 3 * SPACING)/lines ; // of one "line" (control)
    const wxSize sz(w,h) ;
    int x = SPACING * 3/2;
    int y = SPACING * 5/2 ;
    
    new wxButton(panel, Control_Dodec_all , _T("all") , wxPoint(x,y)    , sz) ;
    new wxButton(panel, Control_Dodec_rot , _T("rot."), wxPoint(x,y+h)  , sz) ;
    new wxButton(panel, Control_Dodec_swap, _T("swap"), wxPoint(x,y+2*h), sz) ;
    new wxStaticText(panel, -1, _T("View:"), wxPoint(x,y+3*h+2), sz) ;
    const wxString dodec_names[N_DODECIMES] =
    {
        _T("0"),_T("1"),_T("2"),_T("3"),_T("4"),
        _T("5"),_T("6"),_T("7"),_T("8"),_T("9"),_T("A")
    } ;
    wxChoice * choice = new wxChoice(panel, Control_Dodec_select, wxPoint(x,y+4*h), sz, 11, dodec_names) ;
    choice->SetSelection(0) ;
    wxSize half_sz(sz.GetWidth()/2, sz.GetHeight()) ;
    for (unsigned i = 0 ; i < N_DODECIMES ; i++)
    {
      int y_i = y+(16-i)*h - SPACING/2 ;
      new wxStaticText(panel, -1, dodec_names[i], wxPoint(x,y_i), half_sz) ;
      m_dodec[i] = 
        new DodecimeWidget(db, i, panel, wxPoint(x+SPACING/2+half_sz.GetWidth(),y_i), half_sz);
    }

    // create controls for notes
    // button size shall be the same as calculated for dodecime buttons
    int NOTE_W = sz.GetWidth() * 3/*buttons*/ + SPACING * 7 ; 
    int NOTE_H = sz.GetHeight() * 5/*buttons*/ + SPACING + SPACING * 3/2;
    x = PANEL_W - NOTE_W - SPACING/2 ;
    new wxStaticBox( panel, -1, _T("note"), wxPoint(x, SPACING/2), wxSize(NOTE_W, NOTE_H));
    x += SPACING ;
    y = SPACING * 5/2 ;
    new wxButton(panel, Control_Note_all , _T("all") , wxPoint(x,y                 ), sz) ;
    new wxButton(panel, Control_Note_75  , _T("7+5") , wxPoint(x,y+  sz.GetHeight()), sz) ;
    new wxButton(panel, Control_Note_66  , _T("6+6") , wxPoint(x,y+2*sz.GetHeight()), sz) ;
    new wxButton(panel, Control_Note_rot , _T("rot."), wxPoint(x,y+3*sz.GetHeight()), sz) ;
    new wxButton(panel, Control_Note_swap, _T("swap"), wxPoint(x,y+4*sz.GetHeight()), sz) ;

    // controls for note ends
    x = PANEL_W - NOTE_W + SPACING + sz.GetWidth() ;
    y = SPACING*3/2 + sz.GetHeight()/2 ;
    h = sz.GetHeight() * 3/*buttons*/ + SPACING + SPACING * 3/2;
    new wxStaticBox(panel, -1, _T("ends"), wxPoint(x,y), wxSize(sz.GetWidth() + SPACING*2, h)) ;
    x += SPACING ;
    y += SPACING*2 ;
    new wxButton(panel, Control_Ends_all , _T("all") , wxPoint(x,y)                 , sz) ;
    new wxButton(panel, Control_Ends_rot , _T("rot."), wxPoint(x,y+sz.GetHeight())  , sz) ;
    new wxButton(panel, Control_Ends_swap, _T("swap"), wxPoint(x,y+2*sz.GetHeight()), sz) ;

    // controls for note bodies
    x += (sz.GetWidth() + SPACING*3/2) ;
    y = SPACING*3/2 + sz.GetHeight()/2 ;
    h = sz.GetHeight() * 3/*buttons*/ + SPACING + SPACING * 3/2;
    new wxStaticBox(panel, -1, _T("body"), wxPoint(x,y), wxSize(sz.GetWidth() + SPACING*2, h)) ;
    x += SPACING ;
    y += SPACING*2 ;
    new wxButton(panel, Control_Body_all , _T("all") , wxPoint(x,y)                 , sz) ;
    new wxButton(panel, Control_Body_rot , _T("rot."), wxPoint(x,y+sz.GetHeight())  , sz) ;
    new wxButton(panel, Control_Body_swap, _T("swap"), wxPoint(x,y+2*sz.GetHeight()), sz) ;

    // the scheme
    int SCHEME_X = DODEC_W + SPACING/2 ;
    int SCHEME_Y = NOTE_H + SPACING ;
    int SCHEME_W = PANEL_W - DODEC_W - SPACING ;
    int SCHEME_H = DODEC_H - NOTE_H - SPACING*3/2 ;
    m_scheme = new SchemeWidget(_db, panel, wxPoint(SCHEME_X,SCHEME_Y), wxSize(SCHEME_W,SCHEME_H));
    // background is drawn inside scheme
    {
      int w = SCHEME_W/2 - 6 ;
      int h = SCHEME_H*11/24 - 5 ;
      wxWindow * gray = new wxPanel(m_scheme, -1, wxPoint(3,2), wxSize(w,h)/*, wxTAB_TRAVERSAL*/);
      m_background = new BackgroundWidget(_db, gray, wxPoint(2,2), wxSize(w-4,h-4)) ; 
    }
    w = SCHEME_W / (N_NOTES*4) ;
    h = SCHEME_H / N_NOTES ;
    for (unsigned j = 0 ; j < N_NOTES ; j++)
    {
        x = j * 4 * SCHEME_W / (N_NOTES*4) ;
        y = (11-j) * SCHEME_H / N_NOTES ;
        m_head[j] = new EndsWidget(_db, j, m_scheme, wxPoint(x    ,y), wxSize(w  ,h)) ;
        m_body[j] = new BodyWidget(_db, j, m_scheme, wxPoint(x+w  ,y), wxSize(2*w,h)) ;
        m_tail[j] = new EndsWidget(_db, j, m_scheme, wxPoint(x+3*w,y), wxSize(w  ,h)) ;
    m_head[j]->setOther(m_tail[j]) ;
    m_tail[j]->setOther(m_head[j]) ;
    }

    // the bars, between dodecime and note static lines, above the scheme
    w = h = PANEL_W - DODEC_W - NOTE_W - 2*SPACING ;
    m_bars = new BarsWidget(m_scheme, _db, panel, wxPoint(SCHEME_X+1, SCHEME_Y-h-1), wxSize(w-1,h-1)) ;
    m_background->setSchemeBars(m_scheme, m_bars) ;

    // color cubes, one light, one dark
    w = PANEL_W/2 ;
    h = PANEL_H - DODEC_H ;
    y = DODEC_H ; 
    m_lightCube = new CubeWidget(m_scheme, panel, wxPoint(-w/10, y), wxSize(w,h)) ;
    m_darkCube  = new CubeWidget(m_scheme, panel, wxPoint(PANEL_W/2+w/10, y), wxSize(w,h), true) ;

    // fill style selection buttons
    //w = PANEL_W*2/12 ;
    w = PANEL_W/12 ; // narrower because of strange overlap with CubWidget on Linux
    h -= SPACING ;
    x = PANEL_W/2-PANEL_W/24 ;
    new wxStaticBox(panel, -1, _T("fill"), wxPoint(x, y+SPACING/2), wxSize(w, h));
    h -= SPACING * 2 ;
    h /= (IP_SOLID+1) ;
    w -= SPACING ;
    x += SPACING/2 ;
    y += SPACING*2 ;
    for (unsigned k = IP_HOLLOW ; k <= IP_SOLID ; k++)
    {
      int y_k = y + k * h ;
      // add to panel, style box does not accept wxWindow widgets
      m_style[k] = new StyleWidget(k, m_scheme, panel, wxPoint(x,y_k), wxSize(w,h)) ;
    }

    currentDodec(db->current_dodecime) ;

    Show(true);
}

void SchemeEditorFrame::redisplay_dodecimes() {  for (int i = 0 ; i < N_DODECIMES ; i++)  m_dodec[i]->Refresh() ; }
void SchemeEditorFrame::redisplay_heads()     {  for (int i = 0 ; i < N_NOTES     ; i++) m_head [i]->Refresh() ; }
void SchemeEditorFrame::redisplay_bodies()    {  for (int i = 0 ; i < N_NOTES     ; i++) m_body [i]->Refresh() ; }
void SchemeEditorFrame::redisplay_tails()     {  for (int i = 0 ; i < N_NOTES     ; i++) m_tail [i]->Refresh() ; }


void SchemeEditorFrame::unload_scheme()
{
  COLOR_SCHEME scheme_on_disk ;
  default_scheme(&scheme_on_disk) ;

  if ( !db->path.empty() )
    ::load_scheme(&scheme_on_disk, db->path.ToAscii()) ;
  if ( memcmp(&scheme_on_disk, &(db->scheme), sizeof(scheme_on_disk)) )
  {
    /* different from disk */
    wxString msg = _T("Save changed scheme to file ?\n") ;
    msg.append(db->path) ;
    wxMessageDialog quest(this, msg.c_str(), 
      _T("Question"), wxYES_NO|wxYES_DEFAULT|wxICON_QUESTION ) ;
    if (quest.ShowModal() == wxID_YES)
    {
      wxCommandEvent dummy ;
      if (db->path.empty())
        /* default scheme was edited */
        OnSaveAs(dummy) ;
      else
        /* scheme from disk was edited */
        OnSave(dummy) ;
    }
  }
}

bool SchemeEditorFrame::load_scheme(const char * path)
{
  wxString msg ;
  switch (::load_scheme(&(db->scheme), path))
  {
  case 1:
    {
      msg.append(wxString::FromAscii(path)).append(_T("\nis not a valid scheme file!")) ;
      wxMessageDialog err(this, msg, _T("Error"), wxCANCEL|wxICON_ERROR ) ;
      err.ShowModal() ;
    }
    return false ;
  case -1:
    {
      msg.append(_T("Could not open\n")).append(wxString::FromAscii(path)) ;
      wxMessageDialog err(this, msg, _T("Error"), wxCANCEL|wxICON_ERROR ) ;
      err.ShowModal() ;
    }
    return false ;
  }
  // OK
  m_bars->Refresh() ;
  m_scheme->Refresh() ;
  redisplay_dodecimes() ;
  return true ;
}

void SchemeEditorFrame::OnQuit(wxCommandEvent& WXUNUSED(event)) 
{ 
  Close() ; 
}

bool SchemeEditorFrame::OnClose() 
{ 
  return true ; 
}

void SchemeEditorFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
  unload_scheme() ;

  wxFileDialog dialog(  this, _T("Open color scheme file"),_T(""),_T(""),_T("Color schemes (*.par)|*.par;*.PAR"));
  if (db->dir.empty())
    db->dir = wxGetHomeDir() ;
  dialog.SetDirectory(db->dir);
  if ( dialog.ShowModal() == wxID_OK && load_scheme(dialog.GetPath().ToAscii()) )
  {
    db->dir  = dialog.GetDirectory() ;
    db->path = dialog.GetPath() ;
    db->file = dialog.GetFilename() ;
    m_file_menu->Enable(wxID_SAVE, true) ;
    m_file_menu->Enable(wxID_REVERT, true);
    {
      // append the file name to the window title
      wxString title(_T(TITLE)) ;
      title.append(_T(" - ")).append(dialog.GetFilename()) ;
      SetTitle(title) ;
    }
  }
}

void SchemeEditorFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
  if (save_scheme(&(db->scheme), db->path.ToAscii()) != 0)
  {
    wxString msg ;
    msg.append(_T("Could not write scheme file\n")).append(db->path) ;
    wxMessageDialog err(this, _T("Could not open this scheme file!"), 
      _T("Error"), wxCANCEL|wxICON_ERROR ) ;
    err.ShowModal() ;
  }
}

void SchemeEditorFrame::OnSaveAs(wxCommandEvent& event)
{
  wxFileDialog dialog(this, _T("Save color scheme file"),
                      _T(""),
                      db->file,
                      _T("Color schemes (*.par)|*.par;*.PAR"),
                      wxSAVE|wxOVERWRITE_PROMPT);
  dialog.SetDirectory(db->dir) ;
  dialog.SetFilterIndex(1);
  if (dialog.ShowModal() == wxID_OK)
  {
    db->dir  = dialog.GetDirectory() ;
    db->path = dialog.GetPath() ;
    db->file = dialog.GetFilename() ;
    OnSave(event) ;
  }
}

void SchemeEditorFrame::OnRevertToSaved(wxCommandEvent& WXUNUSED(event))
{
  wxString msg ;
  msg.append(_T("Reload\n")).append(db->path).append(_T("\nand loose changes ?"))  ;
  wxMessageDialog quest(this, msg.c_str(), 
    _T("Question"), wxYES_NO|wxNO_DEFAULT|wxICON_QUESTION ) ;
  if (quest.ShowModal() == wxID_YES)
  {
    load_scheme(db->path.ToAscii()) ;
  }
}

void SchemeEditorFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
    wxMessageDialog dialog(this,
                           _T("Demo of owner-drawn controls\n"),
                           _T("About wxOwnerDrawn"), wxYES_NO | wxCANCEL);
    dialog.ShowModal();
}

void SchemeEditorFrame::OnDodecAll(wxCommandEvent& WXUNUSED(event))
{
  bool select = ( !m_dodec[0]->isSelected() ) ;
  for (unsigned i = 0 ; i < N_DODECIMES ; i++)
    m_dodec[i]->select(select) ;
}

void SchemeEditorFrame::OnDodecRot(wxCommandEvent& WXUNUSED(event))
{
      /* rotate colors and styles */

      int i, buff = db->scheme.dodecime_color[10] ;
      for (i = N_DODECIMES - 1 ; i > 0 ; i--)
        db->scheme.dodecime_color[i] = db->scheme.dodecime_color[i-1] ;
      db->scheme.dodecime_color[0] = buff ;

      buff = db->scheme.dodecime_style[10] ;
      for (i = N_DODECIMES - 1 ; i > 0 ; i--)
        db->scheme.dodecime_style[i] = db->scheme.dodecime_style[i-1] ;
      db->scheme.dodecime_style[0] = buff ;

      redisplay_dodecimes() ;
      Refresh() ;
}

void SchemeEditorFrame::OnDodecSwap(wxCommandEvent& WXUNUSED(event))
{
      for (int i = 0 ; i < N_DODECIMES/2 ; i++)
      {
        int buff = db->scheme.dodecime_color[i] ;
        db->scheme.dodecime_color[i] = db->scheme.dodecime_color[0xA - i] ;
        db->scheme.dodecime_color[0xA - i] = buff ;
        
        buff = db->scheme.dodecime_style[i] ;
        db->scheme.dodecime_style[i] = db->scheme.dodecime_style[0xA - i] ;
        db->scheme.dodecime_style[0xA - i] = buff ;
      }
      redisplay_dodecimes() ;
      m_scheme->Refresh() ;
}

void SchemeEditorFrame::OnEndsAll(wxCommandEvent& WXUNUSED(event))
{
  bool select = ( !m_head[0]->isSelected() ) ;
  for (unsigned i = 0 ; i < N_NOTES ; i++)
  {
    m_head[i]->select(select) ;
    m_tail[i]->select(select) ;
  }
}

void SchemeEditorFrame::OnEndsRot(wxCommandEvent& WXUNUSED(event))
{
      int buff_note_color_ends = db->scheme.note_color_ends[0xB] ;
      int buff_note_style_ends = db->scheme.note_style_ends[0xB] ;

      for (int i = N_NOTES - 1 ; i > 0 ; i--)
      {
        db->scheme.note_color_ends[i] = db->scheme.note_color_ends[i-1] ;
        db->scheme.note_style_ends[i] = db->scheme.note_style_ends[i-1] ;
      }
      db->scheme.note_color_ends[0] = buff_note_color_ends ;
      db->scheme.note_style_ends[0] = buff_note_style_ends ;
      
      redisplay_heads () ;
      redisplay_tails () ;
}

void SchemeEditorFrame::OnEndsSwap(wxCommandEvent& WXUNUSED(event))
{
      for (int i = 0 ; i < N_NOTES/2 ; i++)
      {
        int buff_note_color_ends = db->scheme.note_color_ends[i] ;
        db->scheme.note_color_ends[i] = db->scheme.note_color_ends[11 - i] ;
        db->scheme.note_color_ends[11 - i] = buff_note_color_ends ;
        
        int buff_note_style_ends = db->scheme.note_style_ends[i] ;
        db->scheme.note_style_ends[i] = db->scheme.note_style_ends[11 - i] ;
        db->scheme.note_style_ends[11 - i] = buff_note_style_ends ;
      }
      redisplay_heads () ;
      redisplay_tails () ;
}

void SchemeEditorFrame::OnBodyAll(wxCommandEvent& WXUNUSED(event))
{
  bool select = ( !m_body[0]->isSelected() ) ;
  for (unsigned i = 0 ; i < N_NOTES ; i++)
    m_body[i]->select(select) ;
}

void SchemeEditorFrame::OnBodyRot(wxCommandEvent& WXUNUSED(event))
{
      /* rotate colors and styles */
      int buff_note_color = db->scheme.note_color[0xB] ;
      int buff_note_style = db->scheme.note_style[0xB] ;

      for (int i = N_NOTES - 1 ; i > 0 ; i--)
      {
        db->scheme.note_color[i] = db->scheme.note_color[i-1] ;
        db->scheme.note_style[i] = db->scheme.note_style[i-1] ;
      }
      db->scheme.note_color[0] = buff_note_color ;
      db->scheme.note_style[0] = buff_note_style ;

      redisplay_bodies() ;
}

void SchemeEditorFrame::OnBodySwap(wxCommandEvent& WXUNUSED(event))
{
      for (int i = 0 ; i < N_NOTES / 2 ; i++)
      {
        int buff_note_color = db->scheme.note_color[i] ;
        db->scheme.note_color[i] = db->scheme.note_color[11 - i] ;
        db->scheme.note_color[11 - i] = buff_note_color ;
        
        int buff_note_style = db->scheme.note_style[i] ;
        db->scheme.note_style[i] = db->scheme.note_style[11 - i] ;
        db->scheme.note_style[11 - i] = buff_note_style ;
      }
      redisplay_bodies() ;
}

void SchemeEditorFrame::OnNoteAll(wxCommandEvent& WXUNUSED(event))
{
  bool select = ( !m_body[0]->isSelected() ) ;
  for (unsigned i = 0 ; i < N_NOTES ; i++)
  {
    m_head[i]->select(select) ;
    m_body[i]->select(select) ;
    m_tail[i]->select(select) ;
  }
}

void SchemeEditorFrame::OnNoteRot(wxCommandEvent& WXUNUSED(event))
{
      /* rotate note colors and styles */
      int buff_note_color = db->scheme.note_color[0xB] ;
      int buff_note_style = db->scheme.note_style[0xB] ;

      int buff_note_color_ends = db->scheme.note_color_ends[0xB] ;
      int buff_note_style_ends = db->scheme.note_style_ends[0xB] ;

      for (int i = N_NOTES - 1 ; i > 0 ; i--)
      {
        db->scheme.note_color[i] = db->scheme.note_color[i-1] ;
        db->scheme.note_style[i] = db->scheme.note_style[i-1] ;
        
        db->scheme.note_color_ends[i] = db->scheme.note_color_ends[i-1] ;
        db->scheme.note_style_ends[i] = db->scheme.note_style_ends[i-1] ;
      }
      db->scheme.note_color[0] = buff_note_color ;
      db->scheme.note_style[0] = buff_note_style ;

      db->scheme.note_color_ends[0] = buff_note_color_ends ;
      db->scheme.note_style_ends[0] = buff_note_style_ends ;
      
      redisplay_heads () ;
      redisplay_bodies() ;
      redisplay_tails () ;
}

void SchemeEditorFrame::OnNoteSwap(wxCommandEvent& WXUNUSED(event))
{
      for (int i = 0 ; i < N_NOTES/2 ; i++)
      {
        int buff_note_color      = db->scheme.note_color     [i] ;
        int buff_note_color_ends = db->scheme.note_color_ends[i] ;
        db->scheme.note_color     [i] = db->scheme.note_color     [11 - i] ;
        db->scheme.note_color_ends[i] = db->scheme.note_color_ends[11 - i] ;
        db->scheme.note_color     [11 - i] = buff_note_color      ;
        db->scheme.note_color_ends[11 - i] = buff_note_color_ends ;
        
        int buff_note_style      = db->scheme.note_style     [i] ;
        int buff_note_style_ends = db->scheme.note_style_ends[i] ;
        db->scheme.note_style     [i] = db->scheme.note_style     [11 - i] ;
        db->scheme.note_style_ends[i] = db->scheme.note_style_ends[11 - i] ;
        db->scheme.note_style     [11 - i] = buff_note_style      ;
        db->scheme.note_style_ends[11 - i] = buff_note_style_ends ;
      }
      redisplay_heads () ;
      redisplay_bodies() ;
      redisplay_tails () ;
}

void SchemeEditorFrame::OnNote66(wxCommandEvent& WXUNUSED(event))
{
    for (int i = N_NOTES - 1 ; i >= 0 ; i--)
    {
      if ((m_body[0]->isSelected() && i % 2 == 0) ||
         (!m_body[0]->isSelected() && i % 2 ) )
      {
        m_head[i]->select(false) ;
        m_body[i]->select(false) ;
        m_tail[i]->select(false) ;
      }
      else
      {
        m_head[i]->select(true) ;
        m_body[i]->select(true) ;
        m_tail[i]->select(true) ;
      }
    }
}

void SchemeEditorFrame::OnNote75(wxCommandEvent& WXUNUSED(event))
{
  for (int i = N_NOTES - 1 ; i >= 0 ; i--)
  {
    switch (i)
    {
    case 0: case 2: case 4: case 5: case 7: case 9: case 11:
      if (m_body[0]->isSelected())
      {
        m_head[i]->select(false) ;
        m_body[i]->select(false) ;
        m_tail[i]->select(false) ;
      }
      else
      {
        m_head[i]->select(true) ;
        m_body[i]->select(true) ;
        m_tail[i]->select(true) ;
      }
      break ;
    case 1: case 3: case 6: case 8: case 10:
      if (m_body[0]->isSelected())
      {
        m_head[i]->select(true) ;
        m_body[i]->select(true) ;
        m_tail[i]->select(true) ;
      }
      else
      {
        m_head[i]->select(false) ;
        m_body[i]->select(false) ;
        m_tail[i]->select(false) ;
      }
      break ;
    }
  }
}

void SchemeEditorFrame::OnDodecSelect(wxCommandEvent& event)
{
  currentDodec(event.GetSelection()) ;
  m_scheme->Refresh() ;
}

void SchemeEditorFrame::currentDodec(int i)
{
  db->current_dodecime = i ;
  m_lightCube->setCurrentDodec(m_dodec[i]) ;
  m_darkCube ->setCurrentDodec(m_dodec[i]) ;
  for (unsigned k = IP_HOLLOW ; k <= IP_SOLID ; k++)
  {
    m_style[k]->setCurrentDodec(m_dodec[i]) ;
  }
}
