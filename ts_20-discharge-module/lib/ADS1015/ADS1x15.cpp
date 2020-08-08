/**************************************************************************/
/*!
    @file     ADS1x15.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see LICENSE.txt)

    Ported to mbed by Arve Seljebu - arve0.github.io

    Driver for the ADS1015/ADS1115 ADC

    This is a library for the Adafruit MPL115A2 breakout
    ----> https://www.adafruit.com/products/1083

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0   - First release
    v1.1   - Added ADS1115 support - W. Earl
    v1.1.1 - Ported to mbed - Arve Seljebu
    v1.2   - Fixed error in readADC_SingleEnded() sign bit - Sam Berjawi
	v1.3	- Added ADS1115_REG_CONFIG_DR & m_dataRate 		- Sam W Berjawi Jul 21. 2014
			- Corrected m_conversionDelay
			- Now there is readADC() that returns counts and readADC_V() that returns voltage
*/
/**************************************************************************/

#include "ADS1x15.h"

/**************************************************************************/
/*!
    @brief  Writes 16-bits to the specified destination register
*/
/**************************************************************************/
void ADS1015::writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value) {
  char cmd[3];
  cmd[0] = (char)reg;
  cmd[1] = (char)(value>>8);
  cmd[2] = (char)(value & 0xFF);
  m_i2c->write(i2cAddress, cmd, 3); 
}

/**************************************************************************/
/*!
    @brief  Reads 16-bits from the specified register
*/
/**************************************************************************/
uint16_t ADS1015::readRegister(uint8_t i2cAddress, uint8_t reg) {
  char data[2];
  data[0] = reg; // temporary use this to send address to conversion register
  m_i2c->write(i2cAddress, data, 1);
  m_i2c->read(i2cAddress, data, 2);
  return (data[0] << 8 | data [1]);
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1015 class w/appropriate properties
*/
/**************************************************************************/
ADS1015::ADS1015(I2C* i2c, uint8_t i2cAddress)
{
  // shift 7 bit address 1 left: read expects 8 bit address, see I2C.h
   m_i2cAddress = i2cAddress << 1;
   m_bitShift = 4;
   m_i2c = i2c;
}

/**************************************************************************/
/*!
    @brief  Instantiates a new ADS1115 class w/appropriate properties
*/
/**************************************************************************/
ADS1115::ADS1115(I2C* i2c, uint8_t i2cAddress)
{
  // shift 7 bit address 1 left: read expects 8 bit address, see mbed's I2C.h
  m_i2cAddress = i2cAddress << 1;
  m_bitShift = 0;
  m_i2c = i2c;
}

/**************************************************************************/
/*! 
    @brief	Reads the A2D conversion result, measuring depending on chan parameter either
			the single chan  voltage or the differential voltage between the P (AIN_i) and N (AIN_j) inputs.
			Generates a signed value since the difference can be either positive or negative.

			The +/-6.144V and +/-4.096V settings express the full-scale range of the ADC scaling.
			In no event should more than VDD + 0.3V be applied to this device

	@return Count
*/
/**************************************************************************/
int16_t ADS1015::readADC(chan_t chan, adsVR_t voltageRange, adsDR_t dataRate)
{
	// Prepare Config Register
	uint16_t config = ADS1015_REG_CONFIG_OS_SINGLE    | // Begin a single conversion (when in power-down mode)
					ADS1015_REG_CONFIG_CQUE_NONE    | // Disable the comparator (default val)
					ADS1015_REG_CONFIG_CLAT_NONLAT  | // Non-latching (default val)
					ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
					ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
					ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

	// Set channel(s)
	config |= chan;

	// Set PGA/voltage range
	config |= voltageRange;

	// Set Data rate
	config |= dataRate;

	// Write config register to the ADC
	writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);

	// TODO: Modify to alternatively use the RDY (EOC) pin instead of waiting. Maybe also use interrupt and continuous mode

	if (m_bitShift == 4) // ADS1015
		switch (dataRate) {
			case ADS1015_DR_128SPS:
				m_conversionDelay = 1.0 / 128;
				break;
			case ADS1015_DR_250SPS:
				m_conversionDelay = 1.0 / 250;
				break;
			case ADS1015_DR_490SPS:
				m_conversionDelay = 1.0 / 490;
				break;
			case ADS1015_DR_920SPS:
				m_conversionDelay = 1.0 / 920;
				break;
			case ADS1015_DR_1600SPS:
				m_conversionDelay = 1.0 / 1600;
				break;
			case ADS1015_DR_2400SPS:
				m_conversionDelay = 1.0 / 2400;
				break;
			case ADS1015_DR_3300SPS:
				m_conversionDelay = 1.0 / 3300;
				break;
		}
	else // ADS1115
		switch (dataRate) {
			case ADS1115_DR_8SPS:
				m_conversionDelay = 1.0 / 8;
				break;
			case ADS1115_DR_16SPS:
				m_conversionDelay = 1.0 / 16;
				break;
			case ADS1115_DR_32SPS:
				m_conversionDelay = 1.0 / 32;
				break;
			case ADS1115_DR_64SPS:
				m_conversionDelay = 1.0 / 64;
				break;
			case ADS1115_DR_128SPS:
				m_conversionDelay = 1.0 / 128;
				break;
			case ADS1115_DR_250SPS:
				m_conversionDelay = 1.0 / 250;
				break;
			case ADS1115_DR_475SPS:
				m_conversionDelay = 1.0 / 475;
				break;
			case ADS1115_DR_860SPS:
				m_conversionDelay = 1.0 / 860;
				break;
		}

	// Wait for the conversion to complete
	wait(m_conversionDelay);

	// Read the conversion results
	uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
	if (m_bitShift == 0)
	{
	return (int16_t)res;
	}
	else
	{
	// Shift 12-bit results right 4 bits for the ADS1015,
	// making sure we keep the sign bit intact
	if (res > 0x07FF)
	{
	  // negative number - extend the sign to 16th bit
	  res |= 0xF000;
	}
	return (int16_t)res;
	}
}

