/*****************************************************************************
  Purpose:     Implements main frame window of the MIDI File Printer.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: mainw.cpp,v 1.7 2008/09/29 20:26:55 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include "mainw.h"

#if defined (_WINDOWS)
#  include <io.h>
#  include <direct.h>
#endif
#include <errno.h>

#include <wx/filedlg.h>
#include <wx/metafile.h>
#include <wx/print.h>
#include <wx/file.h>
#include <wx/stdpaths.h>
#include <wx/splash.h>
#include <wx/html/htmlwin.h>

#include <set>
#include <list>

#include <graphicf.h>
#include <wxVDI.h>
#include "profile.h"
#include "scheme.h"
#include "version.h"

#ifndef _WINDOWS
#  define _unlink unlink
#endif

using namespace std ;


static wxString s_save_as_dir ;
static wxString s_open_dir ;

wxString MFPMainFrame::default_profile ;

/** list of instantiated windows */ 
static list<MFPMainFrame *> incarnation_list ;

/** pointer to main window topped most recently */
static MFPMainFrame * current_window = NULL ;


// ----------------------------------------------------------------------------
// MFPDrawingArea
// ----------------------------------------------------------------------------

// the event tables connect the wxWindows events with the functions (event
// handlers) which process them.
BEGIN_EVENT_TABLE(MFPDrawingArea, wxScrolledWindow)
    EVT_PAINT  (MFPDrawingArea::OnPaint)
//    EVT_MOTION (MFPDrawingArea::OnMouseMowxSlider * m_page_sliderve)
END_EVENT_TABLE()

//#include "smile.xpm"

MFPDrawingArea::MFPDrawingArea(DOCUMENT *_db, MFPMainFrame *parent)
        : wxScrolledWindow(parent, -1, wxDefaultPosition, wxDefaultSize,
                           wxHSCROLL | wxVSCROLL | wxNO_FULL_REPAINT_ON_RESIZE)
{
    db = _db ;
    //m_owner = parent;
}



void MFPDrawingArea::OnPaint(wxPaintEvent &WXUNUSED(event))
{
    wxPaintDC dc(this);
    DoPrepareDC(dc);

    wxSize sz ;
    GetVirtualSize(&sz.x, &sz.y) ;
    //dc.BeginDrawing() ;
    VirtualDevice vdi(&dc) ;

    int points[4] ;
    int dummy ;
    int zoom = db->opts.zoom ;

    if (!db->layout.npgs)
        zoom = MAX_ZOOM ;
        
    /* drawing area for draw_page(): */
    int width  = (int)((long)sz.x * zoom / MAX_ZOOM) ;
    int height = (int)((long)sz.y * zoom / MAX_ZOOM) ;
    
    /* font height in points is valid for 100% zoom */
    int pts = (int)((long)db->params.points * zoom / MEAN_ZOOM) ;
    int foot_pts = FOOTLINE_HEIGHT * zoom / MEAN_ZOOM ;

    /* draw background */
    vswr_mode   (&vdi, MD_REPLACE) ;
    vsf_interior(&vdi, FIS_HOLLOW) ;
    vsf_color   (&vdi, WHITE) ;
    points[0] = 0 ;
    points[1] = 0 ;
    points[2] = width ;
    points[3] = height ;
    vsl_color    (&vdi, LRED) ;
    vsl_type     (&vdi, SOLID) ;
    if (db->layout.npgs)
         vsf_perimeter(&vdi, 1) ;
    else vsf_perimeter(&vdi, 0) ; /* no red line */
    v_bar        (&vdi, points) ;

    // gray background
    points[0] = width + 1 ;
    points[2] = sz.x - 1 ;
    vsf_color   (&vdi, LBLACK) ;
    vsf_interior(&vdi, FIS_SOLID) ;
    vr_recfl    (&vdi, points) ;
    points[0] = 0 ;
    points[1] = height + 1 ;
    points[3] = sz.y - 1 ;
    vr_recfl    (&vdi, points) ;

    /* use points for clipping */
    int x_start, y_start ;
    CalcUnscrolledPosition(0, 0, &x_start, &y_start) ;
    sz = GetClientSize() ;
    points[0] = x_start ; 
    points[1] = y_start - 80 ;// workaround redraw errors
    points[2] = x_start + sz.x - 1 ;
    points[3] = y_start + sz.y + 80 ;// workaround redraw errors

    /* draw note systems */
    if (db->layout.npgs /*&& messages /* avoid redraw for garbage */) 
    {
        /* line widths in pixels */
        wxSize ppi = dc.GetPPI() ; // pixels per inch
        // mm_to_pixel needs pixel size in micrometers
        int x_micros = (int)((float)MICROMETER_PER_INCH / ppi.x + 0.5) ;
        int y_micros = (int)((float)MICROMETER_PER_INCH / ppi.y + 0.5) ;
        int line_width         = mm_to_pixel(db->params.line_width        , y_micros) * zoom/MAX_ZOOM ;
        int line_width_bars    = mm_to_pixel(db->params.bar_line_width    , x_micros) * zoom/MAX_ZOOM ; 
        int line_width_sub_bars= mm_to_pixel(db->params.sub_bar_line_width, x_micros) * zoom/MAX_ZOOM ;
        int line_width_notes   = mm_to_pixel(db->params.note_line_width   , y_micros) * zoom/MAX_ZOOM ;

        draw_page(&vdi, 
            0, 0, width, height,
            points,                /* clipping */ 
            y_micros,            /* pixel height in micrometers */
            db->track_table,    /* the data to draw... */
            db->filter.track,
            &(db->layout),        /* with this layout... */
            db->opts.page - 1,    /* index of page to draw */
            db->params.font, pts,    /* font for all texts */
            db->params.effects,        /* text effects for title only */
            foot_pts,
            db->params.title,
            db->filename,    /* for footline ... */
            db->filesize,
            CAPTION, VERSION, PLATFORM,
            db->params.note_height,    
            MAX_DYNSCALE, db->params.note_dynscale,        
            db->params.mode,            /* Rieder, Beyreuther, Mix */
            db->params.note_type,    /* e.g. with tail, etc. */
            &(db->params.scheme),
            db->params.transpose,
            db->params.bars_per_line,
            db->params.sub_bars,
            line_width, 
            line_width_bars, 
            line_width_sub_bars, 
            line_width_notes
        ) ;
    }
    else if (db->filename[0] != 0)
    {
        vst_font (&vdi, (char *)"") ;        /* system font */
        vst_color(&vdi, BLACK) ;    /* RGB not necessary, on screen there is always a palette */
        vst_point(&vdi, 10, &dummy, &dummy, &dummy, &dummy) ; /* out params are dummies */
        vst_alignment(&vdi, 0, 5, &dummy, &dummy) ;
        v_gtext(&vdi, 2, 2, /* + 2 because of red border line */
            (char *)"Nothing to draw. (Did you switch all tracks off ?)") ;
    }

    //dc.EndDrawing() ;
}

/*void MFPDrawingArea::OnMouseMove(wxMouseEvent &event)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    m_owner->PrepareDC(dc);

    wxPoint pos = event.GetPosition();
    long x = dc.DeviceToLogicalX( pos.x );
    long y = dc.DeviceToLogicalY( pos.y );
    wxString str;
    str.Printf( wxT("Current mouse position: %d,%d"), (int)x, (int)y );
    m_owner->SetStatusText( str );
}
*/

class MFPPrintout: public wxPrintout
{
 public:
  MFPPrintout(DOCUMENT *_db):wxPrintout(wxString::FromAscii(_db->pathname)),db(_db) {}
  bool OnPrintPage(int page);
  bool HasPage(int page);
  bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
 private:
   DOCUMENT *db ;
};

