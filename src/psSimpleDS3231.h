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
		rtcDS3231(uint8_t addr=DS3231_ADDRESS);
		void begin();

		uint8_t t[19];
		void test();
		uint8_t read(uint8_t i);
		
	protected:
		uint8_t addressRTC;
};

#endif // PS_SIMPLE_DS3231_H