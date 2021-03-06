/*****************************************************************************
  Purpose:     On Atari TOS palette modes on the output devices are supported.
               Conversions between RGB and palette colors are required.
               In high color mode (xhandle is RGB) RGB colors can be set
               and drawing is performed always on the same palette index 
               (DC_INDEX).
  Author:      Harald Rieder
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <wxVDI.h>
#include <graphic.h>

#define ELL_STRETCH   4/3   /* manipulation of ellipses in y-direction */
#define CIRC_SQUEEZE 13/20  /* manipulation of circle radi */
#define TRI_STRETCH   4/3   /* manipulation of triangles in y-direction */
#define SQRT_3       10/7   /* for ends with equilateral triangles */

void rgb_fstyle_fcolor(VirtualDevice * handle, int style, int color)
{
    int rgb[3] ;
    style_and_int16_to_rgb_array(style, color, rgb) ;
    handle->setColor(DC_INDEX, rgb) ;
    color = DC_INDEX ;
    style = IP_SOLID ;
	if (!style) handle->fillInterior = FIS_HOLLOW ;
	else        handle->fillInterior = FIS_PATTERN ;
	handle->fillStyle = style ;
	handle->setFillColor(color);
}


void rgb_udsty_lcolor(VirtualDevice *  handle, int udsty, int color, int back)
{
    int bits = 0, maxbits = 16 ;
    int red   = get_RED  (color) ;
    int green = get_GREEN(color) ;
    int blue  = get_BLUE (color) ;
    int bred   = get_RED  (back) ;
    int bgreen = get_GREEN(back) ;
    int bblue  = get_BLUE (back) ;
    /* draw mixed color but solid */
    while (udsty)
    {
        if (udsty & 1)
            bits++ ; 
        udsty >>= 1 ;
    }
    red   = bred   - (bred   - red  ) * bits / maxbits ;
    green = bgreen - (bgreen - green) * bits / maxbits ;
    blue  = bblue  - (bblue  - blue ) * bits / maxbits ;
    color = make_RGB(red, green, blue) ;
    
    handle->lineType = SOLID ;
	rgb_lcolor(handle, color) ;
}


void rgb_3d_ltype_lcolor(VirtualDevice *  handle, int style, int color)
{
	int user = 0 ;
    int rgb[3] ;
    /* color diff from light border to dark border 
       shall be twice the color diff from interior to dark border */
    /* IP_SOLID corresponds to the dark border */
    style = IP_SOLID - 2 * (IP_SOLID - style) ;
    if (style < 0)
        style = 0 ; 
    style_and_int16_to_rgb_array(style, color, rgb) ;
    handle->setColor(DC_INDEX, rgb);
    handle->lineUserdefStyle = 0xFFFF ;
    handle->lineType = USERLINE ;
	handle->setLineColor(DC_INDEX) ;
}


void int16_to_rgb_array(int compact_rgb, int rgb[3]) 
{
	rgb[0] = (int)((long)get_RED  (compact_rgb) * 1000 / 0x1F) ;
	rgb[1] = (int)((long)get_GREEN(compact_rgb) * 1000 / 0x1F) ;
	rgb[2] = (int)((long)get_BLUE (compact_rgb) * 1000 / 0x1F) ;
}


void style_and_int16_to_rgb_array(int style, int compact_rgb, int rgb[3])
{
	int i, full = 1000 ;
	int16_to_rgb_array(compact_rgb, rgb) ;
	for (i = 0 ; i < 3 ; i++)
		rgb[i] = rgb[i] + (IP_SOLID - style) * (full - rgb[i]) / IP_SOLID ; 
}

void rgb_fcolor(VirtualDevice * handle, int color)
{
    int rgb[3] ;
    int16_to_rgb_array(color, rgb) ;
    handle->setColor(DC_INDEX, rgb) ;
    color = DC_INDEX ;
	handle->setFillColor(color);
}


