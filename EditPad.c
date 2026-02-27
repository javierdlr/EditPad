/***************************************************************************

 TextEditor-Demo - Textediting MUI Custom Class Demonstration Program
 Copyright (C) 1997-2000 Allan Odgaard
 Copyright (C) 2005-2014 TextEditor.mcc Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 TextEditor class Support Site:  http://www.sf.net/projects/texteditor-mcc

Using as base code: TextEditor-Demo.c 955 2014-04-05 11:46:48Z damato

***************************************************************************/

#include <stdio.h>
#include <string.h>

#include <libraries/mui.h>
#include <libraries/iffparse.h>

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/utility.h>
#include <proto/graphics.h>
//#include <proto/rexxsyslib.h>
#include <proto/asl.h>
#include <proto/gadtools.h>
#include <proto/icon.h>
#include <proto/muimaster.h>

#include <mui/TextEditor_mcc.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <libraries/asl.h>


#include <proto/locale.h>
#define CATCOMP_NUMBERS
#define CATCOMP_BLOCK
#define CATCOMP_CODE
#include "editpad_catalog.h"
struct LocaleInfo li;
//STRPTR textbuf; // for localized messages (and maybe temporal stuff)


#if defined(__amigaos4__)
#include <dos/obsolete.h>
#endif


#include "SDI_hook.h"


#ifndef __amigaos4__
struct GfxBase       *GfxBase;
struct IntuitionBase *IntuitionBase;
//struct Library       *UtilityBase;
struct Library       *IconBase;
struct Library       *MUIMasterBase;
//#ifdef __MORPHOS__
//struct Library *RexxSysBase;
//#else
///struct RxsLib *RexxSysBase;
//#endif
struct Library       *GadToolsBase;
struct Library       *AslBase;
struct LocaleBase    *LocaleBase;
#else
struct Library *GfxBase;
struct Library *IntuitionBase;
//struct Library *UtilityBase;
struct Library *IconBase;
struct Library *MUIMasterBase;
//struct Library *RexxSysBase;
struct Library *GadToolsBase;
struct Library *AslBase;
struct Library *LocaleBase;

struct GraphicsIFace  *IGraphics;
struct IntuitionIFace *IIntuition;
//struct UtilityIFace   *IUtility;
struct IconIFace      *IIcon;
struct MUIMasterIFace *IMUIMaster;
//struct RexxSysIFace   *IRexxSys;
struct GadToolsIFace  *IGadTools;
struct AslIFace       *IAsl;
struct LocaleIFace    *ILocale;
#endif


//#ifndef MUIA_Application_UsedClasses
//#define MUIA_Application_UsedClasses  0x8042e9a7  /* V20 STRPTR * i.. */
//#endif
//#define MUIV_RunARexxScript           0xad800000


#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
#include "Events.h"
#include "debug.h"


static const TEXT USED verstag[] = VERSTAG;
#if defined(__VBCC__)
size_t __stack=65536;                           // 64KB stack-size VBCC
#endif
#if defined(__GNUC__)
static const char min_stack[] = "$STACK:65536"; // 64KB stack-size GCC
#endif


/*
HOOKPROTONH(ARexxHookCode, LONG, Object *app, struct RexxMsg *rexxmsg)
{
  IPTR result;

  result = DoMethod(editorgad, MUIM_TextEditor_ARexxCmd, rexxmsg->rm_Args[0]);

  if(result)
  {
    if(result != TRUE)
    {
      set(app, MUIA_Application_RexxString, result);
      FreeVec((APTR)result);
    }
  }

  return(0);
}
MakeStaticHook(ARexxHook, ARexxHookCode);
*/


