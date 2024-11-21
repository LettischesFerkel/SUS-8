#include <stdio.h>

#define _CONSOLE_COMMAND_COUNT 8
#define _CONSOLE_COMMAND_MAX_LENGTH 12
#define _CONSOLE_COMMAND_ARGUMENT_COUNT 2
#define _CONSOLE_COMMAND_ARGUMENT_SIZE 8
char CONSOLE_COMMANDS[_CONSOLE_COMMAND_COUNT * _CONSOLE_COMMAND_MAX_LENGTH] = 
{
    "exit      "
    "nigger     "
    "halt       "
    "bruh       "
    "           "
    "           "
    "           "
    "           "
};
void* CONSOLE_COMMAND_FUNCTION_POINTERS[_CONSOLE_COMMAND_COUNT];
/*
    \0 - no argument
    c  - char
    s  - string
    n  - whole number
*/
char CONSOLE_COMMAND_ARGUMENT_FORMATS[_CONSOLE_COMMAND_COUNT * _CONSOLE_COMMAND_ARGUMENT_COUNT] = 
{
    'c',  '\0', // black
    's',  'n' , // nigger
    'n',  '\0', // halt
    '\0', '\0'  // bruh
};

void clearString(char* string, int length)
{
    for (int i = 0; i < length - 1; i++) { *(string + i) = ' '; }
    *(string + length - 1) = '\0';
}

int tokeniseString(char* input, int inputLength, char* output, char cosmetic) // returns argument count
{
    char escapeCharacter = 0;   // \ *
    char quote = 0;             // " 
    char halfQuote = 0;         // ' 
    char argIndex = 1 + (48 * cosmetic);
    char newArg = 1;
    char clearEnd = 0;
    int retVal = 0;
    int lastI = 0;
    for (int i = 0; i < inputLength; i++)
    {
        if (clearEnd) { break; }
        char current = *(input + i);
        switch (current)
        {
            case ' ':
                if (escapeCharacter) { *(output + i) = argIndex; escapeCharacter = 0; }
                else if (quote || halfQuote) { *(output + i) = argIndex; }
                else
                {
                    if (!newArg) { argIndex++; newArg = 1; }
                    *(output + i) = 0 + (' ' * cosmetic);
                }
                break;
            case '\\':
                *(output + i) = argIndex;
                if (escapeCharacter) { escapeCharacter = 0; }
                else { escapeCharacter = 1;  }
                newArg = 0;
                break;
            case '"':
                *(output + i) = argIndex;
                if (escapeCharacter) { escapeCharacter = 0; }
                else if (quote) { quote = 0; }
                else { quote = 1; }
                newArg = 0;
                break;
            case '\'':
                *(output + i) = argIndex;
                if (escapeCharacter) { escapeCharacter = 0; }
                else if (halfQuote) { halfQuote = 0; }
                else { halfQuote = 1; }
                newArg = 0;
                break;
            case '\0':
                *(output + i) = 0;
                retVal = argIndex - newArg - (48 * cosmetic);
                clearEnd = 1;
                lastI = i;
                break;
            case '\n':
                *(output + i) = 0;
                retVal = argIndex - newArg - (48 * cosmetic);
                clearEnd = 1;
                lastI = i;
                break;
            default:
                *(output + i) = argIndex;
                if (escapeCharacter) { escapeCharacter = 0; }
                newArg = 0;
                break;
        }
    }
    if (clearEnd)
    {
        for (int i = lastI; i < inputLength; i++) { *(output + i) = 0; }
        return retVal;
    }
    return argIndex - (48 * cosmetic);
}

int decodeCommand(char* input, int inputLength, char* tokens, char* commands, int commandLength, int commandCount, char cosmetic) // returns decoded argument
{
    char suspects[commandCount];
    for (int i = 0; i < commandCount; i++) { suspects[i] = 1; }
    int suspectCount = commandCount;
    int suspectI = -1;
    for (int i = 0; i < inputLength; i++)
    {
        if (cosmetic && (*(tokens + i) == ' ')) { if (suspectCount < 0) { continue; } else { break; } }
        if (*(tokens + i) == 0) { if (suspectCount < 0) { continue; } else { break; } }
        suspectI++;
        char current = *(input + i);
        for (int c = 0; c < commandCount; c++)
        {
            if (suspects[c]) { suspects[c] = (current == *(commands + ((commandLength - 1) * c) + suspectI)); if (!suspects[c]) { suspectCount--; } }
        }
    }
    if (suspectCount == 0) { printf("Warning: No commands match!\n"); return -1; }
    else if (suspectCount > 1) { printf("Warning: Too many commands matched!\n"); return -1; }
    else
    {
        for (int i = 0; i < commandCount; i++) { if (suspects[i]) { return i; } }
    }
    return 0;
}

int decodeArgs(char* input, int inputLength, int command, char* argumentFormats, int commandCount, int argumentCount, void* data, int dataLength) // masīvs ar komandu tipiem, veidiem un argumentu formatēšanu, atgriež funkcijas prototipu
{
    char formats[argumentCount];
    for (int i = 0; i < argumentCount; i++) { formats[i] = *(argumentFormats + (argumentCount * command) + i); }

    printf("Command: %d\n", command);
    printf("Arguments: ");
    for (int i = 0; i < argumentCount; i++) { printf("%c ", formats[i]); }
    printf("\n");

    if (!formats[0]) { return 0; }


    return 0;
}

int runCommand(/* some command function template */)
{
    return 0;
}