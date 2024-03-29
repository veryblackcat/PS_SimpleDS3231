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
    hh  = bcd2bin(dataBuffer[2] & 0x3f);    // hour
    dow = dataBuffer[3] & 0x07;             // day of week
    DD  = bcd2bin(dataBuffer[4] & 0x3f);    // day (date)
    MM  = bcd2bin(dataBuffer[5] & 0x1f);    // month
    // year
    if(dataBuffer[5] & 0x80) YYYY = 2100 + (bcd2bin(dataBuffer[6]));
    else YYYY = 2000 + bcd2bin(dataBuffer[6]);
}
uint8_t rtcDS3231::getTemperature(bool force) {
    uint8_t _timeout;
    uint8_t _temp[2];
    if(force) {
        _timeout = millis() + 10;
        while(readRegBit(DS3231_STATUS_REG, DS3231_STATUS_BSY)) {
            // timeout
            if(millis() >= _timeout) return(0);
        }
        setRegBit(DS3231_CONTROL_REG, DS3231_CTRL_CONV);
    }
    _timeout = millis() + 10;
    while(readRegBit(DS3231_STATUS_REG, DS3231_STATUS_BSY)) {
        // timeout
        if(millis() >= _timeout) return(0);
    }
    // read temperature
    readBytes(DS3231_TEMP_REG_U, _temp, 2);
    if(int(_temp[0]) < 0) {
        temp = int(_temp[0]) - (_temp[1]>>6) * 0.25f;
        tempInt = int(_temp[0]) - (_temp[1]>>7);
    } else {
        temp = int(_temp[0]) + (_temp[1]>>6) * 0.25f;
        tempInt = int(_temp[0]) + (_temp[1]>>7);
    }
    return(1);
}

void rtcDS3231::setDateTime(uint8_t hour, uint8_t minutes, uint8_t seconds, uint8_t day, uint8_t month, uint16_t year) {
    // The day of the week is automatically calculated from the date.
    // The day of the week for DS3231 - 1 equals Sunday, then 2 equals Monday, and so on.
    uint8_t _dt[7];
    _dt[0] = bin2bcd(seconds);
    _dt[1] = bin2bcd(minutes);
    _dt[2] = bin2bcd(hour);
    _dt[3] = dayOfWeek(day, month, year) + 1;
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
    // The day of the week is automatically calculated from the date.
    // The day of the week for DS3231 - 1 equals Sunday, then 2 equals Monday, and so on.
    uint8_t _date[4];
    _date[0] = dayOfWeek(day, month, year) + 1;
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
// RFC 3339 Appendix B. Day of the Week
// The day of the week for dates on or after 0000-03-01.
// 0 equals Sunday, then 1 equals Monday, and so on.
uint8_t rtcDS3231::dayOfWeek(uint8_t day, uint8_t month, uint16_t year) {
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
    // 0 equals Sunday, then 1 equals Monday, and so on.
    return (((26 * month - 2) / 10 + day + year + year / 4 + cent / 4 + 5 * cent) % 7);
}
// It also changes the logic level of the INTCN bit.
void rtcDS3231::setSQW(bool enable, uint8_t freq, bool bbSQW) {
    uint8_t _controlReg;
    
    readBytes(DS3231_CONTROL_REG, &_controlReg, 1);

    if(bbSQW) _controlReg |= DS3231_CTRL_BBSQW;
    else _controlReg &= ~DS3231_CTRL_BBSQW; 
    
    if(enable) _controlReg &= ~DS3231_CTRL_INTCN;
    else _controlReg |= DS3231_CTRL_INTCN; 

    _controlReg &= ~0x18;   // Clear RS2 (BIT 4 - 10h), RS1 (BIT 3 - 08h) to 0
    _controlReg |= freq;    // Set freq: RS2 (BIT 4 - 10h), RS1 (BIT 3 - 08h)

    writeByte(DS3231_CONTROL_REG, _controlReg);
}
// Control Register (0Eh) - BIT7 EOSC (80h)
// When the DS3231 is powered by VCC, the oscillator is always on regardless of the status of the EOSC bit.
void rtcDS3231::enableOscillator(bool enable) { 
    uint8_t _controlReg;
    readBytes(DS3231_CONTROL_REG, &_controlReg, 1);
    if(enable) _controlReg &= ~DS3231_CTRL_EOSC;    // EOSC enable
    else _controlReg |= DS3231_CTRL_EOSC;           // EOSC disable
    writeByte(DS3231_CONTROL_REG, _controlReg);
}
// Control Register (0Eh) - BIT7 INTCN (40h)
void rtcDS3231::enableINTCN(bool enable) {
    uint8_t _controlReg;
    readBytes(DS3231_CONTROL_REG, &_controlReg, 1);
    if(enable) _controlReg |= DS3231_CTRL_INTCN;
    else _controlReg &= ~DS3231_CTRL_INTCN;
    writeByte(DS3231_CONTROL_REG, _controlReg);
}
uint8_t rtcDS3231::readRegBit(uint8_t addrReg, uint8_t bit) {
    uint8_t _reg;
    readBytes(addrReg, &_reg, 1);
    return(_reg & bit);
}
void rtcDS3231::setRegBit(uint8_t addrReg, uint8_t bit) {
    uint8_t _reg;
    readBytes(addrReg, &_reg, 1);
    writeByte(addrReg, _reg | bit);
}
void rtcDS3231::clearRegBit(uint8_t addrReg, uint8_t bit) {
    uint8_t _reg;
    readBytes(addrReg, &_reg, 1);
    writeByte(addrReg, _reg & ~bit);
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
// Returns the number of days in the month.
uint8_t rtcDS3231::nrDaysMonth(uint8_t month, uint16_t year) {
    const uint8_t _days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (month == 2) {
        if(leapYear(year)) return(29);
        else return(28);
    } else return(_days[month-1]);
}
// Returns the number of day in the year.
uint16_t rtcDS3231::dayOfYear() { // getDateTime() ?????
    const uint16_t _days[12] = { 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
    if(MM > 2) {
        if(leapYear(YYYY)) return(_days[MM-2]+DD+1);
        else return(_days[MM-2]+DD);
    } else {
        if(MM == 1) return(DD);
        else return(_days[0]+DD);
    }
}