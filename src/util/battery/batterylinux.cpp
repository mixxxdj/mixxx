#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QRegExp>

#include "util/battery/batterylinux.h"

// these constants are used to retrieve data from infoFile and stateFile
const QString BatteryLinux::s_sMaximumCapacityKeyword = "last full capacity";
const QString BatteryLinux::s_sCurrentCapacityKeyword = "remaining capacity";
const QString BatteryLinux::s_sChargingStateKeyword = "charging state";
const QString BatteryLinux::s_sCurrentRateKeyword = "present rate";

BatteryLinux::BatteryLinux(QObject* pParent, const QString& infoFile,
                           const QString& stateFile)
        : Battery(parent),
          m_sInfoFile(infoFile),
          m_sStateFile(stateFile) {
}

BatteryLinux::~BatteryLinux() {

}

void BatteryLinux::read() {
    int currentCapacity = readCurrentCapacity();
    int maximumCapacity = readMaximumCapacity();
    int currentRate = readCurrentRate();

    m_csChargingState = readChargingState();
    m_dPercentage = 0.0;
    if (maximumCapacity > 0) {
        m_dPercentage = static_cast<double>(currentCapacity) / maximumCapacity;
    }
    m_iMinutesLeft = getMinutesLeft(m_csChargingState, currentCapacity,
                                    maximumCapacity, currentRate);
}

Battery::ChargingState BatteryLinux::readChargingState() {
    QFile qfFile(m_sStateFile);
    if(!qfFile.open(QFile::ReadOnly)) {
        qDebug() << "Error opening " << m_sStateFile;
        return UNKNOWN;
    }
    QTextStream tsFile(&qfFile);
    QString sOut;
    while (!(sOut = tsFile.readLine()).isNull()) {
        if (sOut.contains(s_sChargingStateKeyword)) {
            // remove the keyword (i.e. "charging state:    ")
            sOut.remove(QRegExp(s_sChargingStateKeyword + ":*\\s+"));
            if (sOut == "discharging")
                return DISCHARGING;
            if (sOut == "charging")
                return CHARGING;
            if (sOut == "charged")
                return CHARGED;
            else
                return UNKNOWN;
        }
    }
    return UNKNOWN;
}

int BatteryLinux::getMinutesLeft(ChargingState chargingState, int currentCapacity,
                                 int maximumCapacity, int currentRate) const {
    // Prevent division by 0
    if (currentRate == 0) {
        return 0;
    }

    switch (chargingState) {
        case DISCHARGING:
            return 60 * currentCapacity / currentRate;
        case CHARGING:
            return 60 * (maximumCapacity - currentCapacity) / currentRate;
        case CHARGED:
        case UNKNOWN:
        default:
            return 0;
    }
}

int BatteryLinux::readValue(const QString& sFile, const QString& sKeyword) const {
    QFile qfFile(sFile);
    if(!qfFile.open(QFile::ReadOnly)) {
        qDebug() << "BatteryLinux: Error opening" << sFile;
        return 0;
    }

    QTextStream tsFile(&qfFile);
    QString sOut;
    while (!(sOut = tsFile.readLine()).isNull()) {
        if (sOut.contains(sKeyword)) {
            // remove everything except numbers
            sOut.remove(QRegExp("\\D"));
            return sOut.toInt();
        }
    }
    return 0;
}