void rgb_lcolor(VirtualDevice * handle, int color)
{
    int rgb[3] ;
    int16_to_rgb_array(color, rgb);
    handle->setColor(DC_INDEX, rgb);
	handle->setLineColor(DC_INDEX);
}


void rgb_tcolor(VirtualDevice * handle, int color)
{
    int rgb[3] ;
    int16_to_rgb_array(color, rgb);
    handle->setColor(DC_INDEX, rgb);
	handle->textColor = DC_INDEX;
}

/******** auxiliary procedures *********/

static void ellip 
(
	VirtualDevice * handle, 
	int x0, int x1, int y, int ry, 
	int type, int style, int color 
) 
{
	int dx = abs(x1-x0) ;
	int rx = (int)((float)dx / 2 + 0.5) ;
	int x_mean = (int)((float)(x0+x1) / 2 + 0.5) ;
	
	if (rx == 0) rx++ ;
	if (ry == 0) ry++ ;
	
	rgb_fstyle_fcolor(handle, style, color) ;
	if (type & HEAD_CUT)
		handle->drawEllPie(x0, y, dx, ry, 2700, 4500) ;
	else if (type & TAIL_CUT)
		handle->drawEllPie(x1, y, dx, ry, 900, 2700) ;
	else 
		handle->drawEllipse(x_mean, y, rx, ry) ;

	if ((type & BORDER_FLAGS) == BORDERS_3D)
	{
		/* dark border */
        handle->lineType = SOLID ;
		rgb_lcolor(handle, color) ;
		if (type & HEAD_CUT) 
			handle->drawEllArc(x0, y, dx, ry, 2700, 3800);
		else if (type & TAIL_CUT)
			handle->drawEllArc(x1, y, dx, ry, 2000, 2700);
		else
			handle->drawEllArc(x_mean, y, rx, ry, 2000, 3800);

		/* light border */ 
		rgb_3d_ltype_lcolor(handle, style, color) ;
		if (type & HEAD_CUT) 
			handle->drawEllArc(x0, y, dx, ry, 200, 900) ;
		else if (type & TAIL_CUT)
			handle->drawEllArc(x1, y, dx, ry, 900, 2000) ;
		else
			handle->drawEllArc(x_mean, y, rx, ry, 200, 2000);
	}
	else if ( !(type & BORDERS_NONE) )
	{
        handle->lineType = SOLID ;
		rgb_lcolor(handle, color) ;
		if (type & HEAD_CUT) 
			handle->drawEllArc(x0, y, dx, ry, 2700, 4500) ;
		else if (type & TAIL_CUT)
			handle->drawEllArc(x1, y, dx, ry, 900, 2700) ;
		else
			handle->drawEllArc(x_mean, y, rx, ry, 0, 3600);
	}
}

				
static void rect(VirtualDevice * handle, int *points, int type, int style, int color)
{
	int p[10] ;
	
	p[0] = p[6] = p[8] = points[0] ;
	p[1] = p[3] = p[9] = points[3] ;
	p[2] = p[4] = points[2] ;
	p[5] = p[7] = points[1] ;
	rgb_fstyle_fcolor(handle, style, color) ;
	handle->drawFilledRect(points);	/* without borders => draw self */

	if ((type & BORDER_FLAGS) == BORDERS_3D)
	{
		/* dark border */
        handle->lineType = SOLID ;
		rgb_lcolor(handle, color) ;
		handle->drawPolygon(3, p) ;

		/* light border */ 
		rgb_3d_ltype_lcolor(handle, style, color) ;
		handle->drawPolygon(3, p+4) ;
	}
	else if ( !(type & BORDERS_NONE) )
	{
        handle->lineType = SOLID ;
		rgb_lcolor(handle, color) ;
		handle->drawPolygon(5, p) ;
	}
}


