#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>

#define LED 7

int main(void)
{
    if (wiringPiSetup() == -1)
	exit(1);

    pinMode(LED, OUTPUT);

    digitalWrite(LED, 0);

    int i = 0;
    for (i = 0; i < 10; i++) 
    {
	digitalWrite(LED, 1);
	delay(1000);
	digitalWrite(LED, 0);
	delay(1000);
    }
    
    return 0;
}