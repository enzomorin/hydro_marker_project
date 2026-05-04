#pragma once

#include <Arduino.h>

class Buzzer_Logic {
    public:
        explicit Buzzer_Logic(uint8_t pin);

        void begin();

        // Patterns
        void bootOk();
        void telemetryOn();
        void telemetryOff();
        void logTick();
        void jackBlocked();
        void sdFlush();
        void error(uint8_t errorByte);

    private:
        uint8_t _pin;

        void _wait(uint16_t ms);

        // sound managing
        void on();
        void off();
        void beep(uint16_t durationMs);
};