static void tri 
(
	VirtualDevice * handle, 
	int x0, int x1, int y, int dy,
	int type, int style, int color 
) 
{
	int p[10] ;
	
	rgb_fstyle_fcolor(handle, style, color) ;
	
	p[0] = p[2] = x0 ;
	p[4] = x1 ;
	if (type & HEAD_CUT) 
	{
		p[1] = y - (dy >> 1) ;
		p[3] = y + (dy >> 1) ;
		p[5] = y ;
		p[6] = p[0] ;
		p[7] = p[1] ;
		handle->drawFilledArea(3, p) ;
	}
	else if (type & TAIL_CUT)
	{
		p[1] = y - dy ;
		p[3] = y + dy ;
		p[5] = y + (dy >> 1) ;
		p[6] = p[4] ;
		p[7] = y - (dy >> 1) ;
		p[8] = p[0] ;
		p[9] = p[1] ;
		handle->drawFilledArea(4, p) ;
	}
	else
	{
		p[1] = y - dy ;
		p[3] = y + dy ;
		p[5] = y ;
		p[6] = p[0] ;
		p[7] = p[1] ;
		handle->drawFilledArea(3, p);
	}

	if ((type & BORDER_FLAGS) == BORDERS_3D)
	{
        handle->lineType = SOLID ;
		rgb_lcolor(handle, color) ;
		if (x1 > x0)
			/* dark border only at the bottom */
			handle->drawPolygon(2, p+2) ;
		else if (!(type & TAIL_CUT))
			/* dark border on the right */
			handle->drawPolygon(2, p) ;

		rgb_3d_ltype_lcolor(handle, style, color) ;
		if (x1 > x0)
        {
			/* light border only on the left side */ 
			 if (!(type & HEAD_CUT))
                 handle->drawPolygon(2, p) ;
        }
		else
		{
			/* light border only on top */
			if (type & TAIL_CUT)
				handle->drawPolygon(2, p+6) ;
			else
				handle->drawPolygon(2, p+4) ;
		}
	}
	else if ( !(type & BORDERS_NONE) )
	{
        handle->lineType = SOLID ;
		rgb_lcolor(handle, color) ;
		if (type & TAIL_CUT)
			handle->drawPolygon(5, p) ;
		else
			handle->drawPolygon(4, p) ;
	}
}				


void draw_note 
(
	VirtualDevice * handle, 	/* VDI workstation handle */
	int *points,	/* points[0]:     x-value of 1. corner
   					   points[1]:     y-value of 1. corner
	   				   points[2]:     x-value of 2. corner
		   			   points[3]:     y-value of 2. corner */
	int type,		
	int style, 		/* fill style body */
	int color,		/* fill	color body */
	int end_style,	/* fill style ends */
	int end_color	/* fill	color ends */
)
{
	const int y_mean = (int)( (float)(points[3] + points[1]) / 2 + 0.5 ) ;
	const int dy     = (points[3] - points[1]) >> 1 ;
	/* avoid TAIL_CUT and HEAD_CUT evaluation when drawing the ends: */
	const int end_type = type & BORDER_FLAGS ;	
	
	handle->writeMode = MD_REPLACE ;
	handle->fillPerimeter = 0 ; /* draw perimeter self */
	
	/* draw triangle ends in the background */
	if (type & HEAD_TRI && !(type & HEAD_CUT))
	{
		tri(handle, points[0], points[0] + (dy<<1), y_mean, dy<<1, 
			end_type, end_style, end_color) ; 
	}
	if (type & TAIL_TRI && !(type & TAIL_CUT))
	{
		tri(handle, points[2], points[2] - (dy<<1), y_mean, dy<<1, 
			end_type, end_style, end_color) ; 
	}
	/* draw note body */
	if (type & BODY_ELLIP) 
	{
		ellip(handle, points[0], points[2], y_mean, dy * ELL_STRETCH, 
			type, style, color) ;
	}
	else if (type & BODY_RECT)
	{
		rect(handle, points, type, style, color) ;
	}
	else if (type & BODY_TRI)
	{
		tri(handle, points[0], points[2], y_mean, dy * TRI_STRETCH, 
			type, style, color) ;
	}
	/* draw line or dot ends in the foreground */
	if ( !(type & HEAD_CUT) )
	{ 
		int r = dy * CIRC_SQUEEZE ;
		if (type & HEAD_DOT)
		{
			ellip(handle, points[0]-r, points[0]+r, y_mean, r, 
				end_type, end_style, end_color) ; 
		}
		else if (type & HEAD_LINE)
		{	
			int p[4] ;
			p[0] = p[2] = points[0] ;
			p[1] = points[1] - dy ;
			p[3] = points[3] + dy ;
            handle->lineType = SOLID ;
			rgb_lcolor(handle, end_color) ;
			handle->drawPolygon(2, p) ; 
		}
	}
	if ( !(type & TAIL_CUT) )
	{
		int r = dy * CIRC_SQUEEZE ;
		if (type & TAIL_DOT)
		{
			ellip(handle, points[2]-r, points[2]+r, y_mean, r, 
				end_type, end_style, end_color) ; 
		}
		else if (type & TAIL_LINE)
		{
			int p[4] ;
			p[0] = p[2] = points[2] ;
			p[1] = points[1] - dy ;
			p[3] = points[3] + dy ;
            handle->lineType = SOLID ;
			rgb_lcolor(handle, end_color) ;
			handle->drawPolygon(2, p) ; 
		}
	}
}


