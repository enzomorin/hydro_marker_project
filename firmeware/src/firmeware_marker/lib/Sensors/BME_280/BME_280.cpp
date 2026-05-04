#include "BME_280.h"
#include <Wire.h>

BME_280::BME_280(uint8_t i2cAddress, uint32_t retryIntervalMs, uint32_t timeoutMs)
    : _i2cAddress(i2cAddress)
    , _retryIntervalMs(retryIntervalMs)
    , _timeoutMs(timeoutMs)
    , _ready(false)
{}

bool BME_280::_tryInit() {
    return _bme.begin(_i2cAddress, &Wire);
}

bool BME_280::begin() {
    uint32_t start = millis();

    while (millis() - start < _timeoutMs) {
        if (_tryInit()) { _ready = true; return true; }
        delay(_retryIntervalMs);
    }

    return false;
}

bool BME_280::ready() const { return _ready; }

bool BME_280::update() {
    if (!_ready) return false;

    // BME280 updates automatically — just validate next read
    BMEData data = readALL();
    return data.valid;
}

bool BME_280::_validateData(const BMEData& data) const {
    // catches sensor not connected or returning garbage
    if (data.temperature < -40.0f || data.temperature > 85.0f)  return false;
    if (data.pressure    < 300.0f || data.pressure    > 1100.0f) return false;
    if (data.altitude    < -500.0f || data.altitude   > 9000.0f) return false;
    return true;
}

BMEData BME_280::_buildReading() {
    BMEData data = {0.0f, 0.0f, 0.0f, false};
    if (!_ready) return data;

    data.temperature = _bme.readTemperature();
    data.pressure    = _bme.readPressure() / 100.0f;
    data.altitude    = _bme.readAltitude(data.pressure);
    data.valid       = _validateData(data);
    return data;
}

BMEData BME_280::readALL() {
    return _buildReading();
}