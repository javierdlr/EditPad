#include <stdio.h>
#include <string.h>

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/muimaster_protos.h>
//#include <clib/rexxsyslib_protos.h>
#else
#include <proto/exec.h>
#include <proto/muimaster.h>
#endif

#include <libraries/mui.h>
#include <mui/TextEditor_mcc.h>


//#include <proto/locale.h>
#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
#include "editpad_catalog.h"
extern struct LocaleInfo li;
//extern STRPTR textbuf; // for localized messages


#include "debug.h"

#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
#include "Events.h"
//#include "ArexxCmds.h"
#include "Misc.h"


//static ULONG sX, sY; // PID_MARK_START + PID_MARK_END


/*
 * "Queries" if file has changed and shows requester
 */
static ULONG file_HasChanged(struct editpad_data *epd)
{
  ULONG changed,
        ret = 1;

  get(epd->EditorObj, MUIA_TextEditor_HasChanged, &changed);
DBUG("%s() %ld '%s'\n",__FUNCTION__,changed,epd->epd_FullName);
  if(changed)
  {
    if(epd->epd_FullName == NULL)
    {
      //FreeString(&epd->epd_FullName);
      //strncpy(epd->epd_Filename, "RAM:Unnamed", MAX_DOS_PATH);
      epd->epd_FullName = DupStr((STRPTR)GetString(&li,MSG_FILE_UNNAMED), -1);
    }

    ret = MUI_Request(epd->App, epd->Win, 0L,
                      (STRPTR)GetString(&li,MSG_REQUESTER_TITLE_WARN),
                      (STRPTR)GetString(&li,MSG_REQUESTER_QUIT_BUTTONS),
                      (STRPTR)GetString(&li,MSG_REQUESTER_QUIT_TEXT),epd->epd_FullName);

    //if(ret != 0) { set(epd->EditorObj, MUIA_TextEditor_HasChanged,FALSE); }
  }

  return ret;
}


/*
 * Open Find/Replace window [and enable/disable gadgets]
 */
static void open_FindReplaceWin(struct editpad_data *epd, BOOL ReplaceMode)
{
  if(epd->FindWin)
  {
    ULONG is_open;

    get(epd->FindWin, MUIA_Window_Open, &is_open);
//DBUG("openFindReplaceWin() win_is_open=%ld (ReplaceMode=%ld)\n",is_open,ReplaceMode);
    if(is_open)
    {// Already open -> activate and to front
      set(epd->FindWin, MUIA_Window_Activate, TRUE);
      DoMethod(epd->FindWin, MUIM_Window_ToFront);
      return;
    }

    /*set(epd->FindWin, MUIA_Window_Title, ReplaceMode? GetString(&li,MSG_REPLACE_WINDOW_TITLE):GetString(&li,MSG_FIND_WINDOW_TITLE));
    set(epd->ReplaceLbl, MUIA_Disabled, !ReplaceMode); //MUIA_ShowMe
    set(epd->ReplaceObj, MUIA_Disabled, !ReplaceMode);
    set(epd->Find_Replace,    MUIA_Disabled, !ReplaceMode);
    set(epd->Find_ReplaceAll, MUIA_Disabled, !ReplaceMode);*/
    //DoMethod(epd->FindWin, MUIM_MultiSet, MUIA_Disabled,!ReplaceMode, epd->ReplaceLbl,epd->ReplaceObj,epd->Find_Replace,epd->Find_ReplaceAll,NULL);

    SetAttrs(epd->FindWin, MUIA_Window_ActiveObject,epd->FindObj, MUIA_Window_Open,TRUE, TAG_DONE);
    //SetAttrs(epd->FindWin, MUIA_Window_RefWindow,epd->Win, MUIA_Window_Open,TRUE, TAG_DONE);
    //set(epd->FindWin, MUIA_Window_Open,TRUE);
  }
}

/*
 * Search string
 * Returns TRUE if the string was found, otherwise FALSE.
 */
static BOOL search_String(struct editpad_data *epd, ULONG flags)
{
  STRPTR search_str = NULL;
  ULONG  res;
DBUG("%s() flags=%ld\n",__FUNCTION__,flags);

  set(epd->StatusMsg, MUIA_Text_Contents, NULL);

  get(epd->FindObj, MUIA_Text_Contents, &search_str);
//DBUG("  searching '%s'...\n",search_str);
  get(epd->Find_Case, MUIA_Selected, &res);
  flags |= res? MUIF_TextEditor_Search_CaseSensitive: 0;
  get(epd->Find_FromStart, MUIA_Selected, &res);
  flags |= res? MUIF_TextEditor_Search_FromTop: 0;

  set(epd->Find_FromStart, MUIA_Selected, FALSE); // reset 'From start' checkbox

  return DoMethod(epd->EditorObj, MUIM_TextEditor_Search, search_str, flags);
}


