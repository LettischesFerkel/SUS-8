#include <stdio.h>
#include <math.h>

#define _CONSOLE_COMMAND_COUNT 10
#define _CONSOLE_COMMAND_MAX_LENGTH 8
#define _CONSOLE_COMMAND_ARGUMENT_COUNT 3
#define _CONSOLE_COMMAND_ARGUMENT_SIZE 64 //must be 0 at mod(4)
#define _INPUT_BUFFER_LENGTH 256
char INPUT_BUFFER[_INPUT_BUFFER_LENGTH];
char CONSOLE_COMMANDS[_CONSOLE_COMMAND_COUNT * _CONSOLE_COMMAND_MAX_LENGTH] = 
{
    "exit    "
    "help    "
    "save    "
    "load    "
    "print   "
    "status  "
    "step    "
    "run     "
    "evaluate"
    "git    "
};
void* CONSOLE_COMMAND_FUNCTION_POINTERS[_CONSOLE_COMMAND_COUNT];
void * int
/*
    \0 - no argument
    c  - char
    s  - string
    n  - whole number
*/
char CONSOLE_COMMAND_ARGUMENT_FORMATS[_CONSOLE_COMMAND_COUNT * _CONSOLE_COMMAND_ARGUMENT_COUNT] = 
{
    '\0', '\0', '\0', // exit
    's',  '\0', '\0', // help
    's',  'n',  'n',  // save
    's',  'n',  'n',  // load
    's',  's',  's',  // print
    '\0', '\0', '\0', // status
    's',  'n',  '\0', // step
    'n',  '\0', '\0', // run
    's',  '\0', '\0', // evaluate
    's',  'n',  '\0'  // git
};

void CONSOLE_COMMAND_FUNCTION_PROTOTYPE(int COMMAND_ID, void* ARGS_DATA, int DATA_SIZE)
{
    printf("Default console command function protoype called!\n");
    return;
}

