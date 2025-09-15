#include "Meteo.h"
#include <WiFi.h>
#include <HTTPClient.h>

int sendMessage(const String& request) {
    WiFiClient wifi;
    HTTPClient http;
    int result = 0;
    if (http.begin(wifi, request)) {
        result = http.GET();
    }
    http.end();
    wifi.stop();
    return result;
}

#include <DateTime.h>
#include <Wire.h>
#include <DFRobot_AHT20.h>
#include <DFRobot_BMP280.h>

// Meteostation
// I2C device found at address 0x38  AHT20
// I2C device found at address 0x77  BMP280(small, 1v8-3v3)
// #include <I2CScanner.h> I2CScanner scanner;  scanner.Init(); scanner.Scan();
DFRobot_AHT20 aht20;
DFRobot_BMP280_IIC bmp280(&Wire, DFRobot_BMP280_IIC::eSdoHigh);

void Meteo::initSensors() const {
    auto aht20_status = aht20.begin();

    Serial.print(F("Initializing sensor AHT20... "));
    if (aht20_status) {
        Serial.print(F("FAILED: "));
        Serial.println(aht20_status);
    }
    else Serial.println(F("OK"));

    Serial.print(F("Initializing sensor BMP280... "));
    bmp280.reset();
    if (bmp280.begin()) {
        Serial.print(F("FAILED: "));
        Serial.println(bmp280.lastOperateStatus);
    }
    else Serial.println(F("OK"));
}

bool Meteo::updateSensors() {
    if (aht20.startMeasurementReady(true)) {
        humid = aht20.getHumidity_RH();  // Relative humidity, range 0-100 (%RH)
        temp1 = aht20.getTemperature_C(); // Temperature, range -40-80 (℃)
        auto press_pa = bmp280.getPressure(); // Pressure, (Pa)
        // float alti = bmp280.calAltitude(1015.0f, press_pa); // 1015.0f - sea level pressure
        press = press_pa / 133.3224f; // Pressure, (mmHg)
        temp2 = bmp280.getTemperature();
        ++measx;
        timex = DateTime::now();
        return true;
    }
    return false;
}

void Meteo::formatString(String& format) const {
    format.replace(F("{TIMEX}"), String(timex));
    format.replace(F("{MEASX}"), String(measx));
    format.replace(F("{HUMID}"), String(humid));
    format.replace(F("{TEMP1}"), String(temp1));
    format.replace(F("{PRESS}"), String(press));
    format.replace(F("{TEMP2}"), String(temp2));
}

String Meteo::formatTime(const char* timeCustomFormat) const {
    return DateTime(timex).toString(timeCustomFormat);
}

bool Meteo::writeLogger() const {
    String request(F("http://192.168.0.28:8080/push-data?stream=meteo&data={TIMEX};{MEASX};{HUMID};{TEMP1};{PRESS};{TEMP2}"));
    formatString(request);
    return sendMessage(request) == 200;
}

