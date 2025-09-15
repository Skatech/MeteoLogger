#ifndef DateTime_h // Skatech Lab (c) - DateTime v1.0
#define DateTime_h

#include <Arduino.h>

class DateTime {
private:
    time_t _t;

public:
    constexpr static time_t TIME_NONE = -1; 
    constexpr static time_t TIME_ZERO = 0; 

    DateTime(const time_t sec = TIME_ZERO) : _t(sec) {
    }

    DateTime(const DateTime& rhs) : _t(rhs._t) {
    }

    DateTime& operator = (time_t rhs) {
        _t = rhs; return *this;
    }

    DateTime& operator = (const DateTime& rhs) {
        _t = rhs._t; return *this;
    }

    operator time_t () const {
        return _t;
    }

    // Seconds since Jan 1 1970
    time_t toSecondsSinceEpoch() const {
        return _t;
    }

    DateTime toDateTimeUTC() const {
        struct tm* t = gmtime(&_t); 
        return mktime(t);
    }

    // True when contains correct date, othrwise false
    bool isDateTime() const {
        return _t > TIME_NONE;
    }

    /* Return custom formatted time string, options:
    %a	Abbreviated weekday name *	                                Thu
    %A	Full weekday name *                                         Thursday
    %b	Abbreviated month name *	                                Aug
    %B	Full month name *	August
    %c	Date and time representation *	                            Thu Aug 23 14:55:02 2001
    %C	Year divided by 100 and truncated to integer (00-99)	    20
    %d	Day of the month, zero-padded (01-31)	                    23
    %D	Short MM/DD/YY date, equivalent to %m/%d/%y	                08/23/01
    %e	Day of the month, space-padded ( 1-31)	                    23
    %F	Short YYYY-MM-DD date, equivalent to %Y-%m-%d	            2001-08-23
    %g	Week-based year, last two digits (00-99)	                01
    %G	Week-based year                                             2001
    %h	Abbreviated month name * (same as %b)	                    Aug
    %H	Hour in 24h format (00-23)	                                14
    %I	Hour in 12h format (01-12)	                                02
    %j	Day of the year (001-366)	                                235
    %m	Month as a decimal number (01-12)	                        08
    %M	Minute (00-59)	                                            55
    %n	New-line character ('\n')	
    %p	AM or PM designation	                                    PM
    %r	12-hour clock time *	                                    02:55:02 pm
    %R	24-hour HH:MM time, equivalent to %H:%M	                    14:55
    %S	Second (00-61)	                                            02
    %t	Horizontal-tab character ('\t')	
    %T	ISO 8601 time format (HH:MM:SS), equivalent to %H:%M:%S	    14:55:02
    %u	ISO 8601 weekday as number with Monday as 1 (1-7)	        4
    %U	Week number with the first Sunday as the first day of week one (00-53)	33
    %V	ISO 8601 week number (01-53)    	                        34
    %w	Weekday as a decimal number with Sunday as 0 (0-6)	        4
    %W	Week number with the first Monday as the first day of week one (00-53)	34
    %x	Date representation *	                                    08/23/01
    %X	Time representation *	                                    14:55:02
    %y	Year, last two digits (00-99)	                            01
    %Y	Year	                                                    2001
    %z	ISO 8601 offset from UTC in timezone (1 minute=1, 1 hour=100) If timezone cannot be determined, no characters	+100
    %Z	Timezone name or abbreviation * If timezone cannot be determined, no characters	CDT
    %%	A % sign	                                                % */
    String toString(const char* format = "%Y-%m-%d %H:%M:%S") const;

    // ISO format date-time string YYYYMMDDTHHMMSS
    String toISOString() const {
        return toString("%Y%m%dT%H%M%S");
    }

    // ISO format date-time string YYYY-MM-DDTHH:MM:SS
    String toLongISOString() const {
        return toString("%Y-%m-%dT%H:%M:%S");
    }

    // Date string YYYY-MM-DD
    String toDateString() const {
        return toString("%Y-%m-%d");
    }

    // Time string HH:MM:SS
    String toTimeString() const {
        return toString("%H:%M:%S");
    }

    // Current system time
    static DateTime now() {
        return time(nullptr);
    }

    // Constructs DateTime from parts, mon: 0-11, mday: 1-31, hour: 0-23, min: 0-59, sec: 0-60*
    static DateTime fromParts(uint16_t year, uint8_t mon,
            uint8_t mday, uint8_t hour, uint8_t min, uint8_t sec);

    // Constructs DateTime from string using sscanf format template
    static DateTime parse(const char* input,
            const char* sscanf_format, int parts_expected, size_t min_length);

    // Create DateTime from ISO string format YYYYMMDDTHHMMSS
    static DateTime parseISOString(const char* input) {
        return parse(input, "%4d%2d%2dT%2d%2d%2d", 6, 15);
    }

    // Create DateTime from long ISO string format YYYY-MM-DDTHH:MM:SS
    static DateTime parseLongISOString(const char* input) {
        return parse(input, "%4d-%2d-%2dT%2d:%2d:%2d", 6, 20);
    }

    // Return pointer to struct with current Date details
    static tm* asParts(const DateTime& val) {
        return localtime(&(val._t));
    }

    // Set system time
    static bool setSystemTime(const DateTime& val);
};

#endif