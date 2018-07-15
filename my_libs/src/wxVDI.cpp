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

int vswr_mode( VDI_HANDLE handle, int mode )
{ 
// ATTENTION: write mode is not yet implemented, but seems not yet to
// disturb (because MIDI File Printer uses RGBs only ?).

	((VirtualDevice *)handle)->writeMode = mode ; 
	return mode ;
}

void vs_color( VDI_HANDLE handle, int index, int *rgb_in )
{ 
    assert(index >= 0 && index <= 255) ;
    // VDI uses 0..1000 for RGB values.
    // [wx]Windows uses 0..255.
	unsigned char red   = (unsigned char)(rgb_in[0] * 0xff / 1000) ;
	unsigned char green = (unsigned char)(rgb_in[1] * 0xff / 1000) ;
	unsigned char blue  = (unsigned char)(rgb_in[2] * 0xff / 1000) ;
    ((VirtualDevice *)handle)->palette[index].Set(red,green,blue) ;
}

int vsl_type( VDI_HANDLE handle, int style )
{ 
	((VirtualDevice *)handle)->lineType = style ; 
	return style ;
}

int vsl_width( VDI_HANDLE handle, int width )
{ 
	VirtualDevice *vd = (VirtualDevice *)handle ;
	vd->lineWidth = width ; 
	vd->pen.SetWidth(width) ;
	return width ;
}

void vsl_udsty( VDI_HANDLE handle, int style )
{
	((VirtualDevice *)handle)->lineUserdefStyle = style ; 
}

int vsl_color( VDI_HANDLE handle, int color )
{ 
	VirtualDevice *vd = (VirtualDevice *)handle ;
	vd->lineColor = color ; 
	vd->pen.SetColour(vd->palette[color]) ;
	return color ;
}

int vst_color( VDI_HANDLE handle, int color )
{ 
	((VirtualDevice *)handle)->textColor = color ; 
	return color ;
}

void vst_height( VDI_HANDLE handle, int height, int *char_width,
                    int *char_height, int *cell_width,
                    int *cell_height )
{
	static const wxString text = _("Ab") ;
	VirtualDevice *vd = (VirtualDevice *)handle ;
	// transform pixels to points
	int point = vd->pixel_to_point(height, true) ;
	if (vd->pointSize != point)
	{
		vd->pointSize = point ;
		vd->setFont() ;
	}
	wxCoord w,h, descent, externalLeading ;
	vd->dc->GetTextExtent(text, &w, &h, &descent, &externalLeading) ;
	*char_width  = *cell_width = w/2 ;
	*char_height = h ;
	*cell_height = h + descent + externalLeading ;
}

int vst_point( VDI_HANDLE handle, int point, int *char_width,
                    int *char_height, int *cell_width,
                    int *cell_height )
{
	static const wxString text = _("Ab") ;
	VirtualDevice *vd = (VirtualDevice *)handle ;
	if (vd->pointSize != point)
	{
		vd->pointSize = point ;
		vd->setFont() ;
	}
	wxCoord w,h, descent, externalLeading ;
	vd->dc->GetTextExtent(text, &w, &h, &descent, &externalLeading) ;
	// transform pixels to points
	*char_width  = *cell_width = vd->pixel_to_point(w) ;
	*char_height = vd->pixel_to_point(h, true) ;
	*cell_height = vd->pixel_to_point(h + descent + externalLeading, true) ;
	return vd->pointSize ;
}

// In contrast to original VDI, here a ZIL_LOGICAL_FONT
// is expected.
const VDI_FONT_ID vst_font( VDI_HANDLE handle, const VDI_FONT_ID font_info )
{
/*	UI_DISPLAY *dsp = ((VirtualDevice *)handle)->dsp ;
	return dsp->Font(font) ;*/
	VirtualDevice *vd = (VirtualDevice *)handle ;
	vd->font_info = wxString::FromAscii(font_info) ;
	wxNativeFontInfo info ;
	info.FromString(wxString::FromAscii(font_info)) ;
	wxFont font ;
	font.SetNativeFontInfo(info) ;
	vd->dc->SetFont(font) ;
    //return font.GetNativeFontInfoDesc().c_str() ;
	return font_info ;
}

