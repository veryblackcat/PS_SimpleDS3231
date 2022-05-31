// Piotr Sylwesiuk (c)2022

#ifndef PS_SIMPLE_DS3231_H
#define PS_SIMPLE_DS3231_H

#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>

#define DS3231_ADDRESS 0x68 

class rtcDS3231 {
	public:
		TwoWire *twi;
		uint8_t dataBuffer[7];
		uint8_t hh, mm, ss, dow, DD, MM;
		uint16_t YYYY;
		float temp; // temperature

		rtcDS3231(uint8_t addr=DS3231_ADDRESS);
		void begin(uint32_t clkTWI = 100000);

		void getDateTime();
		void getAlarm1() {} // under construction
		void getAlarm2() {} // under construction
		void getTemperature();
		void getControl();

		uint8_t hour() 			{ return(hh); }
		uint8_t minutes() 		{ return(mm); }
		uint8_t seconds() 		{ return(ss); }
		uint8_t dayOfWeek() 	{ return(dow); }
		uint8_t day() 			{ return(DD); }
		uint8_t month() 		{ return(MM); }
		uint16_t year()			{ return(YYYY); }
		float temperature() 	{ return(temp); }

		void setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t date, uint8_t month, uint16_t year);
		void hour(uint8_t hour);
		void minutes(uint8_t minutes);
		void seconds(uint8_t seconds);
		void dayOfWeek(uint8_t dow);
		void day(uint8_t day);
		void month(uint8_t month);
		void year(uint16_t year);

		uint8_t readBytes(uint8_t startingPointer, uint8_t nrBytes);
		void writeByte(uint8_t startingPointer, uint8_t data);
		void writeBytes(uint8_t data[], uint8_t length, uint8_t startingPointer);

		uint8_t bcd2bin (uint8_t bcd) { return((bcd & 0x0f) + (bcd >> 4) * 10); }
		uint8_t bin2bcd (uint8_t bin) { return((bin/10)<<4 + bin%10); } // input range: 0 - 99
		
	protected:
		uint8_t addressRTC;
};

#endif // PS_SIMPLE_DS3231_H