#include <stdio.h>
#include "libs/console.h"

#define _INPUT_BUFFER_LENGTH 64
char INPUT_BUFFER[64];

char TEST_MODE = 0;


int main(int argc, char* args[])
{
    // decode arguments
    if (argc > 1)
    {
        switch (*args[1])
        {
            case 't':
                TEST_MODE = 1;
                break;
            default:
                break;
        }
    }

    // startup and config loading
    clearString(INPUT_BUFFER, _INPUT_BUFFER_LENGTH);

    // loop
    int t = 256;
    while (t--)
    {
        printf("\n> ");

        // Take input
        clearString(INPUT_BUFFER, _INPUT_BUFFER_LENGTH);
        fgets(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, stdin);

        // Tokenise input
        char tokens[_INPUT_BUFFER_LENGTH];
        int argcount = tokeniseString(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, tokens, 1);
        /*printf("%s\nArgcount: %d\n", tokens, argcount);
        for (int i = 0; i < _INPUT_BUFFER_LENGTH; i++) { printf("%2x ", tokens[i]); }
        printf("\n");*/

        // Decode input for correct command
        
    }
}