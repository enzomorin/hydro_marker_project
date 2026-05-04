#include "SPI_Radio.h"

namespace Config {
    // Config registers
    constexpr uint8_t IOCFG2   = 0x00;
    constexpr uint8_t IOCFG0   = 0x02;
    constexpr uint8_t FIFOTHR  = 0x03;
    constexpr uint8_t PKTLEN   = 0x06;
    constexpr uint8_t PKTCTRL1 = 0x07;
    constexpr uint8_t PKTCTRL0 = 0x08;
    constexpr uint8_t ADDR     = 0x09;
    constexpr uint8_t CHANNR   = 0x0A;
    constexpr uint8_t FSCTRL1  = 0x0B;
    constexpr uint8_t FSCTRL0  = 0x0C;
    constexpr uint8_t FREQ2    = 0x0D;
    constexpr uint8_t FREQ1    = 0x0E;
    constexpr uint8_t FREQ0    = 0x0F;
    constexpr uint8_t MDMCFG4  = 0x10;
    constexpr uint8_t MDMCFG3  = 0x11;
    constexpr uint8_t MDMCFG2  = 0x12;
    constexpr uint8_t MDMCFG1  = 0x13;
    constexpr uint8_t MDMCFG0  = 0x14;
    constexpr uint8_t DEVIATN  = 0x15;
    constexpr uint8_t MCSM1    = 0x17;
    constexpr uint8_t MCSM0    = 0x18;
    constexpr uint8_t FOCCFG   = 0x19;
    constexpr uint8_t BSCFG    = 0x1A;
    constexpr uint8_t AGCCTRL2 = 0x1B;
    constexpr uint8_t AGCCTRL1 = 0x1C;
    constexpr uint8_t AGCCTRL0 = 0x1D;
    constexpr uint8_t FREND1   = 0x21;
    constexpr uint8_t FREND0   = 0x22;
    constexpr uint8_t FSCAL3   = 0x23;
    constexpr uint8_t FSCAL2   = 0x24;
    constexpr uint8_t FSCAL1   = 0x25;
    constexpr uint8_t FSCAL0   = 0x26;
    constexpr uint8_t TEST2    = 0x2C;
    constexpr uint8_t TEST1    = 0x2D;
    constexpr uint8_t TEST0    = 0x2E;
    // Status registers (read-only, need 0xC0 header)
    constexpr uint8_t RXBYTES  = 0x3B;
    constexpr uint8_t TXBYTES  = 0x3A;
    constexpr uint8_t MARCSTATE = 0x35;
    // TX FIFO / RX FIFO
    constexpr uint8_t TXFIFO   = 0x3F;
    constexpr uint8_t RXFIFO   = 0x3F;
    // Command strobes
    constexpr uint8_t SRES     = 0x30;
    constexpr uint8_t SFSTXON  = 0x31;
    constexpr uint8_t SXOFF    = 0x32;
    constexpr uint8_t SCAL     = 0x33;
    constexpr uint8_t SRX      = 0x34;
    constexpr uint8_t STX      = 0x35;
    constexpr uint8_t SIDLE    = 0x36;
    constexpr uint8_t SFRX     = 0x3A;
    constexpr uint8_t SFTX     = 0x3B;
    constexpr uint8_t SNOP     = 0x3D;
    // SPI flags
    constexpr uint8_t READ     = 0x80;
    constexpr uint8_t BURST    = 0x40;
    // PA table
    constexpr uint8_t PATABLE  = 0x3E;
}

// ── Constructor ──────────────────────────────────────────────────────────────
SPI_Radio::SPI_Radio(uint8_t pinGDO0, uint8_t pinCSN)
    : _gdo0(pinGDO0), _cs(pinCSN) {}

// ── Public: begin ────────────────────────────────────────────────────────────
bool SPI_Radio::begin() {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
    pinMode(_gdo0, INPUT);

    SPI.begin();
    delay(10);

    reset();
    initCC1101();

    _ready = true;
    setRx(); // default to receive mode
    return true;
}

