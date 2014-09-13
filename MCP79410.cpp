#include <Wire.h>
#include <avr/pgmspace.h>

#include "MCP79410.h"

#define SECONDS_PER_DAY 86400L

#define SECONDS_FROM_1970_TO_2000 946684800
#if (ARDUINO >= 100)
 #include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
#else
 #include <WProgram.h>
#endif


const uint8_t daysInMonth [] PROGMEM = { 31,28,31,30,31,30,31,31,30,31,30,31 }; //has to be const or compiler compaints

////////////////////////////////////////////////////////////////////////////////
// DateTime implementation - ignores time zones and DST changes
// NOTE: also ignores leap seconds, see http://en.wikipedia.org/wiki/Leap_second

DateTime::DateTime (uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000;    // bring to 2000 timestamp from 1970

    ss = t % 60;
    t /= 60;
    mm = t % 60;
    t /= 60;
    hh = t % 24;
    uint16_t days = t / 24;
    uint8_t leap;
    for (yOff = 0; ; ++yOff) {
        leap = yOff % 4 == 0;
        if (days < 365 + leap)
            break;
        days -= 365 + leap;
    }
    for (m = 1; ; ++m) {
        uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
        if (leap && m == 2)
            ++daysPerMonth;
        if (days < daysPerMonth)
            break;
        days -= daysPerMonth;
    }
    d = days + 1;
}

DateTime::DateTime (uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec) {
    //if (year >= 2000)
    //    year -= 2000;
    yOff = year;
    m = month;
    d = day;
    hh = hour;
    mm = min;
    ss = sec;
}

MCP79410 :: MCP79410 (){
   Wire.begin();
}

uint8_t MCP79410 :: second() { return make_dec(getRTCData(0,7)) ;}
uint8_t MCP79410 :: minute() { return make_dec(getRTCData(1,7)) ;}
uint8_t MCP79410 :: hour24() { return make_dec(getRTCData(2,6)) ;}


uint8_t MCP79410 :: day() { return make_dec(getRTCData(4,6)) ;}
uint8_t MCP79410 :: month() { return make_dec(getRTCData(5,5)) ;}
uint16_t MCP79410 :: year() { return make_dec(getRTCData(6,8)) ;}

uint8_t MCP79410 :: hour() { 
	uint8_t h = getRTCData(2,6) ;
	if (h > 12) h -= 12;
	if (h == 0) h = 12;
	return h;
}

DateTime MCP79410 :: now() {
  
  return DateTime (year(), month(), day(), hour24(), minute(), second());
}


void MCP79410 :: setDateTime (uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second ) {
	uint8_t hh = make_hex(hour);
	uint8_t mm = make_hex(minute);
	uint8_t ss = make_hex(second);
    
	
    
	WriteRTCByte(0,0);       //STOP RTC
	
	WriteRTCByte(7,0b10000011);
	WriteRTCByte(1,mm);    //MINUTE=18
	WriteRTCByte(2,hh);    //HOUR=8
	WriteRTCByte(3,0x09);    //DAY=1(MONDAY) AND VBAT=1
	WriteRTCByte(4,day-1);    //DATE=28
	WriteRTCByte(5,month);    //MONTH=2
	WriteRTCByte(6,year);    //YEAR=11
	WriteRTCByte(0,0x80);    //START RTC, SECOND=00
}

void MCP79410 :: setDateTime (const char* date, const char* time) {
    // sample input: date = "Dec 26 2009", time = "12:34:56"
    
    byte yOff = conv2d(date + 7);
    
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
    uint8_t m;
    switch (date[0]) {
        case 'J': m = date[1] == 'a' ? 1 : m = date[2] == 'n' ? 6 : 7; break;
        case 'F': m = 2; break;
        case 'A': m = date[2] == 'r' ? 4 : 8; break;
        case 'M': m = date[2] == 'r' ? 3 : 5; break;
        case 'S': m = 9; break;
        case 'O': m = 10; break;
        case 'N': m = 11; break;
        case 'D': m = 12; break;
    }
   uint8_t d =  make_hex(conv2d(date + 4));
   uint8_t hh = make_hex(conv2d(time));
   uint8_t mm = make_hex(conv2d(time + 3));
   uint8_t ss = make_hex(conv2d(time + 6));
    
  
    
  WriteRTCByte(0,0);       //STOP RTC
  
  WriteRTCByte(7,0b10000011);

  WriteRTCByte(1,mm);    //MINUTE=18
  WriteRTCByte(2,hh);    //HOUR=8
  WriteRTCByte(3,0x09);    //DAY=1(MONDAY) AND VBAT=1
  WriteRTCByte(4,d);    //DATE=28
  WriteRTCByte(5,m);    //MONTH=2
  WriteRTCByte(6,yOff);    //YEAR=11
  WriteRTCByte(0,0x80);    //START RTC, SECOND=00
}

uint8_t MCP79410 :: conv2d(const char* p) {
    uint8_t v = 0;
    if ('0' <= *p && *p <= '9')
        v = *p - '0';
    return 10 * v + *++p - '0';
}
uint8_t MCP79410 :: make_hex(uint8_t num){
    uint8_t units = num % 10;
    uint8_t tens = num / 10;
    return (tens << 4) | units;
}

uint8_t MCP79410 :: make_dec(uint8_t num){
    uint8_t units = num & 0x0F;
    uint8_t tens = num >> 4;
    return tens*10 + units;
}

unsigned char MCP79410 :: ReadRTCByte(const unsigned char adr){
  unsigned char data;
  Wire.beginTransmission(0x6f);
  Wire.write(adr);
  Wire.endTransmission();
  Wire.requestFrom(0x6f,1);
  while (Wire.available()) data=Wire.read();
  return data;
}
 
void MCP79410 :: WriteRTCByte(const unsigned char adr, const unsigned char data){
  Wire.beginTransmission(0x6f);
  Wire.write(adr);
  Wire.write(data);
  Wire.endTransmission();
} 
 
unsigned char MCP79410 :: getRTCData(const unsigned char adr, const unsigned char validbits){
  unsigned char data;
  data = ReadRTCByte(adr);
  data = data & (0xff >> (8-validbits));
  return data;
}
