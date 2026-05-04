#include "Button_Logic.h"

Button_Logic* Button_Logic::_instance = nullptr;

Button_Logic::Button_Logic(uint8_t pin)
    : _pin(pin)
{}

void Button_Logic::begin() {
    _instance = this;

    pinMode(_pin, INPUT_PULLUP);
    
    attachInterrupt(digitalPinToInterrupt(_pin), _isr, FALLING);
}

void Button_Logic::_isr() {
    uint32_t now = millis();

    if (now - _instance->_lastPress < 50) return;

    _instance->_lastPress = now;
    _instance->_pressed   = true;
}

void Button_Logic::read() {
    if (!_pressed) return;

    noInterrupts();
    _pressed = false;
    interrupts();

    _telemetryActive = !_telemetryActive;
}