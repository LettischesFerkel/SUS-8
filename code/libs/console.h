#define _COMMAND_COUNT 4
#define _ARGUMENT_COUNT 2
char* commands[_COMMAND_COUNT] = 
{
    "black", "nigger", "halt", "bruh"
};
void* commandFunctionPointers[_COMMAND_COUNT];
/*
    \0 - no argument
    c  - char
    s - string
    n - natural number
*/
char commandArgumentFormat[_COMMAND_COUNT][_ARGUMENT_COUNT] = 
{
    { '\0', '\0' }, 
    { '\0', '\0' }, 
    { '\0', '\0' }, 
    { '\0', '\0' }
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

int decodeCommand(/* masīvs ar komandu tipiem, veidiem un argumentu formatēšanu, atgriež funkcijas prototipu */)
{

}

int runCommand(/* some command function template */)
{

}