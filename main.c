//
// ls_i2c - an improved I2C query tool
//
// SPDX-FileCopyrightText: 2026 Larry Bank <bitbank@pobox.com>
// SPDX-License-Identifier: Apache-2.0
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/i2c-dev.h>

// supported devices
enum {
  DEVICE_UNKNOWN = 0,
  DEVICE_SSD1306,
  DEVICE_SH1106,
  DEVICE_VL53L0X,
  DEVICE_BMP180,
  DEVICE_BMP280,
  DEVICE_BME280,
  DEVICE_MPU6000,
  DEVICE_MPU9250,
  DEVICE_MCP9808,
  DEVICE_LSM6DS3,
  DEVICE_ADXL345,
  DEVICE_ADS1115,
  DEVICE_MAX44009,
  DEVICE_MAG3110,
  DEVICE_CCS811,
  DEVICE_HTS221,
  DEVICE_LPS25H,
  DEVICE_LSM9DS1,
  DEVICE_LM8330,
  DEVICE_DS3231,
  DEVICE_LIS3DH,
  DEVICE_LIS3DSH,
  DEVICE_INA219,
  DEVICE_SHT3X,
  DEVICE_HDC1080,
  DEVICE_MPU6886,
  DEVICE_BME680,
  DEVICE_AXP202,
  DEVICE_AXP192,
  DEVICE_24AAXXXE64,
  DEVICE_DS1307,
  DEVICE_MPU688X,
  DEVICE_FT6236G,
  DEVICE_FT6336G,
  DEVICE_FT6336U,
  DEVICE_FT6436,
  DEVICE_BM8563,
  DEVICE_BNO055,
  DEVICE_AHT20,
  DEVICE_TMF882X,
  DEVICE_SCD4X,
  DEVICE_ST25DV,
  DEVICE_LTR390,
  DEVICE_BMP388,
  DEVICE_MMC5603,
  DEVICE_RV3032,
  DEVICE_RV3028,
  DEVICE_SHTC3,
  DEVICE_APDS9930,
  DEVICE_APDS9960,
  DEVICE_LTR553,
  DEVICE_COUNT
};

// Device types
enum {
  DEVICE_TYPE_UNKNOWN=0,
  DEVICE_TYPE_ENVIRO,
  DEVICE_TYPE_IMU,
  DEVICE_TYPE_LIGHT,
  DEVICE_TYPE_TOF_DISTANCE,
  DEVICE_TYPE_RTC,
  DEVICE_TYPE_NFC,
  DEVICE_TYPE_BARCODE,
  DEVICE_TYPE_DISPLAY,
  DEVICE_TYPE_POWER_MEAS,
  DEVICE_TYPE_ADC,
  DEVICE_TYPE_DAC,
  DEVICE_TYPE_CAP_TOUCH,
  DEVICE_TYPE_PMU,
  DEVICE_TYPE_EEPROM,
  DEVICE_TYPE_KEYBOARD,
  DEVICE_TYPE_PROXIMITY,
  DEVICE_TYPE_COUNT
};

static const char *szDeviceNames[] = {
   "Unknown","SSD1306","SH1106","VL53L0X","BMP180", "BMP280","BME280",
   "MPU-60x0", "MPU-9250", "MCP9808","LSM6DS3", "ADXL345", "ADS1115","MAX44009",
   "MAG3110", "CCS811", "HTS221", "LPS25H", "LSM9DS1","LM8330", "DS3231", "LIS3DH",
   "LIS3DSH","INA219","SHT3X","HDC1080","MPU6886","BME680", "AXP202", "AXP192",
   "24AAXXXE64", "DS1307", "MPU688X", "FT6236G", "FT6336G", "FT6336U", "FT6436",
   "BM8563", "BNO055", "AHT20","TMF882X","SCD4X", "ST25DV", "LTR390", "BMP388",
   "MMC5603","RV3032","RV3028","SHTC3","APDS9930","APDS9960","LTR553"
};
static const char *szDeviceTypes[] = {
  "Unknown", "Environmental sensor", "IMU", "Light sensor", "TOF Distance", "Realtime clock",
  "Nearfield Comm", "Barcode reader", "Display", "Power measurement", "ADC", "DAC",
  "Capacitive touch", "Power management", "EEPROM", "Keyboard", "Proximity sensor"
};

