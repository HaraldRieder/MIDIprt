// VirtualDevice.hpp: Schnittstelle für die Klasse VirtualDevice.
//
//////////////////////////////////////////////////////////////////////

#if !defined(VIRTUALDEVICE_HPP)
#define VIRTUALDEVICE_HPP

class VirtualDevice  
{
public:
	VirtualDevice(UI_DISPLAY *_dsp, ZIL_SCREENID _scrID);
	~VirtualDevice();
	UI_PALETTE *GetPalette(int rgb);
	UI_PALETTE *GetFillPalette();
	UI_PALETTE *GetLinePalette();

	UI_DISPLAY *dsp;
	ZIL_SCREENID scrID;
	ZIL_COLOR rgbLineColor;
	ZIL_COLOR rgbFillColor;
	ZIL_COLOR rgbTextColor;
	int writeMode;
	int lineType;
	int lineWidth;
	int fillInterior;
	int fillStyle;
	int fillPerimeter;
	int textAlignHor;
	int textAlignVer;
};

#endif 
