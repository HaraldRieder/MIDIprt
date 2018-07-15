/*****************************************************************************
  Purpose:     Contains common declarations for the frame windows of the
               MIDI file printer (not relevant for the scheme editor).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: commonw.h,v 1.3 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef MFPCOMMONWINDOW_H
#define MFPCOMMONWINDOW_H

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif
#include "wx/font.h"   // wxNativeFontInfo
#include "wx/fontutil.h"
#include "wx/tglbtn.h" // wxToggleButton

#pragma warning( disable : 4786 ) // otherwise too many warnings because of <set> compiliation

#define MFP_TITLE "Mad Harry's MIDI File Printer"
#define MFP_DEFAULT_PROFILE "MIDI_PRT.MI$"

#define MIDI_CHANNELS 16	/**< MIDI files support 16 channels per track */

/** distance between control and frame(s) in pels */
static const int MFP_SPACING = 10 ; 

/** path of the application with terminating slash */
extern wxString apppath ;

/** pointer to application object */
extern wxApp * app ; 

#endif // include blocker
