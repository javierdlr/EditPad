#include "string.h"

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#include <clib/exec_protos.h>
#include <clib/dos_protos.h>
#include <clib/muimaster_protos.h>
#else
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#endif

#include <libraries/gadtools.h>
#include <libraries/mui.h>


//#include <proto/locale.h>
#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
#include "editpad_catalog.h"
extern struct LocaleInfo li;
//extern STRPTR textbuf; // for localized messages


#include "debug.h"

//#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
//#include "Events.h"
//#include "ArexxCmds.h"
//#include "Misc.h"


/*
 * Creates menu options and returns its object
 */
APTR Create_Menu(void)
{
DBUG("%s()\n",__FUNCTION__);
	struct NewMenu newmenus[] = {
//PROJECT
		{ NM_TITLE, GetString(&li,MSG_PROJECT_MENU), NULL, 0, 0, (APTR)MID_PROJECT_MENU                 },
//		{ NM_ITEM, GetString(&li,MSG_PROJECT_OPEN),         "O",  0, 0, (APTR)MID_PROJECT_OPEN          },
//		{ NM_ITEM, GetString(&li,MSG_PROJECT_RECENT),       NULL, 0, 0, (APTR)MID_PROJECT_RECENT        },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_OPEN),         NULL, 0, 0, NULL                           },
			{ NM_SUB,  GetString(&li,MSG_PROJECT_OPENFILE),    "O",  0,               0, (APTR)MID_PROJECT_OPENFILE    },
			{ NM_SUB,  NM_BARLABEL,                            NULL, 0,               0, (APTR)MID_PROJECT_RECENTBAR   },
			  // later we add RECENT1..MAX_RECENT using Add_RecentMenuEntry()
			{ NM_SUB,  GetString(&li,MSG_PROJECT_RECENTCLEAR), NULL, NM_ITEMDISABLED, 0, (APTR)MID_PROJECT_RECENTCLEAR },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_SAVE),         "S",  0, 0, (APTR)MID_PROJECT_SAVE         },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_SAVEAS),       NULL, 0, 0, (APTR)MID_PROJECT_SAVEAS       },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_ICONIFY),      "I",  0, 0, (APTR)MID_PROJECT_ICONIFY      },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_ABOUT),        "?",  0, 0, (APTR)MID_PROJECT_ABOUT        },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_MUI_SETTINGS), NULL, 0, 0, (APTR)MID_PROJECT_MUI_SETTINGS },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_QUIT),         "Q",  0, 0, (APTR)MID_PROJECT_QUIT         },
//EDIT
		{ NM_TITLE, GetString(&li,MSG_EDIT_MENU), NULL, 0, 0, (APTR)MID_EDIT_MENU         },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_CUT),      "X",  0, 0, (APTR)MID_EDIT_CUT      },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_COPY),     "C",  0, 0, (APTR)MID_EDIT_COPY     },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_PASTE),    "V",  0, 0, (APTR)MID_EDIT_PASTE    },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_ERASE),    NULL, 0, 0, (APTR)MID_EDIT_ERASE    },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_MARKALL),  "M",  0, 0, (APTR)MID_EDIT_MARKALL  },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_UNDO),     "U",  0, 0, (APTR)MID_EDIT_UNDO     },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_REDO),     "Z",  0, 0, (APTR)MID_EDIT_REDO     },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_TOUPPER),  NULL,  0, 0, (APTR)MID_EDIT_TOUPPER },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_TOLOWER),  NULL,  0, 0, (APTR)MID_EDIT_TOLOWER },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_CLEARALL), NULL, 0, 0, (APTR)MID_EDIT_CLEARALL },
//NAVIGATION
		{ NM_TITLE, GetString(&li,MSG_NAVIGATION_MENU), NULL, 0, 0, (APTR)MID_NAVIGATION_MENU },
		{ NM_ITEM,  GetString(&li,MSG_NAV_FIND),     "F", 0, 0, (APTR)MID_NAV_FIND            },
		//{ NM_ITEM,  GetString(&li,MSG_NAV_REPLACE),  "R", 0, 0, (APTR)MID_NAV_REPLACE          },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_NAV_GOTOLINE), "J", 0, 0, (APTR)MID_NAV_GOTOLINE        },
