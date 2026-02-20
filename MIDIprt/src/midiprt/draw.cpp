/*****************************************************************************
  Purpose:     Page layout and drawing of music notation systems.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: DRAW.C,v 1.10 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdlib.h>    /* itoa(), ... */
#include <stdio.h>    /* sprintf(), ... */

#include <wx/datetime.h>
#include <graphic.h>
#include <notimtab.h>

#include "draw.h"


/*******************************************************************
*/
    void draw_track
    (
        VirtualDevice * handle,
        int *cpoints,    /* in: clipping rectangle */
        const TRACK & t, /* in: track to draw */
        unsigned channel_filter,    /* bit mask */
        int x_min, int x_max, /* drawing to be spread between x_min and x_max */
        int y,           /* y value where to draw note_of_y */
        float dy,        /* distance between succeeding notes */
        TIME min_time, TIME max_time, /* the notes between these times shall be spread over x_min/x_max */
        NOTE note_of_y,  /* note which corresponds to y */
        float height,    /* height of a note when dynscale = 0 or with mean MIDI note dynamic */
        int maxdynscale, /* n (relative units), max. of following parameter: */       
        int dynscale,    /* 0 .. n (relative units), scales MIDI dynamic influence on note height */       
        int type,             /* note type: with/without head/tail, ... (mask) */
        int style[12],        /* fill style for note bodies */
        int color[12],        /* color for note bodies */
        int end_style[12],    /* fill style for note ends */
        int end_color[12],    /* color for note ends */
        int transpose         /* color + style transpose */
    ) 
/*
* draws all notes of the specified track which 
* are located between "min_time" and "max_time". For drawing
* VDI routines are used with "handle" as parameter.
* 
*******************************************************************/
{
    NOTE_TIME_TABLE_ELEMENT *n ;
    float time_scale = ((float)(x_max - x_min)) / (max_time - min_time) ;
    int points[4] ;
    int y_note, dynamic ;
    int half_height ;
    int original_type = type, index /* for style and color arrays */ ;

    /* draw all notes between minimum and maximum time */
    n = get_element(t.first_element, t.elements_generated, min_time) ;
    if (!n) return ;
    while ( (n < t.first_element + t.elements_generated) && 
            (n->time <= max_time) ) 
    {
        if (channel_filter & (0x1 << n->channel))
        {
            points[0] = x_min + (int)(time_scale * (n->time - min_time) + 0.5) ;
            points[2] = x_min + (int)(time_scale 
                                      * ((n + n->reference)->time - min_time) + 0.5) ;
            /* reference could be negative! */
            if (points[2] < points[0])
            {
                /* swap, so points[2] is always >= points[0] */
                int buf = points[2] ;
                points[2] = points[0] ;
                points[0] = buf ;
            }
            /* if note is wider than the useable x-range: cut */
            if ( points[2] > x_max ) 
            { 
                points[2] = x_max ; 
                type |= TAIL_CUT ;
            }
            if ( points[0] < x_min ) 
            {
                points[0] = x_min ; 
                type |= HEAD_CUT ; 
            }
            /* if points[2] corresponds to a "note on" event or a "note off" */
            /* event with its "note on" in a previous time range: draw note  */
            if ( points[2] > points[0] &&
                /* within clipping ? */
                points[2] > cpoints[0] && points[0] < cpoints[2] )
            {
                if (dy > 0)
                     y_note = (int)(dy * ((n->note & ~_ON_) - note_of_y) + 0.5) + y ;
                else y_note = (int)(dy * ((n->note & ~_ON_) - note_of_y) - 0.5) + y ;
                if (n->note & _ON_) dynamic = n->dynamic ; 
                else dynamic = (n + n->reference)->dynamic ;  
                half_height = (int)((height * dynscale * (dynamic - MEAN_DYNAMIC) 
                    / maxdynscale / MEAN_DYNAMIC + height
                     + 0.5) / 2) ;
                points[1] = y_note - half_height ;
                points[3] = y_note + half_height ;
                /* draw note */
                index = ((n->note & ~_ON_) + 12 - transpose) % 12 ;
                draw_note(handle, points, type, 
                    style    [index], color    [index],
                    end_style[index], end_color[index]) ;
            }
        }
        n += 1 ; type = original_type ;
    }
}