DISPATCHERPROTO(TextEditor_Dispatcher)
{
  struct editpad_data *data;

  switch(msg->MethodID)
  {
    case MUIM_Setup:
    {
//DBUG("[MUIM_Setup]\n",NULL);
      if(DoSuperMethodA(cl, obj, (Msg)msg))
      {
        return TRUE;
      }
      else {
        return FALSE;
      }
    }
    break;

    case MUIM_Cleanup:
    {
//DBUG("[MUIM_Cleanup]\n",NULL);
    }
    break;

    case MUIM_DragQuery:
    {
DBUG("[MUIM_DragQuery]\n",NULL);
      return TRUE;
    }

    case MUIM_DragDrop:
    {
      struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;
      //ULONG active = 0;
DBUG("[MUIM_DragDrop] %p\n",drop_msg->obj);
      //if(GetAttr(MUIA_List_Active, drop_msg->obj, (IPTR *)&active)) {
      //  DoMethod(editorgad, MUIM_TextEditor_InsertText, StdEntries[active]);
      //}
    }
    break;

    case MUIM_TextEditor_HandleError:
    {
      char *errortxt = NULL;
      struct MUIP_TextEditor_HandleError *msgerr;
DBUG("[MUIM_TextEditor_HandleError]\n",NULL);
      data = (struct editpad_data *)INST_DATA(cl, obj);

      msgerr = (struct MUIP_TextEditor_HandleError *)msg;

      switch(msgerr->errorcode)
      {
        case Error_ClipboardIsEmpty:
          errortxt = (STRPTR)GetString(&li,MSG_ERROR_CLIPBOARD_EMPTY);
        break;

        case Error_ClipboardIsNotFTXT:
          errortxt = (STRPTR)GetString(&li,MSG_ERROR_CLIPBOARD_NOTEXT);//"\33cThe clipboard does not contain text.";
        break;

        case Error_MacroBufferIsFull:
        case Error_MemoryAllocationFailed:
        case Error_NoMacroDefined:
        case Error_StringNotFound:
        break;

        case Error_NoAreaMarked:
          errortxt = (STRPTR)GetString(&li,MSG_ERROR_NOTHING_MARKED);
        break;

        case Error_NothingToRedo:
          errortxt = (STRPTR)GetString(&li,MSG_ERROR_NO_REDO);
        break;

        case Error_NothingToUndo:
          errortxt = (STRPTR)GetString(&li,MSG_ERROR_NO_UNDO);
        break;

        case Error_NotEnoughUndoMem:
          errortxt = (STRPTR)GetString(&li,MSG_ERROR_NOT_ENOUGH_MEM);
        break;

        /*case Error_NoBookmarkInstalled:
          errortxt = "There is no bookmark installed!";
        break;*/

        /*case Error_BookmarkHasBeenLost:
          errortxt = "Your bookmark has unfortunately been lost.";
        break;*/
      }

      if(errortxt != NULL)
      {
DBUG("app=%p  win=%p\n",data->App,data->Win);
        MUI_Request(data->App, data->Win, 0L,
                    PROGNAME,
                    (STRPTR)GetString(&li,MSG_CONTINUE_BUTTON_PLAIN),
                    errortxt);
      }
    }
    break;

/*case MUIM_Cleanup:
DBUG("[MUIM_Cleanup]\n",NULL);
break;*/
  }

  return DoSuperMethodA(cl, obj, (Msg)msg);
}


/*static Object *toggleBtnObj(const char *text, UBYTE key)
{
  return(TextObject,
         MUIA_Background,    MUII_ButtonBack,
         MUIA_Frame,         MUIV_Frame_Button,
         MUIA_InputMode,     MUIV_InputMode_Toggle,
         MUIA_ControlChar,   key,
         //MUIA_CycleChain,    TRUE,
         MUIA_FixWidth,      24,
         //MUIA_Font,          MUIV_Font_Tiny,
         MUIA_Text_PreParse, "\33c",
         MUIA_Text_Contents, text,
        End);
}*/


static Object *buttonObj(const char *text)
{
  return(TextObject,
         MUIA_Background,    MUII_ButtonBack,
         MUIA_Frame,         MUIV_Frame_Button,
         MUIA_InputMode,     MUIV_InputMode_RelVerify,
         //MUIA_ControlChar,   key,
         //MUIA_CycleChain,    TRUE,
         //MUIA_FixWidth,      24,
         //MUIA_MaxWidth,      48,
         //MUIA_Font,          MUIV_Font_Tiny,
         MUIA_Text_PreParse, "\33c",
         MUIA_Text_Contents, text,
        End);
}