/*
 * Open file, but 1st we check if file has changed
 */
static void open_File(struct editpad_data *epd)
{
DBUG("%s()\n",__FUNCTION__);
  ULONG res = file_HasChanged(epd);
  // ASL load requester
  //DoMethod(epd->LoadObj, MUIM_Popstring_Open);

  if(res == 0) { return; } // CANCEL

  if(res == 2) // SAVE
  {
    if( FileRequest_ASL(epd,GetString(&li,MSG_ASL_SAVE_TITLE),TRUE) )
    {
      Save_File(epd);
    }
    return;
  }

  if( FileRequest_ASL(epd,GetString(&li,MSG_ASL_LOAD_TITLE),FALSE) )
  {
    Load_File(epd);
  }
}


/*
 * Open recent entry; similar as in load_File()
 */
static void open_RecentFile(struct editpad_data *epd, ULONG id)
{
DBUG("%s()\n",__FUNCTION__);
  STRPTR recent;
  ULONG  val = file_HasChanged(epd);

  if(val == 0) { return; } // CANCEL

  if(val == 2) // SAVE
  {
    if( FileRequest_ASL(epd,GetString(&li,MSG_ASL_SAVE_TITLE),TRUE) )
    {
      Save_File(epd);
     }
     return;
   }

   // Get full filename stored as STRPTR in MUIA_UserData
   val = id - MID_PROJECT_RECENT1 + 1; // "get" MID_PROJECT_RECENTx (1,2,3,..)
   get(epd->MenuRecentItem[val], MUIA_UserData, &recent);
DBUG("  '%s'\n",recent);
   FreeString(&epd->epd_FullName);
   epd->epd_FullName = DupStr(recent, -1);
   Load_File(epd);
}


/*
 * Process GUI/Menu/.. events
 */
