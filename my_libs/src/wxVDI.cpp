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
#include "wxVDI.h"
#include "wx/fontutil.h"

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
	vsf_color(this, WHITE) ;
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
	if (textEffects & BOLD)       font.SetWeight(wxBOLD) ;
	if (textEffects & ITALIC)     font.SetStyle(wxITALIC) ;
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

void vs_color( VirtualDevice * handle, int index, int *rgb_in )
{ 
    assert(index >= 0 && index <= 255) ;
    // VDI uses 0..1000 for RGB values.
    // [wx]Windows uses 0..255.
	unsigned char red   = (unsigned char)(rgb_in[0] * 0xff / 1000) ;
	unsigned char green = (unsigned char)(rgb_in[1] * 0xff / 1000) ;
	unsigned char blue  = (unsigned char)(rgb_in[2] * 0xff / 1000) ;
    handle->palette[index].Set(red,green,blue) ;
}

void vsl_width( VirtualDevice * handle, int width )
{ 
	handle->lineWidth = width ; 
	handle->pen.SetWidth(width) ;
}

void vsl_color( VirtualDevice * handle, int color )
{ 
	handle->lineColor = color ; 
	handle->pen.SetColour(handle->palette[color]) ;
}

void vst_height( VirtualDevice * handle, int height, int *char_width,
                    int *char_height, int *cell_width,
                    int *cell_height )
{
	static const wxString text = _("Ab") ;
	// transform pixels to points
	int point = handle->pixel_to_point(height, true) ;
	if (handle->pointSize != point)
	{
		handle->pointSize = point ;
		handle->setFont() ;
	}
	wxCoord w,h, descent, externalLeading ;
	handle->dc->GetTextExtent(text, &w, &h, &descent, &externalLeading) ;
	*char_width  = *cell_width = w/2 ;
	*char_height = h ;
	*cell_height = h + descent + externalLeading ;
}

void vst_point( VirtualDevice * handle, int point, int *char_width,
                    int *char_height, int *cell_width,
                    int *cell_height )
{
	static const wxString text = _("Ab") ;
	if (handle->pointSize != point)
	{
		handle->pointSize = point ;
		handle->setFont() ;
	}
	wxCoord w,h, descent, externalLeading ;
	handle->dc->GetTextExtent(text, &w, &h, &descent, &externalLeading) ;
	// transform pixels to points
	*char_width  = *cell_width = handle->pixel_to_point(w) ;
	*char_height = handle->pixel_to_point(h, true) ;
	*cell_height = handle->pixel_to_point(h + descent + externalLeading, true) ;
}

void vst_font( VirtualDevice * handle, const VDI_FONT_ID font_info )
{
	handle->font_info = wxString::FromAscii(font_info) ;
	wxNativeFontInfo info ;
	info.FromString(wxString::FromAscii(font_info)) ;
	wxFont font ;
	font.SetNativeFontInfo(info) ;
	handle->dc->SetFont(font) ;
}

void vst_effects( VirtualDevice * handle, int effects )
{
	if (handle->textEffects != effects)
	{
		handle->textEffects = effects ;
		handle->setFont() ;
	}
}

void vst_alignment( VirtualDevice * handle, int hor_in , int ver_in,
                                  int *hor_out, int *ver_out ) 
{
	handle->textAlignHor = hor_in ; 
	handle->textAlignVer = ver_in ; 
	*hor_out = hor_in ;
	*ver_out = ver_in ;
}

// In contrast to original VDI, here a RGB color value
// and no color index is expected.
void vsf_color( VirtualDevice * handle, int color )
{
	handle->fillColor = color ; 
	handle->brush.SetColour(handle->palette[color]) ;
    handle->pen_noPerimeter.SetColour(handle->palette[color]) ;
}

//-------------- output functions -------------------------

void v_pline( VirtualDevice * handle, int count, int *pxyarray )
{
	handle->dc->SetPen(handle->pen) ;
	int i ;
	switch (handle->lineType)
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
			for (int j = 0 ; true ; j += (2*handle->lineWidth))
			{
				float fdx = unit_vector.x*j ;
				float fdy = unit_vector.y*j ;
				if ((int)fabs(fdx) > abs(dx) || (int)fabs(fdy) > abs(dy))
					break ;
				if (handle->lineWidth == 1)
					handle->dc->DrawPoint(x0 + (int)fdx, y0 + (int)fdy) ;
				else
				{
					wxPoint centre(x0 + (int)fdx, y0 + (int)fdy) ; 
					int radius = handle->lineWidth / 2 ;
					// the fill color must be the line color here
					wxBrush brush;
					brush.SetColour(handle->palette[handle->lineColor]) ;
					handle->dc->SetBrush(brush) ;
					handle->dc->DrawCircle(centre, radius) ;
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
			handle->dc->DrawLines(count, points, 0,0) ;
			delete [] points ; 
		}
		return ;
	}

	/* with background filling:
	virtual void DrawPolygon(int n, wxPoint points[],
                              wxCoord xoffset, wxCoord yoffset,
                              int fillStyle = wxODDEVEN_RULE) = 0;*/
}

