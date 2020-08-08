/**************************************************************************/
/*!
    @file       ADS1x15.h
    @author     K. Townsend (Adafruit Industries)
    @license    BSD (see LICENSE.txt)
    
    Ported to mbed by Arve Seljebu - arve0.github.io

	ADS1015 12-bit I2C ADC+PGA and ADS1115 16-bit I2C ADC+PGA

    This is a library for the Adafruit ADS1015 and ADS1015 breakout boards
    ----> https://www.adafruit.com/products/1083

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

	v1.0	- First release
	v1.1	- Added ADS1115 support - W. Earl
	v1.1.1	- Ported to mbed - Arve Seljebu
	v1.2	- Fixed error in readADC_SingleEnded() sign bit	- Sam W Berjawi
	v1.3	- Added ADS1115_REG_CONFIG_DR & ads1015_DR_t 	- Sam W Berjawi Jul 21. 2014
			- Corrected m_conversionDelay
			- Now there is readADC() that returns counts and readADC_V() that returns voltage
*/
/**************************************************************************/

#ifndef ADS1015_H
#define ADS1015_H

#include <mbed.h>

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define ADS1015_ADDRESS                 (0x48)    // 0100 1000 (ADDR = GND)
/*=========================================================================*/

/*=========================================================================
    CONVERSION DELAY (in mS)
    -----------------------------------------------------------------------*/
//    #define ADS1015_CONVERSIONDELAY         (1)
//    #define ADS1115_CONVERSIONDELAY         (8)
/*=========================================================================*/

/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1015_REG_POINTER_MASK        (0x03)
    #define ADS1015_REG_POINTER_CONVERT     (0x00)
    #define ADS1015_REG_POINTER_CONFIG      (0x01)
    #define ADS1015_REG_POINTER_LOWTHRESH   (0x02)
    #define ADS1015_REG_POINTER_HITHRESH    (0x03)
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
    #define ADS1015_REG_CONFIG_OS_MASK      (0x8000)
    #define ADS1015_REG_CONFIG_OS_SINGLE    (0x8000)  // Write: Set to start a single-conversion
    #define ADS1015_REG_CONFIG_OS_BUSY      (0x0000)  // Read: Bit = 0 when conversion is in progress
    #define ADS1015_REG_CONFIG_OS_NOTBUSY   (0x8000)  // Read: Bit = 1 when device is not performing a conversion

//    #define ADS1015_REG_CONFIG_MUX_MASK     (0x7000)
//    #define ADS1015_REG_CONFIG_MUX_DIFF_0_1 (0x0000)  // Differential P = AIN0, N = AIN1 (default)
//    #define ADS1015_REG_CONFIG_MUX_DIFF_0_3 (0x1000)  // Differential P = AIN0, N = AIN3
//    #define ADS1015_REG_CONFIG_MUX_DIFF_1_3 (0x2000)  // Differential P = AIN1, N = AIN3
//    #define ADS1015_REG_CONFIG_MUX_DIFF_2_3 (0x3000)  // Differential P = AIN2, N = AIN3
//    #define ADS1015_REG_CONFIG_MUX_SINGLE_0 (0x4000)  // Single-ended AIN0
//    #define ADS1015_REG_CONFIG_MUX_SINGLE_1 (0x5000)  // Single-ended AIN1
//    #define ADS1015_REG_CONFIG_MUX_SINGLE_2 (0x6000)  // Single-ended AIN2
//    #define ADS1015_REG_CONFIG_MUX_SINGLE_3 (0x7000)  // Single-ended AIN3

