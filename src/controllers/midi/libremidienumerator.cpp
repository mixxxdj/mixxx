#include "controllers/midi/libremidienumerator.h"

#include <QRegularExpression>
#include <libremidi/libremidi.hpp>
#include <libremidi/port_information.hpp>

#include "controllers/defs_controllers.h"
#include "controllers/midi/libremidicontroller.h"
#include "moc_libremidienumerator.cpp"
#include "util/cmdlineargs.h"

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

void input_removed(const libremidi::input_port& port) {
    qWarning() << "Input removed: " << port.port_name.c_str() << "\n";
}

void output_removed(const libremidi::output_port& port) {
    qWarning() << "Input removed: " << port.port_name.c_str() << "\n";
}

LibremidiEnumerator::LibremidiEnumerator(UserSettingsPointer pConfig)
        : obs(libremidi::observer{
                  libremidi::observer_configuration{
                          .input_removed = input_removed,
                          .output_removed = output_removed}}),
          m_pConfig(pConfig) {
}

LibremidiEnumerator::~LibremidiEnumerator() {
    qDebug() << "Deleting Libremidi devices...";
    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
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

/// Enumerate the MIDI devices
/// This method needs a bit of intelligence because Libremidi (and the underlying
/// MIDI APIs) like to split output and input into separate devices. Eg.
/// Libremidi would tell us the Hercules is two half-duplex devices. To help
/// simplify a lot of code, we're going to aggregate these two streams into a
/// single full-duplex device.
QList<Controller*> LibremidiEnumerator::queryDevices() {
    qDebug() << "Scanning PortMIDI devices:";

    const auto in_ports = obs.get_input_ports();
    const auto out_ports = obs.get_output_ports();

    for (const libremidi::input_port& port : in_ports) {
        qWarning() << port.port_name.c_str() << "\n";
    }

    for (const libremidi::output_port& port : out_ports) {
        qWarning() << port.port_name.c_str() << "\n";
    }

    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }

    m_devices.clear();

    const libremidi::input_port* inputPort = nullptr;
    const libremidi::output_port* outputPort = nullptr;
    size_t outputDevIndex = -1;

    QMap<size_t, QString> unassignedOutputDevices;

    // Build a complete list of output devices for later pairing
    for (size_t i = 0; i < out_ports.size(); i++) {
        const auto out_port = out_ports[i];
        if (!recognizeDevice(out_port, m_pConfig)) {
            continue;
        }
        qDebug() << " Found output device"
                 << "#" << i << out_port.port_name.c_str();
        QString deviceName = out_port.port_name.c_str();
        unassignedOutputDevices[i] = deviceName;
    }

    // Search for input devices and pair them with output devices if applicable
    for (size_t i = 0; i < in_ports.size(); i++) {
        auto pPort = in_ports[i];
        if (!recognizeDevice(pPort, m_pConfig)) {
            // Is there a use case for output-only devices such as message
            // displays? Then this condition has to be split and
            // deviceInfo->output also needs to be checked and handled.
            continue;
        }

        qDebug() << " Found input device"
                 << "#" << i << pPort.port_name.c_str();
        inputPort = &pPort;

        // Reset our output device variables before we look for one in case we find none.
        outputPort = nullptr;
        outputDevIndex = -1;

        // Search for a corresponding output device
        QMapIterator<size_t, QString> j(unassignedOutputDevices);
        while (j.hasNext()) {
            j.next();

            QString deviceName = inputPort->port_name.c_str();
            QString outputName = QString(j.value());

            if (libremidiShouldLinkInputToOutput(deviceName, outputName)) {
                outputDevIndex = j.key();
                outputPort = &out_ports[outputDevIndex];

                unassignedOutputDevices.remove(outputDevIndex);

                qDebug() << "    Linking to output device #" << outputDevIndex << outputName;
                break;
            }
        }

        // So at this point, we either have an input-only MIDI device
        // (outputPort == NULL) or we've found a matching output MIDI
        // device (outputPort != NULL).

        //.... so create our (aggregate) MIDI device!
        LibremidiController* currentDevice =
                new LibremidiController(inputPort, outputPort);
        m_devices.push_back(currentDevice);
    }
    return m_devices;
}
