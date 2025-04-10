#include "controllers/controller.h"

#include <QJSEngine>
#include <algorithm>

#include "controllers/scripting/legacy/controllerscriptenginelegacy.h"
#include "moc_controller.cpp"
#include "util/cmdlineargs.h"
#include "util/screensaver.h"

namespace {
QString loggingCategoryPrefix(const QString& deviceName) {
    return QStringLiteral("controller.") +
            RuntimeLoggingCategory::removeInvalidCharsFromCategory(deviceName.toLower());
}
} // namespace

Controller::Controller(const QString& deviceName)
        : m_sDeviceName(deviceName),
          m_logBase(loggingCategoryPrefix(deviceName)),
          m_logInput(loggingCategoryPrefix(deviceName) + QStringLiteral(".input")),
          m_logOutput(loggingCategoryPrefix(deviceName) + QStringLiteral(".output")),
          m_pScriptEngineLegacy(nullptr),
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

QString Controller::physicalTransport2String(PhysicalTransportProtocol protocol) {
    switch (protocol) {
    case PhysicalTransportProtocol::USB:
        return QStringLiteral("USB");
    case PhysicalTransportProtocol::BlueTooth:
        return QStringLiteral("Bluetooth");
    case PhysicalTransportProtocol::I2C:
        return QStringLiteral("I2C");
    case PhysicalTransportProtocol::SPI:
        return QStringLiteral("SPI");
    case PhysicalTransportProtocol::FireWire:
        return QStringLiteral("Firewire - IEEE 1394");
    case PhysicalTransportProtocol::UNKNOWN:
        break; // Effectively fallthrough
    }
    return tr("Unknown");
}

void Controller::startEngine() {
    qCInfo(m_logBase) << "Starting engine";
    if (m_pScriptEngineLegacy) {
        qCWarning(m_logBase) << "startEngine(): Engine already exists! Restarting:";
        stopEngine();
    }
    m_pScriptEngineLegacy = std::make_shared<ControllerScriptEngineLegacy>(this, m_logBase);
    QObject::connect(m_pScriptEngineLegacy.get(),
            &ControllerScriptEngineBase::beforeShutdown,
            this,
            &Controller::slotBeforeEngineShutdown);
    emit engineStarted(m_pScriptEngineLegacy.get());
}

void Controller::stopEngine() {
    qCInfo(m_logBase) << "Shutting down engine";
    if (!m_pScriptEngineLegacy) {
        qCWarning(m_logBase) << "No engine exists!";
        return;
    }
    m_pScriptEngineLegacy.reset();
    emit engineStopped();
}

bool Controller::applyMapping(const QString& resourcePath) {
    qCInfo(m_logBase) << "Applying controller mapping...";

    // Load the script code into the engine
    if (!m_pScriptEngineLegacy) {
        qCWarning(m_logBase) << "No engine exists!";
        return false;
    }

    QList<LegacyControllerMapping::ScriptFileInfo> scriptFiles = getMappingScriptFiles();
    if (scriptFiles.isEmpty()) {
        qCWarning(m_logBase)
                << "No script functions available! Did the XML file(s) load "
                   "successfully? See above for any errors.";
        return true;
    }

    m_pScriptEngineLegacy->setScriptFiles(scriptFiles);

    m_pScriptEngineLegacy->setSettings(getMappingSettings());
#ifdef MIXXX_USE_QML
    m_pScriptEngineLegacy->setModulePaths(getMappingModules());
    m_pScriptEngineLegacy->setInfoScreens(getMappingInfoScreens());
    m_pScriptEngineLegacy->setResourcePath(resourcePath);
#else
    Q_UNUSED(resourcePath);
#endif
    return m_pScriptEngineLegacy->initialize();
}

void Controller::startLearning() {
    qCDebug(m_logBase) << m_sDeviceName << "started learning";
    m_bLearning = true;
}

void Controller::stopLearning() {
    //qDebug() << m_sDeviceName << "stopped learning.";
    m_bLearning = false;

}

void Controller::send(const QList<int>& data, unsigned int length) {
    // If you change this implementation, also change it in HidController (That
    // function is required due to HID devices having report IDs)

    Q_UNUSED(length);
    // The length parameter is here for backwards compatibility for when scripts
    // were required to specify it.

    QByteArray msg;
    msg.resize(data.size());
    std::copy(data.cbegin(), data.cend(), msg.begin());
    sendBytes(msg);
}

void Controller::triggerActivity() {
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
    if (CmdlineArgs::Instance()
                    .getControllerDebug() &&
            m_logInput().isDebugEnabled()) {
        // Formatted packet display
        QString message = QString("t:%2, %3 bytes:\n")
                                  .arg(timestamp.formatMillisWithUnit(),
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
            // cast to quint8 to avoid that negative chars are for instance displayed as ffffffff instead of the desired ff
            message += QString::number(static_cast<quint8>(data.at(i)), 16)
                               .toUpper()
                               .rightJustified(2, QChar('0')) +
                    spacer;
        }
        qCDebug(m_logInput).noquote() << message;
    }

    m_pScriptEngineLegacy->handleIncomingData(data);
}
void Controller::slotBeforeEngineShutdown() {
    /* Override this to get called before the JS engine shuts down */
    qCDebug(m_logInput) << "Engine shutdown";
}
