/*****************************************************************************
  Purpose:     Provide some OS portable definitions (Palm, Windows, Unix,
               Atari TOS)
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: PORTABLE.H,v 1.12 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef INC_PORTABLE
#define INC_PORTABLE

# ifdef __GNUG__
#  include <stdint.h>
# endif
# ifndef INT32 
        typedef int INT32, *PINT32;
# endif
# ifndef UINT32 
        typedef unsigned int UINT32, *PUINT32;
# endif
# ifndef INT16
#  ifdef __GNUG__
    typedef int16_t INT16 ;
#  else // Windows
		typedef __int16 INT16 ;
#  endif		
# endif
# ifndef UINT16
#  ifdef __GNUG__
    typedef uint16_t UINT16 ;
#  else // Windows
    typedef unsigned __int16 UINT16 ;
#  endif		
# endif
# ifndef UBYTE
    typedef unsigned char UBYTE ;
# endif
#endif