//SETTINGS
		{ NM_TITLE, GetString(&li,MSG_SETTINGS_MENU), NULL, 0, 0, (APTR)MID_SETTINGS_MENU                  },
		{ NM_ITEM,  GetString(&li,MSG_SETTINGS_TABS), NULL, CHECKIT|MENUTOGGLE, 0, (APTR)MID_SETTINGS_TABS },

		{ NM_END, NULL, NULL, 0, 0, NULL }
	};

	return MUI_MakeObject(MUIO_MenustripNM, newmenus, 0);
}


void Add_RecentMenuEntry(struct editpad_data *epd, STRPTR str)
{
  STRPTR itemname;
  ULONG  i;
DBUG("%s() recent #%ld+1\n",__FUNCTION__,epd->epd_RecentIdx); // [0] is "parent"

  if(epd->epd_RecentIdx == MAX_RECENT)
  {
    get(epd->MenuRecentItem[1], MUIA_UserData, &itemname);
DBUG("  REMOVING [1]'%s' and SHIFTING entries...\n",itemname);
    FreeString(&itemname);
    DoMethod(epd->MenuStrip, MUIM_Family_Remove, epd->MenuRecentItem[1]);
    i = 2;
    while(i < MAX_RECENT+1) // shifting: [2]..[MAX_RECENT] -> [1]..[MAX_RECENT-1]
    {
//get(epd->MenuRecentItem[i], MUIA_UserData, &itemname);
//DBUG("  [%ld]'%s' (was [%ld])\n",i-1,itemname,i);
      epd->MenuRecentItem[i-1] = epd->MenuRecentItem[i];
      ++i;
    }
    --epd->epd_RecentIdx; // we have [MAX_RECENT] entry free
  }

  i = epd->epd_RecentIdx;

  while(i) // avoid adding same file to recent
  {
    get(epd->MenuRecentItem[i], MUIA_UserData, &itemname);
//DBUG("'%s' == '%s'\n",str,itemname);
    if(strcmp(str,itemname) == 0) { return; }
    --i;
  }

  ++epd->epd_RecentIdx; // "next" free entry

  //itemname = DupStr(epd->epd_FullName, -1);
  itemname = DupStr(str, -1);

  i = MID_PROJECT_RECENTBAR + epd->epd_RecentIdx; // MID_PROJECT_RECENTx (1,2,3,..)
  epd->MenuRecentItem[epd->epd_RecentIdx] = MUI_MakeObject(MUIO_Menuitem, FilePart(itemname), 0,0,i);
  DoMethod(epd->MenuRecentItem[epd->epd_RecentIdx], MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime, epd->App, 2, MUIM_Application_ReturnID, i);
DBUG("  ADDED %p [%ld]'%s'\n",epd->MenuRecentItem[epd->epd_RecentIdx],epd->epd_RecentIdx,itemname);
  // Insert entry below last one and "remember" string pointer
  DoMethod(epd->MenuStrip, MUIM_Family_Insert, epd->MenuRecentItem[epd->epd_RecentIdx], epd->MenuRecentItem[epd->epd_RecentIdx-1]);
  set(epd->MenuRecentItem[epd->epd_RecentIdx], MUIA_UserData, itemname);

  set(findobj(MID_PROJECT_RECENTCLEAR,epd->MenuStrip), MUIA_Menuitem_Enabled, TRUE);
}


/*
 * Saves recent list to ENVARC: and free entries
 */
