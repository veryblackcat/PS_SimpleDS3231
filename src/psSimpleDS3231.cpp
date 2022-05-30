// Piotr Sylwesiuk (c)2022

#include <inttypes.h>
#include <Arduino.h>
#include <Wire.h>
#include <psSimpleDS3231.h>

rtcDS3231::rtcDS3231(uint8_t addr) {
    twi = &Wire;
    addressRTC = addr;
}

void rtcDS3231::begin(uint32_t clkTWI) {
    twi->begin();
    twi->setClock(clkTWI);
}

void rtcDS3231::getDateTime() {
    readBytes(0, 7);
    ss = bcd2bin(dataBuffer[0] & 0x7f); // seconds
    mm = bcd2bin(dataBuffer[1] & 0x7f); // minutes
    hh = bcd2bin(dataBuffer[2] & 0x3f); // hour // 12h time ??????
    dow = dataBuffer[3] & 0x03;         // day of week
    d = bcd2bin(dataBuffer[4] & 0x3f);  // day (date)
    m = bcd2bin(dataBuffer[5] & 0x1f);  // month
    // year
    if(dataBuffer[5] & 0x80) y = 2100 + (bcd2bin(dataBuffer[6]));
    else y = 2000 + bcd2bin(dataBuffer[6]);
}

uint8_t rtcDS3231::readBytes(uint8_t startingPointer, uint8_t nrBytes) {
    twi->beginTransmission(addressRTC);
    twi->write(startingPointer);
    twi->endTransmission();

    uint8_t _nr = twi->requestFrom(addressRTC, nrBytes); 
    uint8_t i = 0;
    while (twi->available()) {
        dataBuffer[i] = twi->read();
        i++;
  }
  return(_nr);
}


void rtcDS3231::test() { //testowe
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

uint8_t rtcDS3231::read(uint8_t i) { // testowe
    return(t[i]);
}