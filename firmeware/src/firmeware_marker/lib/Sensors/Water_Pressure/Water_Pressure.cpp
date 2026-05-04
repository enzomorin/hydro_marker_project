#include "Water_Pressure.h"

Water_Pressure::Water_Pressure(uint8_t pin, uint16_t adcResolution)
    : _pin(pin)
    , _adcResolution(adcResolution)
    , _ready(false)
{}

float Water_Pressure::_adcToCurrentMA(uint16_t raw) const {
    float v_mV = raw * _vRef_mV / _adcResolution;
    return v_mV / _senseOhm;
}

float Water_Pressure::_smoothedCurrentMA() {
    float sum = 0.0f;
    for (uint8_t i = 0; i < _samples; i++) {
        sum += _adcToCurrentMA(analogRead(_pin));
    }
    return sum / _samples;
}

bool Water_Pressure::begin() {
    pinMode(_pin, INPUT);
    analogReadResolution(12);   // force 12-bit on nRF52840

    float current = _adcToCurrentMA(analogRead(_pin));

    if (current < _Ilow_mA || current > _Imax_mA) return false;

    _ready = true;
    return true;
}

WaterPressureData Water_Pressure::read() {
    if (!_ready) return {0.0f, false};

    float currentMA = _smoothedCurrentMA();

    // Fault detection: wire-break or overrange
    if (currentMA < _Ilow_mA || currentMA > _Imax_mA) {
        return {0.0f, false};
    }

    // Linear map: 4mA→0mm, 20mA→maxDepthMM, density-corrected
    float depthMM = (currentMA - _Ilow_mA) * (_currentRange / _waterDensity  / _Ispan_mA);
    if (depthMM < 0.0f) depthMM = 0.0f;

    return {depthMM, true};
}