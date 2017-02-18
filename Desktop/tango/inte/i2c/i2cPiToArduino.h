#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
 
// The PiWeather board i2c address
//#define ADDRESS 0x04
 
// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
//static const char *devName = "/dev/i2c-1";

class I2C {
public:
    I2C() {
	_address = 0x04;
	_devName = "/dev/i2c-1";
	
	printf("I2C: Connecting\n");
	if ((_file = open(_devName, O_RDWR)) < 0) {
	    fprintf(stderr, "I2C: Failed to access %d\n", _devName);
	    exit(1);
	}
	
	printf("I2C: acquiring buss to 0x%x\n", _address);
 
	if (ioctl(_file, I2C_SLAVE, _address) < 0) {
	    fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", _address);
	    exit(1);
	}
    }

    ~I2C() {
	close(_file);
    }
    
    void send(char* input) {
	
	int val;
	unsigned char cmd[16];
 
	if (0 == sscanf(input, "%d", &val)) {
	    fprintf(stderr, "Invalid parameter \"%s\"\n", input);
	    exit(1);
	}
 
	printf("Sending %d\n", val);
 
	cmd[0] = val;
	if (write(_file, cmd, 1) == 1) {
 
	    // As we are not talking to direct hardware but a microcontroller we
	    // need to wait a short while so that it can respond.
	    //
	    // 1ms seems to be enough but it depends on what workload it has
	    usleep(10000);
 
	    char buf[1];
	    if (read(_file, buf, 1) == 1) {
		int temp = (int) buf[0];
 
		printf("Received %d\n", temp);
	    }
	}
 
	// Now wait else you could crash the arduino by sending requests too fast
	usleep(10000);
    }
    
private:
    int _address;
    char* _devName;
    int _file;
};