static BOOL open_Libs(void)
{
  GfxBase       = (APTR)OpenLibrary("graphics.library", 39);
  IntuitionBase = (APTR)OpenLibrary("intuition.library", 39);
  //UtilityBase   = OpenLibrary("utility.library", 39);
  IconBase      = OpenLibrary("icon.library", 39);
  MUIMasterBase = OpenLibrary(MUIMASTER_NAME, MUIMASTER_VMIN);
  //RexxSysBase   = (APTR)OpenLibrary("rexxsyslib.library", 36);
  GadToolsBase  = OpenLibrary("gadtools.library", 39);
  AslBase       = OpenLibrary("asl.library", 39);

  li.li_Catalog = NULL;
  if( (LocaleBase = (APTR)OpenLibrary("locale.library", 39)) )
  {
    #ifdef __amigaos4__
    ILocale = (struct LocaleIFace *)GetInterface((struct Library *)LocaleBase, "main", 1, NULL);
    li.li_ILocale = ILocale;
    #else
    li.li_LocaleBase = (struct Library*)LocaleBase;
    #endif
    li.li_Catalog = OpenCatalog(NULL, "editpad.catalog",
                                OC_BuiltInLanguage, "english",
                               TAG_END);
  }
  /*else
  {
    PutErrStr("Failed to use catalog system. Using built-in strings.\n");
  }*/

  if(GfxBase  &&  IntuitionBase  &&  MUIMasterBase  &&  GadToolsBase
     &&  AslBase)//  &&  RexxSysBase)
  {
    #ifdef __amigaos4__
    IGraphics  = (struct GraphicsIFace *)GetInterface((struct Library *)GfxBase, "main", 1, NULL);
    IIntuition = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase, "main", 1, NULL);
    //IUtility   = (struct UtilityIFace *)GetInterface((struct Library *)UtilityBase, "main", 1, NULL);
    IIcon      = (struct IconIFace *)GetInterface(IconBase, "main", 1, NULL);
    IMUIMaster = (struct MUIMasterIFace *)GetInterface(MUIMasterBase, "main", 1, NULL);
    //IRexxSys   = (struct RexxSysIFace *)GetInterface(RexxSysBase, "main", 1, NULL);
    IGadTools  = (struct GadToolsIFace *)GetInterface(GadToolsBase, "main", 1, NULL);
    IAsl       = (struct IAsl *)GetInterface(AslBase, "main", 1, NULL);
    #endif

    return TRUE;
  }

  return FALSE;
}


static void close_Libs(void)
{
  if(li.li_Catalog) { CloseCatalog(li.li_Catalog); }

  #ifdef __amigaos4__
  if (IGadTools)  { DropInterface((struct Interface *)IGadTools); }
  //if (IRexxSys)   { DropInterface((struct Interface *)IRexxSys); }
  if (ILocale)    { DropInterface((struct Interface *)ILocale); }
  if (IMUIMaster) { DropInterface((struct Interface *)IMUIMaster); }
  //if (IUtility)   { DropInterface((struct Interface *)IUtility); }
  if (IIcon)      { DropInterface((struct Interface *)IIcon); }
  if (IIntuition) { DropInterface((struct Interface *)IIntuition); }
  if (IGraphics)  { DropInterface((struct Interface *)IGraphics); }
  if (IAsl)       { DropInterface((struct Interface *)IAsl); }
  #endif

  if (AslBase)       { CloseLibrary(AslBase); }
  if (GadToolsBase)  { CloseLibrary(GadToolsBase); }
  //if (RexxSysBase)   { CloseLibrary((struct Library *)RexxSysBase); }
  if (MUIMasterBase) { CloseLibrary(MUIMasterBase); }
  if (LocaleBase)    { CloseLibrary((struct Library*)LocaleBase); }
  //if (UtilityBase)   { CloseLibrary(UtilityBase); }
  if (IconBase)      { CloseLibrary(IconBase); }
  if (IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); }
  if (GfxBase)       { CloseLibrary((struct Library *)GfxBase); }
}


/*
 * Set 'epd->epd_FullName' as window title
 */
static void set_WindowTitle(struct editpad_data *epd)
{
  static char wintitle[MAX_DOS_PATH];

  snprintf(wintitle, MAX_DOS_PATH, PROGNAME": %s",epd->epd_FullName);
  set(epd->Win, MUIA_Window_Title,wintitle);
}


/*
 * Splits 'epd_FullName' into 'epd_FRDrawer' and 'epd_FRFile'
 */
static void split_FullName(struct editpad_data *epd)
{
  ULONG len = strlen(epd->epd_FullName);
DBUG("%s()\n",__FUNCTION__);
  FreeString(&epd->epd_FRFile);
  FreeString(&epd->epd_FRDrawer);

  epd->epd_FRFile = DupStr(FilePart(epd->epd_FullName), -1);
DBUG("  epd_FRFile: '%s'\n",epd->epd_FRFile);
  len -= strlen(epd->epd_FRFile);
  epd->epd_FRDrawer = DupStr(epd->epd_FullName, len);
DBUG("  epd_FRDrawer: '%s' [%ld]\n",epd->epd_FRDrawer,len);
}


