#ifndef DeviceConfig_h
#define DeviceConfig_h

#include <Arduino.h>
#include <IPAddress.h>
#include <Csv.h>

class DeviceConfig {
public:
    String ssid;
    String pass;
    String host;
    
    String zone;
    String tsns;

    String auth;

    IPAddress addr;
    IPAddress gate;
    IPAddress mask;
    IPAddress dns1;
    IPAddress dns2;

    bool load(bool defaults);
    bool save();
    bool setField(const String& name, const String& value);
    void print();
};

#endif
