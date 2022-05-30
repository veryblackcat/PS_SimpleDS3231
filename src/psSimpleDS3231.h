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
		uint8_t hour, minutes, seconds, day, date, month;
		uint16_t year;
		float temperature;

		rtcDS3231(uint8_t addr=DS3231_ADDRESS);
		void begin(uint32_t clkTWI = 100000);

		void getDateTime();
		void getAlarm1() {}
		void getAlarm2() {}
		void getTemperature() {}
		void getControl() {}

		uint8_t hour() 			{ return(hour); }
		uint8_t minutes() 		{ return(minutes); }
		uint8_t seconds() 		{ return(seconds); }
		uint8_t day() 			{ return(day); }
		uint8_t date() 			{ return(date); }
		uint8_t month() 		{ return(month); }
		uint8_t year()			{ return(year); }
		float temperature() 	{ return(temperature); }

		void setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t date, uint8_t month, uint16_t year)
		void setHour() {};
		void setMinutes() {};
		void setSeconds() {};
		void setDay() {};
		void setDate() {};
		void setMonth() {};
		void setYear() {};

		uint8_t readBytes(uint8_t startingPointer, uint8_t nrBytes);
		uint16_t bcd2bin (uint8_t bcd) { (bcd & 0x0f) + (bcd >> 4) * 10 }

		// testowe
		uint8_t t[19];
		void test();
		uint8_t read(uint8_t i);
		
	protected:
		uint8_t addressRTC;
};

#endif // PS_SIMPLE_DS3231_H