//
// Read n bytes from the given address, after setting the register number
//
void I2CReadRegister(int file_i2c, uint8_t iAddr, uint8_t u8Register, uint8_t *pData, int iLen)
{
int rc;
    // Reading from an I2C device involves first writing the 8-bit register
    // followed by reading the data
    ioctl(file_i2c, I2C_SLAVE, iAddr);
    rc = write(file_i2c, &u8Register, 1); // write the register value
    if (rc == 1) {
        rc = read(file_i2c, pData, iLen);
    }
} /* I2CReadRegister() */
//
// Write n bytes to the given address
//
int I2CWrite(int file_i2c, uint8_t iAddr, uint8_t *pData, int iLen)
{
int rc;
    ioctl(file_i2c, I2C_SLAVE, iAddr);
    rc = write(file_i2c, pData, iLen);
    return rc;
} /* I2CWrite() */
//
// Read n bytes from the given I2C address
//
static int I2CRead(int file_i2c, uint8_t iAddr, uint8_t *pData, int iLen)
{
int rc;
    ioctl(file_i2c, I2C_SLAVE, iAddr);
    rc = read(file_i2c, pData, iLen);
    return rc;
} /* I2CRead() */
//
// Figure out what device is at that address
// returns the enumerated value
//
int I2CDiscoverDevice(int file_i2c, int i, int *pType)
{
uint8_t j, cTemp[8];
int iDevice = DEVICE_UNKNOWN;
//
// Read registers of each device to see if it matches known values at known
// addresses. Fall through to the next test if it doesn't.
//
  if (i == 0x28 || i == 0x29) { // Probably a Bosch BNO055
      I2CReadRegister(file_i2c, i, 0x00, cTemp, 1); // CHIP_ID register
      if (cTemp[0] == 0xa0) {
          *pType = DEVICE_TYPE_IMU;
          return DEVICE_BNO055;
      }
  }
  if (i == 0x3c || i == 0x3d) { // Probably an OLED display
      I2CReadRegister(file_i2c, i, 0x00, cTemp, 1);
      cTemp[0] &= 0xbf; // mask off power on/off bit
      if (cTemp[0] == 0x8) { // SH1106 
          *pType = DEVICE_TYPE_DISPLAY;
          iDevice = DEVICE_SH1106;
      } else if (cTemp[0] == 3 || cTemp[0] == 6) {
          *pType = DEVICE_TYPE_DISPLAY;
          iDevice = DEVICE_SSD1306;
      }
      if (iDevice != DEVICE_UNKNOWN)
          return iDevice;
  }
  
  if (i >= 0x30 && i <= 0x37) { // Probably a MEMSIC Magnetometer
      I2CReadRegister(file_i2c, i, 0x39, cTemp, 1); // device ID
      if (cTemp[0] == 0x10) {
          *pType = DEVICE_TYPE_IMU;
          iDevice = DEVICE_MMC5603;
          return iDevice;
      }
  }

  if (i == 0x34 || i == 0x35) { // Probably an AXP202/AXP192 PMU chip
      I2CReadRegister(file_i2c, i, 0x03, cTemp, 1); // chip ID
      if (cTemp[0] == 0x41) {
          *pType = DEVICE_TYPE_PMU;
          return DEVICE_AXP202;
      } else if (cTemp[0] == 0x03) {
          *pType = DEVICE_TYPE_PMU;
          return DEVICE_AXP192;
      }
  }
 
  if (i == 0x70) { // SHTC3?
      cTemp[0] = 0xef; cTemp[1] = 0xc8; // Read ID command   
      I2CWrite(file_i2c, i, cTemp, 2);
      I2CRead(file_i2c, i, cTemp, 3); // read status bits
      if ((cTemp[1] & 0x3f) == 7) { // yes, it's the SHTC3
          *pType = DEVICE_TYPE_ENVIRO;
          return DEVICE_SHTC3;
      }
  }

  if (i == 0x23) { // LTR553?
      I2CReadRegister(file_i2c, i, 0x86, cTemp, 1); // get ID
      if (cTemp[0] == 0x92) {
          *pType = DEVICE_TYPE_PROXIMITY;
          return DEVICE_LTR553;
      }
  }
  if (i == 0x39) { // APDS99xx?
      I2CReadRegister(file_i2c, i, 0x80 | 0x12, cTemp, 1); // get ID
      if (cTemp[0] == 0x39) {
          *pType = DEVICE_TYPE_PROXIMITY;
          return DEVICE_APDS9930;
      } else if (cTemp[0] == 0xab || cTemp[0] == 0x9e) {
          *pType = DEVICE_TYPE_PROXIMITY;
          return DEVICE_APDS9960;
      }
  }

  if (i == 0x38) { // Probably a FT6236G/FT6336G/FT6336U/FT6436 touch screen controller chip
                 //  - likely 0x39 address valid as well but no test HW to verify
    // first check for AHT20 Temp+Humid sensor
    I2CReadRegister(file_i2c, i, 0x71, cTemp, 1); // status word
    if ((cTemp[0] & 0x7f) == 0x18 || (cTemp[0] & 0x7f) == 0x1c) { // yes, it's the AHT20
       *pType = DEVICE_TYPE_ENVIRO;
       return DEVICE_AHT20;   
    }
    I2CReadRegister(file_i2c, i, 0xA0, cTemp, 1); // chip ID
    if (cTemp[0] == 0x00)
        iDevice = DEVICE_FT6236G;
    else if (cTemp[0] == 0x01)
        iDevice = DEVICE_FT6336G;
    else if (cTemp[0] == 0x02)
        iDevice = DEVICE_FT6336U;
    else if (cTemp[0] == 0x03)
        iDevice = DEVICE_FT6436;
    if (iDevice != DEVICE_UNKNOWN) {
        *pType = DEVICE_TYPE_CAP_TOUCH;
        return iDevice;
    }
  }
  
  if (i >= 0x40 && i <= 0x4f) { // check for TI INA219 power measurement sensor
      I2CReadRegister(file_i2c, i, 0x00, cTemp, 2);
      if (cTemp[0] == 0x39 && cTemp[1] == 0x9f) {
         *pType = DEVICE_TYPE_POWER_MEAS;
         return DEVICE_INA219;
      }
  }

  if (i == 0x41) { // check for AMS TMF882X TOF sensor
      I2CReadRegister(file_i2c, i, 0xE3, cTemp, 1); // Read ID
      if ((cTemp[0] & 0x3f) == 8) {
          *pType = DEVICE_TYPE_TOF_DISTANCE;
          return DEVICE_TMF882X;
      }
  }
  if (i == 0x44 || i == 0x45) { // check for SHT3X temp/humidity sensor
      cTemp[0] = 0xf3; cTemp[1] = 0x2d; // read status command
      I2CWrite(file_i2c, i, cTemp, 2);
      I2CRead(file_i2c, i, cTemp, 3); // read status bits
      if ((cTemp[1] & 0x10) == 0x10) { // reset bit set
          *pType = DEVICE_TYPE_ENVIRO;
          return DEVICE_SHT3X;
      }
  }

  // Check for Microchip 24AAXXXE64 family serial 2 Kbit EEPROM
  if (i >= 0x50 && i <= 0x57) {
    unsigned int u32Temp = 0;
    I2CReadRegister(file_i2c, i, 0xf8, (unsigned char *)&u32Temp, 3);
    // check for Microchip's OUI
    if (u32Temp == 0x000004a3 || u32Temp == 0x00001ec0 ||
        u32Temp == 0x00d88039 || u32Temp == 0x005410ec) {
       *pType = DEVICE_TYPE_EEPROM;
      return DEVICE_24AAXXXE64;
    }
  }

  if (i == 0x51) { // BM8563, RV3028 or RV3032 RTC
    I2CReadRegister(file_i2c, i, 0x3d, cTemp, 1); // EE Address
    if (cTemp[0] == 0xc0) { // reset value on RV3032
         *pType = DEVICE_TYPE_RTC;
         return DEVICE_RV3032;
    } 
    I2CReadRegister(file_i2c, i, 0x35, cTemp, 1); // EEPROM Clkout
    if (cTemp[0] == 0xc0) { // reset value on RV3028
        *pType = DEVICE_TYPE_RTC;
        return DEVICE_RV3028;
    }
    I2CReadRegister(file_i2c, i, 0x00, cTemp, 1); // Command/Status register 1
    if ((cTemp[0] & 0xDF) == 0x00) { // all bits clear in Normal, ignore BIT5(STOP)
      I2CReadRegister(file_i2c, i, 0x01, cTemp, 1); // Command/Status register 2
      if ((cTemp[0] & 0xE0) == 0x00) {  // BIT5-7 always clear
        I2CReadRegister(file_i2c, i, 0x02, cTemp, 1); // seconds register 
        if ((cTemp[0] & 0x80) == 0x00) {       // BIT7(VL) clear if PWRON    
            *pType = DEVICE_TYPE_RTC;
            return DEVICE_BM8563;
        }
      }
    }
  }

  if (i == 0x53) { // could be Lite-On LTR390 UV light sensor
    I2CReadRegister(file_i2c, i, 0x06, cTemp, 1); // Part ID
    if (cTemp[0] == 0xb2) { // a match!
      *pType = DEVICE_TYPE_LIGHT;
      return DEVICE_LTR390;
    }
  }

  if (i == 0x57) { // could be the ST25DV NFC transeiver
     cTemp[0] = 0; cTemp[1] = 0x17; // 16-bit register address
     I2CWrite(file_i2c, i, cTemp, 2);
     I2CRead(file_i2c, i, cTemp, 1);
     if (cTemp[0] >= 0x24 && cTemp[0] <= 0x26) {
        *pType = DEVICE_TYPE_NFC;
        return DEVICE_ST25DV;
     }
  }
//  else if (i == 0x5b) // MLX90615?
//  {
//    I2CReadRegister(pI2C, i, 0x10, cTemp, 3);
//    for (j=0; j<3; j++) Serial.println(cTemp[j], HEX);
//  }
  // try to identify it from the known devices using register contents
  {    
    // Check for TI HDC1080
    I2CReadRegister(file_i2c, i, 0xff, cTemp, 2);
    if (cTemp[0] == 0x10 && cTemp[1] == 0x50) {
       *pType = DEVICE_TYPE_ENVIRO;
       return DEVICE_HDC1080;
    }

    // Check for BME680
    if (i == 0x76 || i == 0x77) {
       I2CReadRegister(file_i2c, i, 0xd0, cTemp, 1); // chip ID
       if (cTemp[0] == 0x61) { // BME680
          *pType = DEVICE_TYPE_ENVIRO;
          return DEVICE_BME680;
       }
    }
    // Check for VL53L0X
    I2CReadRegister(file_i2c, i, 0xc0, cTemp, 3);
    if (cTemp[0] == 0xee && cTemp[1] == 0xaa && cTemp[2] == 0x10) {
       *pType = DEVICE_TYPE_TOF_DISTANCE;
       return DEVICE_VL53L0X;
    }

    // Check for CCS811
    I2CReadRegister(file_i2c, i, 0x20, cTemp, 1);
    if (cTemp[0] == 0x81) { // Device ID
       *pType = DEVICE_TYPE_ENVIRO;
       return DEVICE_CCS811;
    }

    // Check for LIS3DSH accelerometer from STMicro
    I2CReadRegister(file_i2c, i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0x3f) { // WHO_AM_I
       *pType = DEVICE_TYPE_IMU;
       return DEVICE_LIS3DSH;
    }

    // Check for LIS3DH accelerometer from STMicro
    I2CReadRegister(file_i2c, i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0x33) { // WHO_AM_I
       *pType = DEVICE_TYPE_IMU;
       return DEVICE_LIS3DH;
    }

    // Check for LSM9DS1 magnetometer/gyro/accel sensor from STMicro
    I2CReadRegister(file_i2c, i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0x68) { // WHO_AM_I
       *pType = DEVICE_TYPE_IMU;
       return DEVICE_LSM9DS1;
    }

    // Check for LPS25H pressure sensor from STMicro
    I2CReadRegister(file_i2c, i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0xbd) { // WHO_AM_I
       *pType = DEVICE_TYPE_ENVIRO;
       return DEVICE_LPS25H;
    }
    
    // Check for HTS221 temp/humidity sensor from STMicro
    I2CReadRegister(file_i2c, i, 0x0f, cTemp, 1);
    if (cTemp[0] == 0xbc) { // WHO_AM_I
       *pType = DEVICE_TYPE_ENVIRO;
       return DEVICE_HTS221;
    }
    
    // Check for MAG3110
    I2CReadRegister(file_i2c, i, 0x07, cTemp, 1);
    if (cTemp[0] == 0xc4) { // WHO_AM_I
       *pType = DEVICE_TYPE_IMU;
       return DEVICE_MAG3110;
    }

    // Check for LM8330 keyboard controller
    I2CReadRegister(file_i2c, i, 0x80, cTemp, 2);
    if (cTemp[0] == 0x0 && cTemp[1] == 0x84) { // manufacturer code + software revision
       *pType = DEVICE_TYPE_KEYBOARD;
       return DEVICE_LM8330;
    }

    // Check for MAX44009
    if (i == 0x4a || i == 0x4b) {
      for (j=0; j<8; j++)
        I2CReadRegister(file_i2c, i, j, &cTemp[j], 1); // check for power-up reset state of registers
      if ((cTemp[2] == 3 || cTemp[2] == 2) && cTemp[6] == 0 && cTemp[7] == 0xff) {
         *pType = DEVICE_TYPE_LIGHT;
         return DEVICE_MAX44009;
      }
    }
       
    // Check for ADS1115
    I2CReadRegister(file_i2c, i, 0x02, cTemp, 2); // Lo_thresh defaults to 0x8000
    I2CReadRegister(file_i2c, i, 0x03, &cTemp[2], 2); // Hi_thresh defaults to 0x7fff
    if (cTemp[0] == 0x80 && cTemp[1] == 0x00 && cTemp[2] == 0x7f && cTemp[3] == 0xff) {
        *pType = DEVICE_TYPE_ADC;
        return DEVICE_ADS1115;
    }
    // Check for MCP9808
    I2CReadRegister(file_i2c, i, 0x06, cTemp, 2); // manufacturer ID && get device ID/revision
    I2CReadRegister(file_i2c, i, 0x07, &cTemp[2], 2); // need to read them individually
    if (cTemp[0] == 0 && cTemp[1] == 0x54 && cTemp[2] == 0x04 && cTemp[3] == 0x00) {
       *pType = DEVICE_TYPE_ENVIRO;
       return DEVICE_MCP9808;
    }
       
   // Check for SCD4x CO2 sensors
   if (i == 0x62) {
     // DEBUG - for now, assume it's the SCD4x
      *pType = DEVICE_TYPE_ENVIRO;
      return DEVICE_SCD4X; 
   }
    // Check for BMP280/BME280
    I2CReadRegister(file_i2c, i, 0xd0, cTemp, 1);
    if (cTemp[0] == 0x55) // BMP180
       iDevice = DEVICE_BMP180;
    else if (cTemp[0] == 0x58)
       iDevice = DEVICE_BMP280;
    else if (cTemp[0] == 0x60) // BME280
       iDevice = DEVICE_BME280;
    if (iDevice != DEVICE_UNKNOWN) {
       *pType = DEVICE_TYPE_ENVIRO;
       return iDevice;
    }
    // Check for BMP388
    I2CReadRegister(file_i2c, i, 0x00, cTemp, 1); // CHIP_ID
    if (cTemp[0] == 0x50) { // BMP388
       *pType = DEVICE_TYPE_ENVIRO;
       iDevice = DEVICE_BMP388;
       return iDevice;
    }
    // Check for LSM6DS3
    I2CReadRegister(file_i2c, i, 0x0f, cTemp, 1); // WHO_AM_I
    if (cTemp[0] == 0x69) {
       *pType = DEVICE_TYPE_IMU;
       return DEVICE_LSM6DS3;
    }
       
    // Check for ADXL345
    I2CReadRegister(file_i2c, i, 0x00, cTemp, 1); // DEVID
    if (cTemp[0] == 0xe5) {
       *pType = DEVICE_TYPE_IMU;
       return DEVICE_ADXL345;
    }

    // Check for MPU-6886
    if (i == 0x68 || i == 0x69) {
      I2CReadRegister(file_i2c, i, 0x75, cTemp, 1);   // WHO_AM_I
      if (cTemp[0] == 0x19) {
        *pType = DEVICE_TYPE_IMU;
        return DEVICE_MPU6886;
      }
    }

    // Check for MPU-60x0i, MPU-688X (not MPU-6886) and MPU-9250
    I2CReadRegister(file_i2c, i, 0x75, cTemp, 1);
    if (cTemp[0] == (i & 0xfe)) { // Current I2C address (low bit set to 0)
        *pType = DEVICE_TYPE_IMU;
        return DEVICE_MPU6000;
    } else if (cTemp[0] == 0x71) {
        *pType = DEVICE_TYPE_IMU;
        return DEVICE_MPU9250;
    } else if (cTemp[0] == 0x19) {
        *pType = DEVICE_TYPE_IMU;
        return DEVICE_MPU688X;
    }

    // Check for DS3231 RTC
    I2CReadRegister(file_i2c, i, 0x0e, cTemp, 1); // read the control register
    if (i == 0x68 && cTemp[0] == 0x1c) { // fixed I2C address and power on reset value
        *pType = DEVICE_TYPE_RTC;
        return DEVICE_DS3231;
    }

    // Check for DS1307 RTC
    I2CReadRegister(file_i2c, i, 0x07, cTemp, 1); // read the control register
    if (i == 0x68 && cTemp[0] == 0x03) { // fixed I2C address and power on reset value
        *pType = DEVICE_TYPE_RTC;
        return DEVICE_DS1307;
    }
  }
  return iDevice;
} /* I2CDiscoverDevice() */