// ── Public: send ─────────────────────────────────────────────────────────────
bool SPI_Radio::send(const uint8_t* data, uint8_t len) {
    if (!_ready || !data || len == 0 || len > 61) return false;

    strobe(Config::SIDLE);
    strobe(Config::SFTX);

    // Write length byte + payload to TX FIFO
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO)); // wait for SO low = chip ready
    SPI.transfer(Config::TXFIFO | Config::BURST | 0x00); // burst write to TXFIFO
    SPI.transfer(len);                            // length byte
    for (uint8_t i = 0; i < len; i++) SPI.transfer(data[i]);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();

    strobe(Config::STX);

    // Wait for GDO0 to go HIGH (start of packet) then LOW (end of packet)
    uint32_t t = millis();
    while (!digitalRead(_gdo0) && (millis() - t) < 100);
    t = millis();
    while ( digitalRead(_gdo0) && (millis() - t) < 200);

    strobe(Config::SIDLE);
    setRx();
    return true;
}

// ── Public: receive ───────────────────────────────────────────────────────────
int8_t SPI_Radio::receive(uint8_t* buf, uint8_t maxLen) {
    if (!_ready || !buf) return -1;

    uint8_t rxBytes = readStatus(Config::RXBYTES);
    if (rxBytes == 0) return -1;
    if (rxBytes & 0x80) { // overflow
        strobe(Config::SFRX);
        setRx();
        return -1;
    }

    // First byte in FIFO is the length
    uint8_t len = readReg(Config::RXFIFO | Config::READ);
    if (len == 0 || len > maxLen) {
        strobe(Config::SFRX);
        setRx();
        return -1;
    }

    readBurst(Config::RXFIFO | Config::READ, buf, len);

    // Read 2 status bytes appended by hardware (RSSI, LQI/CRC_OK)
    uint8_t rssi  = readReg(Config::RXFIFO | Config::READ);
    uint8_t status = readReg(Config::RXFIFO | Config::READ);
    (void)rssi;

    if (!(status & 0x80)) { // CRC failed
        strobe(Config::SFRX);
        setRx();
        return -1;
    }

    return (int8_t)len;
}

// ── Public: idle / rx ────────────────────────────────────────────────────────
void SPI_Radio::setIdle() { strobe(Config::SIDLE); }
void SPI_Radio::setRx()   { strobe(Config::SRX);  }

// ── Private: reset ────────────────────────────────────────────────────────────
void SPI_Radio::reset() {
    digitalWrite(_cs, LOW);
    delayMicroseconds(10);
    digitalWrite(_cs, HIGH);
    delayMicroseconds(40);

    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(Config::SRES);
    while (digitalRead(MISO));
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();

    delay(5);
}

