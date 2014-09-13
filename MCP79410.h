#ifndef mcp79410_h
#define mcp79410_h

// Simple general-purpose date/time class (no TZ / DST / leap second handling!)
class DateTime {
public:
    DateTime (uint32_t t =0);
    DateTime (uint16_t year, uint8_t month, uint8_t day,
                uint8_t hour =0, uint8_t min =0, uint8_t sec =0);
    DateTime (const char* date, const char* time);
    uint16_t year() const       { return yOff; }
    uint8_t month() const       { return m; }
    uint8_t day() const         { return d; }
    uint8_t hour() const        { return (hh%12) ? hh%12 : 12; }
    uint8_t hour24() const      { return hh; }
    uint8_t minute() const      { return mm; }
    uint8_t second() const      { return ss; }
    uint8_t dayOfWeek() const;

    // 32-bit times as seconds since 1/1/2000
    long secondstime() const;   
    // 32-bit times as seconds since 1/1/1970
    uint32_t unixtime(void) const;

protected:
    uint8_t yOff, m, d, hh, mm, ss;
};

class MCP79410{
 public:
  MCP79410();
  void setDateTime (const char* date, const char* time);
	void setDateTime (uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second );

  uint8_t second();
  uint8_t hour();
  uint8_t hour24();
  uint8_t minute();
  uint8_t day();
  uint8_t month();
  uint16_t year();
  DateTime now();
 private:
  uint8_t conv2d(const char* p);
  uint8_t make_hex(uint8_t num);
  uint8_t make_dec(uint8_t num);

  unsigned char getRTCData(const unsigned char adr, const unsigned char validbits);
  void WriteRTCByte(const unsigned char adr, const unsigned char data);
  unsigned char ReadRTCByte(const unsigned char adr);

};

#endif
