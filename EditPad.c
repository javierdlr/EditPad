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

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#endif
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
//#include <proto/utility.h>
#include <proto/graphics.h>
#include <proto/asl.h>
#include <proto/gadtools.h>
#include <proto/icon.h>
#include <proto/muimaster.h>

#include <libraries/mui.h>
#include <libraries/iffparse.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TheBar_mcc.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>


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


//struct Library *UtilityBase;
struct Library *IconBase;
struct Library *MUIMasterBase;
struct Library *GadToolsBase;
struct Library *AslBase;

#ifndef __amigaos4__
struct GfxBase       *GfxBase;
struct IntuitionBase *IntuitionBase;
struct LocaleBase    *LocaleBase;
#else
struct Library *GfxBase;
struct Library *IntuitionBase;
struct Library *LocaleBase;

struct GraphicsIFace  *IGraphics;
struct IntuitionIFace *IIntuition;
//struct UtilityIFace   *IUtility;
struct IconIFace      *IIcon;
struct MUIMasterIFace *IMUIMaster;
struct GadToolsIFace  *IGadTools;
struct AslIFace       *IAsl;
struct LocaleIFace    *ILocale;
#endif


#include "debug.h"

#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
#include "Events.h"
#include "ArexxCmds.h"
#include "Misc.h"


static const TEXT USED verstag[] = VERSTAG;
#if defined(__VBCC__)
size_t __stack=65536;                           // 64KB stack-size VBCC
#endif
#if defined(__GNUC__)
static const char min_stack[] = "$STACK:65536"; // 64KB stack-size GCC
#endif


/*static ULONG mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{
  struct editpad_data *data;

  obj = (Object *)DoSuperMethodA(cl, obj, (Msg)msg);
  if(!obj) { return 0; }

  data = INST_DATA(cl, obj);
  memset( data, 0, sizeof(struct editpad_data) );

  return (ULONG)obj;
}*/


/*static ULONG mDispose(struct IClass *cl, Object *obj, Msg msg)
{
  //struct editpad_data *data = INST_DATA(cl,obj);

  return DoSuperMethodA(cl, obj, msg);
}*/


