/*****************************************************************************
  Purpose:     File access functions (copy on disk, to RAM, ...)
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: DISKFILE.C,v 1.5 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#include <stdio.h>

int Fcopy(const char *output_name, const char *input_name)
{
	const unsigned bufsiz = 100 ;
	char buf[bufsiz] ;
	FILE * i = fopen(input_name, "rb") ;
	if (i == NULL)
		return -1 ;
	FILE * o = fopen(output_name, "wb") ;
	if (o == NULL)
	{
		fclose(i) ;
		return -1 ;
	}
	int nread ;
	while ((nread = fread(buf, 1/*byte*/, bufsiz, i)) > 0)
		fwrite(buf, 1, nread, o) ;
	fclose(i) ;
	fclose(o) ;
	return 0 ;
}