//
// Search the given I2C bus for devices
// return: 1=one or more found, 0=none found
//
int I2CFindDevices(int iBus)
{
char szTemp[32];
int iTotal = 0;
int file_i2c;
int iDevice, iType;

    snprintf(szTemp, sizeof(szTemp)-1, "/dev/i2c-%d", iBus);
    file_i2c = open(szTemp, O_RDWR);
    if (file_i2c < 0) {
        printf("Failed to open %s\n", szTemp);
        return 0;
    }
    for (int iAddr=5; iAddr < 124; iAddr++) {
        if (ioctl(file_i2c, I2C_SLAVE, iAddr) >= 0) {
            // Probe this address
            uint8_t ucTemp = 0;
            write(file_i2c, &ucTemp, 1);
            if (read(file_i2c, &ucTemp, 1) >= 0) {
                iTotal++;
                iType = DEVICE_TYPE_UNKNOWN;
                iDevice = I2CDiscoverDevice(file_i2c, iAddr, &iType);
                if (iTotal == 1) printf("\n"); // first device
                if (iDevice == DEVICE_UNKNOWN) {
                    printf("0x%02x: %s\n", iAddr, szDeviceNames[iDevice]);
                } else {
                    printf("0x%02x: %s, %s\n", iAddr, szDeviceNames[iDevice], szDeviceTypes[iType]);
                }
            }
        }
    } // for each address
    close(file_i2c);
    return iTotal;
} /* I2CFindDevices() */