/*
 * Get file protection bits of 'epd->epd_FullName'
 */
static LONG get_FileAttribs(struct editpad_data *epd)
{
  ULONG attribs = FIBF_EXECUTE; // RW-D (E bit not set); looks like is "inversed mode"
  BPTR  lock;
DBUG("%s() %p\n",__FUNCTION__,epd->epd_FullName);
  if( epd->epd_FullName!=NULL  &&  (lock=Lock(epd->epd_FullName,ACCESS_READ)) )
  {
    struct FileInfoBlock *fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if( fib  &&  Examine(lock,fib) )
    {
      attribs = fib->fib_Protection;
    }
    FreeDosObject(DOS_FIB, fib);
    UnLock(lock);
  }
DBUG("  0x%08lx\n",attribs);
  return attribs;
}


HOOKPROTONO(AppMsgFunc, LONG, struct AppMessage **x) {
	struct AppMessage *am = *x;

	if( am->am_Type == AMTYPE_APPWINDOW
	   || (am->am_Type == AMTYPE_APPICON && am->am_Class == AMCLASSICON_Open) )
	{
		struct editpad_data *epd = (struct editpad_data*)hook->h_Data;
		struct WBArg        *arg = am->am_ArgList;
		char                 buf[MAX_DOS_PATH];

		NameFromLock(arg->wa_Lock, buf, MAX_DOS_PATH);
		AddPart(buf, arg->wa_Name, MAX_DOS_PATH);

		FreeString(&epd->epd_FullName);
		epd->epd_FullName = DupStr(buf, -1);
		Load_File(epd);
	}

	return 0;
}
MakeStaticHook(AppMsgHook, AppMsgFunc);


/*HOOKPROTO(ASL_startFunc, ULONG, Object *obj, struct TagItem *ti)//FileRequester *fr)
{
  struct editpad_data *epd = (struct editpad_data*)hook->h_Data;
  struct Window *win;
DBUG("%s()\n",__FUNCTION__);

  set(epd->EditorObj, MUIA_Disabled, TRUE);

  get(epd->Win, MUIA_Window_Window,&win);
DBUG("win = %p\n",win);
  ti->ti_Tag  = ASLFR_Window;
  ti->ti_Data = (ULONG)win;
  ++ti;
  ti->ti_Tag  = ASLFR_SleepWindow;
  ti->ti_Data = (ULONG)TRUE;
  ++ti;
  ti->ti_Tag  = TAG_DONE;

  return TRUE;
}
MakeStaticHook(ASL_startHook, ASL_startFunc);*/


/*HOOKPROTONO(ASL_stopFunc, ULONG, / *Object *obj,* / struct FileRequester *fr)
{
	struct editpad_data *epd = (struct editpad_data*)hook->h_Data;
	char fullpath[MAX_DOS_PATH];
DBUG("%s()\n",__FUNCTION__);

	//SetAttrs(epd->Win, MUIA_Window_Sleep,TRUE, TAG_DONE);

DBUG("  file: '%s'\n",fr->fr_File);
DBUG("  path: '%s'\n",fr->fr_Drawer);
	strncpy(fullpath, fr->fr_Drawer, MAX_DOS_PATH);
	AddPart(fullpath, fr->fr_File, MAX_DOS_PATH);
DBUG("  fullpath: '%s'\n",fullpath);
	//SetAttrs(obj, MUIA_String_Contents,fullpath, TAG_DONE);

	//SetAttrs(epd->Win, MUIA_Window_Sleep,FALSE, TAG_DONE);

	set(epd->EditorObj, MUIA_Disabled, FALSE);

	return TRUE;//(ULONG)load_File(epd, fullpath);
}
MakeStaticHook(ASL_stopHook, ASL_stopFunc);*/



/*
 * MAIN
 */