static void 
draw_background_with_corner
(
    VirtualDevice * handle,
    int x1,
    int y1,
    int x2,
    int y2,
    int corner_width,
    int corner_height,
    int style,
    int color
)
{
    int pts[8] ;
    pts[0] = x1 + corner_width ;
    pts[1] = y1 ;
    pts[2] = x2 ;
    pts[3] = y2 ;
    draw_background(handle, pts, style, color);
    /* color+style set by draw_background */
    pts[2] = pts[0] ;
    pts[4] = pts[6] = x1 ;
    pts[5] = pts[3] ;
    pts[7] = pts[1] + corner_height ;
    handle->fillPerimeter = 0 ;
    handle->drawFilledArea(4, pts);
}


static void 
draw_background_without_corner
(
    VirtualDevice * handle,
    int x1,
    int y1,
    int x2,
    int y2,
    int style,
    int color
)
{
    int pts[4] ; 
    pts[0] = x1 ;
    pts[1] = y1 ;
    pts[2] = x2 ;
    pts[3] = y2 ;
    draw_background(handle, pts, style, color);
}


static void draw_footline
(
    VirtualDevice * handle,
    int x_min,
    int x_max,
    int y,                    /* text cell bottom */
    const char *filename,
    long        filesize,
    const char *caption,
    const char *version,
    const char *platform,
    int page,                /* current page index (0 .. npgs-1) */
    int npgs,                /* total document pages */
    int height                /* text height  */
)
{
    char text[12] ; 
    char footline[120] ;

    handle->setTextPoint(height);
    rgb_tcolor(handle, 0) ;
        
    /*** left part ***/
    /* filename and size */
    sprintf(footline, "%s (%ld byte) printed ", filename, filesize) ;
    /* year month day */
    wxDateTime now = wxDateTime::Now();
    sprintf(text, "%d", now.GetYear());
    strcat(footline, text) ; 
    strcat(footline, "-") ;
    sprintf(text, "%d", now.GetMonth());
    strcat(footline, text) ; 
    strcat(footline, "-") ;
    sprintf(text, "%d", now.GetMonth());
    strcat(footline, text) ; 

    /* right part */
    sprintf(footline, "%d/%d", page+1, npgs) ;        
    handle->setTextAlignment(2, /*4*/3); /* right, char. bottom */
    handle->drawText(x_max, y, footline);
}