void draw_background
(
	VirtualDevice * handle, 	/* VDI workstation handle */
	int *points,	/* points[0]:     x-value of 1. corner
   					   points[1]:     y-value of 1. corner
	   				   points[2]:     x-value of 2. corner
		   			   points[3]:     y-value of 2. corner */
	int style, 		/* fill style */
	int color		/* fill	color */
)
{
	draw_note(handle, points, BORDERS_NONE | BODY_RECT, 
		style, color, style, color) ;
}

	
static void trans_pline(
	VirtualDevice *  handle, 	/* VDI workstation handle */
   int n, int *points) 
{
	handle->writeMode = MD_TRANS ;
	handle->drawPolygon(n, points) ;
}


void draw_lines 
(
	VirtualDevice * handle, 	/* VDI workstation handle */
	int  number,			/* number of lines to be drawn */
	char first_marked, 		/* first line with special marking */
	char d_marked, 			/* special marking each d_marked lines */
	MARK_MODE mode, 		/* where to place rectangle marks */
	int styles[11], 		/* styles to be used after a special line, or NULL */
	int colors[11], 		/* colors to be used after a special line, or NULL */
	char first_number,      /* number (text) for first special line */
	int number_dots,		/* of special line, no dots if -1 */
	int dx_text, 			/* text (number) offset from x_start on */
	int dy_text,			/* text (number) offset from y_start on */
	int x_start, 			/* of first line */
	int y_start,			/* of first line */
	int x_end, 				/* of first line */
	int y_end,				/* of first_line */
	float dx, 				/* distance of succeeding lines */
	float dy,				/* distance of succeeding lines */
	int line_width			/* in pixels */
) 
{
	int i, number_offset, k ; 
	int points[4], dx_m = (int)(dx * d_marked / 12 + 0.5) , 
	               dy_m = (int)(dy * d_marked / 12 + 0.5) ; 
	int dx_int = (int)(dx + 0.5), dy_int ;
	int x_width, y_width ;
	int color = -1, style = -1 ;	/* init: do not call draw_background() */
	char buff[34] = "" ; /* _itoa() on Windows produces up to 33 char. */
	
	if (dy >= 0) dy_int = (int)(dy + 0.5) ;
	else         dy_int = (int)(dy - 0.5) ; /* stupid processors! */

	number_offset = 11 ;
	if (colors)
	{
		color = colors[(first_number + number_offset) % 11] ;
		rgb_lcolor(handle, color) ;
	}
	number_offset = -1 ;
	
	handle->setLineWidth(line_width|1);
	for (i = 0 ; i < number ; i ++ )
	{
		points[0] = (int)(dx * i + 0.5) + x_start ; 
		if (dy > 0) points[1] = (int)(dy * i + 0.5) + y_start ;
		else        points[1] = (int)(dy * i - 0.5) + y_start ;
		points[2] = points[0] + x_end - x_start ; 
		points[3] = points[1] + y_end - y_start ;

		if (i % d_marked != first_marked) 
			trans_pline(handle, 2, points) ;
		else 
		{
			/* switch to next color and style */
			number_offset ++ ;
			if (colors && styles)
			{
				color = colors[(first_number + number_offset) % 11] ;
				style = styles[(first_number + number_offset) % 11] ;
				rgb_lcolor(handle, color) ;
				rgb_tcolor(handle, color) ;
			}
			if (first_marked == -1) 
				trans_pline(handle, 2, points) ;
			else
			{
				/* mark with number */
				if ( dx_text != INVALID_DX && dy_text != INVALID_DY )
				{
					handle->writeMode = MD_TRANS ;
					/* hexadec. can be used instead of dodec. here because max. is 127
					   128/12 < 12, hence one dodec. digit is sufficient */
					sprintf(buff, "%X", first_number + number_offset);
					/* itoa(first_number + number_offset, buff, 11) ; */
					handle->drawText(points[0] + dx_text, points[1] + dy_text, buff);
				}        
				if (style == IP_HOLLOW) handle->setLineWidth((line_width * 2)|1) ;
				if (number_dots <= 0)
				{
					if (mode == behind)
					{
						/* mark non-dotted, rectangle behind line */
						points[0] += dx_m ; points[1] += dy_m ; points[2] -= dx_m ; points[3] -= dy_m ; 
						if (style > IP_HOLLOW) 
							draw_background(handle, points, style, color) ;
						points[0] -= dx_m ; points[1] -= dy_m ; points[2] += dx_m ; points[3] += dy_m ; 
					}
					else 
					{
						/* mark non-dotted, rectangle between lines */
						if (i != (number - 1) && style > IP_HOLLOW) 
						{
							points[0] += dx_int ; 
							points[1] += dy_int ;
							draw_background(handle, points, style, color) ;
							points[0] -= dx_int ; 
							points[1] -= dy_int ;
						}
					}
					rgb_lcolor(handle, color) ;
					trans_pline(handle, 2, points) ;					
				}
				else 
				{
					x_width = ((x_end - x_start) / number_dots) >> 1 ;
					y_width = ((y_end - y_start) / number_dots) >> 1 ;
					for (k = 0 ; k < number_dots ; k++)
					{
						/* draw dot */
						points[0] = (int)(dx * i + x_start 
						            + (float)(x_end - x_start) * k / number_dots + dx_m + 0.5) ; 
						points[1] = (int)(dy * i + y_start 
						            + (float)(y_end - y_start) * k / number_dots + dy_m + 0.5) ;
						points[2] = points[0] + x_width - (dx_m << 1) ;
						points[3] = points[1] + y_width - (dy_m << 1) ;
						if (style > IP_HOLLOW)
							draw_background(handle, points, style, color) ;
					
						points[0] -= dx_m ; points[1] -= dy_m ; points[2] += dx_m ; points[3] += dy_m ; 
						rgb_lcolor(handle, color) ;
						trans_pline(handle, 2, points) ;					
					}
				}
				/* reset line width */
				handle->setLineWidth(line_width|1);
			}
		}
	}
}


int32_t RGB_15to24(uint16_t in) 
{
	int red   = five_to_eight_bits( in & RGB_RED ) ;
	int green = five_to_eight_bits((in & RGB_GREEN)>> 5) ; 
	int blue  = five_to_eight_bits((in & RGB_BLUE) >>10) ; 
	return RGB(red,green,blue) ;
}

int16_t RGB_24to15(uint32_t in)
{
	int red   = eight_to_five_bits( GetRValue(in) ) ;
	int green = eight_to_five_bits( GetGValue(in) ) ;
	int blue  = eight_to_five_bits( GetBValue(in) ) ;
	return (blue << 10) | (green << 5) | red ;
}