int main(int argc, char **argv)
{
  int rc = RETURN_FAIL;

  if( open_Libs() )
  {
    struct RDArgs       *args;
    IPTR                 argarray[1] = { 0 };
    struct editpad_data *EPD = AllocVec(sizeof(struct editpad_data), MEMF_CLEAR);

    EPD->epd_ProgName = AllocVec(MAX_DOS_PATH, 0L);
    GetCurrentDirName(EPD->epd_ProgName, MAX_DOS_PATH);
    AddPart(EPD->epd_ProgName, PROGNAME, MAX_DOS_PATH);
//DBUG("'%s'\n",EPD->epd_ProgName);

    if( (args = ReadArgs("FILENAME/F",(IPTR *)argarray,NULL)) )
    {
      struct MUI_CustomClass *editor_mcc;
      Object *sliderV, *sliderH,
             *open, *save, *copy, *cut, *paste, *erase, *undo, *redo, *clear, *search,
             *gotolineBtn, *findSwapBtn,
             *saveMenu, *copyMenu, *cutMenu, *eraseMenu, *undoMenu, *redoMenu, *tabsMenu;
      //const char *classes[] = { "TextEditor.mcc" };
      char images_btns[TOT_BTNS][64] = {
                {"\33I[4:TBIMAGES:open]\n"  },
                {"\33I[4:TBIMAGES:save]\n"  },
                {"\33I[4:TBIMAGES:copy]\n"  },
                {"\33I[4:TBIMAGES:cut]\n"   },
                {"\33I[4:TBIMAGES:paste]\n" },
                {"\33I[4:TBIMAGES:delete]\n"},
                {"\33I[4:TBIMAGES:undo]\n"  },
                {"\33I[4:TBIMAGES:redo]\n"  },
                {"\33I[4:TBIMAGES:reset]\n" },
                {"\33I[4:TBIMAGES:find]\n"  }
           };

      //textbuf = AllocVec(MAX_DOS_PATH, 0L);

      if( (STRPTR)argarray[0] ) // from Shell/CLI
      {
//DBUG("  from Shell/CLI '%s'\n",(STRPTR)argarray[0]);
        EPD->epd_FullName = DupStr( (STRPTR)argarray[0], -1);
      }

      if(argc == 0) // from Workbench
      {
        struct WBStartup *wbs = (struct WBStartup *)argv;
        struct WBArg     *arg = &wbs->sm_ArgList[1];
        char              buf[MAX_DOS_PATH];
//DBUG("  from WB '%s'\n",arg->wa_Name);
        if(arg->wa_Name)
        {
          NameFromLock(arg->wa_Lock, buf, MAX_DOS_PATH);
          AddPart(buf, arg->wa_Name, MAX_DOS_PATH);

          EPD->epd_FullName = DupStr(buf, -1);
        }
      }

      EPD->MenuStrip = Create_Menu();
      EPD->MenuRecentItem[0] = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData,MID_PROJECT_RECENTBAR);
      EPD->AslReq = AllocAslRequest(ASL_FileRequest, NULL);

      // Images in toolbar/buttons
      strncat( images_btns[BTN_OPEN],  GetString(&li,MSG_TOOLBAR_OPEN),     64);
      strncat( images_btns[BTN_SAVE],  GetString(&li,MSG_TOOLBAR_SAVE),     64);
      strncat( images_btns[BTN_COPY],  GetString(&li,MSG_TOOLBAR_COPY),     64);
      strncat( images_btns[BTN_CUT],   GetString(&li,MSG_TOOLBAR_CUT),      64);
      strncat( images_btns[BTN_PASTE], GetString(&li,MSG_TOOLBAR_PASTE),    64);
      strncat( images_btns[BTN_ERASE], GetString(&li,MSG_TOOLBAR_ERASE),    64);
      strncat( images_btns[BTN_UNDO],  GetString(&li,MSG_TOOLBAR_UNDO),     64);
      strncat( images_btns[BTN_REDO],  GetString(&li,MSG_TOOLBAR_REDO),     64);
      strncat( images_btns[BTN_CLR],   GetString(&li,MSG_TOOLBAR_CLEARALL), 64);
      strncat( images_btns[BTN_FIND],  GetString(&li,MSG_TOOLBAR_FIND),     64);

      //if( (editor_mcc = MUI_CreateCustomClass(NULL, MUIC_TextEditor, NULL, 0, ENTRY(TextEditor_Dispatcher))) )
      if( (editor_mcc = MUI_CreateCustomClass(NULL, MUIC_TextEditor, NULL, sizeof(struct editpad_data), ENTRY(TextEditor_Dispatcher))) )
      {
        struct DiskObject *dobj = GetDiskObjectNew(EPD->epd_ProgName);
        char   statusLC_len[64];

        //InitHook(&EPD->ASL_startHook, ASL_startHook, EPD);
        //InitHook(&EPD->ASL_stopHook,  ASL_stopHook,  EPD);

        // EPD->StatusLC (max length)
        snprintf(statusLC_len, sizeof(statusLC_len), GetString(&li,MSG_STATUS_LIN_COL),8888,88888);


#include "gui_editpad.h" // "inserting" GUI objects


        if(EPD->App)
        {
          ULONG running = 1;

          Load_File(EPD);

          InitHook(&EPD->AppMsgHook, AppMsgHook, EPD);

          if(EPD->epd_FullName)
          {
            set_WindowTitle(EPD);
            split_FullName(EPD);
          }

          // Some menu options
          saveMenu  = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_PROJECT_SAVE);
          copyMenu  = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_EDIT_COPY);
          cutMenu   = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_EDIT_CUT);
          eraseMenu = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_EDIT_ERASE);
          undoMenu  = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_EDIT_UNDO);
          redoMenu  = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_EDIT_REDO);
          tabsMenu  = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_SETTINGS_TABS);

          DoMethod(EPD->Win, MUIM_Notify, MUIA_Window_CloseRequest,TRUE,
                   EPD->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

          DoMethod(EPD->Win, MUIM_Notify, MUIA_AppMessage,MUIV_EveryTime,
                   EPD->Win, 3, MUIM_CallHook,&EPD->AppMsgHook, MUIV_TriggerValue);

          //DoMethod(window, MUIM_Notify, MUIA_Window_InputEvent, "f1", MUIV_Notify_Application, 2, MUIM_Application_ReturnID, MUIV_RunARexxScript);

          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_Flow, MUIV_EveryTime, flow, 3, MUIM_NoNotifySet, MUIA_Cycle_Active, MUIV_TriggerValue);

          //DoMethod(color, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_Pen, MUIV_TriggerValue);

          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, bold,      3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);
          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, italic,    3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);
          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, underline, 3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);

          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_HasChanged, MUIV_EveryTime, ischanged, 3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);
          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_HasChanged, MUIV_EveryTime, ischanged, 3, MUIM_NoNotifySet, MUIA_Image_State, MUIV_TriggerValue);
          //DoMethod(ischanged, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_HasChanged, MUIV_TriggerValue);

          // Enable/disable SAVE button and menu option
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_HasChanged,MUIV_EveryTime, save, 3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_HasChanged,MUIV_EveryTime, saveMenu, 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          // Enable/disable UNDO/REDO buttons and menu options
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_UndoAvailable,MUIV_EveryTime, undo, 3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_RedoAvailable,MUIV_EveryTime, redo, 3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_UndoAvailable,MUIV_EveryTime, undoMenu, 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_RedoAvailable,MUIV_EveryTime, redoMenu, 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          // Enable/disable COPY/CUT/ERASE buttons and menu options
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime, copy,  3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime, cut,   3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime, erase, 3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime, copyMenu,  3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime, cutMenu,   3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime, eraseMenu, 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);

          DoMethod(open,   MUIM_Notify, MUIA_Pressed,FALSE, EPD->App, 2, MUIM_Application_ReturnID, MID_PROJECT_OPENFILE);
          DoMethod(save,   MUIM_Notify, MUIA_Pressed,FALSE, EPD->App, 2, MUIM_Application_ReturnID, MID_PROJECT_SAVE);
          DoMethod(clear,  MUIM_Notify, MUIA_Pressed,FALSE, EPD->App, 2, MUIM_Application_ReturnID, MID_EDIT_CLEARALL);
          DoMethod(search, MUIM_Notify, MUIA_Pressed,FALSE, EPD->App, 2, MUIM_Application_ReturnID, MID_NAV_FIND);

          DoMethod(copy,  MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "COPY");
          DoMethod(cut,   MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "CUT");
          DoMethod(paste, MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "PASTE");
          DoMethod(erase, MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "ERASE");
          DoMethod(undo,  MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "UNDO");
          DoMethod(redo,  MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "REDO");

          //DoMethod(bold,      MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_StyleBold,      MUIV_TriggerValue);
          //DoMethod(italic,    MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_StyleItalic,    MUIV_TriggerValue);
          //DoMethod(underline, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_StyleUnderline, MUIV_TriggerValue);

          // Some menu options events
          DoMethod(tabsMenu, MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
                   EPD->EditorObj, 3, MUIM_NoNotifySet, MUIA_TextEditor_ConvertTabs, MUIV_TriggerValue);

          // Bind scrollers to editor gadget/object
          set(EPD->EditorObj, MUIA_TextEditor_Slider, sliderV);
          set(EPD->EditorObj, MUIA_TextEditor_HorizontalSlider, sliderH);

          // Status: Line and Column
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_CursorX,MUIV_EveryTime,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_CURPOS);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_CursorY,MUIV_EveryTime,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_CURPOS);
          //DoMethod(gotolineBtn, MUIM_Notify, MUIA_Pressed,FALSE, EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "GOTOLINE 10");
          DoMethod(gotolineBtn, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_GOTOLINE);
          DoMethod(EPD->GotoLine, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_GOTOLINE);

          // Find/Replace window events
          DoMethod(EPD->Find_Cancel, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->FindWin, 3, MUIM_NoNotifySet, MUIA_Window_Open,FALSE);
          DoMethod(EPD->Find_Cancel, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->StatusMsg, 3, MUIM_SetAsString, MUIA_Text_Contents, NULL);
          DoMethod(EPD->Find_Next, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_SEARCHNEXT);
          DoMethod(EPD->Find_Prev, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_SEARCHPREV);
          DoMethod(EPD->Find_Replace, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_REPLACE);
          DoMethod(EPD->Find_ReplaceAll, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_REPLACEALL);
          DoMethod(EPD->Find_FromStart, MUIM_Notify, MUIA_Selected,MUIV_EveryTime,
                   EPD->Find_Prev, 3, MUIM_NoNotifySet, MUIA_Disabled, MUIV_TriggerValue);
          DoMethod(findSwapBtn, MUIM_Notify, MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_FIND_SWAP);

          // Open main window
          SetAttrs(EPD->Win, MUIA_Window_ActiveObject,EPD->EditorObj,
                             MUIA_Window_Open,TRUE, TAG_DONE);

