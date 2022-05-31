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
void rtcDS3231::setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t date, uint8_t month, uint16_t year){

}
void rtcDS3231::seconds(uint8_t seconds) {
    seconds = bin2bcd(seconds);
    writeByte(0x00, seconds);
}
void rtcDS3231::minutes(uint8_t minutes) {
    minutes = bin2bcd(minutes);
    writeByte(0x01, minutes);
}
void rtcDS3231::hour(uint8_t hour) {
    hour = bin2bcd(hour);   //mode 24h (BIT6 = 0)
    writeByte(0x02, hour);
}
void rtcDS3231::dayOfWeek(uint8_t dow) {
    writeByte(0x03, dow);
}
void rtcDS3231::day(uint8_t day) {
    day = bin2bcd(day);
    writeByte(0x04, day);
}
void rtcDS3231::month(uint8_t month) {
    month = bin2bcd(month) | (dataBuffer[5] & 0x80); // includes century
    writeByte(0x05, month);
}
/*
void rtcDS3231::year(uint16_t year) {
    uint8_t _century = dataBuffer[5] & 0x80; // includes century
    if (year < 2100) _century = 0x80;
    else _century = 0x00;
    _century |= dataBuffer[5];
    year = bin2bcd(year%100);
    writeByte(0x05, _century);
    writeByte(0x06, year);
}*/
void rtcDS3231::year(uint16_t year) {
    uint8_t _year[2];
    _year[0] = dataBuffer[5] & 0x80; // includes century
    if (year < 2100) _year[0] = 0x80;
    else _year[0] = 0x00;
    _year[0] |= dataBuffer[5];
    _year[1] = bin2bcd(year%100);
    writeBytes(_year, 2, 0x05);
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
void rtcDS3231::writeByte(uint8_t startingPointer, uint8_t data) {
    twi->beginTransmission(addressRTC);
    twi->write(startingPointer);
    twi->write(data);
    twi->endTransmission();
}
void rtcDS3231::writeBytes(uint8_t data[], uint8_t length, uint8_t startingPointer) {
    twi->beginTransmission(addressRTC);
    twi->write(startingPointer);
    twi->write(data, length);
    twi->endTransmission();
}