int main(int argc, char *argv[])
{
int iBus;
DIR *pDir;
struct dirent *pDE;
uint32_t u32Buses = 0; // available I2C bus numbers (0-31)

        printf("ls_i2c - smart i2c bus interrogration by Larry Bank\n");
	// I2C buses in Linux are defined as a file in the /dev directory
        pDir = opendir("/dev");
	if (!pDir) {
		printf("Error searching /dev directory; aborting.\n");
		return -1;
	}
	// Search all names in the /dev directory for those starting with i2c-
        while ((pDE = readdir(pDir)) != NULL) {
		if (memcmp(pDE->d_name, "i2c-", 4) == 0) { // found one!
                    iBus = atoi(&pDE->d_name[4]);
		    u32Buses |= (1 << iBus); // collect the bus numbers
		}
	}
	closedir(pDir);
	if (u32Buses == 0) { // Something went wrong; no I2C buses
	    printf("No I2C buses found!\n");
	    printf("Check your system configuration (e.g. raspi-config)\n");
	    printf("to ensure that I2C is enabled.\n");
	    return -1;
	}
	// Search each I2C bus for a supported proximited sensor
        for (iBus=0; iBus<32; iBus++) {
	    if (u32Buses & (1<<iBus)) { // a bus that we found in /dev
		printf("Searching /dev/i2c-%d...", iBus);
                if (!I2CFindDevices(iBus)) { // scan for recognized devices
                    printf("No devices found\n");
                }
	    }
        } // for each possible bus
return 0;
} /* main() */
