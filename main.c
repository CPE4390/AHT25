
#include <xc.h>
#include "LCD.h"
#include "AHT25.h"

void main(void) {
    OSCTUNEbits.PLLEN = 1;
    LCDInit();
    lprintf(0, "AHT25");
    
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;
    
    while (1) {
        
    }
}

void __interrupt(high_priority) highISR(void) {
    
}