//get(EPD->Win, MUIA_Window_Window,&EPD->MainWin);
//DBUG("win = %p\n",EPD->MainWin);

          do
          {
            running = Process_Events(EPD);
          }
          while(running == 0); // CANCEL

          if(running == 2) // SAVE
          {
            Save_File(EPD);
          }

Remove_RecentMenuEntry(EPD);

          MUI_DisposeObject(EPD->App);
          rc = RETURN_OK;

        }
        else {
          PutStr( GetString(&li,MSG_FAILED_CREATING_APP) );
        }

        FreeDiskObject(dobj);

        MUI_DeleteCustomClass(editor_mcc);
      }
      else {
        Printf("%s "MUIC_TextEditor"\n",GetString(&li,MSG_FAILED_OPEN_RESOURCE));
      }

      FreeString(&EPD->epd_FRDrawer);
      FreeString(&EPD->epd_FRFile);
      FreeString(&EPD->epd_FullName);

      //FreeVec(textbuf);

      FreeAslRequest(EPD->AslReq);

      FreeArgs(args);
    }
    else
    {
      LONG error = IoErr();

      PrintFault(error, EPD->epd_ProgName);
    }

    FreeVec(EPD->epd_ProgName);

    FreeVec(EPD);
  }
  else {
    PutErrStr("Failed to load some resource(s).\n");
  }

  close_Libs();

  return rc;
}



