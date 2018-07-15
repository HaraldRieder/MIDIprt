/*****************************************************************************
  Purpose:     File access functions (copy on disk, to RAM, ...)
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: DISKFILE.H,v 1.4 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef DISKFILE_H 
#define DISKFILE_H 1

/*** general ********************************************************/

#define FILENAME_LENGTH 128  /* max. 127 characters for a file name   */
#define PATHNAME_LENGTH 256  /* max. 255 characters for complete path */

/********************************************************************/



/*******************************************************************
*/
	int Fcopy(
		const char *output_name, /* IN: name of destination file */
		const char *input_name   /* IN: name of source file */
	) ;
/*
*  copies a file using a RAM buffer.
* 
* OUTPUT:  new file on disk
* 
* RETURN:  -1 if no success, 0 else
* 
*******************************************************************/

#endif /* include blocker */

