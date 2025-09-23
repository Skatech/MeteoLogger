#include <ConfigUtils.h>
#include "DeviceConfig.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof" // disable IPAddress fields layout warning
const String __DeviceConfig_File = F("/config/device-config.json");
const FieldHandle __DeviceConfig_Fields[] = {
    { F("ssid"), FieldType::StringT, offsetof(DeviceConfig, ssid) },
    { F("pass"), FieldType::StringT, offsetof(DeviceConfig, pass) },
    { F("host"), FieldType::StringT, offsetof(DeviceConfig, host) },
    
    { F("zone"), FieldType::StringT, offsetof(DeviceConfig, zone) },
    { F("tsns"), FieldType::StringT, offsetof(DeviceConfig, tsns) },

    { F("auth"), FieldType::StringT, offsetof(DeviceConfig, auth) },

    { F("addr"), FieldType::IPAddressT, offsetof(DeviceConfig, addr) },
    { F("gate"), FieldType::IPAddressT, offsetof(DeviceConfig, gate) },
    { F("mask"), FieldType::IPAddressT, offsetof(DeviceConfig, mask) },
    { F("dns1"), FieldType::IPAddressT, offsetof(DeviceConfig, dns1) },
    { F("dns2"), FieldType::IPAddressT, offsetof(DeviceConfig, dns2) },
};
#pragma GCC diagnostic pop

bool DeviceConfig::load(bool defaults) {
    return ConfigUtils::loadJSON(__DeviceConfig_Fields,
        sizeof(__DeviceConfig_Fields) / sizeof(FieldHandle), this,
            defaults ? F("/config/device-config-defaults.json") : __DeviceConfig_File);
}

bool DeviceConfig::save() {
    return ConfigUtils::saveJSON(__DeviceConfig_Fields,
        sizeof(__DeviceConfig_Fields) / sizeof(FieldHandle), this, __DeviceConfig_File);
}

bool DeviceConfig::setField(const String& name, const String& value) {
    return ConfigUtils::setField(__DeviceConfig_Fields,
        sizeof(__DeviceConfig_Fields) / sizeof(FieldHandle), this, name, value);
}

void DeviceConfig::print() {
    ConfigUtils::printFields(__DeviceConfig_Fields,
        sizeof(__DeviceConfig_Fields) / sizeof(FieldHandle), this);
}

