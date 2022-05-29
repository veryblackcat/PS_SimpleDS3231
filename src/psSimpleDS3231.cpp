// Piotr Sylwesiuk (c)2022

#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>
#include <psSimpleDS3231.h>

rtcDS3231::rtcDS3231(uint8_t addr) {
//rtcDS3231::rtcDS3231(TwoWire &twi, uint8_t addr) {
	//rtcDS3231::twi = &twi;
    rtcDS3231::twi = &Wire;
    addressRTC = addr;
}

void rtcDS3231::begin() {
    twi->begin();
}

void rtcDS3231::test() {
    twi->beginTransmission(DS3231_ADDRESS);
    twi->write(0);
    twi->endTransmission();

    uint8_t b = twi->requestFrom(DS3231_ADDRESS, 19); 
    uint8_t i = 0;
    while (twi->available()) {
        t[i] = twi->read();
        i++;
  }
}

uint8_t rtcDS3231::read(uint8_t i) {
    return(t[i]);
}