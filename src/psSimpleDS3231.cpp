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
    ss  = bcd2bin(dataBuffer[0] & 0x7f);    // seconds
    mm  = bcd2bin(dataBuffer[1] & 0x7f);    // minutes
    hh  = bcd2bin(dataBuffer[2] & 0x3f);    // hour // 12h time ??????
    dow = dataBuffer[3] & 0x03;             // day of week
    DD  = bcd2bin(dataBuffer[4] & 0x3f);    // day (date)
    MM  = bcd2bin(dataBuffer[5] & 0x1f);    // month
    // year
    if(dataBuffer[5] & 0x80) YYYY = 2100 + (bcd2bin(dataBuffer[6]));
    else YYYY = 2000 + bcd2bin(dataBuffer[6]);
}

void rtcDS3231::getTemperature() {

}

void rtcDS3231::getControl() {

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
