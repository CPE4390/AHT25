#include <xc.h>
#include "AHT25.h"

#define _XTAL_FREQ  32000000

char chksumIsValid(char *data);

void AHT25InitI2C(void) {
    //Setup I2C
    TRISDbits.TRISD5 = 1; //RD5 and RD6 both inputs 
    TRISDbits.TRISD6 = 1;
    SSP2ADD = 19; //400kHz
    SSP2STATbits.SMP = 0;
    SSP2CON1bits.SSPM = 0b1000;
    SSP2CON1bits.SSPEN = 1;
}

char AHT25Init(void) {
    __delay_ms(AHT25_POWER_ON_DELAY);
    if (!AHT25IsConnected()) {
        return 0;
    }
    AHT25SoftReset();
    if (!AHT25IsCalibrated()) {
        SSP2CON2bits.SEN = 1;
        while (SSP2CON2bits.SEN == 1);
        SSP2BUF = AHT25_DEVICE_ADDRESS << 1;
        while (SSP2STATbits.BF || SSP2STATbits.R_W);
        SSP2BUF = AHT25_INIT_REG; //This process is not documented in datasheet
        while (SSP2STATbits.BF || SSP2STATbits.R_W);
        SSP2BUF = 0x08;
        while (SSP2STATbits.BF || SSP2STATbits.R_W);
        SSP2BUF = 0x00;
        while (SSP2STATbits.BF || SSP2STATbits.R_W);
        SSP2CON2bits.PEN = 1;
        while (SSP2CON2bits.PEN == 1);
    }
    return AHT25IsCalibrated();
}

char AHT25IsConnected(void) {
    char result = 0;
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = AHT25_DEVICE_ADDRESS << 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    result = !SSP2CON2bits.ACKSTAT; //Check for ACK
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    return result;
}

char AHT25IsCalibrated(void) {
    char status = AHT25ReadStatus();
    return status & AHT25_CALIBRATION_BIT;
}

char AHT25IsBusy(void) {
    char status = AHT25ReadStatus();
    return status & AHT25_BUSY_BIT;
}

char AHT25ReadStatus(void) {
    char status = 0;
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = AHT25_DEVICE_ADDRESS << 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2BUF = AHT25_STATUS_REG;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2CON2bits.RSEN = 1;
    while (SSP2CON2bits.RSEN == 1);
    SSP2BUF = (AHT25_DEVICE_ADDRESS << 1) | 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2CON2bits.RCEN = 1;
    while (!SSP2STATbits.BF);
    status = SSP2BUF;
    SSP2CON2bits.ACKDT = 1;
    SSP2CON2bits.ACKEN = 1;
    while (SSP2CON2bits.ACKEN != 0);
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    return status;
}

char AHT25ReadTempAndHumidity(int *temp, int *humidity) {
    char status = 0;
    char buffer[7];
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = AHT25_DEVICE_ADDRESS << 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2BUF = AHT25_START_MEASUREMENT_REG;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2BUF = 0x33;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2BUF = 0x00;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    char delayCount = AHT25_MEASUREMENT_DELAY;
    while (AHT25IsBusy() && delayCount > 0) {
        __delay_ms(1);
        --delayCount;
    };
    if (delayCount == 0) {
        return 0; //Give up if not ready in another 100ms
    }
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = (AHT25_DEVICE_ADDRESS << 1) | 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    for (char i = 0; i < 7; ++i) {
        SSP2CON2bits.RCEN = 1;
        while (!SSP2STATbits.BF);
        buffer[i] = SSP2BUF;
        if (i < 6) {
            SSP2CON2bits.ACKDT = 0;
        } else {
            SSP2CON2bits.ACKDT = 1;
        }
        SSP2CON2bits.ACKEN = 1;
        while (SSP2CON2bits.ACKEN != 0);
    }
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    //TODO check status byte for errors??
    unsigned long raw;
    raw = buffer[1];
    raw <<= 8;
    raw |= buffer[2];
    raw <<= 4;
    raw |= buffer[3] >> 4;
    *humidity = (raw * 100) / 0x100000;  //0x100000 = 2^20
    raw = buffer[3] & 0x0f;
    raw <<= 8;
    raw |= buffer[4];
    raw <<= 8;
    raw |= buffer[5];
    *temp = ((raw * 200) / 0x100000) - 50;
    return chksumIsValid(buffer);
}

void AHT25SoftReset(void) {
    SSP2CON2bits.SEN = 1;
    while (SSP2CON2bits.SEN == 1);
    SSP2BUF = AHT25_DEVICE_ADDRESS << 1;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2BUF = AHT25_SOFT_RESET_REG;
    while (SSP2STATbits.BF || SSP2STATbits.R_W);
    SSP2CON2bits.PEN = 1;
    while (SSP2CON2bits.PEN == 1);
    __delay_ms(AHT25_SOFT_RESET_DELAY);
}

char chksumIsValid(char *data) {
    unsigned char crc = 0xff; //initial value per datasheet
    for (char i = 0; i < 6; i++) {
        crc ^= data[i];
        for (char bitIndex = 8; bitIndex > 0; --bitIndex) {
            if (crc & 0x80) {
                //0x31 = CRC polynomial (actually 0x131 but this is a CRC8 so only low byte used)
                crc = (unsigned char)((crc << 1) ^ 0x31);  
            }
            else {
                crc = (unsigned char)(crc << 1);
            }
        }
    }
    return (crc == data[6]);
}
