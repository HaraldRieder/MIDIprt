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
  RCS-ID:      $Id: wxVDI.h,v 1.8 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#if !defined(VIRTUALDEVICE_HPP)
#define VIRTUALDEVICE_HPP

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "my_vdi.h"

class VirtualDevice  
{
public:
	VirtualDevice(wxDC *dc);
	~VirtualDevice();

	/**
	* Calculates point size (1/72 inch) of specified pixel number.
	* @param vertical if false, take horizontal device resolution
	*/
	int pixel_to_point(int pixel, bool vertical=false) ;

	/**
	* Sets font with height and effects depending
	* on height and text effects.
	*/
	void setFont();

	/**
	* Sets the pen for the fill area perimter.
	* If fillPerimeter is off, the same color as
	* for the fill area is chosen for the pen.
	*/
	void setPerimeterPen();

	wxDC *dc;
	int writeMode;
	int lineType;
	int lineWidth;
    int lineUserdefStyle;
    int lineColor;
	wxPen pen;
    wxPen pen_noPerimeter; /**<< same color as brush */
	int fillInterior;
	int fillStyle;
	int fillPerimeter;
    int fillColor;
	wxBrush brush;
	int textAlignHor;
	int textAlignVer;
    int textColor;
	int textEffects;
	int pointSize;
	wxString font_info ;

	/**
	* The VDI color palette, max. 256 colors (8 bit per pixel).
	* 16 bit per pixel is already direct color (RGB mode, not palette mode).
	*/
	wxColor palette[256] ;
};

#endif 
