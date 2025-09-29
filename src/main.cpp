#include <Arduino.h>
#include "Meteo.h"
#include "Logger.h"
#include "init.h"
#include "webui.h"
#include "DeviceConfig.h"

DeviceConfig cfg;
Meteo meteo(5000UL);
Logger logger(F("http://192.168.0.28:8080/push-data?stream=meteo&data={TIMEX};{MEASX};{HUMID};{TEMP1};{PRESS};{TEMP2}"), 300000UL);

void setup(void) {
    Serial.begin(SERIAL_SPEED);
    Serial.print(F("\n\nESP32-MeteoLogger v"));
    Serial.println(F(FIRMWARE_VERSION));
    Serial.println(F("======================"));

    mountFileSystem();
    
    if (monitorModeWPS(true)) {
        loadDeviceConfig(cfg, true);
        initAP();
        initWebServerAP();
    }
    else {
        loadDeviceConfig(cfg, false);
        initConnection(cfg);
        initTimeSync(cfg);
        initWebServer();
        meteo.initSensors();
    }
}

void loop(void) {
    if (!monitorModeWPS()) {
        watchConnection();

        if (meteo.pollUpdate())
            logger.pollWrite(meteo);
    }

    handleWebServer();
}