#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<windows.h>

/*
    COMPLETE REWRITE URGENTLY REQUIRED
*/

int running = 1;

char lastCommand[128] = "\0";
char command[33] = "\0";
char arguments[4][33] = { "\0", "\0", "\0", "\0" };

const char instructionList[46][16] = // Instruction string list
{
    "nop", "halt", 
    "ioportsetr", "ioportget", "ioportseti", "in", "out", 
    "send", "desend", "receive", "sportsetr", "sportseti", "sout", "sin", "buffersetr", "bufferget", "bufferseti", 
    "irenable", "irdisable", "irsource", "irreturn", "irset", "irget", "irout", 
    "save", "load", "gaset", "gaget", "oaset", "oaget", "mov", "oaadd", "imm", 
    "fpmove", "fpset", "fpget", "spset", "spget", "frset", "frget", "push", "pop", "call", "return", 
    "jmp", "alu"
};
const unsigned char instructionCodes[46] = // Instruction offsets for byte encoding
{
    0, 1,
    2, 3, 8, 4, 6,
    16, 17, 18, 19, 20, 22, 24, 26, 27, 32,
    28, 29, 30, 31, 48, 49, 50,
    56, 60, 64, 82, 100, 118, 64, 128, 192,
    51, 52, 53, 54, 55, 224, 225, 226, 228, 230, 231,
    232, 240,
};
const unsigned char instructionCodesArranged[46] = // Instruction offsets for byte decoding
{
    0, 1,
    2, 3, 4, 6, 8, 
    16, 17, 18, 19, 20, 22, 24, 26, 27, 
    28, 29, 30, 31, 32, 48, 49, 50, 51, 52, 53, 54, 55, 
    56, 60, 64, 64, 82, 100, 118, 128, 192,
    224, 225, 226, 228, 230, 231,
    232, 240,
};
const int instructionArgs[46] = // One's place is the number of arguments, ten's and hundred's places are the types of arguments for each instruction
{
    0, 0,
    0, 0, 91, 21, 21,
    0, 0, 0, 0, 91, 21, 21, 0, 0, 91,
    0, 0, 0, 0, 0, 0, 0,
    31, 31, 41, 41, 41, 41, 652, 91, 792,
    0, 0, 0, 0, 0, 0, 0, 81, 81, 0, 0,
    01, 11
};
const int instructionArgImmSize[46] = // Instruction Imm argument bounds 
{
    0, 0, 
    0, 0, 3, 0, 0, 
    0, 0, 0, 0, 1, 0, 0, 0, 0, 4,
    0, 0, 0, 0, 0, 0, 0, 
    0, 0, 0, 0, 0, 0, 0, 6, 4,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0
};
const char instructionArgCodeScalars[10] = { 1, 1, 1, 1, 1, 1, 8, 16, 9, 1 }; // Instruction argument type scalars for byte encoding
const char instructionArgList[9][16][8] = // Instruction argument type string list
{
    { { "\0" }, { "\0" }, { "z" }, { "nz" }, { "s" }, { "ns" }, { "reg" }, { "nreg" } },
    { { "and" }, { "or" }, { "xor" }, { "add" }, { "sub" }, { "cmp" }, { "not" }, { "incr" }, { "shlr" }, { "shl1" }, { "shl2" }, { "shl4" }, { "shrr" }, { "shr1" }, { "shr2" }, { "shr4" } },
    { { "c" }, { "d" } },
    { { "a" }, { "b" }, { "c" }, { "d" } },
    { { "e" }, { "f" } },
    { { "a" }, { "b" }, { "c" }, { "d" }, { "e" }, { "f" }, { "g" }, { "h" } },
    { { "a" }, { "b" }, { "c" }, { "d" }, { "e" }, { "f" }, { "g" }, { "h" } },
    { { "a" }, { "b" } },
    { { "e" }, { "f" } }
};
const int instructionArgSuspectCount[9] = { 8, 16, 2, 4, 2, 8, 8, 2, 2 }; // Instruction argument type suspect count 
/*
    0 - JMP
    1 - ALU
    2 - CD
    3 - AD
    4 - EF
    5 - AH
    6 - AH offset
    7 - AB offset
    8 - Double EF

    9 - IMM
*/


const char commandList[26][32] =
{
    "backup", "exit", "help", "load", "print", "run", "save", "status", "step",
    "ram", "reg", "general", "pc", "lpc", "fr", "ga", "oa", "sp", "fp", "irs",
    "ird", "iop", "sbp", "csp", "assemble", "sus"
};
const int commandCount = 26;
const char charset[65] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ #=+-*/#_<>##?!'^v.,:;()[]{}?";
const char allofAscii[95] = " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";


