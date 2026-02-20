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
    
    void setLineWidth(int width);
    void setLineColor(int color_index);
    void setTextHeight(int height, int *char_width);
    void setTextPoint(int point);
    void setFont(const wxString & font);
    void setTextEffects(int effect);
    void setTextAlignment(int hor_in, int vert_in);
    void setFillColor(int color_index);
    void setColor(int index, int *rgb_in);


    /****** Output definitions **********************************************/
    void drawBar(int *pxyarray);
    void drawPolygon(int count, int *pxyarray); 
    void drawFilledRect(int *pxyarray);
    void drawFilledArea(int count, int *pxyarray);
    void drawCircle(int x, int y, int radius);
    void drawEllipse(int x, int y, int xradius, int yradius);
    void drawEllArc(int x, int y, int xradius, int yradius, int begang, int endang);
    void drawEllPie(int x, int y, int xradius, int yradius, int begang, int endang);
    void drawText(int x, int y, const wxString & string);

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

/*      MY_VDI.H

        GEM VDI Definitions
        A subset of the Digital Research Virtual Device Interface
        as used on Atari GEM/TOS for graphics output to a device
        (screen, printer, ...).
        This subset is needed by the MIDI File Printer. The MIDI
        File Printer uses native VDI on Atari platforms and an
        emulation on other platforms. This file defines the interface
        of the emulation layer.
        Native VDI is implemented in C, the emulation layer is 
        implemented in C++. This file is included by files that
        are compiled as C and by files that are compiled as C++ !
*/


/****** Attribute definitions *****************************************/

/* the first 16 palette colors */

#define WHITE            0
#define BLACK            1
#define RED              2
#define GREEN            3
#define BLUE             4
#define CYAN             5
#define YELLOW           6
#define MAGENTA          7
#define LWHITE           8
#define LBLACK           9
#define LRED            10
#define LGREEN          11
#define LBLUE           12
#define LCYAN           13
#define LYELLOW         14
#define LMAGENTA        15

/* bit masks for vst_effects() */

#define NORMAL      0x00
#define BOLD        0x01
#define LIGHT       0x02
#define ITALIC      0x04
#define UNDERLINED  0x08
#define HOLLOW      0x10
#define SHADOWED    0x20

/* patterns for vsf_style() */

#define IP_HOLLOW       0
#define IP_1PATT        1
#define IP_2PATT        2
#define IP_3PATT        3
#define IP_4PATT        4
#define IP_5PATT        5
#define IP_6PATT        6
#define IP_7PATT        7
#define IP_SOLID        8


/* gsx write modes */

#define MD_REPLACE      1
#define MD_TRANS        2
#define MD_XOR          3
#define MD_ERASE        4


/* gsx fill styles */

#define FIS_HOLLOW      0
#define FIS_SOLID       1
#define FIS_PATTERN     2
#define FIS_HATCH       3
#define FIS_USER        4


/* bit blt rules for vro_cpyfm() and vrt_cpyfm() */

#define ALL_WHITE        0
#define S_AND_D          1
#define S_AND_NOTD       2
#define S_ONLY           3
#define NOTS_AND_D       4
#define D_ONLY           5
#define S_XOR_D          6
#define S_OR_D           7
#define NOT_SORD         8
#define NOT_SXORD        9
#define D_INVERT        10
#define NOT_D           11
#define S_OR_NOTD       12
#define NOTS_OR_D       13
#define NOT_SANDD       14
#define ALL_BLACK       15


/* linetypes */

#define SOLID           1
#define LONGDASH        2
#define DOT             3
#define DASHDOT         4
#define DASH            5
#define DASH2DOT        6
#define USERLINE        7

/* line ends for vsl_ends() */

#define SQUARE          0
#define ARROWED         1
#define ROUND           2

/* text effects (for vst_effects) */
#define TF_NORMAL			0x00
#define TF_THICKENED		0x01
#define TF_LIGHTENED		0x02
#define TF_SLANTED			0x04
#define TF_UNDERLINED		0x08
#define TF_OUTLINED			0x10
#define TF_SHADOWED			0x20

/* text alignment (for vst_alignment) */
/* conflict with Windows definitions! */
#ifndef _WINDOWS
#define TA_LEFT			0
#define TA_CENTER		1
#define TA_RIGHT		2

#define TA_BASELINE		0
#define TA_HALF			1
#define TA_ASCENT		2
#define TA_BOTTOM		3
#define TA_DESCENT		4
#define TA_TOP			5
#endif


#endif 
