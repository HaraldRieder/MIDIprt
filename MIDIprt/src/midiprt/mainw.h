/*****************************************************************************
  Purpose:     Declares main frame window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: mainw.h,v 1.3 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef MFPMAINWINDOW_H
#define MFPMAINWINDOW_H

#include "slider.h"
#include "infow.h"
#include "paramsw.h"
#include "filterw.h"
#include "common.h"

extern PROFILED_OPTIONS options ;
extern FILE * config_file ;


class MFPDrawingArea ;

/** The main window. */
class MFPMainFrame : public wxFrame
{
public:

  MFPMainFrame(const char * midifile);
  ~MFPMainFrame();

	/**
	* Brings the next incarnation (if there are more than 1)
	* of main frame on top of the screen.
	*/
	static void RaiseNext() ;

	/**
	* Saves pathnames of all open main frames and exits application. 
	*/
	static void quit() ;

	/**
	* Makes all incarnations of the main frame consistent with
	* current profiled options.
	*/
	static void update_options_all() ;

private:

    // notifications
    void OnCloseCmd         (wxCommandEvent& event);
    void OnCloseWindow      (wxCloseEvent& event);
    void OnActivateWindow   (wxActivateEvent& event);
    void OnQuit             (wxCommandEvent& event);
    void OnOpen             (wxCommandEvent& event);
    void OnCloseFile        (wxCommandEvent& event);
    void OnPrint            (wxCommandEvent& event);
    void OnSave             (wxCommandEvent& event);
    void OnSaveAs           (wxCommandEvent& event);
    void OnMoveTo           (wxCommandEvent& event);
    void OnAttach           (wxCommandEvent& event); /**< attach profile */
    void OnRemove           (wxCommandEvent& event); /**< remove profile */
    void OnRevertToSaved    (wxCommandEvent& event); /**< reload profile from disk */
    void OnSaveAsDefault    (wxCommandEvent& event); /**< save as default profile */
    void OnLoadDefault      (wxCommandEvent& event); /**< load default profile */
    void OnDINA4            (wxCommandEvent& event); /**< select DIN A4 width/height ratio */
    void OnUSletter         (wxCommandEvent& event); /**< select US letter width/height ratio */
    void OnInformation      (wxCommandEvent& event); /**< show info window */
    void OnParameters       (wxCommandEvent& event); /**< show params window */
    void OnFilter           (wxCommandEvent& event); /**< show filter window */
    void OnNextWindow       (wxCommandEvent& event); /**< top next main window */
    void OnAbout            (wxCommandEvent& event); /**< show program info */
    void OnHelp             (wxCommandEvent& event); /**< show HTML help file */
    void OnZoomSlider       (wxCommandEvent& event);
    void OnPageSlider       (wxCommandEvent& event);
    void OnTimer            (wxTimerEvent& event);
    void OnKeys                (wxCommandEvent& event); /**< accelerator key handler */

    static wxString default_profile ; /**< path to default profile */

    /**
    * Creates the menu bar with all menus.
    */
    void InitMenu() ;

    /**
    * Defines keyboard short cuts for menu, scroll and other functions.
    */
    void SetAccelerators() ;

    /** 
    * Loads file and shows error messages if necessary.
    * @param pathname of file to load
    * @return true if loaded successfully 
    */
    bool load_file(const char *pathname) ;

    /**
    * Consistently cleans up file info in db.
    */
    void unload_file() ;

    /**
    * Loads given pathname [and profile] and updates
    * window consistently.
    * @param pathname to load, gets into db.filename and db.pathname
    */
    void do_open_file(const char * pathname) ;

    /**
    * Makes file and profile menus consistent with data.
    */
    void update_file_items() ;

    /**
    * Makes profile menu consistent with data.
    */
    void update_profile_items() ;

    /**
    * Makes print items in file menu consistent with data.
    */
    void update_print_items() ;

    /**
    * Makes format menu consistent with options data.
    */
    void update_format_items() ;

    /**
    * Makes all menus consistent with options data.
    */
    void update_menu() ;

    /**
    * Adapts size of drawing area to paper format (letter/DIN).
    */
    void update_format() ;

    /**
    * Update profiled data from current window position and size
    * on screen.
    */
    void update_pos() ;

    /** 
    * Tells this window and the others that db could have changed.
    */
    void set_databases() ;

    /**
    * Redraws inner area.
    */
    void redisplay() ;

    /**
    * Calls the page layouter and redisplays.
    */
    void new_layout() ;

    /**
    * Common code for SaveAs an MoveTo.
    * @param remove_originals true in case of MoveTo
    */
    void do_save_as(bool remove_originals); 

    /**
    * 1. tries to load specific profile.
    * 2. if not OK tries to load default profile.
    * 3. if not OK uses fix coded defaults.
    */
    void do_load_profile() ;

    /**
    * 1. tries to load default profile.
    * 2. if not OK uses fix coded defaults.
    */
    void do_load_default_profile() ;

    /**
    * Writes profile.
    */
    void do_write_profile() ;

    /** consistently updates current dodecime index to i */
    //void currentDodec(int i) ;

    wxMenu *m_file_menu ;             /**< the file menu */
    wxMenu *m_profile_menu ;          /**< the profile menu */
    wxMenu *m_window_menu ;           /**< the window menu */
    wxMenu *m_options_menu ;          /**< the options menu */
    wxMenu *m_help_menu ;             /**< the help menu */
    Slider * m_page_slider ;        /**< page slider of toolbar */
    Slider * m_zoom_slider ;        /**< zoom slider of toolbar */
    MFPDrawingArea * m_area ;         /**< the client window area */

    wxTimer * m_timer ;               /**< for detecting db changes */
    DOCUMENT db ;
    DOCUMENT db_last ;

    /** some print dialog user settings to remember from call to call */
    wxPrintDialogData m_print ;

    DECLARE_EVENT_TABLE()
};

// define a scrollable canvas for drawing onto

class MFPDrawingArea: public wxScrolledWindow
{
public:
    MFPDrawingArea( DOCUMENT *db, MFPMainFrame *parent );
private:
    void OnPaint(wxPaintEvent &event);
//    void OnMouseMove(wxMouseEvent &event);
    //MFPMainFrame *m_owner;
    DOCUMENT * db ;
    DECLARE_EVENT_TABLE()
};


#endif // include blocker
