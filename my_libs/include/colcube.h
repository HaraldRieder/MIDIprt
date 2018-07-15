/*****************************************************************************
  Purpose:     Color selection GUI widget (supporting definitions).
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: COLCUBE.H,v 1.3 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include    <graphic.h>

/* (DIVIDER - 1) should be a divider of 1000 */
#define DIVIDER 9

typedef struct
{
	int x_end1, y_end1 ;
	int x_end2, y_end2 ;
	int color[DIVIDER][DIVIDER] ; /* index in palette mode, 15 bit RGB else */
}
COLOR_PLANE ;

typedef struct
{
	int x0, y0 ;      /* cube's center in pixels from top left corner */
	int a ;
	COLOR_PLANE red ;   /* at the bottom */
	COLOR_PLANE green ; /* right top     */
	COLOR_PLANE blue ;  /* left top      */
}
COLOR_CUBE ;



/** 
* Must be called once to do some calculations and save
* performance later when calling the other functions.
* @param cube to init
* @param width of the cube (and the surrounding rect.) in pixel
* @param height of the cube (and the surrounding rect.) in pixel
* @param black_mid light cube if zero, dark cube else
*/
void init_color_cube(COLOR_CUBE * cube, int width, int height, int black_mid) ;

/**
* Gets the color index/15 bit RGB at the position (x,y).
* The position is measured from the top left corner.
* @return the color, or < 0 if (x,y) is outside the cube area
*/
int get_color_from_xy(const COLOR_CUBE * cube, int x, int y) ;

/**
* Draws the cube with width an height defined previously
* by init_color_cube. The top left corner of the surrouding 
* rectangle is at position (x,y).
*/
void draw_cube(X_HANDLE xhandle, const COLOR_CUBE * cube, int x, int y) ; 

