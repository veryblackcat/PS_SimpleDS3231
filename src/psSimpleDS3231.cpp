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
    dow = dataBuffer[3] & 0x07;             // day of week
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
void rtcDS3231::setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t month, uint16_t year) {
    // dow wyliczać automatycznie
}
void rtcDS3231::setTime(uint8_t hour, uint8_t minutes, uint8_t seconds) {
    uint8_t _time[3];
    _time[0] = bin2bcd(seconds);
    _time[1] = bin2bcd(minutes);
    _time[2] = bin2bcd(hour);
    writeBytes(0x00, _time, 3);
}
void rtcDS3231::setDate(uint8_t day, uint8_t month, uint16_t year) {
    // dow wyliczać automatycznie
}
void rtcDS3231::year(uint16_t year) {
    uint8_t _year[2];
    _year[0] = dataBuffer[5] & 0x80; // includes century
    if (year < 2100) _year[0] = 0x80;
    else _year[0] = 0x00;
    _year[0] |= dataBuffer[5];
    _year[1] = bin2bcd(year % 100);
    writeBytes(0x05, _year, 2);
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
void rtcDS3231::writeBytes(uint8_t startingPointer, uint8_t data[], uint8_t length) {
    twi->beginTransmission(addressRTC);
    twi->write(startingPointer);
    twi->write(data, length);
    twi->endTransmission();
}
uint8_t rtcDS3231::calculateDayOfWeek(uint8_t day, uint8_t month, uint16_t year){
    int cent;
    //char *dayofweek[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
    /* adjust months so February is the last one */
    month -= 2;
    if (month < 1) {
        month += 12;
        --year;
    }
    /* split by century */
    cent = year / 100;
    year %= 100;
    //return (((26 * month - 2) / 10 + day + year + year / 4 + cent / 4 + 5 * cent) % 7);
    // For DS3231 - 1 equals Sunday, then 2 equals Monday, and so on.
    return ((((26 * month - 2) / 10 + day + year + year / 4 + cent / 4 + 5 * cent) % 7)+1);
}