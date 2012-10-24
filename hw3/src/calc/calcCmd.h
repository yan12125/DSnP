/****************************************************************************
  FileName     [ calcCmd.h ]
  PackageName  [ calc ]
  Synopsis     [ Define modular calculator command classes ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef CALC_CMD_H
#define CALC_CMD_H

#include "cmdParser.h"

CmdClass(MsetCmd);
CmdClass(MvarCmd);
CmdClass(MaddCmd);
CmdClass(MsubCmd);
CmdClass(MmultCmd);
CmdClass(McmpCmd);
CmdClass(MprintCmd);

#endif // CALC_CMD_H