int parseIntFromString(char* string, int stringLength)
{
    int num = 0;
    int power = -1;
    int negative = 0;
    int base = 10; // d:10 b:2 x:16
    
    // determine base and negativity
    for (int i = 0; i < stringLength; i++)
    {
        char current = *(string + i);
        char lefter = (i == 0) ? ('\0') : (*(string + i - 1));
        switch (current)
        {
            case 'd':
                if (lefter == '0') { base = 10; }
                break;
            case 'b':
                if (lefter == '0') { base = 2; }
                break;
            case 'x':
                if (lefter == '0') { base = 16; }
                break;
            case '-':
                negative = !negative;
                break;
            default:
                break;
        }
    }
    //printf("Number is negative: %d base %d\n", negative, base);

    // calculate the value
    for (int i = (stringLength - 1); i >= 0; i--)
    {
        char current = *(string + i);
        char digit = 0;

        switch (base)
        {
            case 2:
                if ((current >= 48) && (current <= 57)) { digit = current - 48; power++; }
                break;
            case 10:
                if ((current >= 48) && (current <= 57)) { digit = current - 48; power++; }
                break;
            case 16:
                if ((current >= 48) && (current <= 57)) { digit = current - 48; power++; }
                else if ((current >= 65) && (current <= 70)) { digit = current - 55; power++; }
                else if ((current >= 97) && (current <= 102)) { digit = current - 87; power++; }
                break;
        }
        if (digit >= base) { printf("Warning: Incorrect number representation for base %d!\n", base); return 0; }
        num += digit * pow(base, power);
    }
    return num * (negative ? -1 : 1);
}
// parses all escape character sequences supported by the GNU C compiler collection standards, except octal and unicode.
// returns count of parsed escape character sequences.
// output used if !parseinplace
int parseEscapeCharacters(char* string, int stringLength, char parseinplace, char* output)
{
    char escapeCharacter = 0;
    int offset = 0;
    int hex = 0;
    char hexnum[5] = "0x00";
    //int oct = 0; // not implemented
    int escapeCharacterCount = 0;
    for (int i = 0; i < stringLength; i++)
    {
        char current = *(string + i);
        if (escapeCharacter)
        {
            if (hex)
            {
                if (((current >= 48) && (current <= 57)) || ((current >= 65) && (current <= 70)) || ((current >= 97) && (current <= 102))) // acceptable hexadecimal digit
                {
                    hexnum[4 - hex] = current;
                }
                else
                {
                    printf("Warning: Nonacceptable hexadecimal digit for escape character sequence '%c'!\n", current);
                }
                hex--;
                if (!hex)
                {
                    if (parseinplace) { *(string + i - 1 + offset) = parseIntFromString(hexnum, 4); }
                    else { *(output + i - 1 + offset) = parseIntFromString(hexnum, 4); }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                }
                offset--;
                continue;
            }
            switch (current)
            {
                case 'a':
                    //*(string + i - 1 + offset) = 0x07;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x07; }
                    else { *(output + i - 1 + offset) = 0x07; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'b':
                    //*(string + i - 1 + offset) = 0x08;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x08; }
                    else { *(output + i - 1 + offset) = 0x08; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'e':
                    //*(string + i - 1 + offset) = 0x1B;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x1B; }
                    else { *(output + i - 1 + offset) = 0x1B; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'f':
                    //*(string + i - 1 + offset) = 0x0C;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x0C; }
                    else { *(output + i - 1 + offset) = 0x0C; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'n':
                    //*(string + i - 1 + offset) = 0x0A;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x0A; }
                    else { *(output + i - 1 + offset) = 0x0A; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'r':
                    //*(string + i - 1 + offset) = 0x0D;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x0D; }
                    else { *(output + i - 1 + offset) = 0x0D; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 't':
                    //*(string + i - 1 + offset) = 0x09;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x09; }
                    else { *(output + i - 1 + offset) = 0x09; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'v':
                    //*(string + i - 1 + offset) = 0x0B;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x0B; }
                    else { *(output + i - 1 + offset) = 0x0B; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case '\\':
                    //*(string + i - 1 + offset) = 0x5C;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x5C; }
                    else { *(output + i - 1 + offset) = 0x5C; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case '\'':
                    //*(string + i - 1 + offset) = 0x27;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x27; }
                    else { *(output + i - 1 + offset) = 0x27; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case '\"':
                    //*(string + i - 1 + offset) = 0x22;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x22; }
                    else { *(output + i - 1 + offset) = 0x22; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case '\?':
                    //*(string + i - 1 + offset) = 0x3F;
                    if (parseinplace) { *(string + i - 1 + offset) = 0x3F; }
                    else { *(output + i - 1 + offset) = 0x3F; }
                    escapeCharacterCount++;
                    escapeCharacter = 0;
                    break;
                case 'x':
                    hex = 2;
                    break;
                default:
                    printf("Warning: Incorrent escape character sequence '\\%c'!\n", current);
                    *(string + i - 1 + offset) = current;
                    escapeCharacter = 0;
                    break;
            }
            offset--;
        }
        else
        {
            if (current == '\\') { escapeCharacter = 1; }
            *(string + i + offset) = current;
        }
    }
    return escapeCharacterCount;
}

int removeQuotes(char* string, int stringLength, char excludeEscapeCharacters, char excludeSingle, char excludeDouble) // returns amount of quotations removed
{
    char escapeCharacter = 0;
    int offset = 0;
    for (int i = 0; i < stringLength; i++)
    {
        char current = *(string + i);
        *(string + i + offset) = current;
        switch (current)
        {
            case '\\':
                if (escapeCharacter) { escapeCharacter = 0; }
                else if (excludeEscapeCharacters) { escapeCharacter = 1; }
                break;
            case '\'':
                if (escapeCharacter) { escapeCharacter = 0; }
                else if (!excludeSingle) { offset--; }
                break;
            case '\"':
                if (escapeCharacter) { escapeCharacter = 0; }
                else if (!excludeDouble) { offset--; }
                break;
            default:
                if (escapeCharacter) { escapeCharacter = 0; }
                break;
        }
    }
    return -offset;
}

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
            if (suspects[c]) { suspects[c] = (current == *(commands + (commandLength * c) + suspectI)); if (!suspects[c]) { suspectCount--; } }
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

int decodeArgs(char* input, int inputLength, char* tokens, int command, char* argumentFormats, int commandCount, int argumentCount, void* data, int argumentSize, char cosmetic) // decodes and formats tokens according to command formatting
{
    // load command argument formatting
    char formats[argumentCount];
    for (int i = 0; i < argumentCount; i++) { formats[i] = *(argumentFormats + (argumentCount * command) + i); }

    //printf("Command: %d\n", command);
    //printf("Arguments: ");
    //for (int i = 0; i < argumentCount; i++) { printf("%c ", formats[i]); }
    //printf("\n");

    // check if any format present
    if (!formats[0]) { return 0; }
    int arguments = 0;
    for (int i = 0; i < argumentCount; i++) { if (formats[i]) { arguments++; } }

    // clear output data
    for (int i = 0; i < (argumentCount * argumentSize) - 1; i++) { *((char*)data + i) = (cosmetic ? ' ' : '\0'); } // jābūt \0
    *((char*)data + (argumentCount * argumentSize) - 1) = '\0';

    // load arguments
    int arg = 0;
    int offset = 0;
    int newArg = 1;
    for (int i = 0; i < inputLength; i++)
    {
        if (*(tokens + i) > 1 + (48 * cosmetic)) { if (offset < argumentSize) { *((char*)data + (argumentSize * arg) + offset) = *(input + i); offset++; } newArg = 0; }
        else if (cosmetic && (*(tokens + i) == ' ')) { if (!newArg) { arg++; offset = 0; newArg = 1; } }
        else if (*(tokens + i) == 0) { if (!newArg) { arg++; offset = 0; newArg = 1; } }
        if (arg == arguments) { break; }
    }
    //printf("Args: '%s'\n", (char*)data);

    // parse arguments
    //printf("Num args: ");
    for (int i = 0; i < argumentCount; i++)
    {
        if (formats[i] == 'n')
        {
            int num = parseIntFromString((char*)data + (argumentSize * i), argumentSize); // average parsing implementation
            //printf("%dth num: %d\n", i, num);

            *((int*)data + ((argumentSize >> 2) * i)) = num; // trust me bro
            
            //for (int n = 0; n < argumentSize; n++) { printf("%d ", *((char*)data + (argumentSize * i) + n)); }
            //printf("%d ", *((int*)data + ((argumentSize >> 2) * i)));
        }
        //printf("\n");
    }

    return 0;
}

int runCommand(int command, int commandCount, int argumentCount, void* commandFunctionPointers, void* data, int argumentSize, char debugmode) // runs given command and arguments - large switch statement and also the only thing keeping me from making the console system fully modular.
{
    if (command >= commandCount) { if (debugmode) { printf("Warning: Command ID '%d' of '%d' out of bounds!", command, commandCount); } return -1; }
    ((int, void*, int)(commandFunctionPointers + command))(command, data, argumentCount * argumentSize);
    // void CONSOLE_COMMAND_FUNCTION_PROTOTYPE(int COMMAND_ID, void* ARGS_DATA, int DATA_SIZE)
    return 0;
}