int vst_effects( VDI_HANDLE handle, int effects )
{
/*	UI_DISPLAY *dsp = ((VirtualDevice *)handle)->dsp ;
	STLF_FLAGS flags = STLF_NO_FLAGS ;
	if (effects & BOLD  )     flags |= STLF_BOLD ;
	if (effects & ITALIC)     flags |= STLF_ITALIC ;
	if (effects & UNDERLINED) flags |= STLF_UNDERLINED ;
	// the other effects we do not emulate
	dsp->CharStyle(flags) ;*/
	VirtualDevice *vd = (VirtualDevice *)handle ;
	if (vd->textEffects != effects)
	{
		vd->textEffects = effects ;
		vd->setFont() ;
	}
	return effects ;
}

void vst_alignment( VDI_HANDLE handle, int hor_in , int ver_in,
                                  int *hor_out, int *ver_out ) 
{
	((VirtualDevice *)handle)->textAlignHor = hor_in ; 
	((VirtualDevice *)handle)->textAlignVer = ver_in ; 
	*hor_out = hor_in ;
	*ver_out = ver_in ;
}

int vsf_interior( VDI_HANDLE handle, int interior )
{
	((VirtualDevice *)handle)->fillInterior = interior ; 
	return interior ;
}

// In contrast to original VDI, the style index
// will lead to a brightening of the color.
int vsf_style( VDI_HANDLE handle, int style_index )
{
	((VirtualDevice *)handle)->fillStyle = style_index ; 
	return style_index ; 
}

// In contrast to original VDI, here a RGB color value
// and no color index is expected.
int vsf_color( VDI_HANDLE handle, int color )
{
	VirtualDevice *vd = (VirtualDevice *)handle ;
	vd->fillColor = color ; 
	vd->brush.SetColour(vd->palette[color]) ;
    vd->pen_noPerimeter.SetColour(vd->palette[color]) ;
	return color ;
}

int vsf_perimeter( VDI_HANDLE handle, int per_vis )
{
	((VirtualDevice *)handle)->fillPerimeter = per_vis ; 
	return per_vis ;
}

//-------------- output functions -------------------------

void v_pline( VDI_HANDLE handle, int count, int *pxyarray )
{
	VirtualDevice *vd = (VirtualDevice *)handle ;
	vd->dc->SetPen(vd->pen) ;
	int i ;
	switch (vd->lineType)
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
			for (int j = 0 ; true ; j += (2*vd->lineWidth))
			{
				float fdx = unit_vector.x*j ;
				float fdy = unit_vector.y*j ;
				if ((int)fabs(fdx) > abs(dx) || (int)fabs(fdy) > abs(dy))
					break ;
				if (vd->lineWidth == 1)
					vd->dc->DrawPoint(x0 + (int)fdx, y0 + (int)fdy) ;
				else
				{
					wxPoint centre(x0 + (int)fdx, y0 + (int)fdy) ; 
					int radius = vd->lineWidth / 2 ;
					// the fill color must be the line color here
					wxBrush brush;
					brush.SetColour(vd->palette[vd->lineColor]) ;
					vd->dc->SetBrush(brush) ;
					vd->dc->DrawCircle(centre, radius) ;
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
			vd->dc->DrawLines(count, points, 0,0) ;
			delete [] points ; 
		}
		return ;
	}

	/* with background filling:
	virtual void DrawPolygon(int n, wxPoint points[],
                              wxCoord xoffset, wxCoord yoffset,
                              int fillStyle = wxODDEVEN_RULE) = 0;*/
}

void v_fillarea( VDI_HANDLE handle, int count, int *pxyarray )
{
	VirtualDevice *vd = (VirtualDevice *)handle ;
	// inner area
	wxPoint *points = new wxPoint[count] ;
	for (int i = 0 ; i < count ; i++)
		points[i] = wxPoint(pxyarray[i*2], pxyarray[i*2+1]) ;
	vd->dc->SetBrush(vd->brush) ;
	vd->setPerimeterPen() ;
	vd->dc->DrawPolygon(count, points, 0,0) ;

	delete points ;
}