void v_fillarea( VirtualDevice * handle, int count, int *pxyarray )
{
	// inner area
	wxPoint *points = new wxPoint[count] ;
	for (int i = 0 ; i < count ; i++)
		points[i] = wxPoint(pxyarray[i*2], pxyarray[i*2+1]) ;
	handle->dc->SetBrush(handle->brush) ;
	handle->setPerimeterPen() ;
	handle->dc->DrawPolygon(count, points, 0,0) ;

	delete points ;
}

void v_gtext( VirtualDevice * handle, int x, int y, char *string )
{
	int left, top ;
	wxCoord w, h, dummy ;
	handle->dc->GetTextExtent(wxString::FromAscii(string), &w, &dummy) ;
	handle->dc->GetTextExtent(_("Wq"), &dummy, &h) ;

	// Warning: both Visual C++ and Pure C #define TA_... 
	switch (handle->textAlignHor)
	{
	case /*TA_LEFT*/  0: left = x       ; break ;
	case /*TA_CENTER*/1: left = x - w/2 ; break ;
	case /*TA_RIGHT*/ 2: left = x - w   ; break ;
	}
	switch (handle->textAlignVer)
	{
	case /*TA_BASELINE*/0: top = y - h*3/4 ; break ; // estimated
	case /*TA_HALF*/    1: top = y - h/2   ; break ;
	case /*TA_ASCENT*/  2: top = y - h/5   ; break ; // estimated
	case /*TA_BOTTOM*/  3: top = y - h     ; break ;
	case /*TA_DESCENT*/ 4: top = y - h*4/5 ; break ; // estimated
	case /*TA_TOP*/     5: top = y         ; break ; 
	}
	handle->dc->SetTextForeground(handle->palette[handle->textColor]) ;
	handle->dc->SetTextBackground(handle->brush.GetColour()) ;
	handle->dc->DrawText(wxString::FromAscii(string), left, top) ;
}

void v_bar( VirtualDevice * handle, int *pxyarray )
{
	wxCoord width  = abs(pxyarray[2] - pxyarray[0]) + 1 ;
	wxCoord height = abs(pxyarray[3] - pxyarray[1]) + 1 ;
	handle->dc->SetBrush(handle->brush) ;
	handle->setPerimeterPen() ;
	handle->dc->DrawRectangle(pxyarray[0], pxyarray[1], width, height) ;
}

void vr_recfl( VirtualDevice * handle, int *pxyarray) 
{
	// draws rectangle always without perimeter
	wxCoord width  = abs(pxyarray[2] - pxyarray[0]) + 1 ;
	wxCoord height = abs(pxyarray[3] - pxyarray[1]) + 1 ;
	handle->dc->SetBrush(handle->brush) ;
	handle->dc->SetPen(*wxTRANSPARENT_PEN) ;
	handle->dc->DrawRectangle(pxyarray[0], pxyarray[1], width, height) ;
}

void v_circle( VirtualDevice * handle, int x, int y, int radius )
{
	handle->dc->SetBrush(handle->brush) ;
	handle->setPerimeterPen() ;
	handle->dc->DrawCircle(x, y, radius) ;
}

void v_ellipse( VirtualDevice * handle, int x, int y, int xradius,
                   int yradius  )
{
  handle->dc->SetBrush(handle->brush) ;
  handle->setPerimeterPen() ;
  handle->dc->DrawEllipse(x-xradius, y-yradius, xradius*2+1, yradius*2+1) ;
}

void v_ellarc( VirtualDevice * handle, int x, int y, int xradius,
                  int yradius, int begang, int endang )
{
  // draws arc line without fill area
  handle->dc->SetPen(handle->pen) ;
  // the VDI function does not fill
  handle->dc->SetBrush(*wxTRANSPARENT_BRUSH) ;
  // radius and centre must be transformed to rectanlge corners
  // VDI uses 1/10 degrees angles, wx uses degrees
  handle->dc->DrawEllipticArc(x - xradius, y - yradius, 2*xradius+1, 2*yradius+1, begang/10, endang/10) ;
}

void v_ellpie( VirtualDevice * handle, int x, int y, int xradius,
                  int yradius, int begang, int endang )
{
  // draws fill area without arc line
  // note: VDI expects 1/10 degrees angles [0..3600].
  handle->dc->SetBrush(handle->brush) ;
  handle->dc->SetPen(*wxTRANSPARENT_PEN) ;
  // radius and centre must be transformed to rectanlge corners
  // VDI uses 1/10 degrees angles, wx uses degrees
  handle->dc->DrawEllipticArc(x - xradius, y - yradius, 2*xradius+1, 2*yradius+1, begang/10, endang/10) ;
}


