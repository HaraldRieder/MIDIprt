/*****************************************************************************
  Purpose:     Own slider class saving vertical space by painting the labels
               left and right, not above and below as done by wxSlider
               of newer wxWidgets versions (e.g. 2.8.8).
               So this class behaves like former implementations of 
               wxSlider (e.g. wxWidgets 2.4.2) for horizontal sliders.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: slider.h,v 1.5 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef Slider_H
#define Slider_H

#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/slider.h>
#include <wx/event.h>
#include <wx/toolbar.h>

/** The file information window. */
class Slider : public wxSlider
{
public:
  Slider(wxWindow* parent, wxWindowID id, 
  int value , int minValue, int maxValue, 
  const wxPoint& point = wxDefaultPosition, 
  const wxSize& size = wxDefaultSize, 
  long style = wxSL_HORIZONTAL, 
  const wxValidator& validator = wxDefaultValidator, 
  const wxString& name = _T("slider"));

  void OnScroll(wxScrollEvent &);

  void SetRange(int minValue, int maxValue);
  void SetValue(int value);
  virtual bool Enable(bool enable = true);
  // more to be overloaded in the future, implementation not yet complete

  void AddToToolBar(wxToolBar *);

  DECLARE_EVENT_TABLE()

private:
  wxStaticText * m_min ;
  wxStaticText * m_max ;
  int            m_desired_max;
  wxStaticText * m_current;
  void setCurrent(int value);

  /**
   * Workaround function. SetSize() has no effect on Linux inside the Slider
   * constructor. Therefore we calculate the smaller width with this function
   * and pass it already correctly to the constructor of the base class 
   * wxSlider.
   *    
   * @return new slider width for the wxSlider constructor
   */         
  int init(wxWindow* parent, const wxPoint& point, const wxSize& size, 
    int value, int minValue, int maxValue, long style);
};


#endif // include blocker
