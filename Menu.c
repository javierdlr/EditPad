#include "string.h"

#ifndef __amigaos4__
#include <clib/alib_protos.h>
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


#include "EditPad.h"
#include "Menu.h"
#include "debug.h"


/*
 * Creates menu options nd returns its object
 */
APTR Create_Menu(void)
{
DBUG("%s()\n",__FUNCTION__);
	struct NewMenu newmenus[] = {
//PROJECT
		{ NM_TITLE, GetString(&li,MSG_PROJECT_MENU), NULL, 0, 0, (APTR)MID_PROJECT_MENU                 },
//		{ NM_ITEM, GetString(&li,MSG_PROJECT_OPEN),         "O",  0, 0, (APTR)MID_PROJECT_OPEN         },
//		{ NM_ITEM, GetString(&li,MSG_PROJECT_RECENT),       NULL, 0, 0, (APTR)MID_PROJECT_RECENT       },
		{ NM_ITEM,  GetString(&li,MSG_PROJECT_OPEN),         NULL, 0, 0, NULL                           },
			{ NM_SUB,  GetString(&li,MSG_PROJECT_OPENFILE), "O",  0, 0, (APTR)MID_PROJECT_OPENFILE  },
			{ NM_SUB,  NM_BARLABEL,                         NULL, 0, 0, (APTR)MID_PROJECT_RECENTBAR },
			// later we add RECENT1..5 in Add_RecentMenuEntry()
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
		{ NM_ITEM,  GetString(&li,MSG_EDIT_COPY),     "C",  0, 0, (APTR)MID_EDIT_COPY     },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_CUT),      "X",  0, 0, (APTR)MID_EDIT_CUT      },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_PASTE),    "V",  0, 0, (APTR)MID_EDIT_PASTE    },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_ERASE),    NULL, 0, 0, (APTR)MID_EDIT_ERASE    },
		{ NM_ITEM,  NM_BARLABEL, NULL, 0, 0, NULL },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_UNDO),     "U",  0, 0, (APTR)MID_EDIT_UNDO     },
		{ NM_ITEM,  GetString(&li,MSG_EDIT_REDO),     "Z",  0, 0, (APTR)MID_EDIT_REDO     },
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


static ULONG recent_cnt = 0;
void Add_RecentMenuEntry(struct editpad_data *epd)
{
 STRPTR itemname;
 ULONG  i;
DBUG("%s() recent #%ld+1\n",__FUNCTION__,recent_cnt); // [0] is "parent"

 //if(recent_cnt == MAX_RECENT) { return; }
 if(recent_cnt == MAX_RECENT)
 {
   get(epd->MenuRecentItem[1], MUIA_UserData, &itemname);
DBUG("  removing [1]'%s' and shifting entries...\n",itemname);
   FreeString(&itemname);
   DoMethod(epd->MenuStrip, MUIM_Family_Remove, epd->MenuRecentItem[1]);
   i = 2;
   while(i < MAX_RECENT+1) // shifting [2]..[MAX_RECENT] -> [1]..[MAX_RECENT-1]
   {
//get(epd->MenuRecentItem[i], MUIA_UserData, &itemname);
//DBUG("  [%ld]'%s' (was [%ld])\n",i-1,itemname,i);
     epd->MenuRecentItem[i-1] = epd->MenuRecentItem[i];
     ++i;
   }
   --recent_cnt; // we have [MAX_RECENT] entry free
 }

 i = recent_cnt;

 while(i) // avoid adding same file to recent
 {
   get(epd->MenuRecentItem[i], MUIA_UserData, &itemname);
//DBUG("'%s' == '%s'\n",epd->epd_FullName,itemname);
   if(strcmp(epd->epd_FullName,itemname) == 0) { return; }
   --i;
 }

 ++recent_cnt; // "next" free entry

 itemname = DupStr(epd->epd_FullName, -1);

 i = MID_PROJECT_RECENTBAR + recent_cnt; // MID_PROJECT_RECENTx (1,2,3,..)
 epd->MenuRecentItem[recent_cnt] = MUI_MakeObject(MUIO_Menuitem, FilePart(itemname), 0,0,i);
 DoMethod(epd->MenuRecentItem[recent_cnt], MUIM_Notify, MUIA_Menuitem_Trigger,MUIV_EveryTime, epd->App, 2, MUIM_Application_ReturnID, i);
DBUG("  adding %p [%ld]'%s'\n",epd->MenuRecentItem[recent_cnt],recent_cnt,itemname);
 // Insert entry below last one and "remember" string pointer
 DoMethod(epd->MenuStrip, MUIM_Family_Insert, epd->MenuRecentItem[recent_cnt], epd->MenuRecentItem[recent_cnt-1]);
 set(epd->MenuRecentItem[recent_cnt], MUIA_UserData, itemname);
}


void Remove_RecentMenuEntry(struct editpad_data *epd)
{
 STRPTR itemname;
DBUG("%s() recents=%ld\n",__FUNCTION__,recent_cnt);
 while(recent_cnt)
 {
  get(epd->MenuRecentItem[recent_cnt], MUIA_UserData, &itemname);
DBUG("  %p [%ld]'%s'\n",epd->MenuRecentItem[recent_cnt],recent_cnt,itemname);
  FreeString(&itemname);
  --recent_cnt;
 }
}