void v_gtext( VDI_HANDLE handle, int x, int y, char *string )
{
	VirtualDevice *vd = (VirtualDevice *)handle ;

	int left, top ;
	wxCoord w, h, dummy ;
	vd->dc->GetTextExtent(wxString::FromAscii(string), &w, &dummy) ;
	vd->dc->GetTextExtent(_("Wq"), &dummy, &h) ;

	// Warning: both Visual C++ and Pure C #define TA_... 
	switch (vd->textAlignHor)
	{
	case /*TA_LEFT*/  0: left = x       ; break ;
	case /*TA_CENTER*/1: left = x - w/2 ; break ;
	case /*TA_RIGHT*/ 2: left = x - w   ; break ;
	}
	switch (vd->textAlignVer)
	{
	case /*TA_BASELINE*/0: top = y - h*3/4 ; break ; // estimated
	case /*TA_HALF*/    1: top = y - h/2   ; break ;
	case /*TA_ASCENT*/  2: top = y - h/5   ; break ; // estimated
	case /*TA_BOTTOM*/  3: top = y - h     ; break ;
	case /*TA_DESCENT*/ 4: top = y - h*4/5 ; break ; // estimated
	case /*TA_TOP*/     5: top = y         ; break ; 
	}
	vd->dc->SetTextForeground(vd->palette[vd->textColor]) ;
	vd->dc->SetTextBackground(vd->brush.GetColour()) ;
	vd->dc->DrawText(wxString::FromAscii(string), left, top) ;
}

void v_bar( VDI_HANDLE handle, int *pxyarray )
{
	VirtualDevice *vd = (VirtualDevice *)handle ;
	wxCoord width  = abs(pxyarray[2] - pxyarray[0]) + 1 ;
	wxCoord height = abs(pxyarray[3] - pxyarray[1]) + 1 ;
	vd->dc->SetBrush(vd->brush) ;
	vd->setPerimeterPen() ;
	vd->dc->DrawRectangle(pxyarray[0], pxyarray[1], width, height) ;
}

void vr_recfl( VDI_HANDLE handle, int *pxyarray) 
{
	// draws rectangle always without perimeter
	VirtualDevice *vd = (VirtualDevice *)handle ;
	wxCoord width  = abs(pxyarray[2] - pxyarray[0]) + 1 ;
	wxCoord height = abs(pxyarray[3] - pxyarray[1]) + 1 ;
	vd->dc->SetBrush(vd->brush) ;
	vd->dc->SetPen(*wxTRANSPARENT_PEN) ;
	vd->dc->DrawRectangle(pxyarray[0], pxyarray[1], width, height) ;
}

void v_circle( VDI_HANDLE handle, int x, int y, int radius )
{
	VirtualDevice *vd = (VirtualDevice *)handle ;
	vd->dc->SetBrush(vd->brush) ;
	vd->setPerimeterPen() ;
	vd->dc->DrawCircle(x, y, radius) ;
}

void v_ellipse( VDI_HANDLE handle, int x, int y, int xradius,
                   int yradius  )
{
  VirtualDevice *vd = (VirtualDevice *)handle ;
  vd->dc->SetBrush(vd->brush) ;
  vd->setPerimeterPen() ;
  vd->dc->DrawEllipse(x-xradius, y-yradius, xradius*2+1, yradius*2+1) ;
}

void v_ellarc( VDI_HANDLE handle, int x, int y, int xradius,
                  int yradius, int begang, int endang )
{
  // draws arc line without fill area
  VirtualDevice *vd = (VirtualDevice *)handle ;
  vd->dc->SetPen(vd->pen) ;
  // the VDI function does not fill
  vd->dc->SetBrush(*wxTRANSPARENT_BRUSH) ;
  // radius and centre must be transformed to rectanlge corners
  // VDI uses 1/10 degrees angles, wx uses degrees
  vd->dc->DrawEllipticArc(x - xradius, y - yradius, 2*xradius+1, 2*yradius+1, begang/10, endang/10) ;
}

void v_ellpie( VDI_HANDLE handle, int x, int y, int xradius,
                  int yradius, int begang, int endang )
{
  // draws fill area without arc line
  // note: VDI expects 1/10 degrees angles [0..3600].
  VirtualDevice *vd = (VirtualDevice *)handle ;
  vd->dc->SetBrush(vd->brush) ;
  vd->dc->SetPen(*wxTRANSPARENT_PEN) ;
  // radius and centre must be transformed to rectanlge corners
  // VDI uses 1/10 degrees angles, wx uses degrees
  vd->dc->DrawEllipticArc(x - xradius, y - yradius, 2*xradius+1, 2*yradius+1, begang/10, endang/10) ;
}


