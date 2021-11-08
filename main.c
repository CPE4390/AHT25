
#include <xc.h>
#include "LCD.h"
#include "AHT25.h"

//RD5 = SCA
//RD6 = SCL
//Need 4.7k pull-ups

void main(void) {
    OSCTUNEbits.PLLEN = 1;
    LCDInit();
    lprintf(0, "AHT25");
    AHT25InitI2C();
    if (AHT25Init()) {
        lprintf(0, "AHT25 Init'ed");
    } else {
        lprintf(0, "Init error");
        while (1);
    }
    
    while (1) {
        int temp, humidity;
        if (AHT25ReadTempAndHumidity(&temp, &humidity)) {
            lprintf(1, "T=%dC H=%d%%", temp, humidity);
        } else {
            lprintf(1, "Error");
        }
        __delay_ms(2000);
    }
}


 