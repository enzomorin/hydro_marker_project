#pragma once

#include <Arduino.h>

namespace SystemStatus {
    constexpr uint8_t OK      = 0x00;
    constexpr uint8_t BME     = 0x01; // bit 0
    constexpr uint8_t SD      = 0x02; // bit 1
    constexpr uint8_t RADIO   = 0x04; // bit 2
}

class System_Check {
    public:
        System_Check() = default;

        void checkBME(bool beginResult);
        void checkSD(bool beginResult);
        void checkRadio(bool beginResult);

        uint8_t status() const { return _status; }
        bool ok() const { return _status == SystemStatus::OK; }
        bool failed(uint8_t component) const { return _status & component; }

    private:
        uint8_t _status = SystemStatus::OK;
};