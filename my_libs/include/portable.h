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

#ifndef min
	#define min(A,B) (((A) < (B)) ? (A) : (B))
#endif
#ifndef max
	#define max(A,B) (((A) > (B)) ? (A) : (B))
#endif

#if defined (__PALM__)
#include <PalmOS.h>
#	ifndef INT32 
		typedef Int32 INT32 ;
#	endif
#	ifndef UINT32 
		typedef UInt32 UINT32 ;
#	endif
#	ifndef INT16
		typedef Int16 INT16 ;
#	endif
#	ifndef UINT16
		typedef UInt16 UINT16 ;
#	endif
#	ifndef UBYTE
		typedef UInt8 UBYTE ;
#	endif
#	ifndef FALSE
		#define FALSE false ;
#	endif
#	ifndef TRUE
		#define TRUE true ;
#	endif

#define atoi     StrAToI
#define atol     StrAToI
#define strchr   StrChr
#define strstr   StrStr
#define strlen   StrLen
#define strcpy   StrCopy
#define strncpy  StrNCopy
#define strcat   StrCat
#define strncat  StrNCat
#define strcmp   StrCompare
#define strncmp  StrNCompare
#define sprintf  StrPrintF
#define memcpy   MemMove
#define memmove  MemMove
#define memcmp   MemCmp
#define memset(dest,character,N) MemSet(dest,N,character)

#define isxdigit(c) ((c>='a'&&c<='f')||(c>='A'&&c<='F')||(c>='0'&&c<='9'))

/*static Char *strpbrk(const Char *src, const Char *breaks)
{
  Char *ret = NULL ;
  UInt16 len = StrLen(breaks) ;
  UInt16 i ;
  for (i = 0 ; i < len ; i++)
  {
    Char *found_here = StrChr(src, breaks[i]) ;
    if (found_here)
    {
      if (!ret)
        ret = found_here ;
      else if (ret > found_here)
        ret = found_here ;
    }
  }
  return ret ; fehlerhaft ?
}*/

#else // wxWindows
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
# ifndef FALSE
#  define FALSE 0
# endif
# ifndef TRUE
#  define TRUE 1
# endif
#endif

#endif
