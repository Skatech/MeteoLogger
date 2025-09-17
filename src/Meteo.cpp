#include <Wire.h>
#include <DFRobot_AHT20.h>
#include <DFRobot_BMP280.h>
#include <DateTime.h>
#include "Meteo.h"
#include "Delay.h"

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

bool Meteo::updateSensors() { // 79 millis
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

bool Meteo::pollUpdate() {
    static Delay delay;
    if (delay.await(sensingPeriod))
        return  updateSensors(); 
    return false;
}

