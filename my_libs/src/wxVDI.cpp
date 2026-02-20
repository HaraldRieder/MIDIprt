/*****************************************************************************
  Purpose:     On Atari TOS the VDI (corresponds to GDI on Windows) 
               graphics device functions work with a 16 bit VDI handle. In 
               order to use common code on TOS (C) and on wxWidgets (C++)
               the VDI handle parameter on TOS is really a VDI handle,
               but on wxWidgets it is a pointer to a VirtualDevice object.
               The VirtualDevice stores drawing parameters as
               a TOS VDI workstation (e.g. line width, fill style, ...) does.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: wxVDI.cpp,v 1.12 2008/09/20 20:04:40 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <assert.h>
#include <math.h>
#include <wx/fontutil.h>
#include "wxVDI.h"

#define POINT_PER_INCH          72

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

VirtualDevice::VirtualDevice(wxDC *_dc) :
	fillInterior(FIS_SOLID), 
	fillStyle(IP_1PATT), 
	fillPerimeter(1/*true*/),
	fillColor(1), // index, black
	lineWidth(1),
	lineType(SOLID),
	lineColor(1), 
	textAlignHor(TA_LEFT), 
	textAlignVer(TA_BASELINE),
	textColor(1),
	textEffects(0),
	pointSize(0)
{
	dc = _dc ; 
	assert(dc) ;
	palette[WHITE   ].Set(0xff,0xff,0xff) ; 
	palette[BLACK   ].Set(0x00,0x00,0x00) ; 
	palette[RED     ].Set(0xff,0x00,0x00) ; 
	palette[GREEN   ].Set(0x00,0xff,0x00) ; 
	palette[BLUE    ].Set(0x00,0x00,0xff) ; 
	palette[CYAN    ].Set(0x00,0xff,0xff) ; 
	palette[YELLOW  ].Set(0xff,0xff,0x00) ; 
	palette[MAGENTA ].Set(0xff,0x00,0xff) ; 
	palette[LWHITE  ].Set(0xaa,0xaa,0xaa) ; 
	palette[LBLACK  ].Set(0x55,0x55,0x55) ; 
	palette[LRED    ].Set(0x7f,0x00,0x00) ; 
	palette[LGREEN  ].Set(0x00,0x7f,0x00) ; 
	palette[LBLUE   ].Set(0x00,0x00,0x7f) ; 
	palette[LCYAN   ].Set(0x00,0x7f,0x7f) ; 
	palette[LYELLOW ].Set(0x7f,0x7f,0x00) ; 
	palette[LMAGENTA].Set(0x7f,0x00,0x7f) ; 
	int maxi = 255-16 ;
	for (int i = 0 ; i < maxi ; i++)
		palette[16+i].Set((long)0xffffff * i / maxi) ;

	// avoid invalid brush assertions by wxWindows
	setFillColor(WHITE);
}

VirtualDevice::~VirtualDevice()
{ }


int VirtualDevice::pixel_to_point(int pixel, bool vertical)
{
	wxSize ppi = dc->GetPPI() ; // pixels per inch
	if (vertical)
		return (int)(((float)pixel)/ppi.y * POINT_PER_INCH + 0.5) ;
	return (int)(((float)pixel)/ppi.x * POINT_PER_INCH + 0.5) ;
}

void VirtualDevice::setFont()
{
	wxNativeFontInfo info ;
	info.FromString(font_info) ;
	wxFont font ;
	font.SetNativeFontInfo(info) ;
	// now set the size and the effects
	if (pointSize > 0)
	{
		/* workaround wrong point size on printer - wxWindows 2.4 bug ? */
		wxScreenDC screen ;
		wxSize screen_ppi = screen.GetPPI() ; // pixels per inch
		wxSize this_ppi = dc->GetPPI() ;
		pointSize = pointSize * this_ppi.y/screen_ppi.y ;
		font.SetPointSize(pointSize) ;
	}
	if (textEffects & BOLD)       font.SetWeight(wxFONTWEIGHT_BOLD) ;
	if (textEffects & ITALIC)     font.SetStyle(wxFONTSTYLE_ITALIC) ;
	if (textEffects & UNDERLINED) font.SetUnderlined(true) ;
	// the other effects we do not emulate
	dc->SetFont(font) ;
	pointSize = font.GetPointSize() ; // remember really set size
}

void VirtualDevice::setPerimeterPen()
{
  dc->SetPen(fillPerimeter ? pen : pen_noPerimeter) ;
}