bool MFPPrintout::OnPrintPage(int page)
{
    wxDC *dc = GetDC();
    if (!dc)
        return false ;

    VirtualDevice vdi(dc) ;
    
    /* drawing area for draw_page(): */
    int width, height ;
    GetPageSizePixels(&width, &height) ;
    
    int points[4] = { 0,0, INT_MAX,INT_MAX } ; // no clipping

    /* line widths in pixels */
    wxSize ppi ;
    GetPPIPrinter(&ppi.x, &ppi.y) ; // pixels per inch
    // mm_to_pixel needs pixel size in micrometers
    int x_micros = (int)((float)MICROMETER_PER_INCH / ppi.x + 0.5) ;
    int y_micros = (int)((float)MICROMETER_PER_INCH / ppi.y + 0.5) ;
    int line_width         = mm_to_pixel(db->params.line_width        , y_micros) ;
    int line_width_bars    = mm_to_pixel(db->params.bar_line_width    , x_micros) ; 
    int line_width_sub_bars= mm_to_pixel(db->params.sub_bar_line_width, x_micros) ;
    int line_width_notes   = mm_to_pixel(db->params.note_line_width   , y_micros) ;

    draw_page(&vdi, 
        0, 0, width, height,
        points,                /* clipping */ 
        y_micros,            /* pixel height in micrometers */
        db->track_table,    /* the data to draw... */
        db->filter.track,
        &(db->layout),        /* with this layout... */
        page - 1,            /* index of page to draw */
        db->params.font,     /* font for all texts */
        db->params.points,
        db->params.effects,    /* text effects for title only */
        FOOTLINE_HEIGHT,
        db->params.title,
        db->filename,        /* for footline ... */
        db->filesize,
        CAPTION, VERSION, PLATFORM,
        db->params.note_height,    
        MAX_DYNSCALE, db->params.note_dynscale,        
        db->params.mode,        /* Rieder, Beyreuther, Mix */
        db->params.note_type,    /* e.g. with tail, etc. */
        &(db->params.scheme),
        db->params.transpose,
        db->params.bars_per_line,
        db->params.sub_bars,
        line_width, 
        line_width_bars, 
        line_width_sub_bars, 
        line_width_notes
    ) ;
    return true ;
 }

bool MFPPrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return false;
    return true;
}

void MFPPrintout::GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo)
{
    *minPage = 1;
    *maxPage = db->layout.npgs ;
    *selPageFrom = 1;
    *selPageTo = db->layout.npgs ;
}

bool MFPPrintout::HasPage(int pageNum)
{
    return (pageNum >= 1 && pageNum <= db->layout.npgs);
}



/** control IDs (menu items, sliders, buttons) */
enum 
{
    Menu_Quit = 1, 
    Menu_First = 100,
    Menu_Move_to,
    Menu_Attach, Menu_Remove, Menu_Save_as_default, Menu_Load_default,
    Menu_Information, Menu_Parameters, Menu_Filter, Menu_Next, Menu_Close,
    Menu_DIN_A4, Menu_US_letter,
    
    Control_First = 1000,
    Slider_Zoom, Slider_Page, 

    Key_First = 1500,
    Key_Home, Key_Home_Alt, Key_End, Key_End_Alt,
    Key_Up, Key_Pageup, Key_Down, Key_Pagedown, 
    Key_Pageright, Key_Pageleft,
    Key_Plus, Key_Plus_Alt, Key_Minus, Key_Minus_Alt,
    
    Timer = 2000
    // attention: 5000..6000 reserved for wx IDs
};


BEGIN_EVENT_TABLE(MFPMainFrame, wxFrame)
    EVT_CLOSE   (MFPMainFrame::OnCloseWindow   )
    EVT_ACTIVATE(MFPMainFrame::OnActivateWindow)
    EVT_MENU(wxID_OPEN           , MFPMainFrame::OnOpen         )
    EVT_MENU(wxID_CLOSE          , MFPMainFrame::OnCloseFile    )
    EVT_MENU(wxID_EXIT           , MFPMainFrame::OnQuit         )
    EVT_MENU(wxID_PRINT          , MFPMainFrame::OnPrint        )
    EVT_MENU(wxID_SAVEAS         , MFPMainFrame::OnSaveAs       )
    EVT_MENU(Menu_Move_to        , MFPMainFrame::OnMoveTo       )
    EVT_MENU(Menu_Attach         , MFPMainFrame::OnAttach       )
    EVT_MENU(Menu_Remove         , MFPMainFrame::OnRemove       )
    EVT_MENU(wxID_REVERT         , MFPMainFrame::OnRevertToSaved)
    EVT_MENU(Menu_Save_as_default, MFPMainFrame::OnSaveAsDefault)
    EVT_MENU(Menu_Load_default   , MFPMainFrame::OnLoadDefault  )
    EVT_MENU(Menu_DIN_A4         , MFPMainFrame::OnDINA4        )
    EVT_MENU(Menu_US_letter      , MFPMainFrame::OnUSletter     )
    EVT_MENU(Menu_Information    , MFPMainFrame::OnInformation  )
    EVT_MENU(Menu_Parameters     , MFPMainFrame::OnParameters   )
    EVT_MENU(Menu_Filter         , MFPMainFrame::OnFilter       )
    EVT_MENU(Menu_Next           , MFPMainFrame::OnNextWindow   )
    EVT_MENU(Menu_Close          , MFPMainFrame::OnCloseCmd     )
    EVT_MENU(wxID_ABOUT          , MFPMainFrame::OnAbout        )
    EVT_MENU(wxID_HELP           , MFPMainFrame::OnHelp         )
    EVT_SLIDER(Slider_Zoom       , MFPMainFrame::OnZoomSlider   )
    EVT_SLIDER(Slider_Page       , MFPMainFrame::OnPageSlider   )
    EVT_TIMER(Timer              , MFPMainFrame::OnTimer        )
//    EVT_CHAR(MFPMainFrame::OnKeys)
//    EVT_KEY_UP(MFPMainFrame::OnKeys)
//    EVT_KEY_DOWN(MFPMainFrame::OnKeys)
// <= these don't work as expected, therefore via accelerator:
    EVT_MENU(Key_Home            , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Home_Alt        , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_End             , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_End_Alt         , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Up              , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Down            , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Pageup          , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Pagedown        , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Pageleft        , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Pageright       , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Plus            , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Minus           , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Plus_Alt        , MFPMainFrame::OnKeys         )
    EVT_MENU(Key_Minus_Alt       , MFPMainFrame::OnKeys         )
END_EVENT_TABLE()


