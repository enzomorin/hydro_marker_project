#include "Buzzer_Logic.h"

Buzzer_Logic::Buzzer_Logic(uint8_t pin)
    : _pin(pin)
{}

void Buzzer_Logic::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void Buzzer_Logic::on() {
    digitalWrite(_pin, HIGH);
}

void Buzzer_Logic::off() {
    digitalWrite(_pin, LOW);
}

void Buzzer_Logic::beep(uint16_t durationMs) {
    on();
    _wait(durationMs);
    off();
}

void Buzzer_Logic::_wait(uint16_t ms) {
    uint32_t start = millis();
    while (millis() - start < ms) {}
}

void Buzzer_Logic::bootOk() {
    beep(80);  _wait(60);
    beep(80);  _wait(60);
    beep(160);
}

void Buzzer_Logic::telemetryOn() {
    beep(80);  _wait(60);
    beep(300);
}

void Buzzer_Logic::telemetryOff() {
    beep(300); _wait(60);
    beep(80);
}

void Buzzer_Logic::logTick() {
    beep(140);
}

void Buzzer_Logic::jackBlocked() {
    beep(200); _wait(200);
    beep(200);
}

void Buzzer_Logic::sdFlush() {
    beep(400); _wait(500);
    beep(200); _wait(500);
    beep(200);
}

void Buzzer_Logic::error(uint8_t errorByte) {
    for (uint8_t bit = 0; bit < 8; bit++) {
        if (errorByte & (1 << bit)) {
            beep(300);
            _wait(300);
        }
    }
}