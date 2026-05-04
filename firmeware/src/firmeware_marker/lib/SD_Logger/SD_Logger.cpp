#include "SD_Logger.h"
#include <string.h>
#include <stdio.h>

static char* dtostrf_portable(float val, signed char width, unsigned char prec, char *buf)
{
    sprintf(buf, "%.*f", prec, val);
    return buf;
}

SD_Logger::SD_Logger(HardwareSerial& serial)
    :_serial(serial) {}

bool SD_Logger::begin(uint32_t baud) {
    _serial.begin(baud);
    delay(BOOT_MS);

    sendEscape();
    waitFor('>', TIMEOUT_MS); return false;
    drainRX();


    if (!openFile()) return false; // based on config 2 for command and 1 Sequential Log and 0 to create a new file
    writeHeader();

    _ready = true;
    return true;
}

bool SD_Logger::openFile() {
    snprintf(_filename, sizeof(_filename), "MISSION.CSV");

    // Create if it doesn't exist — OpenLog responds '>'
    _serial.print("new ");
    _serial.print(_filename);
    _serial.write('\r');
    if (!waitFor('>', TIMEOUT_MS)) return false;
    drainRX();

    // Open for appending — OpenLog responds '<' when ready to receive data
    _serial.print("append ");
    _serial.print(_filename);
    _serial.write('\r');
    if (!waitFor('<', TIMEOUT_MS)) return false;
    drainRX();

    return true;
}

void SD_Logger::writeHeader() {
    startLine();

    addField("time_s");
    addField("temperature_C");
    addField("pressure_hPa");
    addField("altitude_m");
    addField("water_kPa");
    addField("water_depth_m");
    addField("rpm");

    endLine();
}

void SD_Logger::startLine() {
    _lineIdx = 0;

    _overflow = false;
}

void SD_Logger::endLine() {
    if (!_overflow && _lineIdx < LINE_BUFFER-2)
        _line[_lineIdx++] = '\n';

    // TXbuffer non-blocking
    pushData((uint8_t*)_line, _lineIdx);
}

void SD_Logger::addField(const char* field) {
    addField(field, strlen(field));
}

void SD_Logger::addField(const char* field, size_t len) {
    if (_overflow) return;

    // add comma
    if (_lineIdx > 0) {
        if (_lineIdx >= LINE_BUFFER-1) { _overflow = true; return; }
        _line[_lineIdx++] = ',';
    }

    if (_lineIdx + len >= LINE_BUFFER) { _overflow = true; return; }

    memcpy(&_line[_lineIdx], field, len);
    _lineIdx += len;
}

void SD_Logger::addInt(int32_t value) {
    char tmp[16];

    snprintf(tmp, sizeof(tmp), "%ld", value);

    addField(tmp);
}

void SD_Logger::addFloat(float value, uint8_t precision) {
    char tmp[20];

    dtostrf_portable(value, 0, precision, tmp);

    addField(tmp);
}

void SD_Logger::pushByte(uint8_t byte) {
    uint16_t next = (_txHead + 1) % TX_BUFFER;
    if (next == _txTail) return;

    _tx[_txHead] = byte;
    _txHead = next;
}

void SD_Logger::pushData(const uint8_t* d, size_t len) {
    for (size_t i=0; i<len; i++) pushByte(d[i]);
}

void SD_Logger::update() {
    while (_txTail != _txHead && _serial.availableForWrite() > 0) {
        _serial.write(_tx[_txTail]);
        _txTail = (_txTail + 1) % TX_BUFFER;
    }
}

void SD_Logger::flush() {
    if (!_ready) return;

    sendEscape();
    if (!waitFor('>', TIMEOUT_MS)) { _ready = false; return; }

    // Sync to card
    _serial.print("sync\r");
    if (!waitFor('>', TIMEOUT_MS)) { _ready = false; return; }

    _serial.print("append ");
    _serial.print(_filename);
    _serial.write('\r');
    if (!waitFor('>', TIMEOUT_MS)) { _ready = false; return; }
}

void SD_Logger::sendEscape() {
    _serial.write(26); delay(ESCAPE_GAP_MS);
    _serial.write(26); delay(ESCAPE_GAP_MS);
    _serial.write(26); delay(CMD_MS);
}

void SD_Logger::drainRX() {
    delay(20);
    while (_serial.available()) _serial.read();
}

bool SD_Logger::waitFor(char c, uint16_t timeoutMs) {
    uint32_t start = millis();
    while (millis() - start < timeoutMs) {
        if (_serial.available()) {
            if (_serial.read() == c)
                return true;
        }
    }
    return false;
}