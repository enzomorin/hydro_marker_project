#pragma once

#include <cstdint>

namespace Pins {
    // D4 = SDA (I2C) -> BME280
    // D5 = SCL (I2C) -> BME280
    constexpr uint8_t BME_ADDR = 0x76;

    // D6 = TX (Serial1) -> OpenLog RXI
    // D7 = RX (Serial1) -> OpenLog TXO

    // D8 = SCK (SPI) -> Radio
    // D9 = MISO (SPI) -> Radio
    // D10 = MOSI (SPI) -> Radio

    constexpr uint8_t BUTTON = D0;
    constexpr uint8_t BUZZER = D1; // use led pin for now aswell
    constexpr uint8_t WATER_PRESSURE = D2;
    constexpr uint8_t HALL_SENSOR = D3;
    constexpr uint8_t RADIO_GDO0 = D17;
    constexpr uint8_t RADIO_CSN = D19;
}

namespace Timings {
    constexpr uint32_t LOG_PERIOD_MS     = 1000;
    constexpr uint32_t FLUSH_PERIOD_MS   = 10000;
}

namespace WaterConfig {
    constexpr uint16_t WATER_ADC_RESOLUTION = 4096; // 12-bit nRF52840
}

namespace BMEConfig {
    constexpr uint32_t RETRY_INTERVAL_MS = 500;
    constexpr uint32_t TIMEOUT_MS = 1000;
}