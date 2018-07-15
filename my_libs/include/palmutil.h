/*****************************************************************************
  Purpose:     Useful functions on top of Palm OS API.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: palmutil.h,v 1.8 2008/09/20 20:04:17 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#ifndef PALMUTIL_H
#define PALMUTIL_H

Err rom_version_compatible(UInt32 required_version, UInt16 launch_flags);
  /* returns errNone if required_version is high enough, otherwise
     ROM_INCOMPATIBLE_ALERT is shown and sysErrRomIncompatible is returned */

void *get_object(FormPtr form, UInt16 object_ID);
  /* returns a pointer to the object with ID object_ID in the specified form */

void *get_object_from_active(UInt16 object_ID);
  /* returns a pointer to the object with ID object_ID in the currently active form */

const Char *get_field_text_id(FormPtr form, UInt16 object_id);
  /* returns a pointer to the text field referred to by object_id */

Err set_field_text(FieldPtr field, const Char *s, Boolean redraw);
  /* sets the field's text, if redraw is true, a redraw is performed */

Err set_field_text_id(FormPtr form, UInt16 object_id, const Char *s, Boolean redraw);
  /* same as set_field_text, but instead a FieldPtr an object ID is accepted
     as input */

Boolean select_popup_item(FormPtr form, 
                          UInt16 trigger_ID, UInt16 list_ID, const Char *item);
  /* Looks for a list item with the same text as item and - if found - sets the
     popup trigger's text to item. If not in the list, false is returned. */ 

Boolean read_record  (DmOpenRef db_ref, UInt16 index , void *data, UInt32 size) ;
  /* Reads record with index from database and stores size bytes of it
     in data. Returns false, if record could not be read. */

Boolean update_record(DmOpenRef db_ref, UInt16 *index, const void *data, UInt32 size) ;
  /* Writes size bytes of data to database record specified by index. 
     If the record does not exist, it will be created. Returns false
     if data could not be written. */

#ifndef max
#  define max(a,b) ( a > b ? a : b )
#endif
#ifndef min
#  define min(a,b) ( a < b ? a : b )
#endif

#endif /* include blocker */
