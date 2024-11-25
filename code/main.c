#include <stdio.h>
#include "libs/console.h"

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
    if (TEST_MODE) { printf("\n\tTEST MODE\n\n"); }

    // loop
    int t = 256;
    while (t--)
    {
        printf("\n>   ");

        // Take input
        clearString(INPUT_BUFFER, _INPUT_BUFFER_LENGTH);
        fgets(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, stdin);

        /*// Clear input from escape characters and quotes
        removeQuotes(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, 1, 0, 0);
        parseEscapeCharacters(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, 1, (char*)0);
        printf("Buf:%s\n", INPUT_BUFFER);
        for (int i = 0; i < _INPUT_BUFFER_LENGTH; i++) { printf("%2x ", INPUT_BUFFER[i]); }
        printf("\n");*/

        // Tokenise input
        char tokens[_INPUT_BUFFER_LENGTH];
        int argcount = tokeniseString(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, tokens, TEST_MODE);
        if (!argcount) { continue; }
        printf("    %s\n", tokens);

        // Decode input for correct command
        int command = decodeCommand(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, tokens, CONSOLE_COMMANDS, _CONSOLE_COMMAND_MAX_LENGTH, _CONSOLE_COMMAND_COUNT, TEST_MODE);
        if (command < 0) { continue; }
        //printf("Command: %d\n", command);

        // Decode command arguments
        char argdata[_CONSOLE_COMMAND_ARGUMENT_COUNT * _CONSOLE_COMMAND_ARGUMENT_SIZE];
        decodeArgs(INPUT_BUFFER, _INPUT_BUFFER_LENGTH, tokens, command, CONSOLE_COMMAND_ARGUMENT_FORMATS, _CONSOLE_COMMAND_COUNT, _CONSOLE_COMMAND_ARGUMENT_COUNT, argdata, _CONSOLE_COMMAND_ARGUMENT_SIZE, TEST_MODE);

        // Run command
        runCommand(command, CONSOLE_COMMAND_ARGUMENT_FORMATS, _CONSOLE_COMMAND_COUNT, _CONSOLE_COMMAND_ARGUMENT_COUNT, argdata, _CONSOLE_COMMAND_ARGUMENT_SIZE, TEST_MODE);
    }
}
