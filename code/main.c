#include <stdio.h>
#include <libs/console.h>

char TEST_MODE = 0;

int main(int argc, char* args[])
{
    // decode arguments
    if (argc > 1)
    {
        switch (*args[1])
        {
            case "test":
                TEST_MODE = 1;
                break;
            default:
                break;
        }
    }

    // startup and config loading
    
}