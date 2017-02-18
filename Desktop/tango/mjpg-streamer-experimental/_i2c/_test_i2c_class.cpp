#include "i2cPiToArduino.h"
#include <iostream>
#include <unistd.h>

int main(int argc, char** argv) 
{
    if (argc == 1) 
    {
	printf("Supply one or more commands to send to the Arduino\n");
	exit(1);
    }
 
    I2C i2c;
    
    int arg;
 
    for (arg = 1; arg < argc; arg++) // arg == 0-> name of exe
	i2c.send(argv[arg]);
    
    return (EXIT_SUCCESS);
}