//    #define ADS1015_REG_CONFIG_PGA_MASK     (0x0E00)
//    #define ADS1015_REG_CONFIG_PGA_6_144V   (0x0000)  // +/-6.144V range = Gain 2/3			ADS1015: 1-bit = 3mV     ADS1115: 0.1875mV
//    #define ADS1015_REG_CONFIG_PGA_4_096V   (0x0200)  // +/-4.096V range = Gain 1			ADS1015: 1-bit = 2mV     ADS1115: 0.125mV
//    #define ADS1015_REG_CONFIG_PGA_2_048V   (0x0400)  // +/-2.048V range = Gain 2 (default)	ADS1015: 1-bit = 1mV     ADS1115: 0.0625mV
//    #define ADS1015_REG_CONFIG_PGA_1_024V   (0x0600)  // +/-1.024V range = Gain 4			ADS1015: 1-bit = 0.5mV   ADS1115: 0.03125mV
//    #define ADS1015_REG_CONFIG_PGA_0_512V   (0x0800)  // +/-0.512V range = Gain 8			ADS1015: 1-bit = 0.25mV  ADS1115: 0.015625mV
//    #define ADS1015_REG_CONFIG_PGA_0_256V   (0x0A00)  // +/-0.256V range = Gain 16			ADS1015: 1-bit = 0.125mV ADS1115: 0.0078125mV

    #define ADS1015_REG_CONFIG_MODE_MASK    (0x0100)
    #define ADS1015_REG_CONFIG_MODE_CONTIN  (0x0000)  // Continuous conversion mode
    #define ADS1015_REG_CONFIG_MODE_SINGLE  (0x0100)  // Power-down single-shot mode (default)

//    #define ADS1015_REG_CONFIG_DR_MASK		(0x00E0)
//    #define ADS1015_REG_CONFIG_DR_128SPS	(0x0000)  //  128 SPS
//    #define ADS1015_REG_CONFIG_DR_250SPS	(0x0020)  //  250 SPS
//    #define ADS1015_REG_CONFIG_DR_490SPS	(0x0040)  //  490 SPS
//    #define ADS1015_REG_CONFIG_DR_920SPS	(0x0060)  //  920 SPS
//    #define ADS1015_REG_CONFIG_DR_1600SPS	(0x0080)  // 1600 SPS (default)
//    #define ADS1015_REG_CONFIG_DR_2400SPS	(0x00A0)  // 2400 SPS
//    #define ADS1015_REG_CONFIG_DR_3300SPS	(0x00C0)  // 3300 SPS
//
//    #define ADS1115_REG_CONFIG_DR_MASK		(0x00E0)
//    #define ADS1115_REG_CONFIG_DR_8SPS		(0x0000)  //    8 SPS
//    #define ADS1115_REG_CONFIG_DR_16SPS		(0x0020)  //   16 SPS
//    #define ADS1115_REG_CONFIG_DR_32SPS		(0x0040)  //   32 SPS
//    #define ADS1115_REG_CONFIG_DR_64SPS		(0x0060)  //   64 SPS
//    #define ADS1115_REG_CONFIG_DR_128SPS	(0x0080)  //  128 SPS (default)
//    #define ADS1115_REG_CONFIG_DR_250SPS	(0x00A0)  //  250 SPS
//    #define ADS1115_REG_CONFIG_DR_475SPS	(0x00C0)  //  475 SPS
//    #define ADS1115_REG_CONFIG_DR_860SPS	(0x00E0)  //  860 SPS

    #define ADS1015_REG_CONFIG_CMODE_MASK   (0x0010)
    #define ADS1015_REG_CONFIG_CMODE_TRAD   (0x0000)  // Traditional comparator with hysteresis (default)
    #define ADS1015_REG_CONFIG_CMODE_WINDOW (0x0010)  // Window comparator

    #define ADS1015_REG_CONFIG_CPOL_MASK    (0x0008)
    #define ADS1015_REG_CONFIG_CPOL_ACTVLOW (0x0000)  // ALERT/RDY pin is low when active (default)
    #define ADS1015_REG_CONFIG_CPOL_ACTVHI  (0x0008)  // ALERT/RDY pin is high when active

    #define ADS1015_REG_CONFIG_CLAT_MASK    (0x0004)  // Determines if ALERT/RDY pin latches once asserted
    #define ADS1015_REG_CONFIG_CLAT_NONLAT  (0x0000)  // Non-latching comparator (default)
    #define ADS1015_REG_CONFIG_CLAT_LATCH   (0x0004)  // Latching comparator

    #define ADS1015_REG_CONFIG_CQUE_MASK    (0x0003)
    #define ADS1015_REG_CONFIG_CQUE_1CONV   (0x0000)  // Assert ALERT/RDY after one conversions
    #define ADS1015_REG_CONFIG_CQUE_2CONV   (0x0001)  // Assert ALERT/RDY after two conversions
    #define ADS1015_REG_CONFIG_CQUE_4CONV   (0x0002)  // Assert ALERT/RDY after four conversions
    #define ADS1015_REG_CONFIG_CQUE_NONE    (0x0003)  // Disable the comparator and put ALERT/RDY in high state (default)