unsigned char ram[256];
unsigned char regG[8] = { 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char regPC = 0;
unsigned char regLPC = 0;
unsigned char regFR = 0;
unsigned char regGA = 0;
unsigned char regOA = 0;
unsigned char regSP = 0;
unsigned char regFP = 0;
unsigned char regIRS = 0;
unsigned char regIRD[8] = { 0, 0, 0, 0, 0, 0, 0, 0};
unsigned char regIOP = 0;
unsigned char regSBP = 0;
unsigned char regCSP = 0;

int emulatorRunning = 0;
unsigned char lastInstruction = 0;
int currentArgumentAVal = 0;
int currentArgumentBVal = 0;
int stepVal = 0;

int emulatorInstructionIDer(unsigned char instruction) // so so la la Implementation
{
    unsigned char suspected = 0;
    int suspect = 0;

    for (int i = 0; i < 47; i++) // find base value of instruction
    {
        if (i == 46) // special case for "alu" instruction
        {
            if (instruction < 256) { suspected = instructionCodesArranged[i - 1]; }
            break;
        }
        if (instruction == instructionCodesArranged[i]) { suspected = instruction; break; }
        else if (instruction < instructionCodesArranged[i]) { suspected = instructionCodesArranged[i - 1]; break; }
    }
    //printf("\n\tsuspected %d", suspected);

    for (int i = 0; i < 46; i++) // find instruction id from suspected base
    {
        if (instructionCodes[i] == suspected) { suspect = i; /*printf("\n%d '%s'", suspect, instructionList[suspect]);*/ return i; }
    }
    //printf("\n\tsuspect not found", suspect, instructionList[suspect]);
    return -1;
}
int emulatorArgumentDecoder(int rawArgs, int* instructionID, int* args, int* argType) // kys
{
    int argCount = instructionArgs[*instructionID] % 10;

    // calculate argument types
    *argType = ((instructionArgs[*instructionID] % 100) - argCount) / 10;
    *(argType + 1) = ((instructionArgs[*instructionID] % 1000) - (*argType + argCount)) / 100;

    if (!argCount) { *args = 0; *(args + 1) = 0; return 0; } // no arguments

    else if ((*instructionID >= 26) && (*instructionID < 30)) // sort through mov case
    {
        if (!(rawArgs % 9)) // gaset, gaget, oaset, oaget
        {
            //printf(" args %d", rawArgs);
            //printf(" type %d,", *argType);
            *args = (rawArgs % 18) / 9;
        }
        else // mov
        {
            // reformat arguments
            rawArgs += instructionCodes[*instructionID] - 64;
            *instructionID = 30;
            //printf(" mov args %d", rawArgs);

            // recalculate argument types
            argCount = instructionArgs[*instructionID] % 10;
            *argType = ((instructionArgs[*instructionID] % 100) - argCount) / 10;
            *(argType + 1) = ((instructionArgs[*instructionID] % 1000) - *argType - argCount) / 100;

            *args = rawArgs % instructionArgCodeScalars[*(argType + 1)]; // filter out first argument
            //printf(" types %d %d,  scal %d.", *(argType + 1), *argType, instructionArgCodeScalars[*(argType + 1)]);
            *(args + 1) = (rawArgs - *args) / instructionArgCodeScalars[*(argType + 1)]; // filter out second argument
        }
        return 0;
    }
    else if (argCount > 1) // argument count is 2
    {
        //printf(" args %d", rawArgs);
        *args = rawArgs % instructionArgCodeScalars[*(argType + 1)]; // filter out first argument
        //printf(" types %d %d,  scal %d.", *(argType + 1), *argType, instructionArgCodeScalars[*(argType + 1)]);
        *(args + 1) = (rawArgs - *args) / instructionArgCodeScalars[*(argType + 1)]; // filter out second argument
        return 0;
    }
    else // argument count is 1
    {
        //printf(" args %d", rawArgs);
        //printf(" type %d,", *argType);
        *args = rawArgs; // first and only argument
        return 0;
    }
    //mov may need assembler argument switching
    //imm not assembling

    return -1;
}


char decodeChar(char sus) { return charset[sus]; }
char encodeChar(char ascii)
{
    if ((ascii >= 48) && (ascii < 58)) { printf("\n\t- %d '%c'->'%c' %d", ascii, ascii, decodeChar(ascii - 48), ascii - 48); return ascii - 48; }
    else if ((ascii >= 97) && (ascii < 123) && (ascii != 118)) { printf("\n\t- %d '%c'->'%c' %d", ascii, ascii, decodeChar(ascii - 87), ascii - 87); return ascii - 87; }
    else if ((ascii >= 65) && (ascii < 91)) { printf("\n\t- %d '%c'->'%c' %d", ascii, ascii, decodeChar(ascii - 55), ascii - 55); return ascii - 55; }
    else
    {
        switch (ascii)
        {
        case 32:
            printf("\n\t- %d '%c'->' ' 36", ascii, ascii);
            return 36; // 
        case 35:
            printf("\n\t- %d '%c'->'#' 37", ascii, ascii);
            return 37; // #
        case 61:
            printf("\n\t- %d '%c'->'=' 38", ascii, ascii);
            return 38; // =
        case 43:
            printf("\n\t- %d '%c'->'+' 39", ascii, ascii);
            return 39; // +
        case 45:
            printf("\n\t- %d '%c'->'-' 40", ascii, ascii);
            return 40; // -
        case 42:
            printf("\n\t- %d '%c'->'*' 41", ascii, ascii);
            return 41; // *
        case 47:
            printf("\n\t- %d '%c'->'/' 42", ascii, ascii);
            return 42; // /
        case 95:
            printf("\n\t- %d '%c'->'_' 44", ascii, ascii);
            return 44; // _
        case 60:
            printf("\n\t- %d '%c'->'<' 45", ascii, ascii);
            return 45; // <
        case 62:
            printf("\n\t- %d '%c'->'>' 46", ascii, ascii);
            return 46; // >
        case 63:
            printf("\n\t- %d '%c'->'?' 49", ascii, ascii);
            return 49; // ?
        case 33:
            printf("\n\t- %d '%c'->'!' 50", ascii, ascii);
            return 50; // !
        case 39:
            printf("\n\t- %d '%c'->''' 51", ascii, ascii);
            return 51; // '
        case 94:
            printf("\n\t- %d '%c'->'^' 52", ascii, ascii);
            return 52; // ^
        case 118:
            printf("\n\t- %d '%c'->'v' 53", ascii, ascii);
            return 53; // v
        case 46:
            printf("\n\t- %d '%c'->'.' 54", ascii, ascii);
            return 54; // .
        case 44:
            printf("\n\t- %d '%c'->',' 55", ascii, ascii);
            return 55; // ,
        case 58:
            printf("\n\t- %d '%c'->':' 56", ascii, ascii);
            return 56; // :
        case 59:
            printf("\n\t- %d '%c'->';' 57", ascii, ascii);
            return 57; // ;
        case 40:
            printf("\n\t- %d '%c'->'(' 58", ascii, ascii);
            return 58; // (
        case 41:
            printf("\n\t- %d '%c'->')' 59", ascii, ascii);
            return 59; // )
        case 91:
            printf("\n\t- %d '%c'->'[' 60", ascii, ascii);
            return 60; // [
        case 93:
            printf("\n\t- %d '%c'->']' 61", ascii, ascii);
            return 61; // ]
        case 123:
            printf("\n\t- %d '%c'->'{' 62", ascii, ascii);
            return 62; // {
        case 125:
            printf("\n\t- %d '%c'->'}' 63", ascii, ascii);
            return 63; // }
        default:
            printf("\n\t- %d '%c'->'' ", ascii, ascii);
            return 64;
        }
    }
}

int compareString(char stringA[32], char stringB[32])
{
    for (int i = 0; i < 32; i++)
    {
        if (stringA[i] != stringB[i]) { return 0; }
        if (stringA[i] == '\0') { return 1; }
    }
    return 1;
}
int stringToIntParserOld(char token[32])
{
    // -48
    int power = 0;
    int num = 0;
    for (int i = 0; i < 31; i++)
    {
        if ((token[31 - i] >= 48) && (token[31 - i] < 59))
        {
            num += (token[31 - i] - 48) * pow(10, power);
            power++;
        }
    }
    return num;
}
int stringToIntParser(char token[32])
{
    int number = 0;
    int power = 0;
    int negative;
    int base = 10; // 0b - 2; 0d - 10; 0x - 16
    int offset = 0;

    negative = (token[0] == '-'); // determine number's sign
    //if (negative) { printf("\n\t Number is negative"); }
    //else { printf("\n\t Number is positive"); }
    offset += negative;
    if (token[0 + negative] == '0')
    {
        offset += 2;
        char baseComparisonChar = token[1 + negative];
        if (baseComparisonChar == 'b') { base = 2; }
        else if (baseComparisonChar == 'd') { base = 10; }
        else if (baseComparisonChar == 'x') { base = 16; }
        else if ((baseComparisonChar >= 48) && (baseComparisonChar < 58)) { offset -= 2; }
        else if ((baseComparisonChar != '\0') && (baseComparisonChar != '\n')) { printf("\n\tIncorrect base represenation for '%s'", token); return -1; } // return if incorrect base represenation
    }
    //printf("\n\t Base is %d", base);

    for (int i = 0; i < 32 - offset; i++)
    {
        int susChar = token[31 - i];
        int digit = 0;

        if ((susChar >= 48) && (susChar < 58))
        {
            digit = susChar - 48;
            if ((base == 2) && (susChar >= 50)) { printf("\n\tIncorrect number represenation for '%s'", token); return -1; } // return if incorrect number represenation
        }
        else if ((base == 16) && (susChar >= 97) && (susChar < 103))
        {
            digit = susChar - 87;
        }
        else if ((susChar == '\0') || (susChar == '\n'))
        {
            power--;
        }
        else { printf("\n\tSuspicious character '%c' for '%s'", susChar, token); return -1; } // return if suspicious character present
        number += digit * pow(base, power);
        power++;
    }
    if (negative) { number = -number; }
    //printf("\n\t Found immediate %d base %d from '%s'", number, base, token);
    return number;
}

int assemblerParser(char* inputString, char* outputStringArray) // outputStringArray requires 32B minimum allocated space
{
    int len = 0;
    int ies = 0;
    
    int argCount;
    int currentArg = 0;
    int lookingForArgs = 0;

    int suspects[46];
    for (int i = 0; i < 46; i++ ) { suspects[i] = 1; }
    int susCount = 46;
    int suspect = -1;

    //printf("\nSTRING PARSING BEGINS '%s'", inputString);

    // ready output array
    *(outputStringArray + 15) = '\n';
    *(outputStringArray + 31) = '\n';

    int i = 0;
    while (1)
    {
        char susChar = *(inputString + i); // current char for parsing
        //printf("\n Parsing %d '%c'", susChar, susChar);
        if ((susChar == '\n') || (susChar == '\0') || (susChar == ' ')) // check char against whitespace and newline/nil
        {
            if ((*(outputStringArray + 15 + (currentArg * 16)) == '\n') && lookingForArgs) { /*printf("\n\t-y len %d; currentArg %d; offset %d", len, currentArg, currentArg * 16);*/ for (int n = len; n < 16; n++) { *(outputStringArray + n + (currentArg * 16)) = '\0'; } }
            currentArg++; // WIP finish up argument
            if (susCount == 1) // check if suspect was found
            {
                for (int s = 0; s < 46; s++) { if(suspects[s]) { suspect = s; break; } } // get suspect id
                //printf("\n\t- Suspect is %d - %s", suspect, instructionList[suspect]);
                argCount = instructionArgs[suspect] % 10; // load suspect's argument count
                currentArg = 0;
                lookingForArgs = 1;
                //printf("\n\t- argCount %d", argCount);
                susCount = -2;
            }
            //else { printf("\n\t- Found %d suspects", susCount); }
            ies += len + 1;
            len = 0;
            if (susChar != ' ') { /*printf("\n NEXT");*/ break; } // if char not whitespace then break loop
        }
        else
        {
            if (!(suspect + 1)) // check if no suspect was already found
            {
                for (int s = 0; s < 46; s++)
                {
                    if (suspects[s]) { suspects[s] = (susChar == instructionList[s][i]); if (!suspects[s]) { susCount--; } } // compare char against suspect list
                }
                if (!susCount) { /*printf("\n\t- Found nothing, ended at %d", i);*/ break; }
            }
            else // if suspect found then get arguments
            {
                //printf("\n\t- argCount %d; currentArg %d; total %d", argCount, currentArg, argCount - (currentArg + 1));
                if (!(argCount - currentArg)) { break; } // no arguments required
                //printf("\n\t- ies %d; i %d; offset %d", ies, i, i - ies + (currentArg * 16));
                *(outputStringArray + i - ies + (currentArg * 16)) = susChar;
            }
            len++;
        }
        i++;
    }
    //printf("\nSTRING PARSING ENDED");
    return suspect;
}
int assemblerArgIDer(char argString[32], int argCount, int argType[2], char* argByteOutput, int suspectInstructionID)
{
    printf("\n-assemblerArgIDer('");
    for (int i = 0; i < 32; i++) { printf("%c", argString[i]); }
    printf("', %d, { %d, %d }, %d,)", argCount, argType[0], argType[1], *argByteOutput);
    int suspect = -1;
    char currentArgument[16];
    for (int a = 0; a < 16; a++) { currentArgument[a] = argString[a]; }
    for (int arg = 0; arg < argCount; arg++)
    {
        if (arg > 0) { for (int a = 0; a < 16; a++) { currentArgument[a] = argString[a + 16]; } }
        printf("\n Start of %d, currentArg '%s'", arg, currentArgument);
        printf("\n CurrentArg :"); for (int a = 0; a < 16; a++) { printf(" %d", currentArgument[a]); }

        if (argType[arg] != 9) // check if argument type isn't of a number
        {
            int suspectsA[instructionArgSuspectCount[argType[arg]]]; // define list of suspects for argument with given argument type of corresponding length
            for (int i = 0; i < instructionArgSuspectCount[argType[arg]]; i++ ) { suspectsA[i] = 1; } // fill list of suspects for argument with true
            for (int i = 0; i < 16; i++) // loop through currentArgument
            {
                for (int s = 0; s < instructionArgSuspectCount[argType[arg]]; s++) // checking against possible suspects from suspect list
                {
                    if (suspectsA[s]) { suspectsA[s] = (currentArgument[i] == instructionArgList[argType[arg]][s][i]); } // eliminating irrevelant suspects from list
                }
                if (currentArgument[i] == '\0') { break; } // stopping loop when end of argument reached
            }

            for (int s = 0; s < instructionArgSuspectCount[argType[0]]; s++)
            {
                if (suspectsA[s]) { printf("\n\t Found A %d : %s ", s, instructionArgList[argType[arg]][s]); suspect = s; } // find first suspect from narrowed list
            }
            if (!(suspect + 1))
            {
                printf("\n\t%d Suspiciously failed to find suspects from", arg);
                for (int sussies = 0; sussies < instructionArgSuspectCount[argType[arg]]; sussies++)
                {
                    printf("\n- '");
                    for (int a = 0; a < 16; a++) { printf(" %d", instructionArgList[argType[arg]][sussies][a]); }
                    printf("'");
                }
                return -1;
            } // return if failed to find suspect
            *argByteOutput += suspect * instructionArgCodeScalars[argType[arg]]; // accumulate scaled suspect ID
        }
        else
        {
            char number = 0;
            int power = 0;
            int negative;
            int base = 10; // 0b - 2; 0d - 10; 0x - 16
            int offset = 0;

            negative = (currentArgument[0] == '-'); // determine number's sign
            //if (negative) { printf("\n\t Number 1 is negative"); }
            //else { printf("\n\t Number 1 is positive"); }
            offset += negative;
            if (currentArgument[0 + negative] == '0')
            {
                offset += 2;
                char baseComparisonChar = currentArgument[1 + negative];
                if (baseComparisonChar == 'b') { base = 2; }
                else if (baseComparisonChar == 'd') { base = 10; }
                else if (baseComparisonChar == 'x') { base = 16; }
                else if ((baseComparisonChar >= 48) && (baseComparisonChar < 58)) { offset -= 2; }
                else if ((baseComparisonChar != '\0') && (baseComparisonChar != '\n')) { printf("\n\tIncorrect base represenation for '%s'", baseComparisonChar); return -1; } // return if incorrect base represenation
            }
            //printf("\n\t Base 1 is %d", base);

            for (int i = 0; i < 16 - offset; i++)
            {
                int susChar = currentArgument[15 - i];
                int digit = 0;

                if ((susChar >= 48) && (susChar < 58))
                {
                    digit = susChar - 48;
                    if ((base == 2) && (susChar >= 50)) { printf("\n\t%d Incorrect number represenation", arg); return -1; } // return if incorrect number represenation
                }
                else if ((base == 16) && (susChar >= 97) && (susChar < 103))
                {
                    digit = susChar - 87;
                }
                else if ((susChar == '\0') || (susChar == '\n'))
                {
                    power--;
                }
                else { printf("\n\t%d Suspicious character '%c'", arg, susChar); return -1; } // return if suspicious character present
                number += digit * pow(base, power);
                power++;
            }
            if (negative) { number = -number; }
            //printf("\n\t Found 1 immediate %d base %d from '%s'", number, base, currentArgument);

            // force number to fit within the given immediate size
            int immSize = instructionArgImmSize[suspectInstructionID];
            printf("\n\t Original %d, size %d, 1 - ", number, immSize);
            unsigned char outNum = number; // cast to unsigned
            printf("%d, 2 - ", outNum);
            outNum = outNum << (8 - immSize); // discard unneeded bits
            printf("%d, 3 - ", outNum);
            outNum = outNum >> (8 - immSize);
            printf("%d", outNum);

            // accumulate cast
            *argByteOutput += outNum;
        }
    }
    return 0;
}

void actionBackup(int type, char* filepath) // type - save / load
{
    FILE* fileP;
    if (type) { fileP = fopen(filepath, "rb"); }
    else { fileP = fopen(filepath, "wb"); }

    char fileContents[256 + 8 + 8 + 11];
    if (fileP != NULL)
    {
        if (type)
        {
            fread(fileContents, 283, 1, fileP);
            for (int i = 0; i < 256 + 8 + 8; i++)
            {
                if ((i >= 0) && (i < 256)) { ram[i] = fileContents[i]; }
                if ((i >= 256) && (i < 264)) { regG[i - 256] = fileContents[i]; }
                if ((i >= 264) && (i < 272)) { regIRD[i - 264] = fileContents[i]; }
            }
            regPC = fileContents[272];
            regLPC = fileContents[273];
            regFR = fileContents[274];
            regGA = fileContents[275];
            regOA = fileContents[276];
            regSP = fileContents[277];
            regFP = fileContents[278];
            regIRS = fileContents[279];
            regIOP = fileContents[280];
            regSBP = fileContents[281];
            regCSP = fileContents[282];
        }
        else
        {
            for (int i = 0; i < 256 + 8 + 8; i++)
            {
                if ((i >= 0) && (i < 256)) { fileContents[i] = ram[i]; }
                if ((i >= 256) && (i < 264)) { fileContents[i] = regG[i - 256]; }
                if ((i >= 264) && (i < 272)) { fileContents[i] = regIRD[i - 264]; }
            }
            fileContents[272] = regPC;
            fileContents[273] = regLPC;
            fileContents[274] = regFR;
            fileContents[275] = regGA;
            fileContents[276] = regOA;
            fileContents[277] = regSP;
            fileContents[278] = regFP;
            fileContents[279] = regIRS;
            fileContents[280] = regIOP;
            fileContents[281] = regSBP;
            fileContents[282] = regCSP;
            fwrite(fileContents, 283, 1, fileP);
        }
    }
    fclose(fileP);
}
void actionExit()
{
    running = 0;
    printf("\nPROGRAM EXITED");
}
void actionHelp(int argumentID) // WIP
{

}
void actionLoad(char* filepath, int ramQuarter, int fileQuarter)
{
    printf("\n-actionLoad('%s', %d, %d )", filepath, ramQuarter, fileQuarter);
    FILE* fileP = fopen(filepath, "rb");
    char fileContents[256];
    int fileSize;
    if (fileP)
    {
        fseek(fileP, 0L, SEEK_END);
        fileSize = ftell(fileP);
        rewind(fileP);
        printf("\n Filesize of '%s' is %d", filepath, fileSize);

        if (fileSize < 256) { fread(fileContents, fileSize, 1, fileP); }
        else { fread(fileContents, 256, 1, fileP); }

        if (fileSize < (64 * fileQuarter) + 64)
        {
            for (int i = 0; i < 64; i++)
            {
                if (i < (fileSize % 64)) { ram[(64 * ramQuarter) + i] = fileContents[(64 * fileQuarter) + i]; }
                else { ram[(64 * ramQuarter) + i] = '\0'; }
            }
        }
        else
        {
            for (int i = 0; i < 64; i++)
            {
                ram[(64 * ramQuarter) + i] = fileContents[(64 * fileQuarter) + i];
            }
        }
        
        fclose(fileP);
    }
    else { printf("\nFailed to open file '%s' for reading", filepath); return; }
}
void actionPrintRAM()
{
    int start = 0; int end = 255;
    
    if ((arguments[1][31] != '\n') && (arguments[2][31] != '\n')) // start & end provided
    {
        printf("/nStart & end provided");
        start = stringToIntParser(arguments[1]);
        end = stringToIntParser(arguments[2]);
        if (start < 0) { start = 0; } if (start > 255) { start = 255; }
        if (end < 0) { end = 0; } if (end > 255) { end = 255; }
    }
    else if (arguments[1][31] != '\n') // start only provided and interpreted as end as well
    {
        printf("/nOnly start provided");
        start = stringToIntParser(arguments[1]);
        if (start < 0) { start = 0; } if (start > 255) { start = 255; }
        end = start;
    }

    int roundStart = (start - (start % 8)); int roundEnd = (end - (end % 8)); // rounding provided start & end values to full 8th's
    //printf("\n\t- start : %d - %d", start, roundStart);
    //printf("\n\t- end : %d - %d", end, roundEnd);
    printf("\nPrinting RAM at %d to %d.", roundStart, roundEnd);
    for (int i = roundStart / 8; i < (roundEnd / 8 + 1); i++) // loop through all provided full 8th's
    {
        int include = 1;
        int barrierStart = -1;
        int barrierEnd = -1;
        if (i == (roundStart / 8)) { include = 0; barrierStart = (start - roundStart); }
        if (i == (roundEnd / 8)) { barrierEnd = (end - roundEnd + 1); }
        if ((barrierStart > 0) || (barrierEnd > 0))
        {
            printf("\n\t%3d : ", 8 * i);
            for (int n = 0; n < 8; n++) // loop through the 8 values of current 8th with respect to start & end barriers
            {
                if ((n == barrierStart) || (n == barrierEnd)) { include = !include; }
                if (include) { printf(" %2x ", ram[8 * i + n] & 0xff); }
                else { printf(" -- "); }
            }
        }
        else
        {
            printf("\n\t%3d : ", 8 * i);
            for (int n = 0; n < 8; n++) // loop through the 8 values of current 8th
            {
                printf(" %2x ", ram[8 * i + n] & 0xff);
            }
        }
    }
}
void actionPrintReg(int regID)
{
    switch (regID)
    {
        case 11: // general
            printf("\n\tGeneral Registers : ");
            for (int i = 0; i < 8; i++) { printf(" %2x ", regG[i] & 0xff); }
            break;
        case 12: // pc
            printf("\n\tProgram Counter :  %2x", regPC & 0xff);
            break;
        case 13: // lpc
            printf("\n\tLast Program Counter :  %2x", regLPC & 0xff);
            break;
        case 14: // fr
            printf("\n\tFlag Register :  %2x", regFR & 0xff);
            break;
        case 15: // ga
            printf("\n\tGeneral Address :  %2x", regGA & 0xff);
            break;
        case 16: // oa
            printf("\n\tOffset Address :  %2x", regOA & 0xff);
            break;
        case 17: // sp
            printf("\n\tStack Pointer :  %2x", regSP & 0xff);
            break;
        case 18: // fp
            printf("\n\tFrame Pointer :  %2x", regFP & 0xff);
            break;
        case 19: // irs
            printf("\n\tInterrupt Source :  %2x", regIRS & 0xff);
            break;
        case 20: // ird
            printf("\n\tInterrupt Destination : ");
            for (int i = 0; i < 8; i++) { printf(" %2x ", regIRD[i] & 0xff); }
            break;
        case 21: // iop
            printf("\n\tIO Pointer :  %2x", regIOP & 0xff);
            break;
        case 22: // sbp
            printf("\n\tSerial Buffer Pointer :  %2x", regSBP & 0xff);
            break;
        case 23: // csp
            printf("\n\tCurrent Serial Pointer :  %2x", regCSP & 0xff);
            break;
        default:
            printf("\n\tGeneral Registers      : ");
            for (int i = 0; i < 8; i++) { printf(" %2x ", regG[i] & 0xff); }
            printf("\n\tProgram Counter        :  %2x", regPC & 0xff);
            printf("\n\tLast Program Counter   :  %2x", regLPC & 0xff);
            printf("\n\tFlag Register          :  %2x", regFR & 0xff);
            printf("\n\tGeneral Address        :  %2x", regGA & 0xff);
            printf("\n\tOffset Address         :  %2x", regOA & 0xff);
            printf("\n\tStack Pointer          :  %2x", regSP & 0xff);
            printf("\n\tFrame Pointer          :  %2x", regFP & 0xff);
            printf("\n\tInterrupt Source       :  %2x", regIRS & 0xff);
            printf("\n\tInterrupt Destination  : ");
            for (int i = 0; i < 8; i++) { printf(" %2x ", regIRD[i] & 0xff); }
            printf("\n\tIO Pointer             :  %2x", regIOP & 0xff);
            printf("\n\tSerial Buffer Pointer  :  %2x", regSBP & 0xff);
            printf("\n\tCurrent Serial Pointer :  %2x", regCSP & 0xff);
            break;
    }
}
void actionRun()
{
    for (int i = 0; i < stepVal; i++)
    {
        // fetch
        unsigned char currentInstruction = ram[i];
        //printf("\n\n\t%dnd Value %d", i, currentInstruction);

        // id instruction
        int currentInstructionID = emulatorInstructionIDer(currentInstruction);
        if (!(currentInstructionID + 1)) { continue; } // skip cycle if instruction could not be ID'd (impossible)

        // get instruction arguments
        int argumentIDs[2] = { -1, -1 };
        int argType[2];
        if (!(emulatorArgumentDecoder(currentInstruction - instructionCodes[currentInstructionID], &currentInstructionID, &argumentIDs[0], &argType[0]) + 1)) { continue; } // ID arguments, skip cycle if arguments could not be ID'd (impossible)
        
        // print current instruction
        printf("\n%s", instructionList[currentInstructionID]);
        if (argType[0] != -1)
        {
            if (argType[0] != 9)
            {
                if (argType[1] != -1)
                {
                    if (argType[1] != 9) { printf(" %s %s", instructionArgList[argType[1]][argumentIDs[1]], instructionArgList[argType[0]][argumentIDs[0]]); }
                    else { printf(" %d %s", argumentIDs[1], instructionArgList[argType[0]][argumentIDs[0]]); }
                }
                else { printf(" %s", instructionArgList[argType[0]][argumentIDs[0]]); }
            }
            else if (argType[1] != -1) { printf(" %s %d", instructionArgList[argType[1]][argumentIDs[1]], argumentIDs[0]); }
            else { printf(" %d", argumentIDs[0]); }
        }
        

        // execute
    }
}
void actionSave(char* filepath, int ramQuarter, int fileQuarter)
{
    printf("\n-actionSave('%s', %d, %d )", filepath, ramQuarter, fileQuarter);
    FILE* fileP = fopen(filepath, "rb");
    char fileContents[256];
    int fileSize;
    if (fileP)
    {
        fseek(fileP, 0L, SEEK_END);
        fileSize = ftell(fileP);
        rewind(fileP);
        printf("\n Filesize of '%s' is %d", filepath, fileSize);

        if (fileSize < 256) { fread(fileContents, fileSize, 1, fileP); }
        else { fread(fileContents, 256, 1, fileP); }

        if (fileSize < (64 * fileQuarter) + 64)
        {
            for (int i = 0; i < 64; i++)
            {
                if (i < (fileSize % 64)) { fileContents[(64 * fileQuarter) + i] = ram[(64 * ramQuarter) + i]; }
                else { fileContents[(64 * fileQuarter) + i] = '\0'; }
            }
        }
        else
        {
            for (int i = 0; i < 64; i++)
            {
                fileContents[(64 * fileQuarter) + i] = ram[(64 * ramQuarter) + i];
            }
        }

        fileP = fopen(filepath, "wb");
        if (fileP) { fwrite(fileContents, 256, 1, fileP); }
        else { printf("\nFailed to open file '%s' for writing", filepath); return; }
        fclose(fileP);
    }
    else { printf("\nFailed to open file '%s' for reading", filepath); return; }
}
void actionAssemble(int type, char* destinationFilepath, char* sourceFilepath) // type - sus / --
{
    unsigned char assembledFile[283];

    FILE* fileP = fopen(sourceFilepath, "r");
    if (fileP)
    {
        int lines = 0;
        for (int i = 0; i < 512; i++) // Read file line by line
        {
            if (lines >= 255) { break; }
            unsigned char assembledBytecode = 0;
            char line[64];
            if (!fgets(line, 64, fileP)) { break; }
            if (line[0] == '\n') { continue; }
            lines++;

            // Parse and ID instruction
            char outputString[32]; char outputPreString[32];
            int suspectInstructionID = assemblerParser(line, &outputString[0]);
        
            int argCount = -1;
            int argType[2] = { -1, -1 };

            unsigned char argOut = 0;

            if (suspectInstructionID + 1) // if instruction suscesfully parsed and ID'd
            {
                // Parse arguments
                argCount = instructionArgs[suspectInstructionID] % 10;
                argType[0] = ((instructionArgs[suspectInstructionID] % 100) - argCount) / 10;
                argType[1] = ((instructionArgs[suspectInstructionID] % 1000) - argType[0] - argCount) / 100;

                if (argCount == 2) // switch arguments if there are two
                {
                    for (int os = 0; os < 32; os++) { outputPreString[os] = outputString[os]; }
                    for (int os = 0; os < 16; os++) { outputString[16 + os] = outputPreString[os]; }
                    for (int os = 0; os < 16; os++) { outputString[os] = outputPreString[16 + os]; }
                    printf("\n Switcheroo performed '");
                    for (int os = 0; os < 32; os++) { printf("%c", outputPreString[os]); }
                    printf("' > '");
                    for (int os = 0; os < 32; os++) { printf("%c", outputString[os]); }
                    printf("'");
                }

                printf("\nLine '%s'", line);
                if (assemblerArgIDer(outputString, argCount, argType, &argOut, suspectInstructionID) + 1)
                {
                    //printf("\n\t- Succesfully IDd arguments for instruction '%s'", instructionList[suspectInstructionID]);
                    printf("\n\t- Calculated offset %d", argOut);
                    printf(" - %s instruction code %d", instructionList[suspectInstructionID], instructionCodes[suspectInstructionID]);
                    assembledBytecode = instructionCodes[suspectInstructionID] + argOut;
                    //printf("\n\n\t- Assembled byte %d", assembledBytecode);
                    assembledFile[lines - 1] = assembledBytecode;
                    printf("\n\n\tAssembled byte %d at %d", assembledFile[lines - 1], lines);
                }
                else { assembledFile[lines - 1] = '\0'; continue; }
            }
            else { assembledFile[lines - 1] = '\0'; continue; }

            // Print results
            /*printf("\n\nInstruction %d '%s'", suspectInstructionID, instructionList[suspectInstructionID]);
            if (argCount > 0)
            {
                printf(" with arguments '");
                for (int p = 0; p < 16; p++) { printf("%c", outputString[p]); }
                if (argCount > 1)
                {
                    printf("' '");
                    for (int p = 0; p < 16; p++) { printf("%c", outputString[p + 16]); }
                }
                printf("'");
            }
            printf("\nInstruction argCount %d of types %d, %d", argCount, argType[0], argType[1]);*/
        }
        printf("\nLines: %d", lines);
        for (int i = lines; i < 283; i++) { assembledFile[i] = '\0'; }
    }
    else { printf("\nFailed to load sourceFilepath '%s'", sourceFilepath); return; }
    fclose(fileP);

    fileP = fopen(destinationFilepath, "wb");
    if (fileP != NULL)
    {
        printf("\nASSEMBLED FILE '");
        for (int i = 0; i < 256; i++) { printf(" %d ", assembledFile[i]); }
        printf("'");
        
        fwrite(assembledFile, 283, 1, fileP);
    }
    else { printf("\nFailed to load destinationFilepath '%s'", destinationFilepath); return;  }
    fclose(fileP);

    // Write to destination
}

void commandParser()
{
    int quote = 0;
    int wasQuote = 0;
    int len = 0;
    int ies = 0;
    //printf("\n\t BEGIN STRING ANALYSING WITH '%s'", lastCommand);
    for (int i = 0; i < 128; i++)
    {
        //if (lastCommand[i] != '\0') { printf("\n\t %d '%c'", i, lastCommand[i]); }
        if ((lastCommand[i] >= 48) && (lastCommand[i] < 58) && !quote) // all numbers
        {
            wasQuote = 0;
            if (command[31] == '\n') { command[i - ies] = lastCommand[i]; len++; } //printf(" - '%c' command", lastCommand[i]); }
            else if (arguments[0][31] == '\n') { arguments[0][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 0", lastCommand[i]); }
            else if (arguments[1][31] == '\n') { arguments[1][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 1", lastCommand[i]); }
            else if (arguments[2][31] == '\n') { arguments[2][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 2", lastCommand[i]); }
            else if (arguments[3][31] == '\n') { arguments[3][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 3", lastCommand[i]); }
            else { break; }
        }
        else if ((lastCommand[i] >= 97) && (lastCommand[i] < 123) && !quote) // all lowercase letters
        {
            wasQuote = 0;
            if (command[31] == '\n') { command[i - ies] = lastCommand[i]; len++; } //printf(" - '%c' command", lastCommand[i]); }
            else if (arguments[0][31] == '\n') { arguments[0][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 0", lastCommand[i]); }
            else if (arguments[1][31] == '\n') { arguments[1][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 1", lastCommand[i]); }
            else if (arguments[2][31] == '\n') { arguments[2][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 2", lastCommand[i]); }
            else if (arguments[3][31] == '\n') { arguments[3][i - ies] = lastCommand[i]; len++; } //printf(" - '%c' argument 3", lastCommand[i]); }
            else { break; }
        }
        else if ((lastCommand[i] >= 65) && (lastCommand[i] < 91) && !quote) // all uppercase letters
        {
            wasQuote = 0;
            if (command[31] == '\n') { command[i - ies] = lastCommand[i] + 32; len++; } //printf(" - '%c' command", lastCommand[i]); }
            else if (arguments[0][31] == '\n') { arguments[0][i - ies] = lastCommand[i] + 32; len++; } //printf(" - '%c' argument 0", lastCommand[i]); }
            else if (arguments[1][31] == '\n') { arguments[1][i - ies] = lastCommand[i] + 32; len++; } //printf(" - '%c' argument 1", lastCommand[i]); }
            else if (arguments[2][31] == '\n') { arguments[2][i - ies] = lastCommand[i] + 32; len++; } //printf(" - '%c' argument 2", lastCommand[i]); }
            else if (arguments[3][31] == '\n') { arguments[3][i - ies] = lastCommand[i] + 32; len++; } //printf(" - '%c' argument 3", lastCommand[i]); }
            else { break; }
        }
        else if (lastCommand[i] == '"') // find quotation
        {
            wasQuote = 0;
            quote = !quote;
            len++;
            //printf("\n\t-- len %d; ies %d", len, ies);
            if (!quote)
            {
                len -= 2;
                if (command[31] == '\n') { for (int n = len; n < 33; n++) { command[n] = '\0'; } }//printf("\n\t- Concluded command as '%s'", command); }
                else if (arguments[0][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[0][n] = '\0'; }}//printf(" %d;", n); } printf("'\n\t- Concluded argument 0 as '%s' '%c'", arguments[0], arguments[0][15]); }
                else if (arguments[1][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[1][n] = '\0'; }}//printf(" %d;", n); } printf("'\t- Concluded argument 1 as '%s' '%c'", arguments[1], arguments[1][15]); }
                else if (arguments[2][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[2][n] = '\0'; }}//printf(" %d;", n); } printf("'\t- Concluded argument 2 as '%s' '%c'", arguments[2], arguments[2][15]); }
                else if (arguments[3][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[3][n] = '\0'; }}//printf(" %d;", n); } printf("'\n\t- Concluded argument 3 as '%s' '%c'", arguments[3], arguments[3][15]); }
                len += 2;
                //printf(" - %d", len);
                ies += len;
                len = 0;
                wasQuote = 1;
                //printf("\n\tQuote finish");
                //printf("\n\t-q len %d; ies %d", len, ies);
            }
            //if (quote) { printf("\n\t-- Quote starts at %d", i); }
            //else { printf("\n\t-- Quote ends at %d", i); }
        }
        else if (quote) // read quotation
        {
            wasQuote = 0;
            if (command[31] == '\n') { command[i - ies - 1] = lastCommand[i]; len++; } //printf(" - '%c' quoted to command", lastCommand[i]); }
            else if (arguments[0][31] == '\n') { arguments[0][i - ies - 1] = lastCommand[i]; len++; } //printf(" - '%c' quoted to argument 0", lastCommand[i]); }
            else if (arguments[1][31] == '\n') { arguments[1][i - ies - 1] = lastCommand[i]; len++; } //printf(" - '%c' quoted to argument 1", lastCommand[i]); }
            else if (arguments[2][31] == '\n') { arguments[2][i - ies - 1] = lastCommand[i]; len++; } //printf(" - '%c' quoted to argument 2", lastCommand[i]); }
            else if (arguments[3][31] == '\n') { arguments[3][i - ies - 1] = lastCommand[i]; len++; } //printf(" - '%c' quoted to argument 3", lastCommand[i]); }
            else { break; }
            //printf(" - in quote");
        }
        else if ((lastCommand[i] == ' ') || (lastCommand[i] == '\n') && !quote) // whitespace and newline
        {
            if (!wasQuote)
            {
                //printf("\n\tIn whitespace");
                wasQuote = 0;
                if (command[31] == '\n') { for (int n = len; n < 33; n++) { command[n] = '\0'; }  }//printf("\n\t- Concluded command as '%s'", command); }
                else if (arguments[0][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[0][n] = '\0'; }}//printf(" %d;", n); } printf("'\n\t- Concluded argument 0 as '%s' '%c'", arguments[0], arguments[0][15]); }
                else if (arguments[1][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[1][n] = '\0'; }}//printf(" %d;", n); } printf("'\n\t- Concluded argument 1 as '%s' '%c'", arguments[1], arguments[1][15]); }
                else if (arguments[2][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[2][n] = '\0'; }}//printf(" %d;", n); } printf("'\n\t- Concluded argument 2 as '%s' '%c'", arguments[2], arguments[2][15]); }
                else if (arguments[3][31] == '\n') { /*printf("\n\t- '");*/ for (int n = len; n < 33; n++) { arguments[3][n] = '\0'; }}//printf(" %d;", n); } printf("'\n\t- Concluded argument 3 as '%s' '%c'", arguments[3], arguments[3][15]); }
            }
            //printf(" - %d", len);
            ies += len + 1;
            len = 0;
            //printf("\n\tWhitespace finish");
            //printf("\n\t-- len %d; ies %d", len, ies);
        }
        else { wasQuote = 0; len++; }
    }
    //printf("\n\t STRING ANALYSED");
    printf("\n\t lastCommand : '%s'", lastCommand);
    printf("\n\t command : '%32s'", command);
    printf(" '%32s'", arguments[0]); if (arguments[0][31] == '\n') { printf("+/n"); }
    printf(" '%32s'", arguments[1]); if (arguments[1][31] == '\n') { printf("+/n"); }
    printf(" '%32s'", arguments[2]); if (arguments[2][31] == '\n') { printf("+/n"); }
    printf(" '%32s'", arguments[3]); if (arguments[3][31] == '\n') { printf("+/n"); }
}
int commandIDer(char token[32])
{
    int suspects[commandCount];
    for (int i = 0; i < commandCount; i++ ) { suspects[i] = 1; }
    for (int i = 0; i < 32; i++)
    {
        for (int s = 0; s < commandCount; s++)
        {
            if (suspects[s]) { suspects[s] = (token[i] == commandList[s][i]); }
        }
        if (token[i] == '\0') { break; }
    }

    for (int s = 0; s < commandCount; s++)
    {
        if (suspects[s]) { printf("  %32d ", s, commandList[s]); return s; }
    }
    printf("  %16d ", -1);
    return -1;
}
void executeCommand(int commandID, int argumentID[4])
{
    switch (commandID)
    {
        case 0: // backup
            printf("\n\t- backup command");
            if ((arguments[0][31] != '\n') && (arguments[1][31] != '\n'))
            {
                char* filepathP = &arguments[1][0]; // set filepath's pointer to 1th argument
                if (argumentID[0] == 6) { actionBackup(0, filepathP); } // call action backup with type save and given filepath
                else if (argumentID[0] == 3) { actionBackup(1, filepathP); } // call action backup with type save and given filepath
            }
            break;
        case 1: // exit
            printf("\n\t- exit command");
            actionExit();
            break;
        case 2: // help
            printf("\n\t- help command");
            printf("\nDefined commands:");
            for (int i = 0; i < commandCount; i++) { printf("\n\t%s", commandList[i]); }
            break;
        case 3: // load
            printf("\n\t- load command");
            if ((arguments[0][31] != '\n') && (arguments[1][31] != '\n'))
            {
                char* filepathP = &arguments[0][0]; // set filepath's pointer to 0th argument
                int quarter = stringToIntParser(arguments[1]); // decode first quarter from 1st argument
                if (arguments[2][31] == '\n') { actionLoad(filepathP, quarter, quarter); }
                else // decode second quarter from 2st argument if provided
                {
                    int quarterSecond = stringToIntParser(arguments[2]);
                    actionLoad(filepathP, quarter, quarterSecond); // read the apropriate quarter 64B chunk of ram from the apropriate file's 64B chunk
                }
            }
            break;
        case 4: // print
            printf("\n\t- print command");
            if (arguments[0][31] != '\n')
            {
                if (argumentID[0] == 9) // ram
                {
                    printf("\n\t- ram argument");
                    actionPrintRAM();
                }
                else if (argumentID[0] == 10) // reg
                {
                    printf("\n\t- reg argument");
                    actionPrintReg(argumentID[1]);
                }
            }
            break;
        case 5: // run
            printf("\n\t- run command");
            actionRun();
            break;
        case 6: // save
            printf("\n\t- save command");
            if ((arguments[0][31] != '\n') && (arguments[1][31] != '\n'))
            {
                char* filepathP = &arguments[0][0]; // set filepath's pointer to 0th argument
                int quarter = stringToIntParser(arguments[1]); // decode first quarter from 1st argument
                if (arguments[2][31] == '\n') { actionSave(filepathP, quarter, quarter); } 
                else // decode second quarter from 2st argument if provided
                {
                    int quarterSecond = stringToIntParser(arguments[2]);
                    actionSave(filepathP, quarter, quarterSecond); // write the apropriate quarter 64B chunk of ram to the apropriate file's 64B chunk
                }
            }
            break;
        case 7: // status
            printf("\n\t- status command");
            break;
        case 8: // step
            printf("\n\t- step command");
            if (arguments[0][31] != '\n')
            {
                stepVal = stringToIntParser(arguments[0]);
                printf(" step value %d", stepVal);
            }
            break;
        case 24: // assemble
            printf("\n\t- assemble command");
            if ((arguments[0][31] != '\n') && (arguments[1][31] != '\n') && (arguments[2][31] != '\n'))
            {
                char* destinationFilepathP = &arguments[1][0];
                char* sourceFilepathP = &arguments[2][0];
                if (argumentID[0] == 25) // sus - suspicious type
                {
                    printf("\n\t- sus type");
                    actionAssemble(0, destinationFilepathP, sourceFilepathP);
                }
            }
            break;
        default:
            break;
    }
}
void freeCommandBuffers()
{
    for (int i = 0; i < 31; i++)
    {
        command[i] = '\0';
        arguments[0][i] = '\0';
        arguments[1][i] = '\0';
        arguments[2][i] = '\0';
        arguments[3][i] = '\0';
    }
    for (int i = 0; i < 127; i++)
    {
        lastCommand[i] = '\0';
    }
    command[31] = '\n';
    arguments[0][31] = '\n';
    arguments[1][31] = '\n';
    arguments[2][31] = '\n';
    arguments[3][31] = '\n';

    lastCommand[127] = '\n';
}

void setProcessorDefaults()
{
    for (int i = 0; i < 256; i++) { ram[i] = 0; }
    //ram[0] = 8; ram[1] = 216; ram[2] = 89; ram[3] = 209; ram[4] = 6; ram[5] = 243; ram[6] = 6; ram[7] = 6; ram[8] = 65; ram[9] = 74; ram[10] = 243; ram[11] = 6; ram[12] = 235; ram[13] = 1;
    for (int i = 0; i < 8; i++) { regG[i] = 0; }
    regPC = 0;
    regLPC = 0;
    regFR = 0;
    regGA = 0;
    regOA = 0;
    regSP = 0;
    regFP = 0;
    regIRS = 0;
    for (int i = 0; i < 8; i++) { regIRD[i] = 0; }
    regIOP = 0;
    regSBP = 0;
    regCSP = 0;
}

void defaultFile(char* filepath)
{
    FILE* fileP = fopen(filepath, "wb");
    char fileContents[512];
    if (fileP != NULL)
    {
        for (int i = 0; i < 256; i++)
        {
            fileContents[i] = i;
        }
        for (int i = 0; i < 256; i++)
        {
            fileContents[i + 256] = i;
        }
        fwrite(fileContents, 512, 1, fileP);
    }
    fclose(fileP);
}

void start()
{
    // initiate console system
    printf("\nWelcome to the universal computer system emulator emulator V1.\nCurrently emulating SUS-8.\nSimulated IO devices:\n\tConsole output at General IO Port 1.\n\tScreen control input/output at General IO Port 2. Screen data bus at Serial IO Port 1.\n\nType 'help' for console commands.");

    // set defaults for all processor values
    setProcessorDefaults();

    // free command buffers
    freeCommandBuffers();

    // write 0 - 255 to sus.amogus
    defaultFile("Files/sus.amogus");

    // start execution loop
}

void update(double deltaTime)
{ 
    // read command
    printf("\n>");
    fgets(&lastCommand[0], 128, stdin);

    // tokenise command
    commandParser();

    // decode command and arguments
    printf("\n\t IDs     :");
    int commandID = commandIDer(command);
    int argumentID[4];
    for (int i = 0; i < 4; i++) { argumentID[i] = commandIDer(arguments[i]); }

    // execute command
    executeCommand(commandID, argumentID);

    // free command buffers
    freeCommandBuffers();

    //char newCommand[128];
    //for (int i = 0; i < 95; i++) { encodeChar(allofAscii[i]); }
    //for (int i = 0; i < 16; i++) { if (lastCommand[i] == '\n') { newCommand[i] == '\0'; break; } newCommand[i] = encodeChar(lastCommand[i]); }
    //printf("\n'");
    //for (int i = 0; i < 16; i++) { if (newCommand[i] == '\0') { break; } printf("%c", decodeChar(newCommand[i])); }
    //printf("'");
}

int main()
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER t1, t2;
    double elapsedTime = 0;
    QueryPerformanceFrequency(&frequency);
    start();
    while (running)
    {
        QueryPerformanceCounter(&t1);
        update(elapsedTime);
        QueryPerformanceCounter(&t2);
        elapsedTime = (t2.QuadPart - t1.QuadPart) / frequency.QuadPart;
    }
    return 0;
}