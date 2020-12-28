/*****************************************************************************
  Purpose:     Declares parameters window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: paramsw.h,v 1.3 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef MFPParamsWindow_H
#define MFPParamsWindow_H

#include "paramsdb.h"
#include "commonw.h"
#include "slider.h"


/** note owner drawn object */
class NoteWidget: public wxWindow
{
public:
    NoteWidget(wxWindow *parent, int id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	/**
	* Tells the widget which data to display. 
	* Calls redisplay if pointer has changed.
	*/
	void set_database(PARAMS_DB *_db) ;
private:
    void OnPaint(wxPaintEvent& event);
	PARAMS_DB * db ; /**< the data displayed */	
    DECLARE_EVENT_TABLE()
};

/** scheme owner drawn object */
class SchemeWidget: public wxWindow
{
public:
    SchemeWidget(wxWindow *parent, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
	/**
	* Tells the widget which data to display. 
	* Calls redisplay if pointer has changed.
	*/
	void set_database(PARAMS_DB *_db) ;
private:
    // notifications
    void OnPaint(wxPaintEvent& event);
	void OnSchemeSelect(wxMouseEvent& event);
	wxStaticText * m_text ; /**< shows filename / path */
	PARAMS_DB * db ; /**< the data displayed */	
    DECLARE_EVENT_TABLE()
};

/** The parameters window. */
class MFPParamsWindow : public wxDialog
{
public:
    // ctor & dtor
    MFPParamsWindow(wxWindow *parent);
    ~MFPParamsWindow() ;

	/**
	* Tells the params window which data to display. 
	* Calls redisplay if pointer has changed.
	*/
	void set_database(PARAMS_DB *_db) ;

    DECLARE_EVENT_TABLE()

private:

    // notifications
    void OnCloseWindow       (wxCloseEvent& event);

	void OnTitleEdit        (wxCommandEvent& event);
    void OnFont             (wxCommandEvent& event);
    void TextEffects        (wxCommandEvent& event); /**< bold, italic, underlined */

    void ShowLineWidthMenu  (wxCommandEvent& event); /**< popup menu */
	void OnLineWidthMenu    (wxCommandEvent& event);

	void OnBordersSelect    (wxCommandEvent& event);
	void OnLeftBorderSlider (wxCommandEvent& event);
	void OnRightBorderSlider(wxCommandEvent& event);

	void OnDistanceSelect   (wxCommandEvent& event);
	void OnDistanceSlider   (wxCommandEvent& event);

	void OnBarsPerLine      (wxCommandEvent& event);
	void OnBarLength        (wxCommandEvent& event);
	void OnSubBars          (wxCommandEvent& event);

	void OnInventorSelect   (wxCommandEvent& event);
	void OnHorlinesSelect   (wxCommandEvent& event);

	void OnHeightSlider     (wxCommandEvent& event);
	void OnDynamicSlider    (wxCommandEvent& event);

	void OnHeadSelect       (wxCommandEvent& event);
	void OnBodySelect       (wxCommandEvent& event);
	void OnTailSelect       (wxCommandEvent& event);
	void OnNoteBorders      (wxCommandEvent& event);
	void OnNotes3D          (wxCommandEvent& event);
	void OnTransposeSelect  (wxCommandEvent& event);

	/** consistently updates border sliders */
	void update_border_sliders() ;

	/** consistently updates distance slider */
	void update_distance_slider() ;

	/** consistently updates 3D borders check box */
	void update_3d_checkbox() ;

	/** consistently updates head, body and tail choice widgets */
	void update_head_body_tail() ;

	wxTextCtrl    * m_title ;         /**< title on 1. page (headline) */
	wxCheckBox    * m_bold ;
	wxCheckBox    * m_italic ;
	wxCheckBox    * m_underlined ;
	wxButton      * m_font ;          /**< opens the font dialog */
	wxChoice      * m_borders_choice ;/**< whether to show left/right or top/bottom */
	wxSlider      * m_left_border ;   /**< or top border */
	//wxTextCtrl    * m_left_val ;      /**< self-constructed slider value display */
	wxSlider      * m_right_border ;  /**< or bottom border */
	//wxTextCtrl    * m_right_val ;     /**< self-constructed slider value display */
	wxChoice      * m_distances ;     /**< which distance to show */
	wxSlider      * m_distance ;
	//wxTextCtrl    * m_distance_val ;  /**< self-constructed slider value display */
	wxSlider      * m_bars_per_line ;
	wxSlider      * m_bar_length ;
	wxSlider      * m_sub_bars ;
	wxChoice      * m_lines ;         /**< horizontal lines per dodec. */
	wxChoice      * m_transpose ;     /**< 0..12 */
	wxChoice      * m_inventor ;      /**< Beyreuther, Rieder, Mix */
	wxSlider      * m_height ;        /**< average note height (corresp. dynamic=0) */
	wxSlider      * m_dynamic ;       /**< note height dependence from note-on dynamic */
	wxCheckBox    * m_borders ;       /**< note borders */
	wxCheckBox    * m_borders_3d ;    /**< 3-dimensional looking note borders */
	NoteWidget    * m_head ;          /**< note head sample */
	NoteWidget    * m_body ;          /**< note body sample */
	NoteWidget    * m_tail ;          /**< note tail sample */
	wxChoice      * m_head_choice ;
	wxChoice      * m_body_choice ;
	wxChoice      * m_tail_choice ;
	SchemeWidget  * m_scheme ;        /**< color scheme sample */

	int m_popup_id ; /**< workaround: radio button behaviour in popup menus */

	PARAMS_DB * db ; /**< the data displayed */	

	/**
	* Redisplays according to current contents of database.
	* Disables all fields if db is NULL.
	*/
	void redisplay() ;

};


extern MFPParamsWindow * params_window ; // singleton window


#endif // include blocker
