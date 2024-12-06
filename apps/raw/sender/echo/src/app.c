#include "echo.h"

int main(int argc, char* argv[])
{
    atexit(cleanup);

    if (init())
    {
        printf("Error initializing networking\n");
        return -1;
    }

    if (start(argc, argv))
    {
        printf("Cannot establish client\n");
        return -2;
    }

    return 0;
}