ULONG Process_Events(struct editpad_data *epd)
{
  //ULONG changed = 0;
  //ULONG ReturnID;
  BOOL  done = FALSE;
  ULONG sigs;

  //while((LONG)(ReturnID = DoMethod(app, MUIM_Application_NewInput, &sigs)) != (LONG)MUIV_Application_ReturnID_Quit)
  while(!done)
  {
    ULONG result = DoMethod(epd->App, MUIM_Application_NewInput, &sigs);
    switch(result)
    {
      case MUIV_Application_ReturnID_Quit:
      case MID_PROJECT_QUIT:
        done = TRUE;
      break;
      case MID_PROJECT_ICONIFY:
        set(epd->App, MUIA_Application_Iconified,TRUE);
      break;
      case MID_PROJECT_ABOUT:
      {
        STRPTR portname;
        get(epd->App, MUIA_Application_Base, &portname);
        MUI_Request(epd->App, epd->Win, 0, NULL,
                    (STRPTR)GetString(&li,MSG_OK_BUTTON_PLAIN),
                    "\033c\033b"VERS" ("DATE")\033n\n\nCopyright (C) 2026 AmigaKit Ltd\n\n"
                    "\033u%s\033n\nJavier de las Rivas\n"
                    "\033u%s\033n\nTextEditor.mcc Open Source Team\n"
                    "\033u%s\033n\nAndy Broad\n\n"
                    "%s %s",
                     GetString(&li,MSG_ABOUT_WRITTENBY),
                     GetString(&li,MSG_ABOUT_SOURCECODE),
                     GetString(&li,MSG_ABOUT_CONTRIB),
                     GetString(&li,MSG_ABOUT_AREXXPORT), portname);
      }
      break;
      case MID_PROJECT_MUI_SETTINGS:
        DoMethod(epd->App, MUIM_Application_OpenConfigWindow, 0);
      break;
      case MID_PROJECT_OPENFILE:
DBUG("  [MID_PROJECT_OPENFILE]\n",NULL);
        open_File(epd);
      /*{
DBUG("  [MID_PROJECT_OPENFILE]\n",NULL);
        ULONG res = file_HasChanged(epd);
        // ASL load requester
        //DoMethod(epd->LoadObj, MUIM_Popstring_Open);

        if(res == 0) { break; } // CANCEL

        if(res == 2) // SAVE
        {
          if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_SAVE_TITLE), TRUE) )
          {
           Save_File(epd);
          }
          break;
        }

        if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_LOAD_TITLE), FALSE) )
        {
         Load_File(epd);
        }
      }*/
      break;
      case MID_PROJECT_RECENTCLEAR:
      {
        STRPTR itemname;
DBUG("  [MID_PROJECT_RECENTCLEAR]\n",NULL);
        while(epd->epd_RecentIdx)
        {
          get(epd->MenuRecentItem[epd->epd_RecentIdx], MUIA_UserData, &itemname);
DBUG("  %p [%ld]'%s'\n",epd->MenuRecentItem[epd->epd_RecentIdx],epd->epd_RecentIdx,itemname);
          FreeString(&itemname);
          DoMethod(epd->MenuStrip, MUIM_Family_Remove, epd->MenuRecentItem[epd->epd_RecentIdx]);
DBUG("             REMOVED from menu\n",NULL);
          --epd->epd_RecentIdx;
        }
        set(findobj(MID_PROJECT_RECENTCLEAR,epd->MenuStrip), MUIA_Menuitem_Enabled, FALSE);
      }
      break;
      case MID_PROJECT_RECENT1:
      case MID_PROJECT_RECENT2:
      case MID_PROJECT_RECENT3:
      case MID_PROJECT_RECENT4:
      case MID_PROJECT_RECENT5:
DBUG("  [MID_PROJECT_RECENT]\n",NULL);
        open_RecentFile(epd, result);
      break;
      case MID_PROJECT_SAVE:
        if(epd->epd_FullName == NULL)
        {
          FreeString(&epd->epd_FRFile); // "clear" ASL file string/name
          if( !FileRequest_ASL(epd,GetString(&li,MSG_ASL_SAVE_TITLE),TRUE) )
          {
            break;
          }
        }
        Save_File(epd);
      break;
      case MID_PROJECT_SAVEAS:
        // ASL save requester
        //set(load, ASLFR_DoSaveMode, TRUE);
        //DoMethod(epd->LoadObj, MUIM_Popstring_Open);

        if( FileRequest_ASL(epd,GetString(&li,MSG_ASL_SAVE_TITLE),TRUE) )
        {
         Save_File(epd);
        }
      break;
      case MID_EDIT_CUT:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "CUT");
      break;
      case MID_EDIT_COPY:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "COPY");
      break;
      case MID_EDIT_PASTE:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "PASTE");
      break;
      case MID_EDIT_ERASE:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "ERASE");
      break;
      case MID_EDIT_MARKALL:
        /*DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "POSITION SOF");
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "MARK ON");
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "POSITION EOF");*/
        DoMethod(epd->EditorObj, MUIM_TextEditor_MarkText, 0, 0, MUIV_TextEditor_MarkText_All,MUIV_TextEditor_MarkText_All);
      break;
      case MID_EDIT_UNDO:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "UNDO");
      break;
      case MID_EDIT_REDO:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "REDO");
      break;
      case MID_EDIT_CLEARALL:
        if(file_HasChanged(epd) == 2)
        {
          Save_File(epd);
        }
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "CLEAR");
        DoMethod(epd->EditorObj, MUIM_NoNotifySet, MUIA_TextEditor_HasChanged, FALSE);
        // Reset window title and filename
        set(epd->Win, MUIA_Window_Title, PROGNAME);
        FreeString(&epd->epd_FullName);
      break;
      /*case MID_SETTINGS_TABS:
        get(epd->MenuStrip, )
        //set(epd->EditorObj, MUIA_TextEditor_ConvertTabs,);
      break;*/
      case MID_NAV_FIND: // aka Find&Replace
        open_FindReplaceWin(epd, FALSE); // 2nd arg does nothing
      break;
      /*case MID_NAV_REPLACE:
        open_FindReplaceWin(epd, TRUE);
      break;*/
      case MID_NAV_GOTOLINE:
        set(epd->Win, MUIA_Window_ActiveObject, epd->GotoLine);
      break;
      case MID_EDIT_TOUPPER:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "TOUPPER");
      break;
      case MID_EDIT_TOLOWER:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "TOLOWER");
      break;
