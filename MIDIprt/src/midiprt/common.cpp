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


int mm_to_pixel(const wxString & mm, int pixel_size)
    /* convert number of millimeters to number of pixels */
    /* pixel_size must be in micrometers */
    /* input must be a 3-char. string like "0.5" or "1.2" */
{
    int pixel = 0 ;
    
    if (mm.length() >= 3) {
        pixel = (1000 * wxAtoi(mm.SubString(0,1)) + 100 * wxAtoi(mm.SubString(2,3)))/pixel_size ;
    }
    
    return (pixel | 1) ; /* make odd */
}

void abspath(wxString & absolute, const wxString & path, const wxString & dir)
    /* prefix path with dir, if path is not absolute, and
       store into absolute. dir must end with a slash. */ 
{
#if defined (_WINDOWS)
    if (!path.Contains(_T(":")))
#else
    if (!path.StartsWith(_T(DIRSEP)))
#endif
    {
        /* no drive letter included, this is a relative path */
        absolute = dir; /* with terminating DIRSEP */
        absolute.append(path);
    }
    else
        /* already absolute path */
        absolute = path;
}


void relpath(wxString & relative, const wxString & path, const wxString & dir) 
    /* remove dir from path if contained therein and store into relative */
{ 
#if defined (_WINDOWS) 
    if (path.Contains(_T(":")))
#else
    if (!path.StartsWith(_T(DIRSEP)))
#endif
    {
        /* drive letter included, this is an absolute path */
        if ( path.StartsWith(dir) )
        {
            /* dir is the first part of path => cut */
            relative = path.Mid(dir.Len());
            return ;
        }
    }
    relative = path;
}


const wxString make_title(
    const wxString & title,  /* as loaded from profile */
    const wxString & path    /* of MIDI file */
)
{
    static const wxString err_title = _T("Error: make_title() called without title and without path!");

    if (!title.empty()) return title ;

    if (!path.empty())
    {
        std::string::size_type pos = path.rfind(_T(DIRSEP));
        if (pos != std::string::npos)
            return path.substr(pos + 1);
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


void init_info_from_tracks(INFO_DB *db, const std::vector<TRACK> & track_table)
{
    db->key = wxString(_T(""));
    db->tempo = wxString(_T(""));
    db->time = wxString(_T(""));
    db->copyright = wxString(_T(""));
    
    for (unsigned i = 0 ; i < track_table.size() ; i++)
    {
        if (db->key.empty() && track_table[i].track_info.key >= 0)
        {
            wxString type;
            switch (track_table[i].track_info.minor)
            {
            case 0: type = _T("maj") ; break ;
            case 1: type = _T("min") ; break ;
            }
            db->key = wxString::Format(_T("%X "), track_table[i].track_info.key);
            db->key.append(type);
        }
        if (db->tempo.empty() && track_table[i].track_info.tempo >= 0)
        {
            /* microseconds => milliseconds */
            db->tempo = wxString::Format(_T("%d") , track_table[i].track_info.tempo / 1000);
        }
        if (db->time.empty() && 
            track_table[i].track_info.numerator   >= 0 && 
            track_table[i].track_info.denominator >= 0)
        {
            db->time = wxString::Format(_T("%d") , track_table[i].track_info.numerator);
            db->time.append(_T("/"));
            db->time.append(wxString::Format(_T("%d") , track_table[i].track_info.denominator));
        }
        if (db->copyright.empty())
            db->copyright = track_table[i].track_info.copyright ;
    }
}


void init_params_from_tracks(PARAMS_DB *db, const std::vector<TRACK> & track_table)
{
    default_params(db) ;
    
    /* search tracks for very first time signature and init bar length */
    for (unsigned i = 0 ; i < track_table.size() ; i++)
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
    for (unsigned i = 0 ; i < track_table.size() ; i++)
    {
        if (track_table[i].track_info.key >= 0)
        {
            db->transpose = track_table[i].track_info.key ;
            break ;
        }
    }
}    


void init_filter_from_tracks(FILTER_DB *db, const std::vector<TRACK> & track_table)
{
    int i ;
    for (i = 0 ; i < track_table.size() ; i++)
    {
        db->track[i].name       = track_table[i].track_info.track_name ;
        db->track[i].device     = track_table[i].track_info.device_name ;
        db->track[i].instrument = track_table[i].track_info.instrument_name ;
        db->track[i].text       = track_table[i].track_info.text ;
        
        db->track[i].select = TR_INSTR ;
        if (db->track[i].instrument.empty()) 
        {
            if      (!db->track[i].name  .empty()) db->track[i].select = TR_NAME ;
            else if (!db->track[i].device.empty()) db->track[i].select = TR_DEVICE ;
            else if (!db->track[i].text  .empty()) db->track[i].select = TR_TEXT ;
        }
        
        db->track[i].channels =
        db->track[i].ch_filter = track_table[i].track_info.channels ;
        if ( track_table[i].elements_generated <= 0 )
             db->track[i].filter = DISABLED ;
        else db->track[i].filter = 1 ;
    }
}    


TIME get_max_time( const std::vector<TRACK> & track_table, const std::vector<FILTERED_TRACK> & ft )
    /* search the highest time of all events of all enabled tracks */
{
    TIME max_time = 0, track_max_time ;
    int i ;

    for (i = 0 ; i < ft.size() ; i++)
        if (ft[i].filter == 1)
        {
            track_max_time = 
                (track_table[i].first_element + track_table[i].elements_generated - 1)->time ;
            if (track_max_time > max_time)
                max_time = track_max_time ;
        }
    return max_time ;
} 
