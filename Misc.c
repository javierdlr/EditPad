#include <stdio.h>
#include "string.h"

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#endif
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/icon.h>
#include <proto/asl.h>

#include <libraries/mui.h>
#include <mui/TextEditor_mcc.h>
//#include <mui/TheBar_mcc.h>
//#include <workbench/workbench.h>
//#include <workbench/startup.h>
//#include <libraries/asl.h>


//#include <proto/locale.h>
#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
#include "editpad_catalog.h"
extern struct LocaleInfo li;
//extern STRPTR textbuf; // for localized messages


#if defined(__amigaos4__)
#include <dos/obsolete.h>
#endif


#include "debug.h"

//#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
#include "Events.h"
//#include "ArexxCmds.h"
#include "Misc.h"


/*
 * Read tooltypes et al
 */
static STRPTR CFGString(struct DiskObject *icon, STRPTR attr, STRPTR def)
{
 STRPTR val;

 val = FindToolType(icon->do_ToolTypes, attr);
DBUG("%s() '%s'\n",val);
 return(val? val:def);
}

void Read_ToolTypes(struct editpad_data *epd)
{
	struct DiskObject *micon;
DBUG("%s() - START\n",__FUNCTION__);
	micon = GetDiskObjectNew(epd->epd_ProgName);
	if(micon)
	{
		STRPTR ttp;
/*
		ttp = FindToolType(micon->do_ToolTypes, "TOOLBARBORDERLESS");
		if(ttp)
		{
			fgui->myTT.tb_borderless = TB_BUTSTYLE_BORDERLESS;
		}
DBUG("  TOOLBARBORDERLESS=0x%08lx\n",fgui->myTT.tb_borderless);

		ttp = FindToolType(micon->do_ToolTypes, "TOOLBARSTYLE");
		if(ttp)
		{
			if( MatchToolValue(ttp,"BOTH") ) fgui->myTT.tb_style = TB_DISPLAY_BOTH;
			else if( MatchToolValue(ttp,"TEXT") ) fgui->myTT.tb_style = TB_DISPLAY_TEXT;
			     //else if( MatchToolValue(ttp,"IMAGES") ) fgui->myTT.tb_style = TB_DISPLAY_IMAGES;
		}
DBUG("  TOOLBARSTYLE=0x%08lx\n",fgui->myTT.tb_style);
*/
		ttp = FindToolType(micon->do_ToolTypes, "PORTNAME");
		if(ttp)
		{
			FreeString(&epd->epd_PortName);
			epd->epd_PortName = DupStr(CFGString(micon,"PORTNAME",PROGNAME), -1);

			ttp = FindToolType(micon->do_ToolTypes, "PORTUNIQUE");
			if(ttp) { epd->epd_PortUnique = TRUE; }
		}
DBUG("  PORTNAME='%s'\n",epd->epd_PortName);

		FreeDiskObject(micon);
	} // END if(micon)
DBUG("%s() - END\n",__FUNCTION__);
}


/*
 * Set 'epd->epd_FullName' as window title
 */
void Set_WindowTitle(struct editpad_data *epd)
{
  static char wintitle[MAX_DOS_PATH];

  snprintf(wintitle, MAX_DOS_PATH, PROGNAME": %s",epd->epd_FullName);
  set(epd->Win, MUIA_Window_Title, wintitle);
}


/*
 * Get file protection bits of 'epd->epd_FullName'
 */
