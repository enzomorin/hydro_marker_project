#include <Arduino.h>
#include <SD_Logger.h>
#include <SPI_Radio.h>
#include <Sensors.h>
#include <Button_Logic.h>
#include <Buzzer_Logic.h>
#include <System_Check.h>
#include "config.h"

// Objects
SD_Logger SD(Serial1);
SPI_Radio radio(Pins::RADIO_GDO0, Pins::RADIO_CSN);

BME_280 bme(Pins::BME_ADDR, BMEConfig::RETRY_INTERVAL_MS, BMEConfig::TIMEOUT_MS);
Water_Pressure water(Pins::WATER_PRESSURE, WaterConfig::WATER_ADC_RESOLUTION);
Hall_Sensor hall(Pins::HALL_SENSOR);

Buzzer_Logic buzzer(Pins::BUZZER);
Button_Logic button(Pins::BUTTON);

System_Check systemCheck;

// Runtime state
uint32_t counter = 0;
uint32_t lastLog   = 0;
uint32_t lastFlush = 0;

void setup() {
    buzzer.begin();
    buzzer.bootOk();
    delay(500);

    systemCheck.checkSD (SD.begin(9600));
    buzzer.bootOk();
    delay(500);

    systemCheck.checkBME (bme.begin());
    buzzer.bootOk();
    delay(500);

    systemCheck.checkRadio(radio.begin());
    buzzer.bootOk();
    delay(500);

    button.begin();
    buzzer.bootOk();
    delay(500);

    buzzer.begin();
    buzzer.bootOk();
    delay(500);

    // Report
    if (!systemCheck.ok()) {
        char msg[24];
        snprintf(msg, sizeof(msg), "ERR 0x%02X", systemCheck.status());
        buzzer.error(systemCheck.status());
    } else {
        buzzer.bootOk();
    }
}

void loop() {

    // Measure mode management
    button.read();
    // Detect if button pressed show on screen once
    static bool telemetryWasActive = true;
    if (button.telemetryActive() != telemetryWasActive) {
        telemetryWasActive = button.telemetryActive();

        if (button.telemetryActive()) { // ← Handle here the diff mode of measure (test or real mission)
            buzzer.telemetryOn();
        } else {
            buzzer.telemetryOff();
        }
        
        delay(500);
    }

    // Mission logic
    const uint32_t now = millis();

    // to prevent crash we put the buffer data in the SDcard every 10 seconds
    if (now - lastFlush >= Timings::FLUSH_PERIOD_MS) {
        lastFlush = now;

        SD.flush();

        buzzer.sdFlush();
    }
}