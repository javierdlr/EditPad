//#include <stdio.h>
#include <string.h>

#ifndef __amigaos4__
#include <clib/alib_protos.h>
#endif
//#include <proto/exec.h>
//#include <proto/muimaster.h>

#include <libraries/mui.h>
//#include <mui/TextEditor_mcc.h>


//#include <proto/locale.h>
#define CATCOMP_NUMBERS
//#define CATCOMP_BLOCK
//#define CATCOMP_CODE
#include "editpad_catalog.h"
extern struct LocaleInfo li;
//extern STRPTR textbuf; // for localized messages


//#include "EditPad_rev.h"
#include "EditPad.h"
#include "Menu.h"
//#include "Events.h"
#include "Arexx.h"
#include "debug.h"


/*
 * Parses Arexx command into ID (RX_#?) and stores ARGument
 * (returns ID=0 and ARG='\0' if nothing found)
 */
static ULONG parse_CommandID(STRPTR const rx_cmd, STRPTR arg)
{
  ULONG  idx                  = 0;
  STRPTR valid_cmd[TOT_RXCMD] = { "OPEN", "SAVE", "OPENFILE", "SAVEAS",
                                  NULL }; // same order as RX_#? in Arexx.h
  ULONG  cmd_len[TOT_RXCMD]   = { 4, 4, 8, 6,
                                  0 };    // length of ARexx commands
DBUG("%s() '%s'\n",__FUNCTION__,rx_cmd);

  arg[0] = '\0'; // clear arguments

  while(valid_cmd[idx] != NULL)
  {
    ULONG len = cmd_len[idx];
    char  c   = *(rx_cmd+len),
          buf[32];

    if( strncmp(valid_cmd[idx],rx_cmd,len)==0  &&  (c==' ' || c=='\0') )
    {
      if(c == ' ') // has arguments
      {
        ULONG offset = cmd_len[idx] + 1; // "start" of arguments string
        len = strlen(rx_cmd) - cmd_len[idx];
        strncpy(arg, rx_cmd+offset, len);
        arg[len] = '\0';
      }
DBUG("  is a supported ARexx command. RX_#%ld: '%s'\n",idx+1,arg);
      return idx+1;
    }
    ++idx;
  }

  return 0;
}


void ProcessArexx(struct editpad_data *epd, struct RexxMsg *rmsg)
{
  char  args[MAX_DOS_PATH];
DBUG("%s() %p\n",__FUNCTION__,epd);
  ULONG id = parse_CommandID(ARG0(rmsg),(STRPTR)args);

  switch(id)
  {
    //case RX_NONE: break;
    case RX_OPEN:
      if(args[0] != '\0')
      {
DBUG("  was:'%s'\n",epd->epd_FullName);
        FreeString(&epd->epd_FullName);
        epd->epd_FullName = DupStr((STRPTR)args, -1);
DBUG("  now:'%s'\n",epd->epd_FullName);
        Load_File(epd);
      }
      else
      {
        if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_LOAD_TITLE), FALSE) )
        {
         Load_File(epd);
         //Add_RecentMenuEntry(epd);
        }
      }
    break;
    case RX_SAVE:
      if(args[0] != '\0')
      {
DBUG("  was: '%s'\n",epd->epd_FullName);
        FreeString(&epd->epd_FullName);
        epd->epd_FullName = DupStr((STRPTR)args, -1);
DBUG("  now: '%s'\n",epd->epd_FullName);
      }
      Save_File(epd);
    break;
    /*case RX_OPENFILE:
      if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_LOAD_TITLE), FALSE) )
      {
       Load_File(epd);
       //Add_RecentMenuEntry(epd);
      }
    break;*/
    case RX_SAVEAS:
      if( FileRequest_ASL(epd, GetString(&li,MSG_ASL_SAVE_TITLE), TRUE) )
      {
       Save_File(epd);
      }
    break;
    default: break;
  }

}