static LONG get_FileAttribs(struct editpad_data *epd)
{
  ULONG attribs = FIBF_EXECUTE; // RW-D (E bit not set); looks like is "inversed mode"
  BPTR  lock;
DBUG("%s() 0x%08lx\n",__FUNCTION__,epd->epd_FullName);
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


/*
 * Splits 'epd_FullName' into 'epd_FRDrawer' and 'epd_FRFile'
 */
void Split_FullName(struct editpad_data *epd)
{
  ULONG len = strlen(epd->epd_FullName);
DBUG("%s()\n",__FUNCTION__);
  FreeString(&epd->epd_FRFile);
  FreeString(&epd->epd_FRDrawer);
DBUG("  epd_FullName 0x%08lx\n",epd->epd_FullName);
  epd->epd_FRFile = DupStr(FilePart(epd->epd_FullName), -1);
DBUG("  epd_FRFile: '%s'\n",epd->epd_FRFile);
  len -= strlen(epd->epd_FRFile);
  epd->epd_FRDrawer = DupStr(epd->epd_FullName, len);
DBUG("  epd_FRDrawer: '%s' [%ld]\n",epd->epd_FRDrawer,len);
}


/*
 * Save file with 'epd->epd_FullName' full path name
 * NOTE: preserves file attributes.
 */
void Save_File(struct editpad_data *epd)
{
  BPTR fh;
  char error_str[250];
  LONG attribs = get_FileAttribs(epd);
DBUG("%s() 0x%08lx (0x%08lx)\n",__FUNCTION__,epd->epd_FullName,attribs);

  if(epd->epd_FullName == NULL) { return; }

  if( (fh=Open(epd->epd_FullName,MODE_NEWFILE)) )
  {
    STRPTR text = (STRPTR)DoMethod(epd->EditorObj, MUIM_TextEditor_ExportText);

    if(Write(fh,text,strlen(text)) != -1)
    {
      Close(fh);
      FreeVec(text);

      SetProtection(epd->epd_FullName, attribs);

      set(epd->EditorObj, MUIA_TextEditor_HasChanged, FALSE);

      Set_WindowTitle(epd);
      Add_RecentMenuEntry(epd, epd->epd_FullName);
DBUG("%s() OK\n",__FUNCTION__);
      return;
    }

    Close(fh);
    FreeVec(text);
  }

DBUG("%s() ERROR\n",__FUNCTION__);
    Fault( IoErr(), (STRPTR)GetString(&li,MSG_ERROR_SAVINGFILE), error_str, sizeof(error_str) );
    MUI_Request(epd->App, epd->Win, 0L, NULL,
                (STRPTR)GetString(&li,MSG_OK_BUTTON_PLAIN),
                error_str);
}


/*
 * Load file from 'epd->epd_FullName' full path name
 */
BOOL Load_File(struct editpad_data *epd)
{
  BPTR fh;
  BOOL ret = FALSE;
DBUG("%s() 0x%08lx\n",__FUNCTION__,epd->epd_FullName);

  if(epd->epd_FullName == NULL) { return FALSE; }

  if( (fh=Open(epd->epd_FullName,MODE_OLDFILE)) )
  {
    struct FileInfoBlock *fib;

    fib = (struct FileInfoBlock *)AllocDosObject(DOS_FIB, NULL);
    if(fib)
    {
      STRPTR fileptr;
      LONG   size = 0;

      if( ExamineFH(fh,fib) )
      {
        size = fib->fib_Size;
      }
      FreeDosObject(DOS_FIB, fib);

      fileptr = AllocVec(size+1024, 0L); // +1024 for "safety"
      if(size  &&  fileptr)
      {
        size = Read(fh, fileptr, size+1024-2);
        fileptr[size] = '\0';

        set(epd->EditorObj, MUIA_TextEditor_Contents, fileptr);
        set(epd->EditorObj, MUIA_TextEditor_ImportHook, MUIV_TextEditor_ImportHook_Plain);
        set(epd->EditorObj, MUIA_TextEditor_HasChanged,FALSE);

        Set_WindowTitle(epd);

        DoMethod(epd->StatusLC, MUIM_SetAsString, MUIA_Text_Contents, GetString(&li,MSG_STATUS_LIN_COL),1,1);
DBUG("%s() OK\n",__FUNCTION__);
        ret = TRUE;
      }
      FreeVec(fileptr);
    }

    Close(fh);
  }
  else
  {
    char error_str[250];
DBUG("%s() ERROR\n",__FUNCTION__);
    Fault( IoErr(), (STRPTR)GetString(&li,MSG_ERROR_LOADINGFILE), error_str, sizeof(error_str) );
    MUI_Request(epd->App, epd->Win, 0L, NULL,
                (STRPTR)GetString(&li,MSG_OK_BUTTON_PLAIN),
                error_str);
  }

  return ret;
}


/*
 * ASL file requester
 * Stores path and name in 'epd->epd_FullName' if OK.
 */
LONG FileRequest_ASL(struct editpad_data *epd, CONST_STRPTR title, BOOL save)
{
DBUG("%s()  [DoSave=%ld] 0x%08lx\n",__FUNCTION__,save,epd->epd_FullName);
	if(epd->AslReq)
	{
		struct Window *win;

		get(epd->Win, MUIA_Window_Window, &win);

		if(epd->epd_FullName != NULL)
		{
			Split_FullName(epd); // "fills" epd_FRDrawer & epd_FRFile
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


/*
 * Print/Send file to PRT:
 */
void Print_File(struct editpad_data *epd)
{
  BPTR fh;
  char error_str[250];
DBUG("%s()\n",__FUNCTION__);
  if( (fh=Open("PRT:",MODE_NEWFILE)) )
  {
    STRPTR text = (STRPTR)DoMethod(epd->EditorObj, MUIM_TextEditor_ExportText);

    Write( fh, text, strlen(text) );
    Close(fh);
    FreeVec(text);
  }
  else
  {
    Fault( IoErr(), (STRPTR)GetString(&li,MSG_ERROR_PRINTINGFILE), error_str, sizeof(error_str) );
    MUI_Request(epd->App, epd->Win, 0L, NULL,
                (STRPTR)GetString(&li,MSG_OK_BUTTON_PLAIN),
                error_str);
  }
}