void draw_page(
    VirtualDevice * handle,
    int x0, 
    int y0, 
    int width,            /* already zoomed */
    int height,
    int *points,        /* clipping */ 
    int pixel_height,    /* in micrometers */
    const std::vector<TRACK> & track,        /* the data to draw... */
    const std::vector<FILTERED_TRACK> & ft,
    const LAYOUT *lt,            /* with this layout... */
    int p,                        /* index of page to draw */
    const wxString & font,    /* font for all textes */
    int pts,        /* font height of title in points */  
    int effects,    /* text effects for title only */
    int foot_pts,    /* font height of footline in points */ 
    const wxString & title,
    const wxString & filename,    /* for footline ... */
    long filesize,
    const wxString & caption,
    const wxString & version,
    const wxString & platform,
    int note_height,    /* relative units */
    int note_maxdynscale,/* n */ 
    int note_dynscale,   /* 0 .. n */ 
    int mode,            /* Rieder, Beyreuther, Mix */
    int type,            /* of note (e.g. with tail, etc.) */
    COLOR_SCHEME & scheme,
    int transpose,
    char bars_per_system,
    char sub_bars,
    int line_width,
    int line_width_bars,
    int line_width_sub_bars,
    int line_width_notes
)
{
    const PAGE_DESCRIPTOR   *page = lt->page + p ;
    const SYSTEM_DESCRIPTOR *syst ;
    const LINE_DESCRIPTOR   *line ;     /* shortcut only */
    int s,l ;                /* system and line indeces */
    int index_width, dots;
    char text[10] ; 

    int x_min = x0 + (int)(lt->x_min * width) ;
    int x_max = x0 + (int)(lt->x_max * width) ;
    int y_min = y0 + (int)(lt->y_min * height) ;
    int y_max = y0 + (int)(lt->y_max * height) ;
/*    int title_height      = (int)(lt->dy_title      * height) ;*/
    int small_text_height = (int)(lt->dy_small_text * height) ;
    int title_height      = pts ;

    float dx_bars ;
    int x_min_lines, y_min_lines, y_max_lines ;
    int y_min_background, y_max_background ;
    int dy_small_text = (int)(lt->dy_note * height) ;
    int extent ;

    int y_min_line, y_max_line ;
    MARK_MODE mark_mode ;

    /* page index out of range ? */
    if (p < 0 || p >= lt->npgs)
        return ;

    handle->setFont(font);

    /* draw title */
    if ( p == 0 && (y0 + title_height) >= points[1] )
    {
        rgb_tcolor(handle, 0) ;
        handle->writeMode = MD_REPLACE ;
        handle->setTextPoint(title_height);
        handle->setTextAlignment(1, 2);
        handle->setTextEffects(effects);
        handle->drawText((x_min + x_max)/2, y_min, title);
    }
    handle->setTextEffects(0);

    /* draw note system(s) */
    handle->setTextHeight(small_text_height, &index_width) ;
    /* reserve a litte bit more space: */
    index_width = index_width * 4 / 3 ;

    x_min_lines = x_min + index_width ;
    /* x_max_lines = x_max */
    dx_bars = (float)(x_max - x_min_lines) / bars_per_system ;

    for (s = page->first_system ; s <= page->last_system ; s++)
    {
        syst = lt->syst + s ;
        
        y_min_lines = y0 + (int)(syst->y_min_lines * height) ;
        y_max_lines = y0 + (int)(syst->y_max_lines * height) ;

        /* draw background */
        y_min_background = y0 + (int)(syst->y_min * height) ;
        y_max_background = y0 + (int)(syst->y_max * height) ;
        if ( syst->type & SUCCEEDING )
            draw_background_without_corner (handle, 
                x_min, y_min_background,
                x_max, y_max_background,
                scheme.back_style, scheme.back_color) ;
        else
            draw_background_with_corner (handle, 
                x_min, y_min_background,
                x_max, y_max_background,
                index_width, small_text_height/2, /* corner */
                scheme.back_style, scheme.back_color) ;

        /* draw track lines */
        for (l = syst->first_line ; l <= syst->last_line ; l++)
        {
            line = lt->line + l ;

            y_min_line = y0 + (int)(line->y_min * height) ;
            y_max_line = y0 + (int)(line->y_max * height) ;
            extent = (int)(lt->dy_extent * height) ;

            /* vertical sub-bar lines */
            /* if track line is visible (inside clipping area) */
            /* Attention: sometimes points[1] ia greater than points[3] !! */
            if ( y_max_line + extent >= points[1] &&
                 y_min_line - extent <= points[3] )
            {
                /* draw horizontal lines with index and shadow */
                handle->lineType = SOLID ;
                handle->setTextAlignment(1, 0); /* center, baseline */
                dots = -1 /* no dots */ ; mark_mode = behind ;
                if      (mode == Rieder) mark_mode = between ;
                else if (mode == Beyreuther)
                    dots = (int)bars_per_system * sub_bars ;
                draw_lines(handle, 
                    line->number_of_lines, 
                    line->first_modulo_0, 
                    lt->hor_lines,
                    mark_mode, 
                    scheme.dodecime_style, scheme.dodecime_color, 
                    (line->note_of_y + 11)/12, dots,
                    - index_width/2, 0,
                    x_min_lines, y_max_line, 
                    x_max      , y_max_line,
                    0.0f, - lt->dy_line * height, /* from lowest to highest */
                    line_width) ; 
                /* draw track name */
                handle->setTextAlignment(0, 3);
                handle->writeMode = MD_TRANS ;
                rgb_tcolor(handle, scheme.text_color) ;
                {
                    wxString s;
                    switch ( ft[line->track].select )
                    {
                    case TR_INSTR:  s = ft[line->track].instrument ; break ;
                    case TR_DEVICE: s = ft[line->track].device     ; break ;
                    case TR_TEXT:   s = ft[line->track].text       ; break ;
                    default:        s = ft[line->track].name       ; 
                    }
                    handle->drawText(x_min_lines, y_min_line - dy_small_text, s);
                }
                /* draw notes of the current track */
                handle->setLineWidth(line_width_notes|1) ;
                draw_track(handle, points, track[line->track],
                           ft[line->track].ch_filter,    
                           x_min_lines, x_max, y_max_line, 
                           - lt->dy_note * height,
                           syst->min_time, syst->min_time + lt->time_per_system, 
                           line->note_of_y, 
/*                           lt->dy_note * height * note_height, */
                           lt->dy_note * height * note_height / 4,
                           note_maxdynscale, note_dynscale, 
                           type, 
                           scheme.note_style     , scheme.note_color, 
                           scheme.note_style_ends, scheme.note_color_ends,
                           transpose) ;
                if (mode != Beyreuther)
                {
                    /* draw dashed vertical lines (sub-bars), transparent/color-mixed */
                    rgb_udsty_lcolor(handle, /*0x2222*/ 0x4924, 
                        scheme.text_color, scheme.back_color) ;
                    draw_lines(handle, 
                        (bars_per_system * sub_bars) + 1, -1, -1,
                        mark_mode, NULL, NULL, 0, -1, 0, 0,
                        x_min_lines, y_min_line,
                        x_min_lines, y_max_line,
                        dx_bars/sub_bars, 0.0f, 
                        line_width_sub_bars) ;
                }
            } /* clipping */
            /* debug only: */
/*            else 
                printf (" l=%i ymax=%i ymin=%i ex=%i p1=%i p3=%i\n", l, y_max_line, y_min_line, extent, points[1], points[3]) ; 
*/
        } /* end foreach line of the system */
        
        /* draw solid vertical lines (bars) */
        handle->lineType = SOLID ; 
        rgb_lcolor(handle, scheme.text_color) ;
        draw_lines(handle, bars_per_system + 1, -1, -1,
               mark_mode, NULL, NULL, 0, -1, 0, 0,
               x_min_lines, y_min_lines, 
               x_min_lines, y_max_lines,
               dx_bars, 0.0f,
               line_width_bars) ;
        /* draw bar number */
        handle->writeMode = MD_TRANS ;
        rgb_tcolor(handle, scheme.text_color) ;
        handle->setTextAlignment(2, 3);
        sprintf(text, "%d", 
          (int)((syst->min_time/lt->time_per_system + 1) * bars_per_system));
        handle->drawText(x_max, y_min_lines - dy_small_text, text);

    } /* end foreach system of the page */

    /* draw foot line */
    if (y0 + (int)(lt->y_max_systems * height) <= points[3])
        draw_footline(handle, x_min, x_max, y_max,
            filename, filesize,
            caption, version, platform,
            p, lt->npgs,
            /*small_text_height*/foot_pts) ;
}


