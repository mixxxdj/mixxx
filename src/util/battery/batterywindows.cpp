#include "util/battery/batterywindows.h"

// tell windows we target XP and later
// http://msdn.microsoft.com/en-us/library/windows/desktop/aa372693(v=vs.85).aspx
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <QDebug>
#include <QString>

BatteryWindows::BatteryWindows(QObject* pParent)
        : Battery(pParent) {
}

BatteryWindows::~BatteryWindows() {
}

void BatteryWindows::read() {
    // SYSTEM_POWER_STATUS doc
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa373232(v=vs.85).aspx
    SYSTEM_POWER_STATUS spsPwr;
    if (GetSystemPowerStatus(&spsPwr)) {
        // get rest power of battery
        m_dPercentage = static_cast<double>(spsPwr.BatteryLifePercent);
        // check for unknown flag and reset to default
        if (m_dPercentage == 255) {
            m_dPercentage = 0;
        }
        int batStat = static_cast<int>(spsPwr.BatteryFlag);
        if (batStat == 1 || batStat == 2 || batStat == 4) {
            m_chargingState = Battery::DISCHARGING;
        } else if (batStat == 8) {
            m_chargingState = Battery::CHARGING;
        } else {
            m_chargingState = Battery::UNKNOWN;
        }
        // I get this directly from the API
        if (m_dPercentage > 99) {
            m_chargingState = Battery::CHARGED;
        }
        // windows tells us the remaining time in seconds (-1 if unknown)
        int seconds_left = static_cast<int>(spsPwr.BatteryLifeTime);
        if (seconds_left >= 0) {
            m_iMinutesLeft = seconds_left / 60;
        } else {
            m_iMinutesLeft = Battery::TIME_UNKNOWN;
        }
    } else {
        m_chargingState = Battery::UNKNOWN;
        m_dPercentage = 0.0;
        m_iMinutesLeft = Battery::TIME_UNKNOWN;
    }
}