/*=========================================================================*/

typedef enum
{
	chan_0_1		= 0x0000, // Differential P = AIN0, N = AIN1 (default)
	chan_0_3		= 0x1000, // Differential P = AIN0, N = AIN3
	chan_1_3		= 0x2000, // Differential P = AIN1, N = AIN3
	chan_2_3		= 0x3000, // Differential P = AIN2, N = AIN3
	chan_0			= 0x4000, // Single-ended AIN0
	chan_1			= 0x5000, // Single-ended AIN1
	chan_2			= 0x6000, // Single-ended AIN2
	chan_3			= 0x7000  // Single-ended AIN3
} chan_t;

// The +/-6.144V and +/-4.096V settings express the full-scale range of the ADC scaling.
// In no event should more than VDD + 0.3V be applied to this device
typedef enum
{	//																	ADS1015		ADS1115
	VR_p_m_6_144V	= 0x0000, // +/-6.144V range = Gain 2/3		1-bit = 3mV     	0.1875mV
	VR_p_m_4_096V	= 0x0200, // +/-4.096V range = Gain 1		1-bit = 2mV      	0.125mV
	VR_p_m_2_048V	= 0x0400, // +/-2.048V range = Gain 2	 	1-bit = 1mV      	0.0625mV
	VR_p_m_1_024V	= 0x0600, // +/-1.024V range = Gain 4		1-bit = 0.5mV    	0.03125mV
	VR_p_m_0_512V	= 0x0800, // +/-0.512V range = Gain 8		1-bit = 0.25mV   	0.015625mV
	VR_p_m_0_256V	= 0x0A00  // +/-0.256V range = Gain 16		1-bit = 0.125mV  	0.0078125mV
} adsVR_t; // VR: Voltage Range

typedef enum
{
	ADS1015_DR_128SPS	= 0x0000, //  128 SPS
	ADS1015_DR_250SPS 	= 0x0020, //  250 SPS
	ADS1015_DR_490SPS 	= 0x0040, //  490 SPS
	ADS1015_DR_920SPS 	= 0x0060, //  920 SPS
	ADS1015_DR_1600SPS 	= 0x0080, // 1600 SPS
	ADS1015_DR_2400SPS 	= 0x00A0, // 2400 SPS
	ADS1015_DR_3300SPS 	= 0x00C0, // 3300 SPS

	ADS1115_DR_8SPS		= 0x0000, //    8 SPS
	ADS1115_DR_16SPS	= 0x0020, //   16 SPS
	ADS1115_DR_32SPS	= 0x0040, //   32 SPS
	ADS1115_DR_64SPS	= 0x0060, //   64 SPS
	ADS1115_DR_128SPS	= 0x0080, //  128 SPS
	ADS1115_DR_250SPS	= 0x00A0, //  250 SPS
	ADS1115_DR_475SPS	= 0x00C0, //  475 SPS
	ADS1115_DR_860SPS	= 0x00E0  //  860 SPS
} adsDR_t; // DR: Data Rate


class ADS1015
{
protected:
   // Instance-specific properties
   uint8_t   	m_i2cAddress;
//   ads1015_DR_t	m_dataRate;
   float		m_conversionDelay;
   uint8_t		m_bitShift;
   I2C*			m_i2c;
   
 public:
  ADS1015(I2C* i2c = 0, uint8_t i2cAddress = ADS1015_ADDRESS); // set i2c ptr = 0 to allow ADS1115 to use this as default constructor
  int16_t   readADC(chan_t chan, adsVR_t voltageRange, adsDR_t dataRate);
  float		readADC_V(chan_t chan, adsVR_t voltageRange, adsDR_t dataRate);
  void      startComparator_SingleEnded(chan_t chan, adsVR_t voltageRange, adsDR_t dataRate, int16_t threshold);
  int16_t   getLastConversionResults();

 private:
  uint16_t  readRegister(uint8_t i2cAddress, uint8_t reg);
  void 		writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value);
};

// Derive from ADS1015 & override construction to set properties
class ADS1115 : public ADS1015
{
 public:
  ADS1115(I2C* i2c, uint8_t i2cAddress = ADS1015_ADDRESS);
};

#endif
