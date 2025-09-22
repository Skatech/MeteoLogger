#pragma once
#include <Arduino.h>
#include "DeviceConfig.h"

bool watchConnection();
void initConnection(const DeviceConfig& cfg);
void initTimeSync(const DeviceConfig& cfg);
void mountFileSystem();
void loadDeviceConfig(DeviceConfig& cfg);
