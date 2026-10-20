#ifndef _GAME_TERMINAL_H_
#define _GAME_TERMINAL_H_

#include <stdio.h>
#include <string.h>
#include "raylib.h"
//#include "src/Third-party/raygui.h"

#define COMMAND_GIVEGUN "givegun"

typedef enum{
    TERMINAL_CLOSED,
    TERMINAL_OPENED
}TerminalState;

void HandleTerminal();

#endif