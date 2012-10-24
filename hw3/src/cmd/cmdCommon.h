/****************************************************************************
  FileName     [ cmdCommon.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define classes for common commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef CMD_COMMON_H
#define CMD_COMMON_H

#include "cmdParser.h"

CmdClass(HelpCmd);
CmdClass(QuitCmd);
CmdClass(HistoryCmd);
CmdClass(DofileCmd);

#endif // CMD_COMMON_H
