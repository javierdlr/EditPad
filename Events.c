#include <stdio.h>
//#include <string.h>

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/muimaster_protos.h>
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


#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
#include "Events.h"
#include "debug.h"


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
 * Process GUI/Menu/.. events
 */
ULONG Process_Events(struct editpad_data *epd)
{
  //struct MsgPort *myport = NULL;
  //struct RexxMsg *rxmsg = NULL;
  //ULONG changed = 0;
  //ULONG ReturnID;
  //BPTR  rxstdout = 0L;
  BOOL  done = FALSE;
  ULONG sigs;

  //while((LONG)(ReturnID = DoMethod(app, MUIM_Application_NewInput, &sigs)) != (LONG)MUIV_Application_ReturnID_Quit)
  while(!done)
  {
    ULONG code = DoMethod(epd->App, MUIM_Application_NewInput, &sigs);
    switch(code)
    {
      case MUIV_Application_ReturnID_Quit:
      case MID_PROJECT_QUIT:
        done = TRUE;
      break;
      //case MUIV_RunARexxScript:
                  /*if(ReturnID == MUIV_RunARexxScript && !myport)
                  {
                    struct MsgPort *rexxport;
                    const char *script = "REXX:TextEditor/Demo.Rexx";
DBUG("%p\n",myport);
                    if((rexxport = FindPort((STRPTR)"REXX")) && (myport = CreateMsgPort()))
                    {
                      if(!rxstdout)
                        rxstdout = Open("CON://640/100/TextEditor-Demo, ARexx output:/Close/Wait/Auto/InActive", MODE_READWRITE);

                      rxmsg = CreateRexxMsg(myport, NULL, "TEXTEDITOR-DEMO.1");
                      rxmsg->rm_Action = RXCOMM;
                      rxmsg->rm_Stdin  = rxstdout;
                      rxmsg->rm_Stdout = rxstdout;
#ifdef __VBCC__
                      rxmsg->rm_Args[0] = CreateArgstring((STRPTR)script, strlen(script));
#else
                      rxmsg->rm_Args[0] = (IPTR)CreateArgstring((STRPTR)script, strlen(script));
#endif
                      PutMsg(rexxport, (struct Message *)rxmsg);
                    }
                  }*/
      //break;
      case MID_PROJECT_ICONIFY:
        set(epd->App, MUIA_Application_Iconified,TRUE);
      break;
      case MID_PROJECT_ABOUT:
        MUI_Request(epd->App, epd->Win, 0, NULL,
                    (STRPTR)GetString(&li,MSG_OK_BUTTON_PLAIN),
                    "\033c\033b"VERS" ("DATE")\033n\n\nCopyright (C) 2026 AmigaKit Ltd\n\n"
                    "\033u%s\033n\nJavier de las Rivas\n"
                    "\033u%s\033n\nTextEditor.mcc Open Source Team\n"
                    "\033u%s\033n\nAndy Broad",
                    GetString(&li,MSG_ABOUT_WRITTENBY), GetString(&li,MSG_ABOUT_SOURCECODE), GetString(&li,MSG_ABOUT_CONTRIB) );
      break;
      case MID_PROJECT_MUI_SETTINGS:
        DoMethod(epd->App, MUIM_Application_OpenConfigWindow, 0);
      break;
      case MID_PROJECT_OPENFILE:
      {
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
         //Add_RecentMenuEntry(epd);
        }
      }
      break;
      case MID_PROJECT_RECENT1:
      case MID_PROJECT_RECENT2:
      case MID_PROJECT_RECENT3:
      case MID_PROJECT_RECENT4:
      case MID_PROJECT_RECENT5:
      {
DBUG("  [MID_PROJECT_RECENT]\n",NULL);
        // Similar as in MID_PROJECT_OPENFILE
        STRPTR recent;
        ULONG  mid;
        ULONG  res = file_HasChanged(epd);

        if(res == 0) { break; } // CANCEL

        if(res == 2) // SAVE
        {
          if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_SAVE_TITLE), TRUE) )
          {
           Save_File(epd);
          }
          break;
        }

        mid = code - MID_PROJECT_RECENT1 + 1; // "get" MID_PROJECT_RECENTx (1,2,3,..)
        get(epd->MenuRecentItem[mid], MUIA_UserData, &recent);
DBUG("  '%s'\n",recent);
        FreeString(&epd->epd_FullName);
        epd->epd_FullName = DupStr(recent, -1);
        Load_File(epd);
      }
      break;
      case MID_PROJECT_SAVE:
        if(epd->epd_FullName == NULL)
        {
          FreeString(&epd->epd_FRFile); // "clear" ASL file string/name
          if( !FileRequest_ASL(epd, GetString(&li,MSG_ASL_SAVE_TITLE), TRUE) )
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

        if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_SAVE_TITLE), TRUE) )
        {
         Save_File(epd);
        }
      break;
      case MID_EDIT_COPY:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "COPY");
      break;
      case MID_EDIT_CUT:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "CUT");
      break;
      case MID_EDIT_PASTE:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "PASTE");
      break;
      case MID_EDIT_ERASE:
        DoMethod(epd->EditorObj, MUIM_TextEditor_ARexxCmd, "ERASE");
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
//MAIN window gadgets
      case GID_CURPOS:
      {
        ULONG lin, col;

        get(epd->EditorObj, MUIA_TextEditor_CursorX, &col);
        get(epd->EditorObj, MUIA_TextEditor_CursorY, &lin);
        DoMethod(epd->StatusLC, MUIM_SetAsString, MUIA_Text_Contents, GetString(&li,MSG_STATUS_LIN_COL),lin+1,col+1);
      }
      break;
      case GID_GOTOLINE:
      {
        ULONG val;
        char buf[32];

        get(epd->GotoLine, MUIA_String_Integer, &val);
        if(val == 0) { val = 1; }
        snprintf(buf, sizeof(buf), "GOTOLINE %lu",val-1); // TextEditor 1st line is 0

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
        if(search_String(epd, MUIF_TextEditor_Search_Backwards) == FALSE)
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
        if(is_marked==FALSE  &&  search_String(epd, 0)==FALSE)
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
          sigs = Wait(sigs | SIGBREAKF_CTRL_C );//| (myport ? 1<<myport->mp_SigBit : 0));
                    /*if(myport && (sigs & 1<<myport->mp_SigBit))
                    {
                      GetMsg(myport);
                      if(!rxmsg->rm_Result1 && rxmsg->rm_Result2)
                        DeleteArgstring((STRPTR)rxmsg->rm_Result2);

                      DeleteArgstring((UBYTE *)rxmsg->rm_Args[0]);
                      DeleteRexxMsg(rxmsg);
                      DeleteMsgPort(myport);
                      myport = NULL;
                    }*/

          if(sigs & SIGBREAKF_CTRL_C)
          {
           //running = 1; // DISCARD if changed
           done = TRUE;
           //break;
          }
        }
      break;
    } // END of switch()

  } // END of while()

                /*if(rxstdout)
                  Close(rxstdout);*/

  return file_HasChanged(epd);
}
