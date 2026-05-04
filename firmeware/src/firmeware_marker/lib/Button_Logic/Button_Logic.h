#pragma once

#include <Arduino.h>

class Button_Logic {
    public:
        explicit Button_Logic(uint8_t BTN_Pin);

        void begin();
        void read();

        bool telemetryActive() const { return _telemetryActive; }

    private:
        uint8_t _pin;

        volatile bool _pressed = false;
        volatile uint32_t _lastPress = 0;
        bool _telemetryActive = true;

        static Button_Logic* _instance;
        static void _isr();
};