//POPUP-MENU
/*case PID_MARK_START:
{
DBUG("  [PID_MARK_START]\n",NULL);
//static ULONG sX, sY;
GetAttr(MUIA_TextEditor_CursorX, epd->EditorObj, &sX);
GetAttr(MUIA_TextEditor_CursorY, epd->EditorObj, &sY);
DBUG("Init block at %lu x %lu\n",sX,sY);
}
break;
case PID_MARK_END:
{
DBUG("  [PID_MARK_END]\n",NULL);
ULONG eX, eY;
GetAttr(MUIA_TextEditor_CursorX, epd->EditorObj, &eX);
GetAttr(MUIA_TextEditor_CursorY, epd->EditorObj, &eY);
DBUG("End block at %lu x %lu\n",eX,eY);
DoMethod(epd->EditorObj, MUIM_TextEditor_MarkText, sX,sY, eX,eY);
}
break;*/
//MAIN window gadgets
      case GID_CURPOS:
      {
        ULONG lin, col;

        get(epd->EditorObj, MUIA_TextEditor_CursorY, &lin);
        get(epd->EditorObj, MUIA_TextEditor_CursorX, &col);
        DoMethod(epd->StatusLC, MUIM_SetAsString, MUIA_Text_Contents, GetString(&li,MSG_STATUS_LIN_COL),lin+1,col+1);
      }
      break;
      case GID_GOTOLINE:
      {
        ULONG val;
        char buf[32];

        get(epd->GotoLine, MUIA_String_Integer, &val);
        if(val == 0) { val = 1; }
        snprintf(buf, sizeof(buf), "GOTOLINE %lu",val-1); // TextEditor's 1st line is 0

        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, buf);
        set(epd->Win, MUIA_Window_ActiveObject, epd->EditorObj);
      }
      break;
// FIND window gadgets
      case GID_FIND_SWAP:
      {
        STRPTR str, tmp;

        get(epd->FindObj,    MUIA_Text_Contents, &str);
        tmp = DupStr(str, -1);
        get(epd->ReplaceObj, MUIA_Text_Contents, &str);
        set(epd->FindObj,    MUIA_Text_Contents, str);
        set(epd->ReplaceObj, MUIA_Text_Contents, tmp);
        FreeString(&tmp);
      }
      break;
      case GID_SEARCHNEXT:
        if(search_String(epd, 0) == FALSE)
        {
          set(epd->StatusMsg, MUIA_Text_Contents, GetString(&li,MSG_FIND_NOMORE_ENTRIES));
        }
      break;
      case GID_SEARCHPREV:
        if(search_String(epd,MUIF_TextEditor_Search_Backwards) == FALSE)
        {
          set(epd->StatusMsg, MUIA_Text_Contents, GetString(&li,MSG_FIND_NOMORE_ENTRIES));
        }
      break;
      case GID_REPLACE:
      {
        ULONG  is_marked;
        STRPTR str;

        get(epd->EditorObj, MUIA_TextEditor_AreaMarked, &is_marked);
DBUG("  [GID_REPLACE] %ld\n",is_marked);
        if(is_marked==FALSE  &&  search_String(epd,0)==FALSE)
        {
          get(epd->FindObj, MUIA_Text_Contents, &str);
          DoMethod(epd->StatusMsg, MUIM_SetAsString, MUIA_Text_Contents, GetString(&li,MSG_FIND_REPLACE_NONE),str? str:"");
          break;
        }

        get(epd->ReplaceObj, MUIA_Text_Contents, &str);
        DoMethod(epd->EditorObj, MUIM_TextEditor_Replace, str, 0);
      }
      break;
      case GID_REPLACEALL:
      {
        STRPTR str;
        ULONG  counter = 0;
DBUG("  [GID_REPLACEALL]\n",NULL);
        get(epd->ReplaceObj, MUIA_Text_Contents, &str);

        while( search_String(epd,0) )
        {
          DoMethod(epd->EditorObj, MUIM_TextEditor_Replace, str, 0);
          ++counter;
        }
        DoMethod(epd->StatusMsg, MUIM_SetAsString, MUIA_Text_Contents, GetString(&li,MSG_FIND_REPLACE_ALL),str? str:"",counter);
      }
      break;

      default:
        if(sigs)
        {
          sigs = Wait(sigs | SIGBREAKF_CTRL_C);

          if(sigs & SIGBREAKF_CTRL_C)
          {
           //running = 1; // DISCARD if changed
           done = TRUE;
           break;
          }
        }
      break;
    } // END of switch()

  } // END of while()

  return file_HasChanged(epd);
}
