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
    m_iMinutesLeft = 0;
    m_dPercentage = 0.0;
    m_chargingState = Battery::UNKNOWN;

    // SYSTEM_POWER_STATUS doc
    // http://msdn.microsoft.com/en-us/library/windows/desktop/aa373232(v=vs.85).aspx
    SYSTEM_POWER_STATUS spsPwr;
    if (GetSystemPowerStatus(&spsPwr)) {
        // get rest power of battery
        m_dPercentage = static_cast<double>(spsPwr.BatteryLifePercent);
        // check for unkown flag and reset to default
        if (m_dPercentage == 255) {
            m_dPercentage = 0;
        }
        int batStat = static_cast<int>(spsPwr.BatteryFlag);
        if (batStat == 1 || batStat == 2 || batStat == 4) {
            m_chargingState = Battery::DISCHARGING;
        } else if (batStat == 8) {
            m_chargingState = Battery::CHARGING;
        }
        // I get this directly from the API
        if (m_dPercentage > 99) {
            m_chargingState = Battery::CHARGED;
        }
        // windows tells us the remainging time in seconds
        m_iMinutesLeft = static_cast<int>(spsPwr.BatteryLifeTime) / 60;
    }

    // QString bat = "unkown";
    // switch (m_chargingState) {
    // case Battery::CHARGING:
    //     bat = "charging";
    //     break;
    // case Battery::DISCHARGING:
    //     bat = "discharging";
    //     break;
    // case Battery::CHARGED:
    //     bat = "charged";
    // }
    // qDebug() << "BatteryWindows::read()"
    //          << "capacity " << m_dPercentage
    //          << "minutes_left " << m_iMinutesLeft
    //          << "battery_status " << bat;
}
