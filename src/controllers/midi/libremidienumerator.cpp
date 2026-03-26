#include "controllers/midi/libremidienumerator.h"

#include <QRegularExpression>
#include <libremidi/libremidi.hpp>
#include <libremidi/port_information.hpp>

#include "controllers/controllermanager.h"
#include "controllers/defs_controllers.h"
#include "controllers/midi/libremidicontroller.h"
#include "moc_libremidienumerator.cpp"
#include "util/cmdlineargs.h"
#include "util/compatibility/qmutex.h"

namespace {

bool recognizeDevice(const libremidi::port_information& deviceInfo, UserSettingsPointer pConfig) {
    // In developer mode we show the MIDI Through Port, otherwise ignore it
    // since it routinely causes trouble.
    return CmdlineArgs::Instance().getDeveloper() ||
            pConfig->getValue(kMidiThroughCfgKey, false) ||
            !QLatin1String(deviceInfo.port_name)
                     .startsWith(kMidiThroughPortPrefix, Qt::CaseInsensitive);
}

// Some platforms format MIDI device names as "deviceName MIDI ###" where
// ### is the instance # of the device. Therefore we want to link two
// devices that have an equivalent "deviceName" and ### section.
const QRegularExpression kMidiDeviceNameRegex(QStringLiteral("^(.*) MIDI (\\d+)( .*)?$"));

const QRegularExpression kInputRegex(QStringLiteral("^(.*) in( \\d+)?( .*)?$"),
        QRegularExpression::CaseInsensitiveOption);
const QRegularExpression kOutputRegex(QStringLiteral("^(.*) out( \\d+)?( .*)?$"),
        QRegularExpression::CaseInsensitiveOption);

// This is a broad pattern that matches a text blob followed by a numeral
// potentially followed by non-numeric text. The non-numeric requirement is
// meant to avoid corner cases around devices with names like "Hercules RMX
// 2" where we would potentially confuse the number in the device name as
// the ordinal index of the device.
const QRegularExpression kDeviceNameRegex(QStringLiteral("^(.*) (\\d+)( [^0-9]+)?$"));

bool namesMatchRegexes(const QRegularExpression& kInputRegex,
        const QString& input_name,
        const QRegularExpression& kOutputRegex,
        const QString& output_name) {
    QRegularExpressionMatch inputMatch = kInputRegex.match(input_name);
    if (inputMatch.hasMatch()) {
        QString inputDeviceName = inputMatch.captured(1);
        QString inputDeviceIndex = inputMatch.captured(2);
        QRegularExpressionMatch outputMatch = kOutputRegex.match(output_name);
        if (outputMatch.hasMatch()) {
            QString outputDeviceName = outputMatch.captured(1);
            QString outputDeviceIndex = outputMatch.captured(2);
            if (outputDeviceName.compare(inputDeviceName, Qt::CaseInsensitive) == 0 &&
                    outputDeviceIndex == inputDeviceIndex) {
                return true;
            }
        }
    }
    return false;
}

bool namesMatchMidiPattern(const QString& input_name,
        const QString& output_name) {
    return namesMatchRegexes(kMidiDeviceNameRegex, input_name, kMidiDeviceNameRegex, output_name);
}

bool namesMatchInOutPattern(const QString& input_name,
        const QString& output_name) {
    return namesMatchRegexes(kInputRegex, input_name, kOutputRegex, output_name);
}

bool namesMatchPattern(const QString& input_name,
        const QString& output_name) {
    return namesMatchRegexes(kDeviceNameRegex, input_name, kDeviceNameRegex, output_name);
}

bool namesMatchAllowableEdgeCases(const QString& input_name,
        const QString& output_name) {
    // Mac OS 10.12 & Korg Kaoss DJ 1.6:
    // Korg Kaoss DJ has input 'KAOSS DJ CONTROL' and output 'KAOSS DJ SOUND'.
    // This means it doesn't pass the libremidiShouldLinkInputToOutput test. Without an
    // output linked, the MIDI output for the device fails, as the device is
    // NULL in LibremidiController
    if (input_name == "KAOSS DJ CONTROL" && output_name == "KAOSS DJ SOUND") {
        return true;
    }
    // Ableton Push on Windows
    // Shows 2 different devices for MIDI input and output.
    if (input_name == "MIDIIN2 (Ableton Push)" && output_name == "MIDIOUT2 (Ableton Push)") {
        return true;
    }

    // Novation Launchpad X (macOS)
    if (input_name == "Launchpad X LPX DAW Out" && output_name == "Launchpad X LPX DAW In") {
        return true;
    }

    return false;
}

} // namespace

void LibremidiEnumerator::addDevice(const libremidi::input_port* inputPort,
        const libremidi::output_port* outputPort) {
    auto pCurrentDevice = std::make_unique<LibremidiController>(inputPort, outputPort);
    pCurrentDevice->moveToThread(m_pControllerManager->thread());
    pCurrentDevice->setParent(m_pControllerManager);

    // Is this better than manually triggering the slot?
    // connect(pCurrentDevice, QObject::destroyed, manager, &ControllerManager::slotRemoveDevice);

    emit deviceAdded(pCurrentDevice.get());
    auto locker = lockMutex(&m_mutex);
    m_devices.push_back(std::move(pCurrentDevice));
    locker.unlock();
}

