/*****************************************************************************
  Purpose:     Own slider class saving vertical space by painting the labels
               left and right, not above and below as done by wxSlider
               of newer wxWidgets versions (e.g. 2.8.8).
               So this class behaves like former implementations of 
               wxSlider (e.g. wxWidgets 2.4.2) for horizontal sliders.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: slider.cpp,v 1.5 2008/09/20 20:07:42 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include  "slider.h"

BEGIN_EVENT_TABLE(Slider, wxSlider)
	EVT_SCROLL(Slider::OnScroll)
END_EVENT_TABLE()

using namespace std;

int Slider::init(wxWindow* parent, const wxPoint& point, const wxSize& size, 
  int value, int minValue, int maxValue, long style)
{
    m_min     = NULL;
    m_max     = NULL;
    m_current = NULL;

	if (style & wxSL_LABELS)
	{
		int spacing = 6;
		int ytext = point.y;
		wxSize sz;

		wxString smax = wxString::Format(_T("%d"), maxValue);
		m_max =	new wxStaticText(parent, -1, smax,
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		sz = m_max->GetSize();
		m_max->Move(point.x + size.x - sz.x, ytext); // move to the right	

		m_current =	new wxTextCtrl(parent, -1, smax/*adjust to max. width*/,
			wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_CENTER);
		setCurrent(value);
		sz = m_current->GetSize();
		size_t len = smax.size();
		if (len != 0)
		{
			// more space (for the border)
			m_current->SetSize(m_max->GetSize().x * (len + 1) / len , sz.y);
		}
		m_current->Move(point.x, point.y);

		wxString smin = wxString::Format(_T("%d"), minValue);
		m_min =	new wxStaticText(parent, -1, smin,
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		m_min->Move(point.x + m_current->GetSize().x + spacing, ytext);

		// make the slider more narrow to leave space for the labels 
//		Move(m_min->GetPosition().x + m_min->GetSize().x, GetPosition().y);
		return size.x - (m_max->GetSize().x + m_min->GetSize().x + m_current->GetSize().x + spacing);
	}
	return size.x;
}


Slider::Slider(wxWindow* parent, wxWindowID id, 
		int value , int minValue, int maxValue, 
		const wxPoint& point, const wxSize& size, long style, 
		const wxValidator& validator, const wxString& name)
: wxSlider(parent, id, value, minValue, maxValue, point, 
//		   wxSize(size.x, size.y*9/10),
       wxSize(init(parent, point, size, value, minValue, maxValue, style), size.y*10/9), 
		   style & ~wxSL_LABELS, validator, name)
{
	if (style & wxSL_LABELS)
	{
/*		int spacing = 6;
		int ytext = point.y;
		wxSize sz;

		wxString smax = wxString::Format(_T("%d"), maxValue);
		m_max =	new wxStaticText(parent, -1, smax,
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		sz = m_max->GetSize();
		m_max->Move(point.x + GetSize().x - sz.x, ytext); // move to the right	

		m_current =	new wxTextCtrl(parent, -1, smax,
			wxDefaultPosition, wxDefaultSize, wxTE_READONLY|wxTE_CENTER);
		setCurrent(value);
		sz = m_current->GetSize();
		size_t len = smax.size();
		if (len != 0)
		{
			// more space (for the border)
			m_current->SetSize(m_max->GetSize().x * (len + 1) / len , sz.y);
		}
		m_current->Move(point.x, point.y);

		wxString smin = wxString::Format(_T("%d"), minValue);
		m_min =	new wxStaticText(parent, -1, smin,
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
		m_min->Move(point.x + m_current->GetSize().x + spacing, ytext);

		// make the slider more narrow to leave space for the labels
*/ 
		Move(m_min->GetPosition().x + m_min->GetSize().x, point.y);
//		sz = GetSize();
//		sz.x -= (m_max->GetSize().x + m_min->GetSize().x + m_current->GetSize().x + spacing);
//		SetSize(sz);  no effect on Linux!
	}
	SetRange(minValue, maxValue); 
}

void Slider::OnScroll(wxScrollEvent &)
{
	setCurrent(GetValue());
}

void Slider::SetRange(int minValue, int maxValue)
{
  // max: avoid Gtk-CRITICAL **: gtk_range_set_range: assertion `min < max' failed
  m_desired_max = maxValue;
	wxSlider::SetRange(minValue, max(maxValue,minValue+1));
	if (m_min != NULL && m_max != NULL)
	{
		wxString smin = wxString::Format(_T("%d"), minValue);
		wxString smax = wxString::Format(_T("%d"), maxValue);
		m_min->SetLabel(smin);
		m_max->SetLabel(smax);
	}
	if (m_desired_max <= minValue)
	  wxSlider::Disable();
}

bool Slider::Enable(bool enable)
{
  // on Linux crashes with enabled slider and unreasonable range
  if (m_desired_max <= GetMin())
    enable = false;
  return wxSlider::Enable(enable);
}

void Slider::SetValue(int value)
{
	wxSlider::SetValue(value);
	setCurrent(value);
}

void Slider::setCurrent(int value)
{
	if (m_current != NULL)
	{
		wxString sval = wxString::Format(_T("%d"), value);
		m_current->SetValue(sval);
	}
}

void Slider::AddToToolBar(wxToolBar *toolbar)
{
	wxStaticText *dummy = new wxStaticText(toolbar, -1, _T(" "));
    toolbar->AddControl(m_current);
    toolbar->AddControl(dummy); // for spacing only
    toolbar->AddControl(m_min);
    toolbar->AddControl(this);
    toolbar->AddControl(m_max);
}




