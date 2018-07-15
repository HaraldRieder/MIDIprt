/*****************************************************************************
  Purpose:     Declares MIDI file information window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: infow.h,v 1.3 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef MFPInfoWindow_H
#define MFPInfoWindow_H

#include "infodb.h"
#include "commonw.h"

/** The file information window. */
class MFPInfoWindow : public wxDialog
{
public:
    // ctor & dtor
    MFPInfoWindow(wxWindow *parent);
    ~MFPInfoWindow() ;

	/**
	* Tells the info window which data to display. 
	* Calls redisplay if pointer has changed.
	*/
	void set_database(INFO_DB *_db) ;

    DECLARE_EVENT_TABLE()

private:
	wxStaticText * m_transformed_size ;
	wxStaticText * m_file_size ;
	wxStaticText * m_midi_file_type ;
	wxStaticText * m_number_of_tracks ;
	wxStaticText * m_ticks_per_beat ;
	wxStaticText * m_smpte ;
	wxStaticText * m_ticks_per_frame ; 
	wxStaticText * m_tempo ; 
	wxStaticText * m_time ;
	wxStaticText * m_key ;
	wxTextCtrl   * m_copyright_notice ;

	INFO_DB * db ; /**< the data displayed */	

    // notifications
    void OnCloseWindow       (wxCloseEvent& event);

	/**
	* Redisplays according to current contents of database.
	* Disables all fields if db is NULL.
	*/
	void redisplay() ;

};


extern MFPInfoWindow * info_window ;


#endif // include blocker
