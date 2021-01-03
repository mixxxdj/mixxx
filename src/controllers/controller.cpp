#include "controllers/controller.h"

#include <QApplication>
#include <QJSValue>

#include "controllers/controllerdebug.h"
#include "controllers/defs_controllers.h"
#include "moc_controller.cpp"
#include "util/screensaver.h"

Controller::Controller()
        : m_pScriptEngineLegacy(nullptr),
          m_bIsOutputDevice(false),
          m_bIsInputDevice(false),
          m_bIsOpen(false),
          m_bLearning(false) {
    m_userActivityInhibitTimer.start();
}

Controller::~Controller() {
    // Don't close the device here. Sub-classes should close the device in their
    // destructors.
}

ControllerJSProxy* Controller::jsProxy() {
    return new ControllerJSProxy(this);
}

void Controller::startEngine()
{
    controllerDebug("  Starting engine");
    if (m_pScriptEngineLegacy) {
        qWarning() << "Controller: Engine already exists! Restarting:";
        stopEngine();
    }
    m_pScriptEngineLegacy = new ControllerScriptEngineLegacy(this);
}

void Controller::stopEngine() {
    controllerDebug("  Shutting down engine");
    if (!m_pScriptEngineLegacy) {
        qWarning() << "Controller::stopEngine(): No engine exists!";
        return;
    }
    delete m_pScriptEngineLegacy;
    m_pScriptEngineLegacy = nullptr;
}

bool Controller::applyMapping() {
    qDebug() << "Applying controller mapping...";

    const LegacyControllerMapping* pMapping = mapping();

    // Load the script code into the engine
    if (!m_pScriptEngineLegacy) {
        qWarning() << "Controller::applyMapping(): No engine exists!";
        return false;
    }

    QList<LegacyControllerMapping::ScriptFileInfo> scriptFiles = pMapping->getScriptFiles();
    if (scriptFiles.isEmpty()) {
        qWarning() << "No script functions available! Did the XML file(s) load successfully? See above for any errors.";
        return true;
    }

    m_pScriptEngineLegacy->setScriptFiles(scriptFiles);
    return m_pScriptEngineLegacy->initialize();
}

void Controller::startLearning() {
    qDebug() << m_sDeviceName << "started learning";
    m_bLearning = true;
}

void Controller::stopLearning() {
    //qDebug() << m_sDeviceName << "stopped learning.";
    m_bLearning = false;

}

void Controller::send(const QList<int>& data, unsigned int length) {
    // If you change this implementation, also change it in HidController (That
    // function is required due to HID devices having report IDs)

    // The length parameter is here for backwards compatibility for when scripts
    // were required to specify it.
    length = data.size();
    QByteArray msg(length, 0);
    for (unsigned int i = 0; i < length; ++i) {
        msg[i] = data.at(i);
    }
    sendBytes(msg);
}

void Controller::triggerActivity()
{
     // Inhibit Updates for 1000 milliseconds
    if (m_userActivityInhibitTimer.elapsed() > 1000) {
        mixxx::ScreenSaverHelper::triggerUserActivity();
        m_userActivityInhibitTimer.start();
    }
}
void Controller::receive(const QByteArray& data, mixxx::Duration timestamp) {
    if (!m_pScriptEngineLegacy) {
        //qWarning() << "Controller::receive called with no active engine!";
        // Don't complain, since this will always show after closing a device as
        //  queued signals flush out
        return;
    }
    triggerActivity();

    int length = data.size();
    if (ControllerDebug::isEnabled()) {
        // Formatted packet display
        QString message = QString("%1: t:%2, %3 bytes:\n")
                                  .arg(m_sDeviceName,
                                          timestamp.formatMillisWithUnit(),
                                          QString::number(length));
        for (int i = 0; i < length; i++) {
            QString spacer;
            if ((i + 1) % 16 == 0) {
                spacer = QStringLiteral("\n");
            } else if ((i + 1) % 4 == 0) {
                spacer = QStringLiteral("  ");
            } else {
                spacer = QStringLiteral(" ");
            }
            message += QString::number(data.at(i), 16)
                               .toUpper()
                               .rightJustified(2, QChar('0')) +
                    spacer;
        }
        controllerDebug(message);
    }

    m_pScriptEngineLegacy->handleIncomingData(data);
}
