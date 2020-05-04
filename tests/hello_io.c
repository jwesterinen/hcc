// I/O test

#include <hack_stdlib.h>

void main()
{
    SetGpioDir(1);
    while (1)
    {
        SetGpio(1);
        SetLed(0);
        DelayMs(500);
        SetGpio(0);
        SetLed(1);
        DelayMs(500);
    }
}