void LibremidiEnumerator::inputAdded(const libremidi::input_port& inputPort) {
    if (!recognizeDevice(inputPort, m_pConfig)) {
        return;
    }
    qWarning() << "Input added: " << inputPort.port_name.c_str();

    QString inputName = inputPort.port_name.c_str();
    const libremidi::output_port* outputPort = nullptr;

    auto locker = lockMutex(&m_mutex);

    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        const auto* device = it->get();
        if (!device->m_pOutputPort.has_value()) {
            continue;
        }

        QString outputName = device->m_pOutputPort->port_name.c_str();
        if (libremidiShouldLinkInputToOutput(inputName, outputName)) {
            // Currently simply removing previous device, and creating new one
            outputPort = &device->m_pOutputPort.value();
            auto ptr = std::move(*it);
            emit deviceRemoved(ptr.get());
            m_devices.erase(it);
            ptr.release()->deleteLater();
            break;
        }
    }
    locker.unlock();

    addDevice(&inputPort, outputPort);

    // qDebug() << "Linking to output device: " << outputName;
}

// Check mutex locking for multiple callbacks at once
void LibremidiEnumerator::inputRemoved(const libremidi::input_port& inputPort) {
    qWarning() << "Input removed: " << inputPort.port_name.c_str();

    auto locker = lockMutex(&m_mutex);
    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        auto* const device = it->get();

        if (device->m_pInputPort.has_value() &&
                device->m_pInputPort.value().port != inputPort.port) {
            auto ptr = std::move(*it);
            emit deviceRemoved(ptr.get());
            m_devices.erase(it);
            ptr.release()->deleteLater();
            // emit deviceRemoved(device);
            // m_devices.erase(it);
            break;
        }
    }
    locker.unlock();
}

void LibremidiEnumerator::outputAdded(const libremidi::output_port& outputPort) {
    if (!recognizeDevice(outputPort, m_pConfig)) {
        return;
    }
    qWarning() << "Output added: " << outputPort.port_name.c_str();

    QString outputName = outputPort.port_name.c_str();
    const libremidi::input_port* inputPort = nullptr;
    auto locker = lockMutex(&m_mutex);

    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        auto* const controller = it->get();
        if (!controller->m_pInputPort.has_value()) {
            continue;
        } else {
        }

        QString inputName = controller->m_pInputPort->port_name.c_str();
        if (libremidiShouldLinkInputToOutput(inputName, outputName)) {
            inputPort = &controller->m_pInputPort.value();
            // Currently simply removing previous device, and creating new one
            auto ptr = std::move(*it);
            emit deviceRemoved(ptr.get());
            m_devices.erase(it);
            ptr.release()->deleteLater();
            return;
        }
    }

    locker.unlock();
    addDevice(inputPort, &outputPort);
}

void LibremidiEnumerator::outputRemoved(const libremidi::output_port& outputPort) {
    qWarning() << "Output removed: " << outputPort.port_name.c_str();

    auto locker = lockMutex(&m_mutex);
    for (auto it = m_devices.begin(); it != m_devices.end(); it++) {
        const auto& device = it->get();

        if (device->m_pOutputPort.has_value() &&
                device->m_pOutputPort.value().port == outputPort.port) {
            auto ptr = std::move(*it);
            emit deviceRemoved(device);
            m_devices.erase(it);
            ptr.release()->deleteLater();
            break;
        }
    }
}

LibremidiEnumerator::LibremidiEnumerator(UserSettingsPointer pConfig, ControllerManager* manager)
        : m_pConfig(pConfig),
          m_pControllerManager(manager) {
    connect(this, &LibremidiEnumerator::deviceAdded, manager, &ControllerManager::slotAddDevice);
    connect(this,
            &LibremidiEnumerator::deviceRemoved,
            manager,
            &ControllerManager::slotRemoveDevice);

    m_observer = libremidi::observer{libremidi::observer_configuration{
            .input_added =
                    [this](const libremidi::input_port& port) {
                        inputAdded(port);
                    },
            .input_removed =
                    [this](const libremidi::input_port& port) {
                        inputRemoved(port);
                    },
    }};
}

bool libremidiShouldLinkInputToOutput(const QString& input_name,
        const QString& output_name) {
    // Early exit.
    if (input_name == output_name || namesMatchAllowableEdgeCases(input_name, output_name)) {
        return true;
    }

    // Some device drivers prepend "To" and "From" to the names of their MIDI
    // ports. If the output and input device names don't match, let's try
    // trimming those words from the start, and seeing if they then match.

    // Ignore "From" text in the beginning of device input name.
    QString input_name_stripped = input_name;
    if (input_name.indexOf("from", 0, Qt::CaseInsensitive) == 0) {
        input_name_stripped = input_name.right(input_name.length() - 4);
    }

    // Ignore "To" text in the beginning of device output name.
    QString output_name_stripped = output_name;
    if (output_name.indexOf("to", 0, Qt::CaseInsensitive) == 0) {
        output_name_stripped = output_name.right(output_name.length() - 2);
    }

    if (output_name_stripped != input_name_stripped) {
        // Ignore " input " text in the device names
        int offset = input_name_stripped.indexOf(" input ", 0, Qt::CaseInsensitive);
        if (offset != -1) {
            input_name_stripped = input_name_stripped.replace(offset, 7, " ");
        }

        // Ignore " output " text in the device names
        offset = output_name_stripped.indexOf(" output ", 0, Qt::CaseInsensitive);
        if (offset != -1) {
            output_name_stripped = output_name_stripped.replace(offset, 8, " ");
        }
    }

    if (input_name_stripped == output_name_stripped ||
            namesMatchMidiPattern(input_name_stripped, output_name_stripped) ||
            namesMatchMidiPattern(input_name, output_name) ||
            namesMatchInOutPattern(input_name_stripped, output_name_stripped) ||
            namesMatchInOutPattern(input_name, output_name) ||
            namesMatchPattern(input_name_stripped, output_name_stripped) ||
            namesMatchPattern(input_name, output_name)) {
        return true;
    }

    return false;
}
