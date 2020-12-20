/*****************************************************************************
  Purpose:     Declares track and channel filter window.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: filterw.h,v 1.3 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef MFPFilterWindow_H
#define MFPFilterWindow_H

#include "filterdb.h"
#include "commonw.h"
#include "slider.h"

/** The filter window. */
class MFPFilterWindow : public wxDialog
{
public:
    // ctor & dtor
    MFPFilterWindow(wxWindow *parent);
    ~MFPFilterWindow() ;

	/**
	* Tells the info window which data to display. 
	* Calls redisplay if pointer has changed.
	*/
	void set_database(FILTER_DB *_db) ;

	/**
	* Returns true if database changed since last call of this function.
	*/
	bool has_changed() ;

    DECLARE_EVENT_TABLE()

private:

	Slider           * m_track_slider ;
	wxCheckBox       * m_show_track;               /**< current track on/off */
	wxButton         * m_tracks_on ;               /**< all tracks on button */
	wxButton         * m_tracks_off ;              /**< all tracks off button */
	wxToggleButton   * m_channels[MIDI_CHANNELS] ; /**< channel filter buttons */
	wxButton         * m_channels_on ;             /**< all channels on button */
	wxButton         * m_channels_off ;            /**< all channels off button */
	wxRadioButton    * m_trackname ;
	wxRadioButton    * m_instrument ;
	wxRadioButton    * m_device ;
	wxRadioButton    * m_text ;
	wxTextCtrl       * m_trackname_txt ;
	wxTextCtrl       * m_instrument_txt ;
	wxTextCtrl       * m_device_txt ;
	wxTextCtrl       * m_text_txt ;

	FILTER_DB *db ;  /**< the data displayed */
	bool m_changed ; /**< db changed */

    // notifications
    void OnCloseWindow       (wxCloseEvent& event);
	void OnTrackSlider       (wxCommandEvent& event);
	void OnShowTrack         (wxCommandEvent& event);
	void OnAllTracksOn       (wxCommandEvent& event);
	void OnAllTracksOff      (wxCommandEvent& event);
	void OnChannelToggle     (wxCommandEvent& event);
	void OnAllChannelsOn     (wxCommandEvent& event);
	void OnAllChannelsOff    (wxCommandEvent& event);
	void OnTrackName         (wxCommandEvent& event);
	void OnInstrument        (wxCommandEvent& event);
	void OnDevice            (wxCommandEvent& event);
	void OnText              (wxCommandEvent& event);

	/**
	* Redisplays according to current contents of database.
	* Disables all fields if db is NULL.
	*/
	void redisplay() ;

	/** Makes channel buttons consistent with database. */
	void update_channel_buttons() ;

	/** Makes track on/off buttons consistent with database. */
	void update_on_off_buttons () ;

	/** Makes radio buttons consistent with database. */
	void update_string_buttons () ;

};


extern MFPFilterWindow * filter_window ; // singleton window


#endif // include blocker
