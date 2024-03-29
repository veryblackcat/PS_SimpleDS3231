// Piotr Sylwesiuk (c)2022

#ifndef PS_SIMPLE_DS3231_H
#define PS_SIMPLE_DS3231_H

#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>

#define DS3231_ADDRESS 			0x68 
/* 
Control Register (0Eh)
| BIT7 | BIT6  | BIT5 | BIT4 | BIT3 | BIT2  | BIT1 | BIT0 |
| ____ |       |      |      |      |       |      |      |
| EOSC | BBSQW | CONV | RS2  | RS1  | INTCN | A2IE | A1IE |
|  0   |   0   |  0   |  1   |  1   |   1   |  0   |  0   | When power is first applied.
*/
#define DS3231_CONTROL_REG 		0x0e
#define DS3231_CTRL_EOSC		0x80
#define DS3231_CTRL_BBSQW		0x40
#define DS3231_CTRL_CONV		0x20
#define DS3231_CTRL_INTCN		0x04
#define DS3231_CTRL_A2IE		0x02
#define DS3231_CTRL_A1IE		0x01
#define DS3231_SQW_FREQ_1HZ 	0x00 // FREQ - RS2, RS1
#define DS3231_SQW_FREQ_1024HZ 	0x08
#define DS3231_SQW_FREQ_4096HZ 	0x10
#define DS3231_SQW_FREQ_8192HZ 	0x18
/* 
Status Register (0Fh)
| BIT7 | BIT6  | BIT5 | BIT4 |   BIT3  | BIT2 | BIT1 | BIT0 |
| OSF  |  0    |  0   |  0   | EN32kHz | BSY  | A2F  | A1F  |
|  0   |  0    |  0   |  0   |    1    |  x   |  x   |  x   | When power is first applied.
*/
#define DS3231_STATUS_REG	0xf0
#define DS3231_STATUS_OSF 	0x80
#define DS3231_STATUS_EN32	0x08
#define DS3231_STATUS_BSY	0x04
#define DS3231_STATUS_A2F	0x02
#define DS3231_STATUS_A1F	0x01
// Aging Offset (10h)
#define DS3231_AGING_REG	0x10
// Temperature Registers (11h–12h)
#define DS3231_TEMP_REG_U	0x11
#define DS3231_TEMP_REG_L	0x12

class rtcDS3231 {
	public:
		TwoWire *twi;
		uint8_t dataBuffer[7];
		uint8_t hh, mm, ss, dow, DD, MM;
		uint16_t YYYY;
		float temp; // temperature
		int8_t tempInt; // temperature as an integer rounded to 1

		rtcDS3231(uint8_t addr=DS3231_ADDRESS);
		void begin(uint32_t clkTWI = 100000);

		void getDateTime();
		uint8_t getTemperature(bool force = false);

		uint8_t hour() 			{ return(hh); }
		uint8_t minutes() 		{ return(mm); }
		uint8_t seconds() 		{ return(ss); }
		uint8_t dayOfWeek() 	{ return(dow); }
		uint8_t day() 			{ return(DD); }
		uint8_t month() 		{ return(MM); }
		uint16_t year()			{ return(YYYY); }
		float temperature() 	{ return(temp); }
		int8_t temperatureInt() { return(tempInt); }

		void setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t month, uint16_t year);
		void setTime(uint8_t hour, uint8_t minutes, uint8_t seconds);
		void setDate(uint8_t date, uint8_t month, uint16_t year);
		void seconds(uint8_t seconds) 	{ writeByte(0x00, bin2bcd(seconds)); }
		void minutes(uint8_t minutes) 	{ writeByte(0x01, bin2bcd(minutes)); }
		void hour(uint8_t hour) 		{ writeByte(0x02, bin2bcd(hour)); }  //mode 24h (BIT6 = 0)
		void day(uint8_t day) 			{ writeByte(0x04, bin2bcd(day)); } 
		void month(uint8_t month) 		{ writeByte(0x05, (bin2bcd(month) | (dataBuffer[5] & 0x80))); } // includes century
		void year(uint16_t year);
		// The day of the week for DS3231 - 1 equals Sunday, then 2 equals Monday, and so on.
		void dayOfWeek(uint8_t dow) 	{ writeByte(0x03, dow); } 
		// RFC 3339 Appendix B. Day of the Week
		// The day of the week for dates on or after 0000-03-01.
		// 0 equals Sunday, then 1 equals Monday, and so on.
		uint8_t dayOfWeek(uint8_t day, uint8_t month, uint16_t year);

		void setSQW(bool enable = true, uint8_t freq = DS3231_SQW_FREQ_1HZ, bool bbSQW = false);
		void enableOscillator(bool enable);
		void enableINTCN(bool enable = true);
		uint8_t readRegBit(uint8_t addrReg, uint8_t bit);
		void setRegBit(uint8_t addrReg, uint8_t bit);
		void clearRegBit(uint8_t addrReg, uint8_t bit);

		uint8_t readBytes(uint8_t startingPointer, uint8_t data[], uint8_t length);
		void writeByte(uint8_t startingPointer, uint8_t data);
		void writeBytes(uint8_t startingPointer, uint8_t data[], uint8_t length);

		uint8_t bcd2bin (uint8_t bcd) { return((bcd & 0x0f) + (bcd >> 4) * 10); }
		uint8_t bin2bcd (uint8_t bin) { return(((bin/10)<<4) + bin%10); } // input range: 0 - 99

		// RFC 3339 Appendix C. Leap Years
		uint8_t leapYear(uint16_t year) { return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)); }
		
		// Returns the last Sunday in March.
		uint8_t summerTimeDay() { return(31 - dayOfWeek(31, 03, YYYY)); }
		uint8_t summerTimeDay(uint16_t YYYY) { return(31 - dayOfWeek(31, 03, YYYY)); }
		// Returns the last Sunday in October.
		uint8_t winterTimeDay() { return(31 - dayOfWeek(31, 10, YYYY)); }
		uint8_t winterTimeDay(uint16_t YYYY) { return(31 - dayOfWeek(31, 10, YYYY)); }
		
		// Returns the number of days in the month.
		uint8_t nrDaysMonth(uint8_t month, uint16_t year);

		// Returns the number of day in the year.
		uint16_t dayOfYear();
		
	protected:
		uint8_t addressRTC;
};

#endif // PS_SIMPLE_DS3231_H