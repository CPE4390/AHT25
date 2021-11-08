#ifndef AHT25_H
#define	AHT25_H

#define AHT25_DEVICE_ADDRESS            0x38
//Commands/Register
#define AHT25_INIT_REG                  0xBE  //initialization register. Not in datasheet
#define AHT25_STATUS_REG                0x71  //read status byte register
#define AHT25_START_MEASUREMENT_REG     0xAC  //start measurement register
#define AHT25_SOFT_RESET_REG            0xBA  //soft reset register
//Status bits
#define AHT25_BUSY_BIT                  0x80
#define AHT25_CALIBRATION_BIT           0x08
//Delays
#define AHT25_POWER_ON_DELAY            100
#define AHT25_SOFT_RESET_DELAY          20
#define AHT25_MEASUREMENT_DELAY         80

void AHT25InitI2C(void);
char AHT25Init(void);
char AHT25IsConnected(void);
char AHT25IsCalibrated(void);
char AHT25IsBusy(void);
char AHT25ReadStatus(void);
char AHT25ReadTempAndHumidity(int *temp, int *humidity);
void AHT25SoftReset(void);

#endif	/* AHT25_H */

