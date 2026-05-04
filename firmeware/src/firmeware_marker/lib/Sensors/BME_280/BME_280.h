#pragma once

#include <Arduino.h>
#include <Adafruit_BME280.h>

struct BMEData {
    float temperature;  // °C
    float pressure;     // hPa
    float altitude;     // m
    bool  valid;        // false if the read failed or sensor not ready
};

class BME_280 {
    public:
        explicit BME_280(
            uint8_t i2cAddress         = 0x76,
            uint32_t retryIntervalMs   = 500,
            uint32_t timeoutMs         = 1000
        );

        bool begin();

        bool update();

        bool ready() const;

        BMEData readALL();

    private:
        uint8_t  _i2cAddress;
        uint32_t _retryIntervalMs;
        uint32_t _timeoutMs;

        // state
        bool _ready;

        // config
        Adafruit_BME280 _bme;

        // helpers
        bool _tryInit();
        bool _validateData(const BMEData& data) const;
        BMEData _buildReading();
};