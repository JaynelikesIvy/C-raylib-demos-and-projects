#include "terminal.h"

char* buffer;
int btn_activate = -1;

int DrawTerminal()
{
    GuiTextInputBox((Rectangle){0, 0, 500, 100}, "Terminal", "Input commands", buffer, 24, "Dummy Text", &btn_activate, false);
    if(btn_activate == 0) return 0;
    if(btn_activate == 1)
    {
        int argc = 0;
        char** temp = {0};
        for(size_t i = 0; i < strlen(buffer); i++)
        {
            if(strcmp(buffer[i], " "))
            {
                argc++;
                char* temp2 = buffer;
                temp2[i] = 0;
                for(size_t j = 0; j < i; j++)
                {
                    strcpy(temp[argc][j], temp2[j]);   
                }
            }
        }

        for(int i = 0; i < argc; i++)
        {
            if(strcmp(temp[i], COMMAND_GIVEGUN));
        }
    }

}