/**************************************************************************/
/*!
    @brief	Reads the A2D conversion result, measuring depending on chan parameter either
			the single chan  voltage or the differential voltage between the P (AIN_i) and N (AIN_j) inputs.
			Generates a signed value since the difference can be either positive or negative.

			The +/-6.144V and +/-4.096V settings express the full-scale range of the ADC scaling.
			In no event should more than VDD + 0.3V be applied to this device

	@return Voltage
*/
/**************************************************************************/
float ADS1015::readADC_V(chan_t chan, adsVR_t voltageRange, adsDR_t dataRate)
{
float bit_V = 0.;

	//											ADS1015		ADS1115
	// +/-6.144V range = Gain 2/3		1-bit = 3mV     	0.1875mV
	// +/-4.096V range = Gain 1			1-bit = 2mV      	0.125mV
	// +/-2.048V range = Gain 2		 	1-bit = 1mV      	0.0625mV
	// +/-1.024V range = Gain 4			1-bit = 0.5mV    	0.03125mV
	// +/-0.512V range = Gain 8			1-bit = 0.25mV   	0.015625mV
	// +/-0.256V range = Gain 16		1-bit = 0.125mV  	0.0078125mV

	switch (voltageRange) {
		case VR_p_m_6_144V:
			bit_V = 3.0e-3f * powf(2, -4 + m_bitShift);
			break;
		case VR_p_m_4_096V:
			bit_V = 2.0e-3f * powf(2, -4 + m_bitShift);
			break;
		case VR_p_m_2_048V:
			bit_V = 1.0e-3f * powf(2, -4 + m_bitShift);
			break;
		case VR_p_m_1_024V:
			bit_V = 0.5e-3f * powf(2, -4 + m_bitShift);
			break;
		case VR_p_m_0_512V:
			bit_V = 0.25e-3f * powf(2, -4 + m_bitShift);
			break;
		case VR_p_m_0_256V:
			bit_V = 0.125e-3f * powf(2, -4 + m_bitShift);
			break;
	}

	return (readADC(chan, voltageRange, dataRate) * bit_V);
}

/**************************************************************************/
/*!
    @brief  Sets up the comparator to operate in basic mode, causing the
            ALERT/RDY pin to assert (go from high to low) when the ADC
            value exceeds the specified threshold.

            This will also set the ADC in continuous conversion mode.
*/
/**************************************************************************/
void ADS1015::startComparator_SingleEnded(chan_t chan, adsVR_t voltageRange, adsDR_t dataRate, int16_t threshold)
{
	// Prepare Config Register
	  uint16_t config = ADS1015_REG_CONFIG_CQUE_1CONV   | // Comparator enabled and asserts on 1 match
			  	  	  	ADS1015_REG_CONFIG_CLAT_LATCH   | // Latching mode
			  	  	  	ADS1015_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
			  	  	  	ADS1015_REG_CONFIG_CMODE_TRAD   | // Traditional comparator (default val)
			  	  	  	ADS1015_REG_CONFIG_MODE_SINGLE;   // Single-shot mode (default)

	// Set channel(s)
	config |= chan;

	// Set PGA/voltage range
	config |= voltageRange;

	// Set Data rate
	config |= dataRate;

	// Set the high threshold register
	// Shift 12-bit results left 4 bits for the ADS1015
	writeRegister(m_i2cAddress, ADS1015_REG_POINTER_HITHRESH, threshold << m_bitShift);

	// Write config register to the ADC
	writeRegister(m_i2cAddress, ADS1015_REG_POINTER_CONFIG, config);
}

/**************************************************************************/
/*!
    @brief  In order to clear the comparator, we need to read the
            conversion results.  This function reads the last conversion
            results without changing the config value.
*/
/**************************************************************************/
int16_t ADS1015::getLastConversionResults()
{
  // Wait for the conversion to complete
  wait_ms(m_conversionDelay);

  // Read the conversion results
  uint16_t res = readRegister(m_i2cAddress, ADS1015_REG_POINTER_CONVERT) >> m_bitShift;
  if (m_bitShift == 0)
  {
    return (int16_t)res;
  }
  else
  {
    // Shift 12-bit results right 4 bits for the ADS1015,
    // making sure we keep the sign bit intact
    if (res > 0x07FF)
    {
      // negative number - extend the sign to 16th bit
      res |= 0xF000;
    }
    return (int16_t)res;
  }
}