//-------------- attribute functions -------------------------

void VirtualDevice::setColor(int index, int *rgb_in)
{ 
    assert(index >= 0 && index <= 255) ;
    // VDI uses 0..1000 for RGB values.
    // [wx]Windows uses 0..255.
	unsigned char red   = (unsigned char)(rgb_in[0] * 0xff / 1000) ;
	unsigned char green = (unsigned char)(rgb_in[1] * 0xff / 1000) ;
	unsigned char blue  = (unsigned char)(rgb_in[2] * 0xff / 1000) ;
    palette[index].Set(red,green,blue) ;
}

void VirtualDevice::setLineWidth(int width)
{ 
	pen.SetWidth(lineWidth = width) ;
}

void VirtualDevice::setLineColor(int color)
{ 
	pen.SetColour(palette[lineColor = color]) ;
}

void VirtualDevice::setTextHeight(int height, int *char_width)
{
	static const wxString text = _("Ab") ;
	// transform pixels to points
	int point = pixel_to_point(height, true) ;
	if (pointSize != point)
	{
		pointSize = point ;
		setFont() ;
	}
	wxCoord w,h, descent, externalLeading ;
	dc->GetTextExtent(text, &w, &h, &descent, &externalLeading) ;
	*char_width = w/2; 
}

void VirtualDevice::setTextPoint(int point)
{
	static const wxString text = _("Ab") ;
	if (pointSize != point)
	{
		pointSize = point ;
		setFont() ;
	}
	wxCoord w,h, descent, externalLeading ;
	dc->GetTextExtent(text, &w, &h, &descent, &externalLeading) ;
}

void VirtualDevice::setFont(const wxString & font_info)
{
	wxNativeFontInfo info ;
	info.FromString(font_info);
	wxFont font ;
	font.SetNativeFontInfo(info) ;
	dc->SetFont(font) ;
}

void VirtualDevice::setTextEffects(int effects)
{
	if (textEffects != effects)
	{
		textEffects = effects ;
		setFont() ;
	}
}

void VirtualDevice::setTextAlignment(int hor_in , int ver_in) 
{
	textAlignHor = hor_in ; 
	textAlignVer = ver_in ; 
}

// In contrast to original VDI, here a RGB color value
// and no color index is expected.
void VirtualDevice::setFillColor(int color)
{
	fillColor = color ; 
	brush.SetColour(palette[color]) ;
    pen_noPerimeter.SetColour(palette[color]) ;
}

//-------------- output functions -------------------------

void VirtualDevice::drawPolygon(int count, int *pxyarray )
{
	dc->SetPen(pen) ;
	int i ;
	switch (lineType)
	{
	case DOT:
		for (i = 0 ; i < count - 1 ; i++)
		{
			struct { float x ; float y ; } unit_vector ;
			int x0 = pxyarray[i*2] ;
			int y0 = pxyarray[i*2+1] ;
			int dx = pxyarray[i*2+2] - x0 ;
			int dy = pxyarray[i*2+3] - y0 ;
			float denom = sqrt((double)(dx*dx + dy*dy)) ;
			unit_vector.x = (float)dx/denom ;
			unit_vector.y = (float)dy/denom ;
			for (int j = 0 ; true ; j += (2*lineWidth))
			{
				float fdx = unit_vector.x*j ;
				float fdy = unit_vector.y*j ;
				if ((int)fabs(fdx) > abs(dx) || (int)fabs(fdy) > abs(dy))
					break ;
				if (lineWidth == 1)
					dc->DrawPoint(x0 + (int)fdx, y0 + (int)fdy) ;
				else
				{
					wxPoint centre(x0 + (int)fdx, y0 + (int)fdy) ; 
					int radius = lineWidth / 2 ;
					// the fill color must be the line color here
					wxBrush brush;
					brush.SetColour(palette[lineColor]) ;
					dc->SetBrush(brush) ;
					dc->DrawCircle(centre, radius) ;
				}
			}
		}
		return ;
	case USERLINE:
	case SOLID:
	default: // others not yet supported
		{
			wxPoint *points = new wxPoint[count] ;
			for (i = 0 ; i < count ; i++)
			{
				points[i].x = pxyarray[i*2] ;
				points[i].y = pxyarray[i*2+1] ;
			}
			dc->DrawLines(count, points, 0,0) ;
			delete [] points ; 
		}
		return ;
	}
	/* with background filling:
	virtual void DrawPolygon(int n, wxPoint points[],
                              wxCoord xoffset, wxCoord yoffset,
                              int fillStyle = wxODDEVEN_RULE) = 0;*/
}

