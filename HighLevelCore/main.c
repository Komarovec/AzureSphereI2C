#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "MT3620_Grove_Shield_Library/Grove.h"
#include "MT3620_Grove_Shield_Library/Sensors/GroveTempHumiSHT31.h"

#include <applibs/log.h>

int main(void) {
    int i2cFd;
    
    GroveShield_Initialize(&i2cFd, 115200); // baudrate - 9600,14400,19200,115200,230400 

    void* sht31 = GroveTempHumiSHT31_Open(i2cFd);

    const struct timespec sleepTime = {1, 0};

    char msg[80];
    while (true) {
        GroveTempHumiSHT31_Read(sht31);
        float temp = GroveTempHumiSHT31_GetTemperature(sht31);
        float humi = GroveTempHumiSHT31_GetHumidity(sht31);

        snprintf(msg, 80, "%f, %f\n", temp, humi);
        Log_Debug(msg);
        nanosleep(&sleepTime, NULL);
    }
}
