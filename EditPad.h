#ifndef EDITPAD_H
#define EDITPAD_H 1


#ifndef __amigaos4__
#define MAX_DOS_NAME   100
#define MAX_DOS_PATH  2048
#endif

#define PROGNAME  "EditPad"


enum {
 BTN_OPEN = 0,
 BTN_SAVE,
 BTN_COPY,
 BTN_CUT,
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
 GID_CURPOS,
 GID_SEARCHNEXT,
 GID_SEARCHPREV,
 GID_REPLACE,
 GID_REPLACEALL,
 GID_GOTOLINE,
//FIND window
 GID_FIND_SWAP,

 TOT_GID
};


struct editpad_data {
 struct Hook  AppMsgHook; //ASL_startHook, ASL_stopHook;
 Object      *App, *Win, *EditorObj, *StatusLC, *StatusMsg, *GotoLine,
             *MenuRecentItem[5+1], // [0] is MID_PROJECT_RECENTBAR ("parent")
             *FindWin,
             *FindObj, *ReplaceObj,
             *Find_Case, *Find_FromStart,
             *Find_Next, *Find_Prev, *Find_Replace, *Find_ReplaceAll, *Find_Cancel;
 APTR         MenuStrip;

 struct FileRequester *AslReq;
 //struct Window        *MainWin;

 STRPTR epd_ProgName;
 STRPTR epd_FullName;
 STRPTR epd_FRDrawer, epd_FRFile;
 STRPTR epd_SearchStr;
};


void Save_File(struct editpad_data *);
BOOL Load_File(struct editpad_data *);
LONG FileRequest_ASL(struct editpad_data *, CONST_STRPTR title, BOOL save);
STRPTR DupStr(STRPTR str, LONG length);
VOID FreeString(STRPTR *string);


#endif
