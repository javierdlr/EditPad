//#include "string.h"

#ifndef __amigaos4__
#include <clib/alib_protos.h>
//#include <clib/dos_protos.h>
#include <clib/muimaster_protos.h>
#else
#include <proto/exec.h>
//#include <proto/dos.h>
#include <proto/muimaster.h>
#endif

//#include <libraries/gadtools.h>
#include <libraries/mui.h>

/*
//#include <proto/locale.h>
#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
#include "editpad_catalog.h"
extern struct LocaleInfo li;
//extern STRPTR textbuf; // for localized messages
*/

#include "SDI_hook.h"


#include "debug.h"

#include "EditPad.h"
#include "Menu.h"
//#include "Events.h"
#include "ArexxCmds.h"
#include "Misc.h"


/* QUIT */
HOOKPROTONHNP(arexxQuitFunc, ULONG, Object *obj)
{
DBUG("%s()\n",__FUNCTION__);
  DoMethod(obj, MUIM_Application_ReturnID, MID_PROJECT_QUIT);

  return 0;
}
MakeStaticHook(arexxQuitHook, arexxQuitFunc);


/* OPEN FILENAME/F */
HOOKPROTO(arexxOpenFunc, ULONG, Object *obj, ULONG *args)
{
  char *filename = (char *)*args;
DBUG("%s() '%s'\n",__FUNCTION__,filename);
  if(filename)
  {
    struct editpad_data *epd = (struct editpad_data *)hook->h_Data;
//DBUG("  was: '%s'\n",epd->epd_FullName);
    FreeString(&epd->epd_FullName);
    epd->epd_FullName = DupStr( (STRPTR)filename, -1 );
//DBUG("  now: '%s'\n",epd->epd_FullName);
    Load_File(epd);
  }
  else
  {
    DoMethod(obj, MUIM_Application_ReturnID, MID_PROJECT_OPENFILE);
  }

  return 0;
}
MakeStaticHook(arexxOpenHook, arexxOpenFunc);


/* SAVE */
HOOKPROTONHNP(arexxSaveFunc, ULONG, Object *obj)
{
  DoMethod(obj, MUIM_Application_ReturnID, MID_PROJECT_SAVE);

  return 0;
}
MakeStaticHook(arexxSaveHook, arexxSaveFunc);


/* SAVEAS FILENAME/F */
HOOKPROTO(arexxSaveAsFunc, ULONG, Object *obj, ULONG *args)
{
  char *filename = (char *)*args;
DBUG("%s() '%s'\n",__FUNCTION__,filename);
  if(filename)
  {
    struct editpad_data *epd = (struct editpad_data *)hook->h_Data;
//DBUG("  was: '%s'\n",epd->epd_FullName);
    FreeString(&epd->epd_FullName);
    epd->epd_FullName = DupStr( (STRPTR)filename, -1 );
//DBUG("  now: '%s'\n",epd->epd_FullName);
    Save_File(epd);
  }
  else
  {
    DoMethod(obj, MUIM_Application_ReturnID, MID_PROJECT_SAVEAS);
  }

  return 0;
}
MakeStaticHook(arexxSaveAsHook, arexxSaveAsFunc);


/* ARexx command list */
static struct MUI_Command ARexxCmds[] = {
 {"QUIT",   NULL,         0, &arexxQuitHook  },
 {"OPEN",   "FILENAME/F", 1, &arexxOpenHook  },
 {"SAVE",   NULL,         0, &arexxSaveHook  },
 {"SAVEAS", "FILENAME/F", 1, &arexxSaveAsHook},

 {NULL, NULL, 0L, NULL}
};


/*
 * ARexx commands init (and InitHook() if needed)
 */
void Init_ArexxCmds(struct editpad_data *epd)
{
DBUG("%s()\n",__FUNCTION__);
  epd->ARexxCmds = ARexxCmds;

  InitHook(&arexxOpenHook,   arexxOpenHook,   epd);
  InitHook(&arexxSaveAsHook, arexxSaveAsHook, epd);
}
