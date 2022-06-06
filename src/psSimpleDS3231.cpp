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
    readBytes(0, dataBuffer, 7);
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

void rtcDS3231::setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t month, uint16_t year) {
    // dow liczony automatycznie
    uint8_t _dt[7];
    _dt[0] = bin2bcd(seconds);
    _dt[1] = bin2bcd(minutes);
    _dt[2] = bin2bcd(hour);
    _dt[3] = calculateDayOfWeek(day, month, year);
    _dt[4] = bin2bcd(day);
    _dt[5] = (year < 2100) ? bin2bcd(month) : (bin2bcd(month) & 0x80);
    _dt[6] = bin2bcd(year % 100);
    writeBytes(0x00, _dt, 7);
}
void rtcDS3231::setTime(uint8_t hour, uint8_t minutes, uint8_t seconds) {
    uint8_t _time[3];
    _time[0] = bin2bcd(seconds);
    _time[1] = bin2bcd(minutes);
    _time[2] = bin2bcd(hour);
    writeBytes(0x00, _time, 3);
}
void rtcDS3231::setDate(uint8_t day, uint8_t month, uint16_t year) {
    // dow liczony automatycznie
    uint8_t _date[4];
    _date[0] = calculateDayOfWeek(day, month, year);
    _date[1] = bin2bcd(day);
    _date[2] = (year < 2100) ? bin2bcd(month) : (bin2bcd(month) & 0x80); // includes century
    _date[3] = bin2bcd(year % 100);
    writeBytes(0x03, _date, 4);
}
void rtcDS3231::year(uint16_t year) {
    uint8_t _year[2];
    _year[0] = dataBuffer[5] & 0x80; // includes century
    if (year < 2100) _year[0] = 0x00;
    else _year[0] = 0x80;
    _year[0] |= dataBuffer[5];
    _year[1] = bin2bcd(year % 100);
    writeBytes(0x05, _year, 2);
}
void rtcDS3231::setSQW(bool enable, uint8_t freq) {
    uint8_t _controlReg;
    
    readBytes(0x0e, &_controlReg, 1); // Read Control Register (0Eh)
    
    _controlReg &= ~0x40; // Clear BBSQW (BIT 6 - 40h) - SQW disable

    if(enable) {
        _controlReg &= ~0x18;       // Clear RS2 (BIT 4 - 10h), RS1 (BIT 3 - 08h)
        _controlReg |= 0x40 | freq; // Set BBSQW (BIT 6 - 40h), RS2 (BIT 4 - 10h), RS1 (BIT 3 - 08h)
    }

    writeByte(0x0e, _controlReg); // Write Control Register (0Eh)
}
uint8_t rtcDS3231::readByte(uint8_t startingPointer) {
    twi->beginTransmission(addressRTC);
    twi->write(startingPointer);
    twi->endTransmission();

    uint8_t x = 1;
    //int _nr = twi->requestFrom(addressRTC, 1); 
    //if (twi->available()) data = twi->read();
    //return(_nr);
}
uint8_t rtcDS3231::readBytes(uint8_t startingPointer, uint8_t data[], uint8_t length) {
    twi->beginTransmission(addressRTC);
    twi->write(startingPointer);
    twi->endTransmission();

    uint8_t _nr = twi->requestFrom(addressRTC, length); 
    uint8_t i = 0;
    while (twi->available()) {
        data[i] = twi->read();
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
// RFC 3339 Appendix B. Day of the Week
// The day of the week for dates on or after 0000-03-01.
// For DS3231 - 1 equals Sunday, then 2 equals Monday, and so on.
uint8_t rtcDS3231::calculateDayOfWeek(uint8_t day, uint8_t month, uint16_t year) {
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
    return ((((26 * month - 2) / 10 + day + year + year / 4 + cent / 4 + 5 * cent) % 7) + 1);
}
void rtcDS3231::calculateSummerWinterDay() {
    summerTimeDay = 31 - calculateDayOfWeek(31, 03, YYYY) + 1; // The last Sunday of March.
    winterTimeDay = 31 - calculateDayOfWeek(31, 10, YYYY) + 1; // The last Sunday of October.
}