bool MFPMainFrame::load_file(const char *pathname)
{
    wxString msg = wxString::FromAscii(pathname) ;
    if (!wxFile::Exists(msg))
    {
        msg.append(_T("\ndoes not exist!")) ;
        wxMessageDialog err(this, msg.c_str(), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
        return false ;
    }
    if (!wxFile::Access(msg, wxFile::read))
    {
        msg.append(_T(":\nAccess denied!")) ;
        wxMessageDialog err(this, msg.c_str(), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
        return false ;
    }

    unload_file() ;

    wxFile f(wxString::FromAscii(pathname)) ;
    /*db.print.filesize =*/ db.info.filesize = db.filesize = f.Length() ;
    db.RAM_file = malloc(db.filesize);
    if (db.RAM_file == NULL)
    {
        wxMessageDialog err(this, _T("Out of memory!"), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
        unload_file() ;
        return false ;
    }
    // copy to RAM
    if (f.Read(db.RAM_file, db.filesize) == wxInvalidOffset)
    {
        wxString msg = _T("Error while reading\n");
        msg.append(wxString::FromAscii(pathname)).append(_T(" !")) ;
        wxMessageDialog err(this, msg.c_str(), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
        unload_file() ;
        return false ;
    }
    return true ;
}


void MFPMainFrame::unload_file()
{
    if (db.RAM_file != NULL)
    {
        free(db.RAM_file) ;
        db.RAM_file = NULL ;
    }
}

void MFPMainFrame::update_print_items()
{
    if (db.filename[0] != 0)
    {    
        if (db.layout.npgs > 0)
        {
            m_file_menu->Enable(wxID_PRINT, true) ;
//            wi->menu[_COPY].ob_state &= ~DISABLED ;
            return ;
        }
    }
    m_file_menu->Enable(wxID_PRINT, false) ;
//    wi->menu[_COPY].ob_state |= DISABLED ;
}

void MFPMainFrame::update_profile_items()
{
    bool has_file = (db.filename[0] != 0) ;
    m_profile_menu->Enable(Menu_Load_default   , has_file) ;
    m_profile_menu->Enable(Menu_Save_as_default, has_file) ;
    if (has_file)
    {
        if (db.has_profile)
        {
            m_profile_menu->Enable(wxID_REVERT, true) ;
            m_profile_menu->Enable(Menu_Attach, false) ;
            m_profile_menu->Enable(Menu_Remove, true) ;
        }
        else
        {
            m_profile_menu->Enable(wxID_REVERT, false) ;
            m_profile_menu->Enable(Menu_Attach, true) ;
            m_profile_menu->Enable(Menu_Remove, false) ;
        }
    }
    else
    {
        m_profile_menu->Enable(wxID_REVERT, false) ;
        m_profile_menu->Enable(Menu_Attach, false) ;
        m_profile_menu->Enable(Menu_Remove, false) ;
    }
}

void MFPMainFrame::update_file_items()
{
    bool has_file = (db.filename[0] != 0) ;
    m_file_menu->Enable(wxID_CLOSE  , has_file) ;
    m_file_menu->Enable(wxID_SAVEAS , has_file) ;
    m_file_menu->Enable(Menu_Move_to, has_file) ;
    update_profile_items() ;
    update_print_items() ;
}

void MFPMainFrame::update_format_items()
{
    m_options_menu->Check(Menu_DIN_A4   , options.format == PROFILED_OPTIONS::FORMAT_DIN_A4   );
    m_options_menu->Check(Menu_US_letter, options.format == PROFILED_OPTIONS::FORMAT_US_LETTER);
}

void MFPMainFrame::update_menu()
{
    update_file_items         () ; 
    update_format_items       () ;
}


void MFPMainFrame::InitMenu()
{
    // construct menu
    m_file_menu = new wxMenu;
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_OPEN, _T("&Open...\tCtrl+O"), _T("Open MIDI file"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_CLOSE, _T("&Close\tCtrl+C"), _T("Close MIDI file"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_SAVEAS, _T("Save &as...\tCtrl+S"), _T("Copy MIDI file [+ profile] to another directory"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, Menu_Move_to, _T("&Move &to...\tCtrl+M"), _T("Like \"Save as\" but delete original file [and profile]"), wxITEM_NORMAL));
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_PRINT, _T("&Print...\tCtrl+P"), _T("Print MIDI file"), wxITEM_NORMAL));
    m_file_menu->AppendSeparator();
    m_file_menu->Append(new wxMenuItem(m_file_menu, wxID_EXIT, _T("&Quit\tCtrl+Q"), _T("Quit "MFP_TITLE), wxITEM_NORMAL));

    m_profile_menu = new wxMenu;
    m_profile_menu->Append(new wxMenuItem(m_profile_menu, Menu_Attach, _T("&Attach\tCtrl+A"), _T("Attach a profile (*.MI$) to the MIDI file"), wxITEM_NORMAL)); 
    m_profile_menu->Append(new wxMenuItem(m_profile_menu, Menu_Remove, _T("&Remove\tCtrl+R"), _T("Remove existing profile (*.MI$) of the MIDI file"), wxITEM_NORMAL)); 
    m_profile_menu->Append(new wxMenuItem(m_profile_menu, wxID_REVERT, _T("Reload from disk...\tCtrl+Z"), _T("Load profile from disk and loose changes"), wxITEM_NORMAL)); 
    m_profile_menu->Append(new wxMenuItem(m_profile_menu, Menu_Save_as_default, _T("Save as default"), _T("Save current profile as default "MFP_DEFAULT_PROFILE), wxITEM_NORMAL)); 
    m_profile_menu->Append(new wxMenuItem(m_profile_menu, Menu_Load_default, _T("Load default"), _T("Load default profile "MFP_DEFAULT_PROFILE" from disk as current profile"), wxITEM_NORMAL)); 

    m_window_menu = new wxMenu;
    m_window_menu->Append(new wxMenuItem(m_window_menu, Menu_Information, _T("File &information\tF2"), _T("Open/top information window"), wxITEM_NORMAL)); 
    m_window_menu->Append(new wxMenuItem(m_window_menu, Menu_Parameters, _T("&Parameters\tF3"), _T("Open/top parameters window"), wxITEM_NORMAL)); 
    m_window_menu->Append(new wxMenuItem(m_window_menu, Menu_Filter, _T("&Filter\tF4"), _T("Open/top filter window"), wxITEM_NORMAL)); 
    m_window_menu->AppendSeparator();
    m_window_menu->Append(new wxMenuItem(m_window_menu, Menu_Next, _T("&Top next\tCtrl+W"), _T("Open/top next document window"), wxITEM_NORMAL)); 
    m_window_menu->Append(new wxMenuItem(m_window_menu, Menu_Close, _T("&Close window\tAlt+F4"), _T("Close this document window"), wxITEM_NORMAL)); 

    m_options_menu = new wxMenu;
    m_options_menu->Append(new wxMenuItem(m_options_menu, Menu_DIN_A4, _T("DIN A4"), _T("Width/Height ratio"), wxITEM_RADIO)); 
    m_options_menu->Append(new wxMenuItem(m_options_menu, Menu_US_letter, _T("US letter"), _T("Width/Height ratio"), wxITEM_RADIO)); 

    m_help_menu = new wxMenu;
    m_help_menu->Append(new wxMenuItem(m_help_menu, wxID_HELP, _T("&Help...\tF1"), _T("Open help"), wxITEM_NORMAL)); 
    m_help_menu->Append(new wxMenuItem(m_help_menu, wxID_ABOUT, _T("&About "CAPTION" ..."), _T("Show version and copyright information"), wxITEM_NORMAL)); 

    // make a menubar
    wxMenuBar *menu_bar = new wxMenuBar;
    menu_bar->Append(m_file_menu, _T("&File"));
    menu_bar->Append(m_profile_menu, _T("&Profile"));
    menu_bar->Append(m_window_menu, _T("&Window"));
    menu_bar->Append(m_options_menu, _T("&Format"));
    menu_bar->Append(m_help_menu, _T("&Help"));
    SetMenuBar(menu_bar);
}

void MFPMainFrame::SetAccelerators()
{
  const unsigned max_entries = 40 ;
  unsigned i = 0 ;
  wxAcceleratorEntry entries[max_entries];
/*  entries[i++].Set(wxACCEL_CTRL  , (int)'O'  , wxID_OPEN       );*/

  entries[i++].Set(wxACCEL_NORMAL, WXK_F11     , wxID_HELP       ); // Atari Help on ST Emulator
  entries[i++].Set(wxACCEL_NORMAL, WXK_F12     , wxID_REVERT     ); // Atari Undo on ST Emulator

  entries[i++].Set(wxACCEL_NORMAL, WXK_START   , Key_Home        );
  entries[i++].Set(wxACCEL_CTRL  , WXK_START   , Key_Home_Alt    );
  entries[i++].Set(wxACCEL_SHIFT , WXK_START   , Key_Home_Alt    );
  entries[i++].Set(wxACCEL_NORMAL, WXK_HOME    , Key_Home        );
  entries[i++].Set(wxACCEL_CTRL  , WXK_HOME    , Key_Home_Alt    );
  entries[i++].Set(wxACCEL_SHIFT , WXK_HOME    , Key_Home_Alt    );

  entries[i++].Set(wxACCEL_NORMAL, WXK_END     , Key_End         );
  entries[i++].Set(wxACCEL_CTRL  , WXK_END     , Key_End_Alt     );
  entries[i++].Set(wxACCEL_SHIFT , WXK_END     , Key_End_Alt     );

  entries[i++].Set(wxACCEL_NORMAL, WXK_UP      , Key_Up          );
  entries[i++].Set(wxACCEL_CTRL  , WXK_UP      , Key_Pageup      );
  entries[i++].Set(wxACCEL_SHIFT , WXK_UP      , Key_Pageup      );
  entries[i++].Set(wxACCEL_NORMAL, WXK_PAGEUP  , Key_Pageup      );

  entries[i++].Set(wxACCEL_NORMAL, WXK_DOWN    , Key_Down        );
  entries[i++].Set(wxACCEL_CTRL  , WXK_DOWN    , Key_Pagedown    );
  entries[i++].Set(wxACCEL_SHIFT , WXK_DOWN    , Key_Pagedown    );
  entries[i++].Set(wxACCEL_NORMAL, WXK_PAGEDOWN, Key_Pagedown    );

  entries[i++].Set(wxACCEL_CTRL  , WXK_LEFT    , Key_Pageleft    );
  entries[i++].Set(wxACCEL_SHIFT , WXK_LEFT    , Key_Pageleft    );

  entries[i++].Set(wxACCEL_CTRL  , WXK_RIGHT   , Key_Pageright   );
  entries[i++].Set(wxACCEL_SHIFT , WXK_RIGHT   , Key_Pageright   );

  entries[i++].Set(wxACCEL_NORMAL, (int)'+'    , Key_Plus        );
  entries[i++].Set(wxACCEL_CTRL  , (int)'+'    , Key_Plus_Alt    );
  entries[i++].Set(wxACCEL_SHIFT , (int)'+'    , Key_Plus_Alt    );
  entries[i++].Set(wxACCEL_NORMAL, (int)'-'    , Key_Minus       );
  entries[i++].Set(wxACCEL_CTRL  , (int)'-'    , Key_Minus_Alt   );
  entries[i++].Set(wxACCEL_SHIFT , (int)'-'    , Key_Minus_Alt   );

  assert(i <= max_entries) ;
  wxAcceleratorTable accel(i, entries);
  SetAcceleratorTable(accel);
}

/** main frame constructor */
MFPMainFrame::MFPMainFrame(const char * midifile) : wxFrame(NULL, -1, _T(MFP_TITLE))

{
    incarnation_list.push_back(this) ;
    current_window = this ;

    default_profile = wxGetHomeDir() + _T(DIRSEP) + _T("."DEFAULT_PROFILE_NAME) ; 

    // set the icon
    SetIcon(wxIcon(_T("AMIDIPRT"))); // app. icon from first in alphabet ???

    // create the menu and set accelerator keys
    InitMenu();
    SetAccelerators();

    // create the status line
    CreateStatusBar(1/*fields*/,0/*without resizing grip*/); 

    wxSize sz;

    // create the tool bar + controls
    int x = MFP_SPACING ;
    wxToolBar * toolbar = CreateToolBar(wxTB_HORIZONTAL);
    //toolbar->SetToolPacking(3);
    {
        sz = toolbar->GetSize();
        sz.y = wxButton::GetDefaultSize().y;
        toolbar->SetSize(sz);
    }
    wxStaticText * label = new wxStaticText(toolbar, -1, _T("Page:"), wxPoint(x,MFP_SPACING/2)) ; 
	toolbar->AddControl(label);
    x += (MFP_SPACING + label->GetSize().x) ;
    sz = wxButton::GetDefaultSize() ;
    sz.x *= 3 ;
    m_page_slider = new Slider(toolbar, Slider_Page, 1, 1, MAX_PAGES, wxPoint(x,2),sz,wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_AUTOTICKS+wxSL_LABELS) ;
    m_page_slider->SetPageSize(1) ;
    m_page_slider->Disable() ;
    m_page_slider->AddToToolBar(toolbar);
	//toolbar->AddControl(m_page_slider);
    toolbar->AddSeparator();
    x += (MFP_SPACING + sz.x) ;
    label = new wxStaticText(toolbar, -1, _T("Zoom:"), wxPoint(x,MFP_SPACING/2)) ;
	toolbar->AddControl(label);
    x += (MFP_SPACING + label->GetSize().x) ;
    sz.x = wxButton::GetDefaultSize().x * 6 /* * 8 */ ;
    m_zoom_slider = new Slider(toolbar, Slider_Zoom, 100, 33, 250, wxPoint(x,2),sz,wxSL_HORIZONTAL+wxSL_BOTTOM+wxSL_LABELS) ;
    m_zoom_slider->SetPageSize(1) ;
    m_zoom_slider->Disable() ;
    m_zoom_slider->AddToToolBar(toolbar);
    //toolbar->AddControl(m_zoom_slider);
    toolbar->Realize();

    // the inner drawing (client) area
    m_area = new MFPDrawingArea( &db, this );
    m_area->SetScrollRate(UP_DOWN_SCROLL_PIXELS,UP_DOWN_SCROLL_PIXELS) ;

    /* make menu consistent with data */
    memset(&db     , 0, sizeof(db     )) ;
    memset(&db_last, 0, sizeof(db_last)) ;
    update_menu() ;    

    if ( !midifile )
    {
        /* have the program user selected a MIDI file */
        wxCommandEvent dummy ;
        OnOpen(dummy) ;
    }
    else do_open_file(midifile) ;

    /* create timer for change detection */
    m_timer = new wxTimer(this, Timer) ;
    m_timer->Start(333) ;

    Show(true);
}


MFPMainFrame::~MFPMainFrame()
{
    // remove this window from the list
    list<MFPMainFrame *>::iterator i = incarnation_list.begin() ;
    while (i != incarnation_list.end())
    {
        if (*i == this)
        {
            incarnation_list.erase(i) ;
            break ;
        }
        ++i ;
    }
    if (incarnation_list.size() == 0)
        // was the last incarnation
        app->ExitMainLoop() ;
    delete m_timer ;
}


void MFPMainFrame::update_format()
{
    /* drawing area for draw_page in pixels (100% zoom) */
    float normal_width, normal_height ;

    switch (options.format)
    {
    case PROFILED_OPTIONS::FORMAT_US_LETTER:
        normal_width  = US_LETTER_WIDTH  ; 
        normal_height = US_LETTER_HEIGHT ; 
        break ;
    case PROFILED_OPTIONS::FORMAT_DIN_A4:
    default:
        normal_width  = DIN_A4_WIDTH  ;
        normal_height = DIN_A4_HEIGHT ; 
    }
    wxScreenDC screen ;
    wxSize ppi = screen.GetPPI() ; // pixels per inch
    // micrometers => pixels
    normal_width  = normal_width  / MICROMETER_PER_INCH * ppi.x ;
    normal_height = normal_height / MICROMETER_PER_INCH * ppi.y ;

    /* the following calculation is performed with floats */
/*    m_area->SetScrollbars( 
        UP_DOWN_SCROLL_PIXELS, 
        UP_DOWN_SCROLL_PIXELS, 
        (int)(normal_width  * MAX_ZOOM / MEAN_ZOOM), 
        (int)(normal_height * MAX_ZOOM / MEAN_ZOOM) );*/
    int width  = (int)(normal_width  * MAX_ZOOM / MEAN_ZOOM) ;
    int height = (int)(normal_height * MAX_ZOOM / MEAN_ZOOM) ;
    m_area->SetVirtualSize(width, height) ;
    m_area->Refresh() ;
}


void MFPMainFrame::do_load_default_profile() 
    /* 1. tries to load default profile */
    /* 2. if not OK uses fix coded defaults */
{
    static int already_warned = FALSE ;

    if ( read_profile(default_profile.ToAscii(), apppath.ToAscii(),
            &(db.params), &(db.filter), &(db.opts), TRUE) != 0 )
    {
        if ( !already_warned )
        {
            wxMessageDialog err(this, _T("There is no default profile.|Using fix coded defaults."), _T("Warning"), wxOK|wxICON_EXCLAMATION ) ;
            already_warned = TRUE ;
        }
    }

    do_load_scheme(&(db.params), db.params.scheme_path) ;
}


void MFPMainFrame::do_load_profile()
{
    /* init defaults from code */
    default_doc_params(&(db.opts)) ;
    init_info_from_tracks  (&(db.info  ), db.track_table, db.filter.number_tracks) ;
    init_params_from_tracks(&(db.params), db.track_table, db.filter.number_tracks) ;
    init_filter_from_tracks(&(db.filter), db.track_table) ;
    
    if ( read_profile(db.profile, apppath.ToAscii(),
            &(db.params), &(db.filter), &(db.opts), FALSE) != 0 )
    {
        /* no specific profile */
        db.has_profile = FALSE ;
        /* try to load default profile */
        do_load_default_profile() ;
    }
    else db.has_profile = TRUE ;
    
    do_load_scheme(&(db.params), db.params.scheme_path) ;
}


void MFPMainFrame::redisplay()
{
    update_print_items() ;    /* depends on: number of layouted pages > 0 */

    if ( db.layout.npgs == 0 )
    {
        m_page_slider->Disable() ;
        m_zoom_slider->Disable() ;
    }
    else
    {
        m_page_slider->Enable() ;
        m_zoom_slider->Enable() ;
        db.opts.page = min(db.opts.page, db.layout.npgs) ;
        db.opts.page = max(db.opts.page, 1) ;
        m_page_slider->SetRange(1, db.layout.npgs) ;
        m_page_slider->SetValue(db.opts.page) ;
        db.opts.zoom = min(db.opts.zoom, MAX_ZOOM) ;
        db.opts.zoom = max(db.opts.zoom, MIN_ZOOM) ;
        m_zoom_slider->SetValue(db.opts.zoom) ;
    }        
    
    /* redisplay */
    Refresh() ;
    m_area->Refresh() ;
}


void MFPMainFrame::set_databases()
    /* set databases of all slave windows */
{
    current_window = this ;
    update_file_items() ;
    
    /* During termination main frame can be activated after destruction 
       of dialog windows. This code would crash. */
    if (info_window)   info_window   ->set_database(&(db.info   ));
    if (filter_window) filter_window ->set_database(&(db.filter ));
    if (params_window) params_window ->set_database(&(db.params ));
}



void MFPMainFrame::new_layout()
{
    if (db.filename[0] == 0)
    {
        db.layout.npgs = 0 ;
        redisplay() ; 
    }

    wxString msg = _T("Out of resources:\nToo many ") ;
    bool err = false ;

    switch ( page_layouter(
        &(db.layout),
        (float)(db.params.left_border ) / 100,
        (float)(db.params.right_border) / 100,
        (float)(db.params.upper_border) / 100,
        (float)(db.params.lower_border) / 100,
        (float)(db.params.system_distance) / 100,
        (float)(db.params.track_distance ) / 100,
        (float)(db.params.note_distance  ) / 100,
        (char) (db.params.horizontal_lines),
        db.filter.number_tracks,
        db.filter.track,
        db.track_table,
        get_max_time(db.filter.number_tracks, db.track_table, db.filter.track),
        time_per_system(db.info.midi_header.ticks_per_beat,db.params.bar_length,db.params.bars_per_line) )
    )
    {
    case LAYOUT_ERR_TOO_MANY_PAGES:   msg.append(_T("pages!")) ; err = true ; break ;
    case LAYOUT_ERR_TOO_MANY_SYSTEMS: msg.append(_T("note systems!")) ; err = true ; break ;
    case LAYOUT_ERR_TOO_MANY_LINES:   msg.append(_T("track lines!")) ; err = true ; break ;
    case LAYOUT_ERR_TRACK_TOO_HIGH:   msg = _T("A track height exceeds the\n"
                                               "available drawing area.\n"
                                               "Choose smaller distances or\n"
                                               "smaller lower and upper\n"
                                               "borders.") ; err = true ; break ;
    case LAYOUT_ERR_INVALID_BORDERS:  msg.append(_T("The borders are invalid, for example:\n"
                                                    "left border + right border together\n"
                                                    "are more than 100%.")) ; err = true ; break ;
    }
    if (err)
    {
        wxMessageDialog err(this, msg.c_str(), _T("Error"), wxCANCEL|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
    }
    redisplay() ; 
}

void MFPMainFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{
    bool db_changed = (memcmp(&db, &db_last, sizeof(db)) != 0) ;
    if (db_changed || filter_window->has_changed())
    {
        // something changed
        memcpy(&db_last, &db, sizeof(db)) ;
        new_layout() ;
    }
}


void MFPMainFrame::OnQuit(wxCommandEvent &WXUNUSED(event))
{
    quit() ;
}


void MFPMainFrame::quit()
{
    /* save filenames and profiles */
    list<MFPMainFrame *>::const_iterator i = incarnation_list.begin() ;
    while (i != incarnation_list.end())
    {
        if ((*i)->db.filename[0])
        {
            // save filename
            fprintf(config_file, "%s\n", (*i)->db.pathname) ;
            // save profile
            wxCommandEvent dummy ;
            (*i)->OnCloseFile(dummy) ;
        }
        (*i)->Destroy() ;
        ++i ;
    }

    app->ExitMainLoop() ;
}


void MFPMainFrame::OnOpen(wxCommandEvent& WXUNUSED(event))
{
    if (db.filename[0] != 0)
    {
        new MFPMainFrame(NULL);
        return ;
    }

    wxFileDialog dialog(this, _T("Open MIDI file"),_T(""),
        wxString::FromAscii(db.filename),
        _T("MIDI files (*.mid,*.midi)|*.mid;*.MID;*.midi;*.MIDI"));
    if (s_open_dir.empty())
        s_open_dir = wxStandardPaths::Get().GetDocumentsDir() ;
    dialog.SetDirectory(s_open_dir) ;

    if ( dialog.ShowModal() != wxID_OK )
        return ;

    s_open_dir = dialog.GetDirectory() ;
    char pathname[PATHNAME_LENGTH] ;
    strcpy(pathname, dialog.GetPath().ToAscii()) ;
    {        
        /* accept also .MI$ files for opening (replace extension with .MID) */
        char *s ;
        if ((s = strstr(pathname, ".MI$")) != NULL) 
            strcpy(s, ".MID") ;
        else if ((s = strstr(pathname, ".MID$")) != NULL)
            strcpy(s, ".MIDI") ;
        else if ((s = strstr(pathname, ".mi$")) != NULL)
            strcpy(s, ".mid") ;
        else if ((s = strstr(pathname, ".mid$")) != NULL)
            strcpy(s, ".midi") ;
    }
    do_open_file(pathname) ;
}


void MFPMainFrame::do_open_file(const char * pathname) 
{
    /* load the selected file into RAM */
    if ( !load_file(pathname) )
        return ;

    strcpy(db.pathname, pathname) ;
    strcpy(db.filename, wxFileName::FileName(wxString::FromAscii(pathname)).GetFullName().ToAscii()) ;
    profile_path(db.profile, db.pathname) ;

    /* enter pathname into window info line */
    wxString title(_T(MFP_TITLE)) ;
    title.append(_T(" - ")).append(wxString::FromAscii(pathname)) ;
    SetTitle(title) ;

    /* enter filename into DBs of slave windows */
    db.info.filename  = db.filter.filename  = 
    db.params.filename = db.filename ;
    
    /* fit window format to paper format */
    update_format() ;
    
    db.info.midi_header = read_midi_file_header((const unsigned char *)db.RAM_file) ;
    if (db.info.midi_header.midi_file_type < 0 || db.info.midi_header.number_tracks <= 0 )
    {
        wxString msg = wxString::FromAscii(db.pathname);
        msg.append(_T("\nis no MIDI standard file.")) ;
        wxMessageDialog err(this, msg.c_str(), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
        unload_file() ;
        return ; 
    }
    if (db.info.midi_header.smpte) 
    {
        wxMessageDialog err(this, _T("Sorry, SMPTE format files\nare not supported."), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
        err.ShowModal() ;
        /* but continue */
    }

    switch ( make_track_table(
        (const unsigned char *)db.RAM_file,
        db.filesize, 
        db.info.midi_header.number_tracks,
        &(db.track_table),
        &(db.info.transformed_size)) )
    {
        case TRANSFORM_INVALID_FILE_FORMAT:
            {
                wxString msg = wxString::FromAscii(db.pathname) ;
                msg.append(_T("\nis no MIDI standard file.")) ;
                wxMessageDialog err(this, msg.c_str(), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
                err.ShowModal() ;
            }
            unload_file() ;
            return ;
        case TRANSFORM_OUT_OF_MEMORY:
            {
                wxMessageDialog err(this, _T("Out of memory!"), _T("Error"), wxOK|wxICON_EXCLAMATION ) ;
                err.ShowModal() ;
            }
            unload_file() ;
            return ; 
    }
    
    /* in any case file is no longer needed in RAM */
    unload_file() ; 

    if (!set_number_tracks(&(db.filter), db.info.midi_header.number_tracks ))
        return ;
    
    do_load_profile() ;

//    if (!db->opts.has_toolbar)
//        do_hide_toolbar(wi) ;

    wxScreenDC screen ;
    wxSize sz = screen.GetSize() ;
    wxRect rect ;
    rect.x      = (int)((float)(db.opts.left  ) / 1000 * sz.x + 0.5) ;
    rect.y      = (int)((float)(db.opts.top   ) / 1000 * sz.y + 0.5) ;
    rect.width  = (int)((float)(db.opts.right ) / 1000 * sz.x + 0.5) - rect.x + 1 ;
    rect.height = (int)((float)(db.opts.bottom) / 1000 * sz.y + 0.5) - rect.y + 1 ;
        
    /* correct possible inplausible values from file */
    rect.x = wxMin(sz.x - 32, wxMax(-32, rect.x)) ;
    rect.y = wxMin(sz.y - 32, wxMax(  0, rect.y)) ;
    rect.height = wxMax(200, rect.height) ;
    rect.width  = wxMax(200, rect.width ) ;

    SetSize(rect) ;

/*    if (db->opts.is_open)
        wi->open(wi) ;
*/
    set_databases() ;
    new_layout();
}

void MFPMainFrame::OnCloseCmd( wxCommandEvent &WXUNUSED(event) )
{
    wxCloseEvent dummy ;
    OnCloseWindow( dummy ) ;
}

void MFPMainFrame::OnCloseWindow( wxCloseEvent &WXUNUSED(event) )
{
    m_timer->Stop() ;
    wxCommandEvent dummy ;
    OnCloseFile(dummy) ;
    Destroy() ;
}

void MFPMainFrame::OnActivateWindow( wxActivateEvent &WXUNUSED(event) )
{
    //app->SetTopWindow(this);
    current_window = this ;
    if (db.filename[0] != 0)
    {
        set_databases() ;
    }
}

void MFPMainFrame::OnCloseFile( wxCommandEvent &WXUNUSED(event) )
{
    if (db.has_profile)
        do_write_profile() ;
                
    /* inform slave windows that data is no longer valid */
    //if (wi == current_wi)
    {
        if (info_window)   info_window->set_database(NULL) ;
        if (filter_window) filter_window->set_database(NULL) ;
        if (params_window) params_window->set_database(NULL) ;
    }
    /* save file path in common config file */
    /* if started with command line filenames: save NOT! */
/*    if ( ACSblk->argc <= 1 && ACSblk->appexit )
    {
        if ( !config_file )
            config_file = fopen(config_filepath, "w") ;
        if ( config_file )
            fprintf(config_file, "%s\n", db->pathname) ;
    }*/
    /* own object instance */
    unload_file() ;
    free_track_table(db.track_table) ;
    set_number_tracks(&(db.filter), 0) ;
    memset(&db, 0, sizeof(db));
    update_file_items() ;
    SetTitle(_T(MFP_TITLE)) ;
}

void MFPMainFrame::OnPrint( wxCommandEvent &WXUNUSED(event) )
{
    m_print.SetFromPage(wxMax(m_print.GetFromPage(),1)) ;
    m_print.SetToPage  (wxMin(m_print.GetToPage(),db.layout.npgs));
    m_print.SetNoCopies(1) ;
    m_print.SetMinPage(1) ;
    m_print.SetMaxPage(db.layout.npgs) ;
    m_print.SetAllPages(false) ;
    m_print.SetCollate(false) ;
    m_print.SetPrintToFile(m_print.GetPrintToFile()) ;
    //m_print.SetSetupDialog(false) ;
    m_print.EnablePrintToFile(true) ;
    m_print.EnablePageNumbers(true) ;
    m_print.SetSelection(false) ;
    m_print.EnableSelection(false) ;
    m_print.EnableHelp(false) ;

    wxPrinter printer(&m_print);
    MFPPrintout printout(&db);
    if (!printer.Print(this, &printout, TRUE))
    {
        if (wxPrinter::GetLastError() == wxPRINTER_ERROR)
            wxMessageBox(_T("There was a problem."
                            "\nPerhaps your current printer is not set correctly?"), 
                         _T("Printing"), wxOK);
    }
    // remember (possibly changed) settings for next call of dialog
    m_print = printer.GetPrintDialogData();
}

void MFPMainFrame::do_save_as(bool remove_originals)
{
    wxString msg ;
    if (remove_originals)
        msg.append(_T("Move ")).append(wxString::FromAscii(db.filename)).append(_T(" to")) ;
    else
        msg.append(_T("Save ")).append(wxString::FromAscii(db.filename)).append(_T(" as")) ;
    if (s_save_as_dir.empty())
        s_save_as_dir = wxString::FromAscii(db.pathname) ;
    wxFileDialog dialog(this, msg, s_save_as_dir, wxString::FromAscii(db.filename),
                        _T("MIDI files (*.mid,*.midi)|*.mid;*.MID;*.midi;*.MIDI"),
                        wxFD_SAVE|wxFD_OVERWRITE_PROMPT) ;
    dialog.SetFilterIndex(1);
    if (dialog.ShowModal() == wxID_OK)
    {
        s_save_as_dir = dialog.GetDirectory() ;
        if ( Fcopy(dialog.GetPath().ToAscii(), db.pathname) != 0 )
        {
            wxString msg = _T("Could not copy\n") ;
            msg.append(wxString::FromAscii(db.pathname)) ;
            wxMessageDialog err(this, msg, _T("Error"), wxCANCEL|wxICON_ERROR ) ;
            err.ShowModal() ;
            return ;
        }
        if (db.has_profile)
        {
            char buff[256] ;
            profile_path(buff, dialog.GetPath().ToAscii()) ;
            if ( Fcopy(buff, db.profile) != 0 )
            {
                wxString msg = _T("Could not copy\n") ;
                msg.append(wxString::FromAscii(db.pathname)) ;
                wxMessageDialog err(this, msg, _T("Error"), wxCANCEL|wxICON_ERROR ) ;
                err.ShowModal() ;
                return ;
            }
        }
        if (remove_originals)
        {
            if (_unlink(db.pathname) != 0)
            {
                wxString msg = _T("Could not remove\n");
                msg.append(wxString::FromAscii(db.pathname)) ;
                wxMessageDialog err(this, msg, _T("Error"), wxOK|wxICON_ERROR ) ;
                err.ShowModal() ;
            }
            if (_unlink(db.profile) != 0)
            {
                wxString msg = _T("Could not remove\n");
                msg.append(wxString::FromAscii(db.profile)) ;
                wxMessageDialog err(this, msg, _T("Error"), wxOK|wxICON_ERROR ) ;
                err.ShowModal() ;
            }
        }
        strcpy(db.pathname, dialog.GetPath().ToAscii()) ;
        strcpy(db.filename, dialog.GetFilename().ToAscii()) ;
        profile_path(db.profile, db.pathname) ;
    
        /* update pathname in window info line */
        wxString title(_T(MFP_TITLE)) ;
        title.append(_T(" - ")).append(dialog.GetPath()) ;
        SetTitle(title) ;
    }
}

void MFPMainFrame::OnSaveAs( wxCommandEvent &WXUNUSED(event) )
{
    do_save_as(false) ;
}

void MFPMainFrame::OnMoveTo( wxCommandEvent &WXUNUSED(event) )
{
    do_save_as(true) ;
}

void MFPMainFrame::update_pos()
{
    wxScreenDC screen ;
    wxSize sz = screen.GetSize() ;
    wxRect rect = GetRect() ;

    db.opts.is_open = !IsIconized() ;
    db.opts.left  = (int)((float)rect.x * 1000 / sz.x + 0.5) ;
    db.opts.top   = (int)((float)rect.y * 1000 / sz.y + 0.5) ;
    db.opts.right = (int)((float)(rect.x + rect.width  - 1) * 1000 / sz.x + 0.5) ;
    db.opts.bottom= (int)((float)(rect.y + rect.height - 1) * 1000 / sz.y + 0.5) ;
}

void MFPMainFrame::do_write_profile()
{
    update_pos() ;

    if ( write_profile(db.profile, apppath.ToAscii(), 
            &(db.params), &(db.filter), &(db.opts), FALSE) != 0 )
    {
        wxString msg ;
        msg.append(_T("Could not write profile\n")).append(wxString::FromAscii(db.profile)) ;
        wxMessageDialog err(this, msg, _T("Error"), wxCANCEL|wxICON_ERROR ) ;
        err.ShowModal() ;
        return ;
    }
    db.has_profile = TRUE ;
}

void MFPMainFrame::OnAttach( wxCommandEvent &WXUNUSED(event) )
{
    do_write_profile() ;
    update_profile_items() ;
}

void MFPMainFrame::OnRemove( wxCommandEvent &WXUNUSED(event) )
{
    wxString msg = wxString::FromAscii(db.profile) ;
    if ( _unlink(db.profile) != 0 ) 
    {
        switch (errno)
        {
        case ENOENT: msg.append(_T("\ndoes not exist!")) ; break ;
        case EACCES: msg.append(_T(":\nAccess denied!")) ; break ;
        }
        wxMessageDialog err(this, msg.c_str(), _T("Error"), wxOK|wxICON_EXCLAMATION) ;
        err.ShowModal() ;
        return ;
    }
    db.has_profile = FALSE ;
    update_profile_items() ;
}

void MFPMainFrame::OnRevertToSaved( wxCommandEvent &WXUNUSED(event) )
{
    if (db.filename[0] == 0)
        return ;

    wxString msg ;
    msg.append(_T("Reload\n"))
     .append(wxString::FromAscii(db.profile))
     .append(_T("\nand loose changes ?"))  ;
    wxMessageDialog quest(this, msg.c_str(), 
        _T("Question"), wxYES_NO|wxYES_DEFAULT|wxICON_QUESTION ) ;
    if (quest.ShowModal() == wxID_YES)
    {
        do_load_profile() ;

        params_window ->set_database(&(db.params)) ;
        filter_window ->set_database(&(db.filter)) ;
        // redraw done by timer supervision
    }
}

void MFPMainFrame::OnSaveAsDefault( wxCommandEvent &WXUNUSED(event) )
{
    if ( write_profile(
            default_profile.ToAscii(), apppath.ToAscii(),
            &(db.params), 
            &(db.filter),
            &(db.opts), TRUE) != 0)
    {
        wxString msg ;
        msg.append(_T("Could not write default profile\n")).append(default_profile) ;
        wxMessageDialog err(this, msg, _T("Error"), wxCANCEL|wxICON_ERROR ) ;
        err.ShowModal() ;
    }
}

void MFPMainFrame::OnLoadDefault( wxCommandEvent &WXUNUSED(event) )
{
    do_load_default_profile() ;        
    params_window ->set_database(&(db.params)) ;
    filter_window ->set_database(&(db.filter)) ;
    // redraw done by timer supervision
    update_menu() ;
}

void MFPMainFrame::update_options_all()
{
    list<MFPMainFrame *>::const_iterator i = incarnation_list.begin() ;
    while (i != incarnation_list.end())
    {
        (*i)->update_format_items() ;
        (*i)->update_format() ;
        ++i ;
    }
}

void MFPMainFrame::OnDINA4( wxCommandEvent &WXUNUSED(event) )
{
    options.format = PROFILED_OPTIONS::FORMAT_DIN_A4 ;
    update_options_all() ;
}

void MFPMainFrame::OnUSletter( wxCommandEvent &WXUNUSED(event) )
{
    options.format = PROFILED_OPTIONS::FORMAT_US_LETTER ;
    update_options_all() ;
}


void MFPMainFrame::OnInformation( wxCommandEvent &WXUNUSED(event) )
{
    info_window->Show() ;
    info_window->Raise() ;
}

void MFPMainFrame::OnParameters( wxCommandEvent &WXUNUSED(event) )
{
    params_window->Show() ;
    params_window->Raise() ;
}

void MFPMainFrame::OnFilter( wxCommandEvent &WXUNUSED(event) )
{
    filter_window->Show() ;
    filter_window->Raise() ;
}

void MFPMainFrame::OnNextWindow( wxCommandEvent &WXUNUSED(event) )
{
    RaiseNext() ;
}

void MFPMainFrame::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxBitmap bitmap;
  if (!bitmap.LoadFile(_T("midiprt.bmp"), wxBITMAP_TYPE_BMP))
      return ;

  int lrborder = 90 ; // left/right
  wxSplashScreen* splash = new wxSplashScreen(bitmap,
         wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
         30000, NULL, -1, wxDefaultPosition, wxDefaultSize,
         wxDOUBLE_BORDER|wxSTAY_ON_TOP);
  wxSize sz = splash->GetSize() ;
  sz.x -= 2*lrborder ;
  sz.y = wxButton::GetDefaultSize().y ;

  // headline
  wxString caption(_T(CAPTION" V"VERSION" for "PLATFORM)) ;
  wxTextCtrl * headline = new wxTextCtrl(splash, -1, _T(""), 
      wxPoint(lrborder,25), sz, wxTE_READONLY | wxTE_CENTRE) ;
  wxFont font = *wxNORMAL_FONT ;
  font.SetWeight(wxBOLD) ;
  font.SetPointSize(font.GetPointSize() + 2) ;
  headline->SetFont(font) ;
  headline->AppendText(caption);

  // copyright
  sz.y = sz.y * 7/2 ;
  wxString copyright(_T("\n1993 - 2009 (c) Harald Rieder\n\n"
                        "This program is under the\n"
                        "GNU General Public License V3.")) ;
  int y = 280 ;
  new wxTextCtrl(splash, -1, copyright, 
      wxPoint(lrborder,y), sz, wxTE_READONLY | wxTE_CENTRE | wxTE_MULTILINE | wxTE_NO_VSCROLL) ;
}

void MFPMainFrame::OnHelp( wxCommandEvent &WXUNUSED(event) )
{
    wxFrame * frame = new wxFrame(this, -1, _T(CAPTION)) ;
    wxHtmlWindow * help = new wxHtmlWindow(frame) ;
    wxString helppath = apppath + _T(HELP_DIRECTORY)+_T(DIRSEP)+_T("INDEX.HTM");
    if (!help->LoadFile(helppath))
    {
        wxString msg ;
        msg.append(_T("Could not load\n")).append(helppath).append(_T(" !")) ;
        wxMessageDialog err(this, msg.c_str(),
                           _T("Error"), wxICON_ERROR | wxCANCEL);
        err.ShowModal();
        return ;
    }
    frame->Show(true) ;
}

void MFPMainFrame::OnZoomSlider( wxCommandEvent &WXUNUSED(event) )
{
    db.opts.zoom = m_zoom_slider->GetValue() ;
    m_area->Refresh() ;
}

void MFPMainFrame::OnPageSlider( wxCommandEvent &WXUNUSED(event) )
{
    // hier wahrscheinlich falscher event, besser END_SLIDER, siehe widgets sample
    db.opts.page = m_page_slider->GetValue() ;
    m_area->Refresh() ;
}


void MFPMainFrame::OnKeys(wxCommandEvent & event)
{
    int x,y,vw,vh,cw,ch,x0,y0 ;

    if (db.filename[0] == 0)
        return ;

    switch (event.GetId())
    {
    case Key_Home_Alt:
        // to first page 
        db.opts.page = 1 ;
    case Key_Home:
        // to top of current page 
        m_area->Scroll(-1,0) ;
        break ;
    case Key_End_Alt: 
        // to last page 
        db.opts.page = db.layout.npgs ;
    case Key_End:
        // to bottom of current page
        m_area->GetVirtualSize(&vw, &vh) ;
        m_area->GetClientSize(&cw, &ch) ;
        m_area->GetScrollPixelsPerUnit(&x, &y) ;
        m_area->Scroll(-1, (vh * db.opts.zoom / MAX_ZOOM - ch) / y) ;
        break ;
    case Key_Pageup:
    case Key_Up:
        m_area->GetViewStart(&x0, &y0) ;
        if (y0 <= 0)
        {
            // to bottom of previous page 
            if (db.opts.page > 1)
            {
                db.opts.page-- ;
                wxCommandEvent e ;
                e.SetId(Key_End) ;
                OnKeys(e) ;
            }
            break ;
        }
        if (event.GetId() == Key_Pageup)
        {
            // scroll whole client size
            m_area->GetClientSize(&cw, &ch) ;
            m_area->GetScrollPixelsPerUnit(&x, &y) ;
            m_area->Scroll(-1, y0 - wxMax(1,ch/y)) ;
        }
        else
            // scroll 1 unit
            m_area->Scroll(-1, y0 - 1) ;
        break ;
    case Key_Pagedown:
    case Key_Down:
        m_area->GetViewStart(&x0, &y0) ;
        m_area->GetVirtualSize(&vw, &vh) ;
        m_area->GetClientSize(&cw, &ch) ;
        m_area->GetScrollPixelsPerUnit(&x, &y) ;
        if (y0 >= (vh * db.opts.zoom / MAX_ZOOM - ch) / y)
        {
            // to top of next page 
            if (db.opts.page < db.layout.npgs)
            {
                db.opts.page++ ;
                wxCommandEvent e ;
                e.SetId(Key_Home) ;
                OnKeys(e) ;
            }
            break ;
        }
        if (event.GetId() == Key_Pagedown)
        {
            // scroll whole client size
            m_area->Scroll(-1, y0 + wxMax(1,ch/y)) ;
        }
        else
            // scroll 1 unit
            m_area->Scroll(-1, y0 + 1) ;
        break ;
    case Key_Pageleft:
        m_area->GetViewStart(&x0, &y0) ;
        m_area->GetClientSize(&cw, &ch) ;
        m_area->GetScrollPixelsPerUnit(&x, &y) ;
        m_area->Scroll(x0 - wxMax(1,cw/x), -1) ;
        break ;
    case Key_Pageright:
        m_area->GetViewStart(&x0, &y0) ;
        m_area->GetClientSize(&cw, &ch) ;
        m_area->GetScrollPixelsPerUnit(&x, &y) ;
        m_area->Scroll(x0 + wxMax(1,cw/x), -1) ;
        break ;
    case Key_Plus:
        db.opts.zoom = wxMin(m_zoom_slider->GetValue() + 1, m_zoom_slider->GetMax()) ;
        redisplay() ;
        break ;
    case Key_Plus_Alt:
        db.opts.zoom = m_zoom_slider->GetMax() ;
        redisplay() ;
        break ;
    case Key_Minus:
        db.opts.zoom = wxMax(m_zoom_slider->GetValue() - 1, m_zoom_slider->GetMin()) ;
        redisplay() ;
        break ;
    case Key_Minus_Alt:
        db.opts.zoom = m_zoom_slider->GetMin() ;
        redisplay() ;
        break ;
    }
}


void MFPMainFrame::RaiseNext()
{
    list<MFPMainFrame *>::const_iterator i = incarnation_list.begin() ;
    while (i != incarnation_list.end())
    {
        if (*i == current_window)
        {
            ++i ;
            if (i == incarnation_list.end())
                // we are at the end, raise first
                (*incarnation_list.begin())->Raise() ;
            else
                (*i)->Raise() ;
            return ;
        }
        ++i ;
    }
}

/*void MFPMainFrame::Raise()
{
    set_databases() ;
    wxFrame::Raise() ;
}*/
