#include "Hall_Sensor.h"

Hall_Sensor* Hall_Sensor::_instance = nullptr;

Hall_Sensor::Hall_Sensor(uint8_t pin)
    : _pin(pin)
    , _pulseCount(0)
    , _lastTime(0)
{}

void Hall_Sensor::begin() {
    _instance = this;
    pinMode(_pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(_pin), _isr, FALLING); // if pass high to low aswell FALLING
    _lastTime = millis();
}

void Hall_Sensor::_isr() {
    _instance->_pulseCount++;
}

float Hall_Sensor::getRPM() {
    uint32_t now     = millis();
    uint32_t elapsed = now - _lastTime;

    if (elapsed == 0) return 0.0f;

    // Atomically grab and reset pulse count
    noInterrupts();
    uint32_t pulses = _pulseCount;
    _pulseCount = 0;
    interrupts();

    _lastTime = now;

    return (pulses / (float)elapsed) * 60000.0f;
}