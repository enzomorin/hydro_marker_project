#pragma once

#include <Arduino.h>
#include <SPI.h>

class SPI_Radio {
public:
    SPI_Radio(uint8_t pinGDO0, uint8_t pinCSN);
    bool    begin();
    bool    send(const uint8_t* data, uint8_t len);
    int8_t  receive(uint8_t* buf, uint8_t maxLen); // returns bytes read, -1 if none
    void    setIdle();
    void    setRx();

private:
    uint8_t _cs, _gdo0;
    bool    _ready = false;

    void    writeReg(uint8_t addr, uint8_t val);
    uint8_t readReg(uint8_t addr);
    void    writeBurst(uint8_t addr, const uint8_t* data, uint8_t len);
    void    readBurst(uint8_t addr, uint8_t* data, uint8_t len);
    void    strobe(uint8_t cmd);
    uint8_t readStatus(uint8_t addr);
    void    initCC1101();
    void    reset();
};