/************************************************************/
/* This part of code is from Andy Broad's dockapps examples */
STRPTR DupStr(STRPTR str, LONG length)
{
  STRPTR dup;
  ULONG str_len = 0;

  if(!str) { return NULL; }

  str_len = strlen(str);
  if(length < 0)
  {
    length = str_len;
  }

  if( (dup=AllocVec(length+1, MEMF_CLEAR)) )
  {
    ULONG copy_len = str_len;

    if(length < str_len)
    {
      copy_len = length;
    }
    memmove(dup, str, copy_len);
    dup[length] = '\0';
  }

  return dup;
}

VOID FreeString(STRPTR *string)
{
  if(string)
  {
    if(*string)
    {
      FreeVec( (APTR)*string );
    }
    *string = NULL;
  }
}
/* This part of code is from Andy Broad's dockapps examples */
/************************************************************/


/*
 * Save file with 'epd->epd_FullName' full path name
 * NOTE: preserves file attributes.
 */
void Save_File(struct editpad_data *epd)
{
  BPTR   fh;
  STRPTR text    = (STRPTR)DoMethod(epd->EditorObj, MUIM_TextEditor_ExportText);
  LONG   attribs = get_FileAttribs(epd);
DBUG("%s() %p 0x%08lx\n",__FUNCTION__,epd->epd_FullName,attribs);

  if( epd->epd_FullName!=NULL  &&  (fh = Open(epd->epd_FullName,MODE_NEWFILE)) )
  {
    Write( fh, text, strlen(text) );
    Close(fh);

    SetProtection(epd->epd_FullName, attribs);//epd->epd_FileAttribs);

    set(epd->EditorObj, MUIA_TextEditor_HasChanged, FALSE);

    set_WindowTitle(epd);
    Add_RecentMenuEntry(epd);
DBUG("%s() OK\n",__FUNCTION__);
  }
  FreeVec(text);
}