void SaveRecentList_FreeEntries(struct editpad_data *epd)
{
  STRPTR itemname;
  BPTR   fh;
DBUG("%s() recents=%ld\n",__FUNCTION__,epd->epd_RecentIdx);

  if(epd->epd_RecentIdx == 0)
  {
    DeleteFile(RECENT_PATH""RECENT_FILE);
    return;
  }

  if( (fh=Open(RECENT_PATH""RECENT_FILE,MODE_NEWFILE)) == 0 )
  {
    char error_str[250];

    Fault( IoErr(), (STRPTR)GetString(&li,MSG_ERROR_SAVINGRECENT), error_str, sizeof(error_str) );
    MUI_Request(epd->App, epd->Win, 0L,
                PROGNAME,
                (STRPTR)GetString(&li,MSG_OK_BUTTON_PLAIN),
                error_str);
  }

  while(epd->epd_RecentIdx)
  {
    get(epd->MenuRecentItem[epd->epd_RecentIdx], MUIA_UserData, &itemname);
DBUG("  %p [%ld]'%s'\n",epd->MenuRecentItem[epd->epd_RecentIdx],epd->epd_RecentIdx,itemname);
    if(fh)
    {
DBUG("             SAVED in '"RECENT_PATH""RECENT_FILE"\n",NULL);
      //FWrite(fh, itemname, strlen(itemname), 1);
      //FWrite(fh, "\n", 1, 1);
      Write( fh, itemname, strlen(itemname) );
      Write( fh, "\n", 1 );
    }
    FreeString(&itemname);
DBUG("             FREED entry\n",NULL);
    --epd->epd_RecentIdx;
  }

  Close(fh);
}


/*void Save_RecentList(struct editpad_data *epd)
{
  STRPTR itemname;
  BPTR   fh;
DBUG("%s() recents=%ld\n",__FUNCTION__,epd->epd_RecentIdx);

  if(epd->epd_RecentIdx == 0) { return; }

  if( (fh=Open("ENVARC:"RECENT_FILE,MODE_NEWFILE)) )
  {
    ULONG i = epd->epd_RecentIdx;
    while(i)
    {
      get(epd->MenuRecentItem[i], MUIA_UserData, &itemname);
DBUG("  %p [%ld]'%s'\n",epd->MenuRecentItem[i],i,itemname);
      Write( fh, itemname, strlen(itemname) );
      Write( fh, "\n", 1);
      --i;
    }
    Close(fh);
  }

}*/


void Load_RecentsFile(struct editpad_data *epd)
{
  BPTR  fh;
  ULONG buf_size = MAX_DOS_PATH * (MAX_RECENT+1); // +1 for "safety"
DBUG("%s()\n",__FUNCTION__);

  //CreateDir(RECENT_PATH);

  if( (fh=Open(RECENT_PATH""RECENT_FILE,MODE_OLDFILE)) )
  {
    WORD   pos    = 0;
    STRPTR buffer = AllocVec(buf_size, MEMF_CLEAR);
    char   itemname[MAX_DOS_PATH];

    Read(fh, buffer, buf_size);

    while(pos != -1)
    {
      pos = SplitName(buffer, '\n', itemname, pos, MAX_DOS_PATH);
      if(pos != -1)
      {
        Add_RecentMenuEntry(epd, itemname);
      }
    }
DBUG("  recents=%ld\n",epd->epd_RecentIdx);
    Close(fh);
  }

}


/*
 * Creates pop-up menu options and returns its object
 */
APTR Create_PopUpMenu(void)
{
DBUG("%s()\n",__FUNCTION__);
 struct NewMenu popupmenus[] =
 {
   { NM_TITLE, PROGNAME, 0, 0, 0, (APTR)0 },
   //{ NM_ITEM,  "Block-mark start", NULL, 0, 0, (APTR)PID_MARK_START },
   //{ NM_ITEM,  "Block-mark end",   NULL, 0, 0, (APTR)PID_MARK_END   },
   { NM_ITEM, GetString(&li,MSG_EDIT_CUT),     NULL, 0, 0, (APTR)PID_CUT     },
   { NM_ITEM, GetString(&li,MSG_EDIT_COPY),    NULL, 0, 0, (APTR)PID_COPY    },
   { NM_ITEM, GetString(&li,MSG_EDIT_PASTE),   NULL, 0, 0, (APTR)PID_PASTE   },
   { NM_ITEM, GetString(&li,MSG_EDIT_ERASE),   NULL, 0, 0, (APTR)PID_ERASE   },
   { NM_ITEM, GetString(&li,MSG_EDIT_MARKALL), NULL, 0, 0, (APTR)PID_MARKALL },

   { NM_END,   NULL, 0, 0, 0, (APTR)0 }
 };

	return MUI_MakeObject(MUIO_MenustripNM, popupmenus, 0);
}
