/**
* @file controller.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief Base class representing a physical (or software) controller.
*/

#include <QApplication>
#include <QJSValue>

#include "controllers/controller.h"
#include "controllers/controllerdebug.h"
#include "controllers/defs_controllers.h"
#include "util/screensaver.h"

Controller::Controller()
        : m_pEngine(nullptr),
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
    if (m_pEngine != NULL) {
        qWarning() << "Controller: Engine already exists! Restarting:";
        stopEngine();
    }
    m_pEngine = new ControllerEngine(this);
}

void Controller::stopEngine() {
    controllerDebug("  Shutting down engine");
    if (m_pEngine == NULL) {
        qWarning() << "Controller::stopEngine(): No engine exists!";
        return;
    }
    m_pEngine->gracefulShutdown();
    delete m_pEngine;
    m_pEngine = NULL;
}

bool Controller::applyPreset(bool initializeScripts) {
    qDebug() << "Applying controller preset...";

    const ControllerPreset* pPreset = preset();

    // Load the script code into the engine
    if (m_pEngine == NULL) {
        qWarning() << "Controller::applyPreset(): No engine exists!";
        return false;
    }

    QList<ControllerPreset::ScriptFileInfo> scriptFiles = pPreset->getScriptFiles();
    if (scriptFiles.isEmpty()) {
        qWarning() << "No script functions available! Did the XML file(s) load successfully? See above for any errors.";
        return true;
    }

    bool success = m_pEngine->loadScriptFiles(scriptFiles);
    if (success && initializeScripts) {
        m_pEngine->initializeScripts(scriptFiles);
    }

    // QFileInfo does not have a isValid/isEmpty/isNull method to check if it
    // actually contains a reference, so we check if the filePath is empty as a
    // workaround.
    // See https://stackoverflow.com/a/45652741/1455128 for details.
    if (initializeScripts && !pPreset->moduleFileInfo().filePath().isEmpty()) {
        m_pEngine->loadModule(pPreset->moduleFileInfo());
    }
    return success;
}

void Controller::startLearning() {
    qDebug() << m_sDeviceName << "started learning";
    m_bLearning = true;
}

void Controller::stopLearning() {
    //qDebug() << m_sDeviceName << "stopped learning.";
    m_bLearning = false;

}

void Controller::send(QList<int> data, unsigned int length) {
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
void Controller::receive(const QByteArray data, mixxx::Duration timestamp) {

    if (m_pEngine == NULL) {
        //qWarning() << "Controller::receive called with no active engine!";
        // Don't complain, since this will always show after closing a device as
        //  queued signals flush out
        return;
    }
    triggerActivity();

    int length = data.size();
    if (ControllerDebug::enabled()) {
        // Formatted packet display
        QString message = QString("%1: t:%2, %3 bytes:\n")
                .arg(m_sDeviceName).arg(timestamp.formatMillisWithUnit()).arg(length);
        for(int i=0; i<length; i++) {
            QString spacer=" ";
            if ((i+1) % 4 == 0) spacer="  ";
            if ((i+1) % 16 == 0) spacer="\n";
            message += QString("%1%2")
                        .arg((unsigned char)(data.at(i)), 2, 16, QChar('0')).toUpper()
                        .arg(spacer);
        }
        controllerDebug(message);
    }

    foreach (QString function, m_pEngine->getScriptFunctionPrefixes()) {
        if (function == "") {
            continue;
        }
        function.append(".incomingData");
        QJSValue incomingDataFunction = m_pEngine->wrapFunctionCode(function, 2);
        m_pEngine->executeFunction(incomingDataFunction, data);
    }

    m_pEngine->handleInput(data, timestamp);
}