/*
 * Load file from 'epd->epd_FullName' full path name
 */
BOOL Load_File(struct editpad_data *epd)
{
  BPTR fh;
  BOOL ret = FALSE;
DBUG("%s() %p\n",__FUNCTION__,epd->epd_FullName);

  if( epd->epd_FullName!=NULL  &&  (fh = Open(epd->epd_FullName, MODE_OLDFILE)) )
  {
    struct FileInfoBlock *fib;

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if(fib)
    {
      STRPTR fileptr;
      LONG   size;

      if( ExamineFH(fh, fib) )
      {
        size = fib->fib_Size;
      }
      FreeDosObject(DOS_FIB, fib);

      fileptr = AllocVec(size+1024, 0L); // +1024 for "safety"
      if(fileptr)
      {
        size = Read(fh, fileptr, size+1024-2);
        fileptr[size] = '\0';

        set(epd->EditorObj, MUIA_TextEditor_Contents, fileptr);
        FreeVec(fileptr);
        set(epd->EditorObj, MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain);

        set(epd->EditorObj, MUIA_TextEditor_HasChanged,FALSE);

        set_WindowTitle(epd);

        DoMethod(epd->StatusLC, MUIM_SetAsString, MUIA_Text_Contents, GetString(&li,MSG_STATUS_LIN_COL),1,1);
DBUG("%s() OK\n",__FUNCTION__);
        ret = TRUE;
      }
    }

    Close(fh);
  }

  return ret;
}


/*
 * ASL file requester
 * Stores path and name in 'epd->epd_FullName' if OK.
 */
LONG FileRequest_ASL(struct editpad_data *epd, CONST_STRPTR title, BOOL save)
{
DBUG("%s()  [DoSave=%ld]\n",__FUNCTION__,save);

	if(epd->AslReq)
	{
		struct Window *win;

		get(epd->Win, MUIA_Window_Window, &win);

		if(epd->epd_FullName)
		{
			split_FullName(epd); // "fills" epd_FRDrawer & epd_FRFile
		}

		if( AslRequestTags(epd->AslReq,
		                   ASLFR_TitleText,     title,
		                   ASLFR_InitialDrawer, epd->epd_FRDrawer,
		                   ASLFR_InitialFile,   epd->epd_FRFile,
		                   //ASLFR_Screen,        epd->epd_Screen,
		                   ASLFR_Window,        win,
		                   ASLFR_SleepWindow,   TRUE,
		                   ASLFR_DoSaveMode,    save,
		                  TAG_DONE) )
		{
			ULONG len;

			if(epd->AslReq->fr_File)
			{
				if(epd->epd_FRDrawer)
				{
					FreeString(&epd->epd_FRDrawer);
				}
				epd->epd_FRDrawer = DupStr(epd->AslReq->fr_Drawer,-1);

				if(epd->epd_FRFile)
				{
					FreeString(&epd->epd_FRFile );
				}
				epd->epd_FRFile = DupStr(epd->AslReq->fr_File,-1);

				len = strlen(epd->AslReq->fr_Drawer) + strlen(epd->AslReq->fr_File) + 2; // term plus possible extra seperator

				FreeString(&epd->epd_FullName);
				epd->epd_FullName = DupStr(epd->AslReq->fr_Drawer, len);
				AddPart(epd->epd_FullName, epd->AslReq->fr_File, len);
DBUG("%s() OK\n",__FUNCTION__);
				return TRUE;
			}
		}

	}

	return FALSE;
}