/********************/
static void expand /*
*********************/
(
    char lines_per_12,            /* IN: so many horizontal lines are used per 12 notes */
    NOTE min_note,                 /* IN: marks the minimum of the range to be expanded */
    NOTE max_note,                 /* IN: marks the maximum of the range to be expanded */
    NOTE *note_of_first_line ,    /* OUT: first note of the expanded range */
    char *number_of_lines,         /* OUT: the expanded range consists of n*(*number_of_lines) notes */
    char *first_modulo_0        /* OUT: indicates the first line with note mod m == 0 */
)
{
    char notes_per_line = 12 / lines_per_12 ;
    *note_of_first_line = min_note / notes_per_line * notes_per_line ;

    *number_of_lines = (max_note - *note_of_first_line + notes_per_line - 1) / 
                        notes_per_line + 1 ; 
    *first_modulo_0 = (lines_per_12 - *note_of_first_line % 12 / notes_per_line) %
                        lines_per_12 ;
}


/* width and height are normalized to 1.0f in the layout */
static const float W = 1.0f ;
static const float H = 1.0f ;

int page_layouter
(
    LAYOUT *lt,            /* OUT: the layout, all other params are IN */
    float left_border,  /* 0..1 */
    float right_border, 
    float upper_border, 
    float lower_border,
    float system_distance,
    float track_distance,
    float note_distance,
    char  hor_lines,                /* per dodecime */
    const std::vector<FILTERED_TRACK> & ft,
    const std::vector<TRACK> & track_table, /* from MIDI file */
    TIME  max_time,                    /* occuring in all tracks */
    TIME  time_per_system            /* in ticks */
)
{
    short first_active_track = -1, last_active_track = -1 ;
    int   i = 0 ;
    int   t = 0 ;    /* track number */
    float y = 0.0f ; /* current y value */
    float upper_extent ;
    TIME time = 0 ;
    int rc = LAYOUT_GOOD ;

    /* only shortcuts: */
    PAGE_DESCRIPTOR   *page  = lt->page ;   
    SYSTEM_DESCRIPTOR *syst  = lt->syst ;   
    LINE_DESCRIPTOR   *line  = lt->line ;   

    /* maximum area allowed for whole drawing */
    lt->x_min = left_border ;
    lt->y_min = upper_border ;
    lt->x_max = W - right_border ;
    lt->y_max = H - lower_border ;

    /* check for plausibility */
    if ( (lt->x_min >= lt->x_max) || (lt->y_min >= lt->y_max) )
    {
        lt->npgs = 0 ;
        return LAYOUT_ERR_INVALID_BORDERS ; 
    }

    /* get y-distances */
    lt->dy_system = system_distance ;
    lt->dy_track  = track_distance ;
    lt->dy_note   = note_distance ;
    lt->dy_extent = note_distance ;
    lt->dy_line   = lt->dy_note * (12/hor_lines) ; 
    lt->dy_title  = lt->dy_system * TITLE_FACTOR ;
    lt->dy_small_text = lt->dy_note * SMALL_TEXT_FACTOR ;
    upper_extent = lt->dy_extent + lt->dy_small_text ;
    
    /* maximum area allowed for drawing note systems */
    /* space for title: 0th page */
    lt->y_min_systems_0 = lt->y_min + lt->dy_system * 2/3 + lt->dy_title ;
    /* all pages except 0th */
    lt->y_min_systems   = lt->y_min ;    
    /* space for footline: */
    lt->y_max_systems   = lt->y_max - lt->dy_small_text ;

    lt->hor_lines       = hor_lines ;
    lt->max_time        = max_time  ;
    lt->time_per_system = time_per_system ;

    /* get first and last active tracks */
    while (i < ft.size()) 
    {
        if (ft[i].filter == 1) /* 1 = on and not disabled */
        {
            /* track is on */
            if (first_active_track == -1)
                first_active_track = i ;
            last_active_track = i ;
        }
        i++ ;
    }
    if ( first_active_track < 0 )
    {
        lt->npgs = 0 ;
        return LAYOUT_ERR_ALL_TRACKS_OFF ;
    }

    /* layout all pages */
    page->first_system =  0 ;
    page->last_system  = -1 ;
    lt->npgs = 1 ;
     
    syst->type = NO_TYPE ;
    syst->first_line = 0 ;

    lt->nlns = lt->nsts = 0 ; 

    t = first_active_track ;
    y = lt->y_min_systems_0 ;
    time = 0 ;

    while ( time < max_time && 
            lt->npgs < MAX_PAGES-1 && 
            lt->nsts < MAX_SYSTEMS-1 && 
            lt->nlns < MAX_LINES-1 ) 
    {
        syst->min_time = time ;
        syst->y_min = y ;
        y += upper_extent ;
        syst->y_min_lines = y ;
        if (syst->type & SUCCEEDING)
            y += lt->dy_track ;
        /* layout one system */
        while ( y <= lt->y_max_systems &&
                t <= last_active_track && 
                time < max_time && 
                lt->npgs < MAX_PAGES-1 && 
                lt->nsts < MAX_SYSTEMS-1 && 
                lt->nlns < MAX_LINES-1 ) 
        {
            while ( (t <  last_active_track && y < lt->y_max_systems) ||
                    (t == last_active_track && y + lt->dy_extent - lt->dy_track < lt->y_max_systems) )
            {
                /* if this track is active (on and not disabled) */
                if ( ft[t].filter == 1 )
                {
                    line->track = t ;
                    if ( get_min_max( track_table[t],
                                ft[t].ch_filter, 
                                time, time + lt->time_per_system,
                                &(line->min_note), 
                                &(line->max_note) ) )
                    {
                        expand(hor_lines, 
                               line->min_note, 
                               line->max_note, 
                               &(line->note_of_y), 
                               &(line->number_of_lines), 
                               &(line->first_modulo_0) ) ;
                    }
                    else
                    {
                        /* no events inside channel filter 
                           and time range found */
                        line->note_of_y = 0 ;
                        line->number_of_lines = 1 ;
                        line->first_modulo_0 = 0 ;
                    }           
                    /* fits each track into the allowed y-range of a page ? */
                    line->y_min = y ;
                    y += lt->dy_line * (line->number_of_lines - 1) ;
                    line->y_max = y ;
                    if ( (line->y_max - line->y_min) > 
                         (lt->y_max_systems - lt->y_min_systems - lt->dy_track - lt->dy_extent) )
                    {
                        lt->npgs-- ;    /* this page is not displayable */    
                        return LAYOUT_ERR_TRACK_TOO_HIGH ;
                    }
                    lt->nlns++ ; line++ ;
                    y += lt->dy_track ;
                }
                t++ ; 
            }
        } /* layout one system */
        t-- ; 
        y -= lt->dy_track ;
        if ( t <  last_active_track ||
            (t == last_active_track && y + lt->dy_extent > lt->y_max_systems) )
        {
            /* the current note system does not fit onto this page */
            y = lt->y_min_systems ;
            if (page->last_system == -1)
            {
                /* this is the first system for this page,
                   split it and put track t onto next page */
                lt->nlns-- ; line-- ;
                syst->type |= PRECEEDING ; 
                syst->last_line = lt->nlns - 1 ;
                syst->y_max =
                syst->y_max_lines = (line-1)->y_max + lt->dy_track ;
                page->first_system = page->last_system = lt->nsts ; 
                syst++ ; lt->nsts++ ;
                syst->type = SUCCEEDING ;
            }
            else 
            {
                /* there are already one or more systems on this page,
                   put the whole system onto the next page */
                lt->nlns = (syst-1)->last_line + 1 ;
                line = lt->line + lt->nlns ;
                syst->type = NO_TYPE ;
                /* retry with first track of system on top of new page */
                t = first_active_track ;
            }
            page++ ; lt->npgs ++ ;
            page->first_system = lt->nsts ;
            page->last_system  = -1 ;
        }
        else
        {
            /* the current note system fits onto this page */
            syst->y_max_lines = y ;
            syst->y_max       = y + lt->dy_extent ; 
            syst->last_line = lt->nlns - 1 ;
            if (syst->type == SUCCEEDING)
                /* one and only system for this page, 
                   start a new page afterwards */
                y = lt->y_max_systems ;
            else
                /* one more system for this page */
                y += lt->dy_system ;
            page->last_system = lt->nsts ;
            syst++ ; lt->nsts++ ;
            syst->type = NO_TYPE ;
            t = first_active_track ;
            time += lt->time_per_system ;
        }
        syst->first_line = syst->last_line = lt->nlns ;
    }
    if ( lt->npgs >= MAX_PAGES-1 )   rc = LAYOUT_ERR_TOO_MANY_PAGES ;
    if ( lt->nlns >= MAX_SYSTEMS-1 ) rc = LAYOUT_ERR_TOO_MANY_SYSTEMS ;
    if ( lt->nlns >= MAX_LINES-1 )   rc = LAYOUT_ERR_TOO_MANY_LINES ;
    if (rc != LAYOUT_GOOD)
        lt->npgs = 0 ;
    return rc ;
}