// ── Private: SPI helpers ──────────────────────────────────────────────────────
void SPI_Radio::writeReg(uint8_t addr, uint8_t val) {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(addr);
    SPI.transfer(val);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

uint8_t SPI_Radio::readReg(uint8_t addr) {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(addr | Config::READ);
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    return val;
}

void SPI_Radio::writeBurst(uint8_t addr, const uint8_t* data, uint8_t len) {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(addr | Config::BURST);
    for (uint8_t i = 0; i < len; i++) SPI.transfer(data[i]);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

void SPI_Radio::readBurst(uint8_t addr, uint8_t* data, uint8_t len) {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(addr | Config::READ | Config::BURST);
    for (uint8_t i = 0; i < len; i++) data[i] = SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
}

void SPI_Radio::strobe(uint8_t cmd) {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(cmd);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    delayMicroseconds(100);
}

uint8_t SPI_Radio::readStatus(uint8_t addr) {
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(addr | Config::READ | Config::BURST); // status reg needs burst flag
    uint8_t val = SPI.transfer(0x00);
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();
    return val;
}

// ── Private: initCC1101 ───────────────────────────────────────────────────────
// Target: 433.92 MHz, GFSK, 4.8 kbps, max sensitivity, +10 dBm TX (EU limit)
void SPI_Radio::initCC1101() {
    // GDO0 = asserted while packet is being received/sent (sync word to end)
    writeReg(Config::IOCFG0,   0x06);

    // FIFO threshold: TX assert at 61, RX assert at 4
    writeReg(Config::FIFOTHR,  0x47);

    // Max packet length = 61 bytes (fits in one FIFO page)
    writeReg(Config::PKTLEN,   0x3D);

    // Packet control:
    //   PKTCTRL1: append RSSI+LQI status bytes, no address check
    writeReg(Config::PKTCTRL1, 0x04);
    //   PKTCTRL0: variable length, CRC enabled, no data whitening
    writeReg(Config::PKTCTRL0, 0x05);

    // Frequency synthesizer: IF = 152.3 kHz (optimal for narrow BW)
    writeReg(Config::FSCTRL1,  0x06);
    writeReg(Config::FSCTRL0,  0x00);

    // Frequency: 433.92 MHz
    // f = FREQ * f_xosc / 2^16 = FREQ * 26e6 / 65536
    // FREQ = 433.92e6 / (26e6 / 65536) = 0x10A762
    writeReg(Config::FREQ2,    0x10);
    writeReg(Config::FREQ1,    0xA7);
    writeReg(Config::FREQ0,    0x62);

    // Modem config:
    //   MDMCFG4: BW = 58.03 kHz (narrowest for max range), DRATE_E=5
    writeReg(Config::MDMCFG4,  0xF5);
    //   MDMCFG3: DRATE_M=131 → data rate = 4.79 kbps ≈ 4.8 kbps
    writeReg(Config::MDMCFG3,  0x83);
    //   MDMCFG2: GFSK, 16/16 sync word, Manchester off
    writeReg(Config::MDMCFG2,  0x13);
    //   MDMCFG1: FEC off, 4 preamble bytes, CHANSPC_E=2
    writeReg(Config::MDMCFG1,  0x22);
    //   MDMCFG0: CHANSPC_M=248 → 200 kHz channel spacing
    writeReg(Config::MDMCFG0,  0xF8);

    // Deviation: 25.39 kHz (≈ BW/2 for GFSK, good modulation index)
    writeReg(Config::DEVIATN,  0x35);

    // Main state machine:
    //   MCSM1: after TX → go to RX; after RX → stay in RX
    writeReg(Config::MCSM1,    0x3F);
    //   MCSM0: auto-calibrate when going from IDLE to RX or TX
    writeReg(Config::MCSM0,    0x18);

    // Frequency offset compensation
    writeReg(Config::FOCCFG,   0x16);
    writeReg(Config::BSCFG,    0x6C);

    // AGC: max sensitivity settings
    writeReg(Config::AGCCTRL2, 0x43);
    writeReg(Config::AGCCTRL1, 0x40);
    writeReg(Config::AGCCTRL0, 0x91);

    // RF front-end
    writeReg(Config::FREND1,   0x56);
    writeReg(Config::FREND0,   0x10);

    // Frequency calibration (from SmartRF Studio for 26 MHz XTAL)
    writeReg(Config::FSCAL3,   0xE9);
    writeReg(Config::FSCAL2,   0x2A);
    writeReg(Config::FSCAL1,   0x00);
    writeReg(Config::FSCAL0,   0x1F);

    // Test registers (required values from datasheet)
    writeReg(Config::TEST2,    0x81);
    writeReg(Config::TEST1,    0x35);
    writeReg(Config::TEST0,    0x09);

    // PA power table: +10 dBm = 0xC0 for 433 MHz (EU limit: 10 mW EIRP)
    // CC1101 PA table index 0 is used in basic TX power config
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
    digitalWrite(_cs, LOW);
    while (digitalRead(MISO));
    SPI.transfer(Config::PATABLE | Config::BURST);
    SPI.transfer(0xC0); // +10 dBm
    digitalWrite(_cs, HIGH);
    SPI.endTransaction();

    // Calibrate
    strobe(Config::SCAL);
    delay(5);
}