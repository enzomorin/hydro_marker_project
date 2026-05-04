#pragma once

#include <Arduino.h>

struct WaterPressureData {
    float depthMM;       // millimeters of water depth
    bool valid;
};

class Water_Pressure {
    public:
        explicit Water_Pressure(
            uint8_t pin,
            uint16_t adcResolution
        );

        bool begin();

        WaterPressureData read();

    private:
        uint8_t         _pin;
        uint16_t        _adcResolution;
        bool            _ready = false;

        uint8_t         _depthMM           = 0;
        float           _senseOhm          = 120.0f;
        uint16_t        _currentRange      = 5000;
        uint8_t         _waterDensity      = 1; // g/cm³

        static constexpr float   _vRef_mV  = 3300.0f;
        static constexpr float   _Imax_mA  = 20.0f;     // 20mA = maxDepthMM
        static constexpr float   _Ilow_mA  = 4.00f;
        static constexpr float   _Ispan_mA = 16.0f;    // 20 - 4 = 16mA span
        static constexpr uint8_t _samples  = 16;

        float _adcToCurrentMA(uint16_t raw) const;
        float _smoothedCurrentMA();
};