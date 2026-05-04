#include "System_Check.h"

void System_Check::checkBME(bool beginResult) {
    if (!beginResult) _status |= SystemStatus::BME;
}

void System_Check::checkSD(bool beginResult) {
    if (!beginResult) _status |= SystemStatus::SD;
}

void System_Check::checkRadio(bool beginResult) {
    if (!beginResult) _status |= SystemStatus::RADIO;
}