DISPATCHERPROTO(TextEditor_Dispatcher)
{
  //struct editpad_data *data;

  switch(msg->MethodID)
  {
    /*case OM_NEW:
DBUG("[OM_NEW]\n",NULL);
      return mNew(cl, obj, (APTR)msg);
    break;
    case OM_DISPOSE:
DBUG("[OM_DISPOSE]\n",NULL);
      return mDispose(cl, obj, (APTR)msg);
    break;*/
    case MUIM_Setup:
DBUG("[MUIM_Setup]\n",NULL);
      if(DoSuperMethodA(cl, obj, (APTR)msg)) { return TRUE; }
      else { return FALSE; }
    break;

    case MUIM_Cleanup:
DBUG("[MUIM_Cleanup]\n",NULL);
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
DBUG("[MUIM_DragDrop] 0x%08lx\n",drop_msg->obj);
      //if( GetAttr(MUIA_List_Active,drop_msg->obj,(IPTR *)&active) ) {
      //  DoMethod(editorgad, MUIM_TextEditor_InsertText, StdEntries[active]);
      //}
    }
    break;

    case MUIM_TextEditor_HandleError:
    {
      char                               *errortxt = NULL;
      struct MUIP_TextEditor_HandleError *msgerr;
DBUG("[MUIM_TextEditor_HandleError]\n",NULL);
      //struct editpad_data *data = INST_DATA(cl, obj);
      struct editpad_data *epd = (struct editpad_data *)cl->cl_UserData;
//DBUG("   EPD = 0x%08lx (0x%08lx)\n",epd,cl);
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
//DBUG("  app=0x%08lx  win=0x%08lx\n",data->App,data->Win);
//        MUI_Request(data->App, data->Win, 0L,
        MUI_Request(epd->App, epd->Win, 0L, NULL,
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
    PutStr("Failed to use catalog system. Using built-in strings.\n");
  }*/

  if(GfxBase  &&  IntuitionBase  &&  IconBase  &&  MUIMasterBase 
     &&  GadToolsBase  &&  AslBase)//  &&  RexxSysBase)
  {
    #ifdef __amigaos4__
    IGraphics  = (struct GraphicsIFace *)GetInterface((struct Library *)GfxBase, "main", 1, NULL);
    IIntuition = (struct IntuitionIFace *)GetInterface((struct Library *)IntuitionBase, "main", 1, NULL);
    //IUtility   = (struct UtilityIFace *)GetInterface((struct Library *)UtilityBase, "main", 1, NULL);
    IIcon      = (struct IconIFace *)GetInterface(IconBase, "main", 1, NULL);
    IMUIMaster = (struct MUIMasterIFace *)GetInterface(MUIMasterBase, "main", 1, NULL);
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
  if(IGadTools)  { DropInterface((struct Interface *)IGadTools); }
  if(ILocale)    { DropInterface((struct Interface *)ILocale); }
  if(IMUIMaster) { DropInterface((struct Interface *)IMUIMaster); }
  //if(IUtility)   { DropInterface((struct Interface *)IUtility); }
  if(IIcon)      { DropInterface((struct Interface *)IIcon); }
  if(IIntuition) { DropInterface((struct Interface *)IIntuition); }
  if(IGraphics)  { DropInterface((struct Interface *)IGraphics); }
  if(IAsl)       { DropInterface((struct Interface *)IAsl); }
  #endif

  if(AslBase)       { CloseLibrary(AslBase); }
  if(GadToolsBase)  { CloseLibrary(GadToolsBase); }
  if(MUIMasterBase) { CloseLibrary(MUIMasterBase); }
  if(LocaleBase)    { CloseLibrary((struct Library*)LocaleBase); }
  //if(UtilityBase)   { CloseLibrary(UtilityBase); }
  if(IconBase)      { CloseLibrary(IconBase); }
  if(IntuitionBase) { CloseLibrary((struct Library *)IntuitionBase); }
  if(GfxBase)       { CloseLibrary((struct Library *)GfxBase); }
}


static Object *create_Toolbar(void)
{
  // Images for toolbar/buttons used in buttons[]
  const char *images_btns[] = {
                    "open", "save", // 0 1
                    "cut", "copy", "paste", "delete", // 2 3 4 5
                    "undo", "redo", // 6 7
                    "reset", // 8
                    "find",  // 9
              NULL };

  struct MUIS_TheBar_Button buttons[] = {
             {0, BTN_OPEN, GetString(&li,MSG_TOOLBAR_OPEN),
                 NULL, 0, 0, NULL, NULL},
             {1, BTN_SAVE, GetString(&li,MSG_TOOLBAR_SAVE),
                 NULL, 0, 0, NULL, NULL},

             {MUIV_TheBar_ButtonSpacer, (ULONG)~0, NULL, NULL, 0, 0, NULL, NULL},

             {2, BTN_CUT, GetString(&li,MSG_TOOLBAR_CUT),
                 NULL, 0, 0, NULL, NULL},
             {3, BTN_COPY, GetString(&li,MSG_TOOLBAR_COPY),
                 NULL, 0, 0, NULL, NULL},
             {4, BTN_PASTE, GetString(&li,MSG_TOOLBAR_PASTE),
                 NULL, 0, 0, NULL, NULL},
             {5, BTN_ERASE, GetString(&li,MSG_TOOLBAR_ERASE),
                 NULL, 0, 0, NULL, NULL},

             {MUIV_TheBar_ButtonSpacer, (ULONG)~0, NULL, NULL, 0, 0, NULL, NULL},

             {6, BTN_UNDO, GetString(&li,MSG_TOOLBAR_UNDO),
                 NULL, 0, 0, NULL, NULL},
             {7, BTN_REDO, GetString(&li,MSG_TOOLBAR_REDO),
                 NULL, 0, 0, NULL, NULL},

             {MUIV_TheBar_ButtonSpacer, (ULONG)~0, NULL, NULL, 0, 0, NULL, NULL},

             {8, BTN_CLR, GetString(&li,MSG_TOOLBAR_CLEARALL),
                 NULL, 0, 0, NULL, NULL},

             {MUIV_TheBar_ButtonSpacer, (ULONG)~0, NULL, NULL, 0, 0, NULL, NULL},

             {9, BTN_FIND, GetString(&li,MSG_TOOLBAR_FIND),
                 NULL, 0, 0, NULL, NULL},

             {MUIV_TheBar_End, (ULONG)~0, NULL, NULL, 0, 0, NULL, NULL},
         };

  return TheBarObject,
            MUIA_Group_Horiz, TRUE,
            //MUIA_TheBar_Frame,      TRUE,
            //MUIA_TheBar_ViewMode,   MUIV_TheBar_ViewMode_TextGfx,
            MUIA_TheBar_Buttons,    buttons,
            MUIA_TheBar_PicsDrawer, "TBIMAGES:",
            MUIA_TheBar_Pics,       images_btns,
            //MUIA_TheBar_DragBar,    TRUE,
         End;
}


HOOKPROTONO(AppMsgFunc, LONG, struct AppMessage **x)
{
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



/*
 * MAIN
 */
int main(int argc, char **argv)
{
  int rc = RETURN_FAIL;
DBUG("*****"PROGNAME" START*****\n",NULL);
  if( open_Libs() )
  {
    struct RDArgs       *args;
    IPTR                 argarray[3] = { 0, 0, 0 };
    struct editpad_data *EPD = AllocVec(sizeof(struct editpad_data), MEMF_CLEAR);

    EPD->epd_ProgName = AllocVec(MAX_DOS_PATH, 0L);
    GetCurrentDirName(EPD->epd_ProgName, MAX_DOS_PATH);
    AddPart(EPD->epd_ProgName, PROGNAME, MAX_DOS_PATH);
//DBUG("'%s'\n",EPD->epd_ProgName);
    EPD->epd_PortName = DupStr(PROGNAME, -1);

    if( (args=ReadArgs("FILENAME,PORTNAME/K,PORTUNIQUE=UNIQUE/S",(IPTR *)argarray,NULL)) )
    {
      struct MUI_CustomClass *editor_mcc;
      Object *sliderV, *sliderH,
             *tbar,
             *gotolineBtn, *findSwapBtn;
      //const char *classes[] = { "TextEditor.mcc", "TheBar.mcc" };

      //textbuf = AllocVec(MAX_DOS_PATH, 0L);

      // From Shell/CLI
      if( (STRPTR)argarray[0] ) // FILENAME
      {
        EPD->epd_FullName = DupStr( (STRPTR)argarray[0], -1);
      }
//DBUG("  from Shell/CLI: FILENAME='%s'\n",(STRPTR)argarray[0]);
      if( (STRPTR)argarray[1] ) // PORTNAME
      {
        FreeString(&EPD->epd_PortName);
        EPD->epd_PortName = DupStr( (STRPTR)argarray[1], -1);

        if( (STRPTR)argarray[2] ) // PORTUNIQUE=UNIQUE
        {
          EPD->epd_PortUnique = TRUE;
        }
      }
//DBUG("  from Shell/CLI PORTNAME='%s' (UNIQUE=%ld)\n",(STRPTR)argarray[1],EPD->epd_PortUnique);

      // From Workbench
      if(argc == 0)
      {
        struct WBStartup *wbs = (struct WBStartup *)argv;
DBUG("  from WB: %ld\n",wbs->sm_NumArgs);
        if(wbs->sm_NumArgs != 1)
        {
          struct WBArg *arg = &wbs->sm_ArgList[1];
          char          buf[MAX_DOS_PATH];
DBUG("           0x%08lx '%s'\n",arg->wa_Name,arg->wa_Name);
          NameFromLock(arg->wa_Lock, buf, MAX_DOS_PATH);
          AddPart(buf, arg->wa_Name, MAX_DOS_PATH);

          EPD->epd_FullName = DupStr(buf, -1);
        }
        Read_ToolTypes(EPD);
      }

      //if( (editor_mcc = MUI_CreateCustomClass(NULL, MUIC_TextEditor, NULL, sizeof(struct editpad_data), ENTRY(TextEditor_Dispatcher))) )
      if( (editor_mcc=MUI_CreateCustomClass(NULL, MUIC_TextEditor,NULL,0,ENTRY(TextEditor_Dispatcher))) )
      {
        struct DiskObject *dobj = GetDiskObjectNew(EPD->epd_ProgName);
        char               statusLC_len[64];

        //editor_mcc->mcc_UserData = (APTR)EPD;
        editor_mcc->mcc_Class->cl_UserData = (ULONG)EPD; // used in DISPATCHERPROTO()'s MUIM_TextEditor_HandleError
//DBUG("   EPD = 0x%08lx (0x%08lx)\n",EPD,editor_mcc->mcc_Class);

        // Init some GUI stuff
        EPD->MenuStrip = Create_Menu();
        EPD->MenuRecentItem[0] = (APTR)DoMethod(EPD->MenuStrip, MUIM_FindUData, MID_PROJECT_RECENTBAR);
        EPD->AslReq = AllocAslRequest(ASL_FileRequest, NULL);
        EPD->EditPopup = Create_PopUpMenu();

        Init_ArexxCmds(EPD); // see ArexxCmds.h

        // Calculate 'EPD->StatusLC' string max length
        snprintf(statusLC_len, sizeof(statusLC_len), GetString(&li,MSG_STATUS_LIN_COL),8888,88888);


#include "gui_editpad.h" // "inserting" GUI an its objects


        if(EPD->App)
        {
          ULONG running = 1;

          Load_File(EPD);

          InitHook(&EPD->AppMsgHook, AppMsgHook, EPD);

          if(EPD->epd_FullName)
          {
            Set_WindowTitle(EPD);
            Split_FullName(EPD);
          }

          Load_RecentsFile(EPD);

          DoMethod(EPD->Win, MUIM_Notify, MUIA_Window_CloseRequest,TRUE,
                   EPD->App, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

          DoMethod(EPD->Win, MUIM_Notify, MUIA_AppMessage,MUIV_EveryTime,
                   EPD->Win, 3, MUIM_CallHook,&EPD->AppMsgHook, MUIV_TriggerValue);

          //DoMethod(color, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_Pen, MUIV_TriggerValue);

          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_StyleBold,      MUIV_EveryTime, bold,      3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);
          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_StyleItalic,    MUIV_EveryTime, italic,    3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);
          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_StyleUnderline, MUIV_EveryTime, underline, 3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);

          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_HasChanged, MUIV_EveryTime, ischanged, 3, MUIM_NoNotifySet, MUIA_Selected, MUIV_TriggerValue);
          //DoMethod(editorgad, MUIM_Notify, MUIA_TextEditor_HasChanged, MUIV_EveryTime, ischanged, 3, MUIM_NoNotifySet, MUIA_Image_State, MUIV_TriggerValue);
          //DoMethod(ischanged, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_HasChanged, MUIV_TriggerValue);

          // Enable/disable SAVE button and menu option
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_HasChanged,MUIV_EveryTime,
                   tbar, 4, MUIM_TheBar_NoNotifySetAttr, BTN_SAVE, MUIV_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_HasChanged,MUIV_EveryTime,
                   findobj(MID_PROJECT_SAVE,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          // Enable/disable UNDO/REDO buttons and menu options
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_UndoAvailable,MUIV_EveryTime,
                   tbar, 4, MUIM_TheBar_NoNotifySetAttr, BTN_UNDO, MUIV_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_RedoAvailable,MUIV_EveryTime,
                   tbar, 4, MUIM_TheBar_NoNotifySetAttr, BTN_REDO, MUIV_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_UndoAvailable,MUIV_EveryTime,
                   findobj(MID_EDIT_UNDO,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_RedoAvailable,MUIV_EveryTime,
                   findobj(MID_EDIT_REDO,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          // Enable/disable COPY/CUT/ERASE buttons and menu options
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   tbar, 4, MUIM_TheBar_NoNotifySetAttr, BTN_CUT,   MUIV_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   tbar, 4, MUIM_TheBar_NoNotifySetAttr, BTN_COPY,  MUIV_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   tbar, 4, MUIM_TheBar_NoNotifySetAttr, BTN_ERASE, MUIV_TheBar_Attr_Disabled, MUIV_NotTriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(MID_EDIT_CUT,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(MID_EDIT_COPY,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(MID_EDIT_ERASE,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          // Enable/disable TOUPPER/TOLOWER menu options
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(MID_EDIT_TOUPPER,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(MID_EDIT_TOLOWER,EPD->MenuStrip), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);

          // Toolbar
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_OPEN,  MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, MID_PROJECT_OPENFILE);
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_SAVE,  MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, MID_PROJECT_SAVE);
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_CLR,   MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, MID_EDIT_CLEARALL);
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_FIND,  MUIA_Pressed,FALSE,
                   EPD->App, 2, MUIM_Application_ReturnID, MID_NAV_FIND);
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_CUT,   MUIA_Pressed,FALSE,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "CUT");
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_COPY,  MUIA_Pressed,FALSE,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "COPY");
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_PASTE, MUIA_Pressed,FALSE,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "PASTE");
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_ERASE, MUIA_Pressed,FALSE,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "ERASE");
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_UNDO,  MUIA_Pressed,FALSE,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "UNDO");
          DoMethod(tbar, MUIM_TheBar_Notify, BTN_REDO,  MUIA_Pressed,FALSE,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "REDO");

          //DoMethod(bold,      MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_StyleBold,      MUIV_TriggerValue);
          //DoMethod(italic,    MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_StyleItalic,    MUIV_TriggerValue);
          //DoMethod(underline, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, editorgad, 3, MUIM_NoNotifySet, MUIA_TextEditor_StyleUnderline, MUIV_TriggerValue);

          // Some menu options events
          DoMethod(findobj(MID_SETTINGS_TABS,EPD->MenuStrip), MUIM_Notify, MUIA_Menuitem_Checked, MUIV_EveryTime,
                   EPD->EditorObj, 3, MUIM_NoNotifySet, MUIA_TextEditor_ConvertTabs, MUIV_TriggerValue);

          // PopUp menu options
          DoMethod(findobj(PID_CUT,EPD->EditPopup), MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "CUT");
          DoMethod(findobj(PID_COPY,EPD->EditPopup), MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "COPY");
          DoMethod(findobj(PID_PASTE,EPD->EditPopup), MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "PASTE");
          DoMethod(findobj(PID_ERASE,EPD->EditPopup), MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime,
                   EPD->EditorObj, 2, MUIM_TextEditor_ARexxCmd, "ERASE");
          DoMethod(findobj(PID_MARKALL,EPD->EditPopup), MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime,
                   EPD->App, 2, MUIM_Application_ReturnID, MID_EDIT_MARKALL);
          // Enable/disable PopUp menu options
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(PID_CUT,EPD->EditPopup), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(PID_COPY,EPD->EditPopup), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_AreaMarked,MUIV_EveryTime,
                   findobj(PID_ERASE,EPD->EditPopup), 3, MUIM_NoNotifySet, MUIA_Menuitem_Enabled, MUIV_TriggerValue);

          // Bind scrollers to editor gadget/object
          set(EPD->EditorObj, MUIA_TextEditor_Slider, sliderV);
          set(EPD->EditorObj, MUIA_TextEditor_HorizontalSlider, sliderH);

          // Status: Line and Column
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_CursorX,MUIV_EveryTime,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_CURPOS);
          DoMethod(EPD->EditorObj, MUIM_Notify, MUIA_TextEditor_CursorY,MUIV_EveryTime,
                   EPD->App, 2, MUIM_Application_ReturnID, GID_CURPOS);
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

          do
          {
            running = Process_Events(EPD);
          }
          while(running == 0); // CANCEL

          if(running == 2) // SAVE
          {
            Save_File(EPD);
          }

          SaveRecentList_FreeEntries(EPD);

          MUI_DisposeObject(EPD->EditPopup);

          MUI_DisposeObject(EPD->App);
          rc = RETURN_OK;

        }
        else {
          //PutStr( GetString(&li,MSG_FAILED_CREATING_APP) );
          MUI_Request(NULL, NULL, 0L,
                      PROGNAME,
                      (STRPTR)GetString(&li,MSG_CONTINUE_BUTTON_PLAIN),
                      (STRPTR)GetString(&li,MSG_FAILED_CREATING_APP));
        }

        FreeDiskObject(dobj);

        FreeAslRequest(EPD->AslReq);

        MUI_DeleteCustomClass(editor_mcc);
      }
      else {
        //Printf("%s '"MUIC_TextEditor"'.\n",GetString(&li,MSG_FAILED_OPEN_RESOURCE));
        MUI_Request(NULL, NULL, 0L,
                    PROGNAME,
                    (STRPTR)GetString(&li,MSG_CONTINUE_BUTTON_PLAIN),
                    (STRPTR)GetString(&li,MSG_FAILED_OPEN_RESOURCE2),MUIC_TextEditor);

      }

      FreeString(&EPD->epd_FRDrawer);
      FreeString(&EPD->epd_FRFile);
      FreeString(&EPD->epd_FullName);
      FreeString(&EPD->epd_PortName);

      //FreeVec(textbuf);

      FreeArgs(args);
    }
    else {
      LONG error = IoErr();

      PrintFault(error, EPD->epd_ProgName);
    }

    FreeVec(EPD->epd_ProgName);

    FreeVec(EPD);
  }
  else {
    PutStr("Failed to load some resource(s).\n");
  }

  close_Libs();
DBUG("*****"PROGNAME" END (RC=%ld)*****\n",rc);
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

  if( (dup=AllocVec(length+1,MEMF_CLEAR)) )
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
