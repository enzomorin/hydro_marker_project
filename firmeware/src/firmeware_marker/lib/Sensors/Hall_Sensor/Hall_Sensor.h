#pragma once

#include <Arduino.h>

class Hall_Sensor {
    public:
        explicit Hall_Sensor(uint8_t pin);

        void begin();
        float getRPM();

    private:
        uint8_t _pin;

        volatile uint32_t _pulseCount;
        uint32_t _lastTime;

        static Hall_Sensor* _instance; // needed for static ISR
        static void _isr();
};