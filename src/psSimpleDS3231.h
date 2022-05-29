// Piotr Sylwesiuk (c)2022

#ifndef PS_SIMPLE_DS3231_H
#define PS_SIMPLE_DS3231_H

#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>

class rtcDS3231 {
	public:
		rtcDS3231(TwoWire &twi);
		
	protected:
}

#endif // PS_SIMPLE_DS3231_H