void VirtualDevice::drawFilledArea(int count, int *pxyarray)
{
	// inner area
	wxPoint *points = new wxPoint[count] ;
	for (int i = 0 ; i < count ; i++)
		points[i] = wxPoint(pxyarray[i*2], pxyarray[i*2+1]) ;
	dc->SetBrush(brush) ;
	setPerimeterPen() ;
	dc->DrawPolygon(count, points, 0,0) ;
	delete [] points ;
}

void VirtualDevice::drawText(int x, int y, const wxString & string)
{
	int left, top ;
	wxCoord w, h, dummy ;
	dc->GetTextExtent(string, &w, &dummy) ;
	dc->GetTextExtent(_("Wq"), &dummy, &h) ;
	// Warning: both Visual C++ and Pure C #define TA_... 
	switch (textAlignHor)
	{
	case /*TA_LEFT*/  0: left = x       ; break ;
	case /*TA_CENTER*/1: left = x - w/2 ; break ;
	case /*TA_RIGHT*/ 2: left = x - w   ; break ;
	}
	switch (textAlignVer)
	{
	case /*TA_BASELINE*/0: top = y - h*3/4 ; break ; // estimated
	case /*TA_HALF*/    1: top = y - h/2   ; break ;
	case /*TA_ASCENT*/  2: top = y - h/5   ; break ; // estimated
	case /*TA_BOTTOM*/  3: top = y - h     ; break ;
	case /*TA_DESCENT*/ 4: top = y - h*4/5 ; break ; // estimated
	case /*TA_TOP*/     5: top = y         ; break ; 
	}
	dc->SetTextForeground(palette[textColor]) ;
	dc->SetTextBackground(brush.GetColour()) ;
	dc->DrawText(string, left, top) ;
}

void VirtualDevice::drawBar(int *pxyarray)
{
	wxCoord width  = abs(pxyarray[2] - pxyarray[0]) + 1 ;
	wxCoord height = abs(pxyarray[3] - pxyarray[1]) + 1 ;
	dc->SetBrush(brush) ;
	setPerimeterPen() ;
	dc->DrawRectangle(pxyarray[0], pxyarray[1], width, height) ;
}

void VirtualDevice::drawFilledRect(int *pxyarray) 
{
	// draws rectangle always without perimeter
	wxCoord width  = abs(pxyarray[2] - pxyarray[0]) + 1 ;
	wxCoord height = abs(pxyarray[3] - pxyarray[1]) + 1 ;
	dc->SetBrush(brush) ;
	dc->SetPen(pen_noPerimeter) ; 
	dc->DrawRectangle(pxyarray[0], pxyarray[1], width, height) ;
}

void VirtualDevice::drawCircle(int x, int y, int radius )
{
	dc->SetBrush(brush) ;
	setPerimeterPen() ;
	dc->DrawCircle(x, y, radius) ;
}

void VirtualDevice::drawEllipse(int x, int y, int xradius, int yradius)
{
  dc->SetBrush(brush) ;
  setPerimeterPen() ;
  dc->DrawEllipse(x-xradius, y-yradius, xradius*2+1, yradius*2+1) ;
}

void VirtualDevice::drawEllArc(int x, int y, int xradius, int yradius, int begang, int endang)
{
  // draws arc line without fill area
  dc->SetPen(pen) ;
  // the VDI function does not fill
  dc->SetBrush(*wxTRANSPARENT_BRUSH) ;
  // radius and centre must be transformed to rectanlge corners
  // VDI uses 1/10 degrees angles, wx uses degrees
  dc->DrawEllipticArc(x - xradius, y - yradius, 2*xradius+1, 2*yradius+1, begang/10, endang/10) ;
}

void VirtualDevice::drawEllPie(int x, int y, int xradius, int yradius, int begang, int endang)
{
  // draws fill area without arc line
  // note: VDI expects 1/10 degrees angles [0..3600].
  dc->SetBrush(brush) ;
  dc->SetPen(pen_noPerimeter) ;
  // radius and centre must be transformed to rectanlge corners
  // VDI uses 1/10 degrees angles, wx uses degrees
  dc->DrawEllipticArc(x - xradius, y - yradius, 2*xradius+1, 2*yradius+1, begang/10, endang/10) ;
}


