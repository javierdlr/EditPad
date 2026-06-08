#ifndef EDITPAD_H
#define EDITPAD_H 1


#ifndef __amigaos4__
#define MAX_DOS_NAME   100
#define MAX_DOS_PATH  2048
#endif

#define PROGNAME  "EditPad"

#define MAX_RECENT  5

//#ifndef MUIA_Application_UsedClasses
//#define MUIA_Application_UsedClasses  0x8042e9a7  /* V20 STRPTR * i.. */
//#endif
//#define MUIV_RunARexxScript           0xad800000


//Macros https://library.morph.zone/Locating_Objects_in_the_Object_Tree
#define findobj(id,parent)  (Object*)DoMethod(parent, MUIM_FindUData, id)


enum {
 BTN_OPEN = 0,
 BTN_SAVE,
 BTN_CUT,
 BTN_COPY,
 BTN_PASTE,
 BTN_ERASE,
 BTN_UNDO,
 BTN_REDO,
 BTN_CLR,
 BTN_FIND,

 TOT_BTNS
};

enum {
 GID_DUMMY = 0,
//MAIN window (status)
 GID_CURPOS,
 GID_GOTOLINE,
//FIND window
 GID_SEARCHNEXT,
 GID_SEARCHPREV,
 GID_REPLACE,
 GID_REPLACEALL,
 GID_FIND_SWAP,

 TOT_GID
};


struct editpad_data {
 struct Hook  AppMsgHook;//, ARexxHook; //ASL_startHook, ASL_stopHook;
 Object      *App, *Win, *EditorObj, *StatusLC, *StatusMsg, *GotoLine,
             *MenuRecentItem[MAX_RECENT+1], // [0] is MID_PROJECT_RECENTBAR ("parent")
             *EditPopup,
             *FindWin,
             *FindObj, *ReplaceObj,
             *Find_Case, *Find_FromStart,
             *Find_Next, *Find_Prev, *Find_Replace, *Find_ReplaceAll, *Find_Cancel;
 APTR         MenuStrip;

 struct FileRequester *AslReq;
 //struct Window        *MainWin;

 STRPTR epd_ProgName, epd_FullName;
 STRPTR epd_FRDrawer, epd_FRFile;
 STRPTR epd_SearchStr;
 STRPTR epd_PortName;
 ULONG  epd_PortUnique; // TRUE or FALSE
 ULONG  epd_RecentIdx;

 struct MUI_Command *ARexxCmds;
};


STRPTR DupStr(STRPTR str, LONG length);
VOID FreeString(STRPTR *string);


#endif
