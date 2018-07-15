/*****************************************************************************
  Purpose:     Useful functions on top of Palm OS API.
  Author:      Harald Rieder
  Modified by: $Author: Harald $
  RCS-ID:      $Id: palmutil.c,v 1.9 2008/09/20 20:04:33 Harald Exp $
  Copyright:   (c) Harald Rieder
  Licence:     GNU General Public License V3
*****************************************************************************/

#define DO_NOT_ALLOW_ACCESS_TO_INTERNALS_OF_STRUCTS
#include <BuildDefines.h>
#ifdef DEBUG_BUILD
#  define ERROR_CHECK_LEVEL ERROR_CHECK_FULL
#endif
#include <PalmOS.h>
#include "portable.h"
#include "resource.h"
#include "palmutil.h"


Err rom_version_compatible(UInt32 requiredVersion, UInt16 launchFlags)
{
  UInt32 romVersion;

  // See if we're on in minimum required version of the ROM or later.
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);
  if (romVersion < requiredVersion) 
  {
    UInt16 safeToCallAlertFlags;
    
    safeToCallAlertFlags = sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp;
    if ((launchFlags & (safeToCallAlertFlags)) == safeToCallAlertFlags) 
    {
      FrmAlert (ROM_INCOMPATIBLE_ALERT);
    
      // Pilot 1.0 will continuously relaunch this app unless we switch to 
      // another safe one.
      if (romVersion < sysMakeROMVersion(2,0,0,sysROMStageRelease,0))
        AppLaunchWithCommand(sysFileCDefaultApp, sysAppLaunchCmdNormalLaunch, NULL);
    }
    return sysErrRomIncompatible;
  }
  return errNone;
}


void *get_object(FormPtr form, UInt16 object_ID)
{ return FrmGetObjectPtr(form, FrmGetObjectIndex(form, object_ID)); }

void *get_object_from_active(UInt16 object_ID)
{ return get_object(FrmGetActiveForm(), object_ID); }


const Char *get_field_text_id(FormPtr form, UInt16 object_id)
{
	const Char *text = FldGetTextPtr( (FieldPtr)get_object(form, object_id) ) ;
	return (text ? text : "") ;
}

Err set_field_text_id(FormPtr form, UInt16 object_id, const Char *s, Boolean redraw)
{
	return set_field_text((FieldPtr)get_object(form, object_id), s, redraw) ;
}

Err set_field_text(FieldPtr field, const Char *s, Boolean redraw)
{
   MemHandle h = FldGetTextHandle(field);
   if (h) 
   {
     Err	err;
     
     FldSetTextHandle(field, NULL);
     err = MemHandleResize(h, StrLen(s) + 1);
     if (err) {
       FldSetTextHandle(field, h);  // restore handle
       return err;
     }
   } 
   else 
   {
     h = MemHandleNew(StrLen(s) + 1);
     if (!h)
       return memErrNotEnoughSpace;
   }
   // at this point, we have a handle of the correct size

   // copy the string to the locked handle.
   StrCopy((Char *) MemHandleLock(h), s);
   // unlock the string handle.
   MemHandleUnlock(h);
   
   FldSetTextHandle(field, h);
   if (redraw)
   	 FldDrawField(field);
   return errNone;
} 


Boolean select_popup_item(FormPtr form, 
  UInt16 trigger_ID, UInt16 list_ID, const Char *item)
{
  ListPtr list = get_object( form, list_ID ) ;
  Int16 i, n = LstGetNumberOfItems(list) ;
  for (i = 0 ; i < n ; i++)
  {
    Char *text = LstGetSelectionText(list, i) ;
    if (text && item && StrCompare(text, item) == 0)
    {
      /* value is really present in the list */
      ControlPtr popup = get_object( form, trigger_ID ) ;
      CtlSetLabel(popup, text) ;
      LstSetSelection(list, i) ;
      return true ; 
    }
  }
  return false ;
}


/* ========== database ========= */

Boolean read_record(DmOpenRef db_ref, UInt16 index, void *data, UInt32 size)
{
  MemHandle record = DmQueryRecord(db_ref, index) ;
  if (record)
  {
    MemPtr ptr = MemHandleLock(record) ;
    if (!ptr)
      return false ;
    memcpy(data, ptr, size) ;
    MemHandleUnlock(record) ;
    return true ;
  }
  return false ;
}


Boolean update_record(DmOpenRef db_ref, UInt16 *index, const void *data, UInt32 size)
{ 
  void *dest ;
  Boolean changed = false ;

  MemHandle record = DmQueryRecord(db_ref, *index) ;
  if (record)
  {
    // exists already, reserve it for writing
    // the record may be from an older version with different size
    record = DmResizeRecord(db_ref, *index, size) ;
  }
  if (!record)
    // create record
    record = DmNewRecord(db_ref, index, size) ;

  if (!record)
    return false ;

  dest = MemHandleLock(record) ;
  if ( MemCmp(dest, data, size) )
  {
    // data has changed, should be backed up during next hotsync
    changed = true ;
    if (DmWrite(dest, 0, data, size) != errNone)
      return false ;
  }
  MemHandleUnlock(record) ;
  DmReleaseRecord(db_ref, *index, changed) ;
  return true ;
}
