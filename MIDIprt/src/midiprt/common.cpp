/*****************************************************************************
  Purpose:     Defines functions common to the platforms (Atari TOS and
               wxWidgets) that don't belong to any other of the common 
               files.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: COMMON.C,v 1.7 2008/09/20 20:06:39 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include "stdio.h"
#include "common.h" 


int mm_to_pixel(const char *mm, int pixel_size)
    /* convert number of millimeters to number of pixels */
    /* pixel_size must be in micrometers */
    /* input must be a 3-char. string like "0.5" or "1.2" */
{
    int pixel = 0 ;
    
    if (mm && mm[0] && mm[1] && mm[2])
        pixel = (1000 * (mm[0]-'0') + 100 * (mm[2]-'0'))/pixel_size ;
    
    return (pixel | 1) ; /* make odd */
}

void abspath(char *absolute, const char *path, const char *dir)
    /* prefix path with dir, if path is not absolute, and
       store into absolute, which must be long at least 
       strlen(dir)+strlen(path)+1. dir must end with '\' . */ 
{
#if defined (_WINDOWS)
    if (!strchr(path, ':'))
#else
    if (path[0] != DIRSEP)
#endif
    {
        /* no drive letter included, this is a relative path */
        strcpy(absolute, dir) ; /* with terminating DIRSEP */
        strcat(absolute, path) ;
    }
    else
        /* already absolute path */
        strcpy(absolute, path) ;
}


void relpath(char *relative, const char *path, const char *dir) 
    /* remove dir from path if contained therein and store into relative,
       which must be long enough (at least strlen(path)+1) */
{ 
#if defined (_WINDOWS) 
    if (strchr(path, ':'))
#else
    if (path[0] != '/')
#endif
    {
        /* drive letter included, this is an absolute path */
        int len = (int)strlen(dir) ;
#if defined (_WINDOWS)
        if ( strnicmp(path, dir, len) == 0 )
#else
        if ( strncmp(path, dir, len) == 0 )
#endif
        {
            /* dir is the first part of path => compress */
            strcpy(relative, path + len) ;
            return ;
        }
    }
    strcpy(relative, path) ;
}


const char * make_title(
    const char * title,        /* as loaded from profile */
    const char * path        /* of MIDI file */
)
{
    static const char err_title[] = "Error: make_title() called without title and without path!" ;

    if (title && *title) return title ;

    if (path && *path)
    {
        const char *s = strrchr(path, DIRSEP) ;
        if (s) return (s + 1) ;    /* cut away path */
        return path ;    /* is already pure filename */
    }

    return err_title ;
}


TIME time_per_system(int ticks_per_quarter, int eighths_per_bar, int bars_per_line)
    /* note: params measures bar length in 1/8,
       info measures bar length in 1/4 (= 'beat' in MIDI file),
       therefore we need to divide by 2 */ 
{
    TIME time_per_bar = (TIME)ticks_per_quarter * eighths_per_bar / 2; 
    return (time_per_bar * bars_per_line) ;
}


void init_info_from_tracks(
    INFO_DB *db, 
    const TRACK track_table[],
    unsigned number_tracks)
{
    unsigned i ;

    db->key[0] = db->tempo[0] = db->time[0] = 0 ;
    db->copyright = 0 ;
    
    for (i = 0 ; i < number_tracks ; i++)
    {
        if (!db->key[0] && track_table[i].track_info.key >= 0)
        {
            const char *type = "" ;
            switch (track_table[i].track_info.minor)
            {
            case 0: type = "maj" ; break ;
            case 1: type = "min" ; break ;
            }
            sprintf(db->key, "%X %s", 
                track_table[i].track_info.key, type) ;
        }
        if (!db->tempo[0] && track_table[i].track_info.tempo >= 0)
        {
            /* microseconds => milliseconds */
            sprintf(db->tempo, "%d", track_table[i].track_info.tempo / 1000) ;
        }
        if (!db->time[0] && 
            track_table[i].track_info.numerator   >= 0 && 
            track_table[i].track_info.denominator >= 0)
        {
            sprintf(db->time, "%d/%d", 
                track_table[i].track_info.numerator,
                track_table[i].track_info.denominator) ;
        }
        if (!db->copyright && track_table[i].track_info.copyright)
            db->copyright = track_table[i].track_info.copyright ;
    }
}


void init_params_from_tracks(
    PARAMS_DB *db, 
    const TRACK track_table[],
    unsigned number_tracks)
{
    unsigned i ;
    default_params(db) ;
    
    /* search tracks for very first time signature and init bar length */
    for (i = 0 ; i < number_tracks ; i++)
    {
        /* n/d time signature */
        const int n = track_table[i].track_info.numerator ;
        const int d = track_table[i].track_info.denominator ;
        if (n >= 0 && d >= 0 && d <= 8)
        {
            /* then d is 1,2,4 or 8 */
            db->bar_length = n * 8 / d ; /* we use 1/8 units */
            db->sub_bars = n ;
            break ;
        }
    }
    /* search tracks for very first key signature and init bar length */
    for (i = 0 ; i < number_tracks ; i++)
    {
        if (track_table[i].track_info.key >= 0)
        {
            db->transpose = track_table[i].track_info.key ;
            break ;
        }
    }
}    


void init_filter_from_tracks(
    FILTER_DB *db, 
    const TRACK track_table[])
{
    unsigned i ;
    for (i = 0 ; i < db->number_tracks ; i++)
    {
        db->track[i].name       = track_table[i].track_info.track_name ;
        db->track[i].device     = track_table[i].track_info.device_name ;
        db->track[i].instrument = track_table[i].track_info.instrument_name ;
        db->track[i].text       = track_table[i].track_info.text ;
        
        db->track[i].select = TR_INSTR ;
        if (!db->track[i].instrument) 
        {
            if      (db->track[i].name  ) db->track[i].select = TR_NAME ;
            else if (db->track[i].device) db->track[i].select = TR_DEVICE ;
            else if (db->track[i].text  ) db->track[i].select = TR_TEXT ;
        }
        
        db->track[i].channels =
        db->track[i].ch_filter = track_table[i].track_info.channels ;
        if ( track_table[i].elements_generated <= 0 )
             db->track[i].filter = DISABLED ;
        else db->track[i].filter = 1 ;
    }
}    


TIME get_max_time( int number_tracks, const TRACK track_table[], const FILTERED_TRACK ft[] )
    /* search the highest time of all events of all enabled tracks */
{
    TIME max_time = 0, track_max_time ;
    int i ;

    for (i = 0 ; i < number_tracks ; i++)
        if (ft[i].filter == 1)
        {
            track_max_time = 
                (track_table[i].first_element + track_table[i].elements_generated - 1)->time ;
            if (track_max_time > max_time)
                max_time = track_max_time ;
        }
    return max_time ;
} 
