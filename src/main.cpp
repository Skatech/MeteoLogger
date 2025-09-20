#include <Arduino.h>
#include "Meteo.h"
#include "Logger.h"
#include "init.h"
#include "webui.h"

Meteo meteo(5000UL);
Logger logger(F("http://192.168.0.28:8080/push-data?stream=meteo&data={TIMEX};{MEASX};{HUMID};{TEMP1};{PRESS};{TEMP2}"), 300000UL);

void setup(void) {
    Serial.begin(SERIAL_SPEED);
    Serial.print(F("\n\nESP32-MeteoLogger v"));
    Serial.println(F(FIRMWARE_VERSION));
    Serial.println(F("======================"));
    initConnection();
    initTimeSync();
    initWebServer();
    meteo.initSensors();
}

void loop(void) {
    watchConnection();
    handleWebServer();

    if (meteo.pollUpdate()) {
        logger.pollWrite(meteo);
    }
}