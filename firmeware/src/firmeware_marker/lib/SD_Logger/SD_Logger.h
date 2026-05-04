#pragma once

#include <Arduino.h>

class SD_Logger {
public:
    explicit SD_Logger(HardwareSerial& serial);

    bool begin(uint32_t baud);

    void startLine();
    void endLine();

    void addInt(int32_t value);
    void addFloat(float value, uint8_t precision = 2);
    void addField(const char* field);
    void addField(const char* field, size_t len);

    void flush();     // optional (calls OpenLog sync)
    void update();    // non-blocking UART TX

    const uint8_t* getLastLine() const { return (const uint8_t*)_lastLine; }
    size_t getLastLineLength() const { return _lastLineLen; }

private:
    HardwareSerial& _serial;

    static constexpr size_t LINE_BUFFER = 128;
    static constexpr size_t TX_BUFFER   = 256;

    static constexpr size_t BOOT_MS = 2000;
    static constexpr size_t TIMEOUT_MS = 200;
    static constexpr size_t ESCAPE_GAP_MS = 10;
    static constexpr size_t CMD_MS = 150;

    char _line[LINE_BUFFER];
    size_t _lineIdx = 0;
    bool _overflow = false;

    char _lastLine[LINE_BUFFER];
    size_t _lastLineLen = 0;

    uint8_t _tx[TX_BUFFER];
    volatile uint16_t _txHead = 0;
    volatile uint16_t _txTail = 0;

    char _filename[13];
    bool _ready = false;

    void pushByte(uint8_t b);
    void pushData(const uint8_t* d, size_t len);

    void sendEscape();
    void drainRX();
    bool waitFor(char c, uint16_t timeoutMs);

    // CSV building
    bool openFile();
    void writeHeader();
};