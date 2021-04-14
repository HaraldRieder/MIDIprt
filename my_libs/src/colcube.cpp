/*****************************************************************************
  Purpose:     Color selection GUI widget (supporting functions).
  Author:      Harald Rieder
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include    <portable.h>
#include    <graphic.h>
#include    "colcube.h"

#define SQRT_3 17/10	/* sqrt(3) is around 1.7 */

void init_color_cube(COLOR_CUBE * cube, int ob_width, int ob_height, int black_mid)
{
	int i,j, steps = DIVIDER - 1 ;
	int rgb, white = RGB_RED + RGB_GREEN + RGB_BLUE, black = 0 ;
	
	cube->x0 = ob_width  / 2 ;
	cube->y0 = ob_height / 2 ;
	cube->a  = ob_height / 2 - 2 ; /* 2 pixels frame size */

	cube->red.x_end1 = cube->blue.x_end2 = -(cube->a * SQRT_3 / 2) ;
	cube->red.y_end1 = cube->red.y_end2 = 
	cube->green.y_end1 = cube->blue.y_end2 = cube->a / 2 ;
	cube->red.x_end2 = cube->green.x_end1 = -cube->red.x_end1 ;
	cube->green.x_end2 = cube->blue.x_end1 = 0 ;
	cube->green.y_end2 = cube->blue.y_end1 = -(cube->a) ;

	for (i = 0 ; i < DIVIDER ; i++)
	{
		for (j = 0 ; j < DIVIDER ; j++)
		{
			int col_max ;
			col_max = 0x1F ; /* 5 least significat bits set */
			if (black_mid)
				rgb = ((col_max * i / steps) << 5) /* green */
				    + ((col_max * j / steps) << 10 ) ; /* blue */
			else
				rgb = white 
					- ((col_max * i / steps) << 5) /* green */
					- ((col_max * j / steps) << 10 ) ; /* blue */
			cube->red.color[i][j] = rgb ;

			if (black_mid)
				rgb = ((col_max * i / steps) << 10) /* blue */
					+ (col_max * j / steps) ; /* red */
			else
				rgb = white 
					- ((col_max * i / steps) << 10) /* blue */
					- (col_max * j / steps) ; /* red */
			cube->green.color[i][j] = rgb ;
				
			if (black_mid)
				rgb = (col_max * i / steps) /* red */
					+ ((col_max * j / steps) << 5) ; /* green */
			else
				rgb = white 
					- (col_max * i / steps) /* red */
					- ((col_max * j / steps) << 5) ; /* green */
			cube->blue.color[i][j] = rgb ;
		}
	}
}


static void rgb_recfl(VirtualDevice * handle, int color, int points[4]) 
{
	rgb_fcolor(handle, color) ;
	handle->fillInterior = FIS_SOLID ;
	handle->fillPerimeter = 0 ;
	handle->writeMode =  MD_REPLACE ;
	handle->drawFilledRect(points);
}


static long vector_product(long x1, long y1, long x2, long y2)
{
	return (x1 * x2 + y1 * y2) ;
}

static int get_color_from_plane(
	const COLOR_PLANE *plane, 
	long x, 
	long y
)
{
	long i,j ;
	
	/* We have to solve the equation:
	
	   (x,y) = (i/(DIVIDER-1)) * (x_end1,y_end1) + (j/(DIVIDER-1)) * (x_end2,y_end) 
	   
	   We do this by multiplying with vectors normal to the plane's 
	   base vectors (y_end1,-x_end1) and (y_end2,-x_end2) and get:
	   
	   (DIVIDER-1) * (x,y) * (y_end2,-x_end2) = i * (x_end1,y_end1) * (y_end2,-x_end2)
	   
	   and
	   
	   (DIVIDER-1) * (x,y) * (y_end1,-x_end1) = j * (x_end2,y_end2) * (y_end1,-x_end1)
	   
	   */
	int divi = DIVIDER ;   
	i = vector_product(x * divi, y * divi, plane->y_end2, -plane->x_end2) 
		/ vector_product(plane->x_end1, plane->y_end1, plane->y_end2, -plane->x_end2) ; 
	j = vector_product(x * divi, y * divi, plane->y_end1, -plane->x_end1) 
		/ vector_product(plane->x_end2, plane->y_end2, plane->y_end1, -plane->x_end1) ; 
		
	if (i >= 0 && i < DIVIDER && j >= 0 && j < DIVIDER)
		return plane->color[(int)i][(int)j] ;	/* (x,y) is inside plane */

	return -1 ; /* (x,y) not inside plane */
}


int get_color_from_xy(const COLOR_CUBE * cube, int x, int y) 
{
	int color ;

	/* x,y from the cube's center */
	x -= cube->x0 ;
	y -= cube->y0 ;
	
	color = get_color_from_plane(&(cube->red), x,y) ;
	if (color < 0)
		color = get_color_from_plane(&(cube->green), x,y) ;
	if (color < 0)
		color = get_color_from_plane(&(cube->blue), x,y) ;
	return color ;
}


static void draw_plane(VirtualDevice * handle, const COLOR_PLANE *plane, int x0, int y0)
{
	int points[8], i,j ;

	for (i = 0 ; i < DIVIDER ; i++)
	{
		for (j = 0 ; j < DIVIDER ; j++)
		{
			points[0] = x0 
				+ (int)((long)plane->x_end1 * i / DIVIDER)
				+ (int)((long)plane->x_end2 * j / DIVIDER) ;
			points[1] = y0
				+ (int)((long)plane->y_end1 * i / DIVIDER)
				+ (int)((long)plane->y_end2 * j / DIVIDER) ;
			points[2] = x0 
				+ (int)((long)plane->x_end1 * (i+1) / DIVIDER)
				+ (int)((long)plane->x_end2 * j     / DIVIDER) ;
			points[3] = y0
				+ (int)((long)plane->y_end1 * (i+1) / DIVIDER)
				+ (int)((long)plane->y_end2 * j     / DIVIDER) ;
			points[4] = x0
				+ (int)((long)plane->x_end1 * (i+1) / DIVIDER)
				+ (int)((long)plane->x_end2 * (j+1) / DIVIDER) ;
			points[5] = y0
				+ (int)((long)plane->y_end1 * (i+1) / DIVIDER)
				+ (int)((long)plane->y_end2 * (j+1) / DIVIDER) ;
			points[6] = x0
				+ (int)((long)plane->x_end1 * i     / DIVIDER)
				+ (int)((long)plane->x_end2 * (j+1) / DIVIDER) ;
			points[7] = y0
				+ (int)((long)plane->y_end1 * i     / DIVIDER)
				+ (int)((long)plane->y_end2 * (j+1) / DIVIDER) ;

			rgb_fcolor   (handle, plane->color[i][j]) ; 
			handle->fillPerimeter =  0/*no perimeter*/ ;
			handle->fillInterior = FIS_SOLID ;
			handle->writeMode = MD_REPLACE ;
			handle->drawFilledArea(4, points);
		}
	}
}

	
void draw_cube(VirtualDevice * handle, const COLOR_CUBE * cube, int x, int y) 
{
	int x0 = x + cube->x0 ;
	int y0 = y + cube->y0 ;
	
	draw_plane(handle, &(cube->red  ), x0, y0) ;
	draw_plane(handle, &(cube->green), x0, y0) ;
	draw_plane(handle, &(cube->blue ), x0, y0) ;
}

