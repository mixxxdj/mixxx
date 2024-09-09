#include "controllers/midi/portmidienumerator.h"

#include <portmidi.h>

#include <QLatin1StringView>
#include <QRegularExpression>

#include "controllers/midi/portmidicontroller.h"
#include "moc_portmidienumerator.cpp"
#include "util/cmdlineargs.h"

using namespace Qt::StringLiterals;

namespace {

const QLatin1String kMidiThroughPortPrefix = "MIDI Through Port"_L1;

bool recognizeDevice(const PmDeviceInfo& deviceInfo) {
    // In developer mode we show the MIDI Through Port, otherwise ignore it
    // since it routinely causes trouble.
    return CmdlineArgs::Instance().getDeveloper() ||
            !QLatin1String(deviceInfo.name)
                     .startsWith(kMidiThroughPortPrefix, Qt::CaseInsensitive);
}

// Some platforms format MIDI device names as "deviceName MIDI ###" where
// ### is the instance # of the device. Therefore we want to link two
// devices that have an equivalent "deviceName" and ### section.
const QRegularExpression kMidiDeviceNameRegex(u"^(.*) MIDI (\\d+)( .*)?$"_s);

const QRegularExpression kInputRegex(u"^(.*) in( \\d+)?( .*)?$"_s,
        QRegularExpression::CaseInsensitiveOption);
const QRegularExpression kOutputRegex(u"^(.*) out( \\d+)?( .*)?$"_s,
        QRegularExpression::CaseInsensitiveOption);

// This is a broad pattern that matches a text blob followed by a numeral
// potentially followed by non-numeric text. The non-numeric requirement is
// meant to avoid corner cases around devices with names like "Hercules RMX
// 2" where we would potentially confuse the number in the device name as
// the ordinal index of the device.
const QRegularExpression kDeviceNameRegex(u"^(.*) (\\d+)( [^0-9]+)?$"_s);

bool namesMatchRegexes(const QRegularExpression& kInputRegex,
        QLatin1StringView input_name,
        const QRegularExpression& kOutputRegex,
        QLatin1StringView output_name) {
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

bool namesMatchMidiPattern(QLatin1StringView input_name,
        QLatin1StringView output_name) {
    return namesMatchRegexes(kMidiDeviceNameRegex, input_name, kMidiDeviceNameRegex, output_name);
}

bool namesMatchInOutPattern(QLatin1StringView input_name,
        QLatin1StringView output_name) {
    return namesMatchRegexes(kInputRegex, input_name, kOutputRegex, output_name);
}

bool namesMatchPattern(QLatin1String input_name,
        QLatin1StringView output_name) {
    return namesMatchRegexes(kDeviceNameRegex, input_name, kDeviceNameRegex, output_name);
}

bool namesMatchAllowableEdgeCases(QLatin1StringView input_name,
        QLatin1StringView output_name) {
    // Mac OS 10.12 & Korg Kaoss DJ 1.6:
    // Korg Kaoss DJ has input 'KAOSS DJ CONTROL' and output 'KAOSS DJ SOUND'.
    // This means it doesn't pass the shouldLinkInputToOutput test. Without an
    // output linked, the MIDI output for the device fails, as the device is
    // NULL in PortMidiController
    if (input_name == "KAOSS DJ CONTROL"_L1 && output_name == "KAOSS DJ SOUND"_L1) {
        return true;
    }
    // Ableton Push on Windows
    // Shows 2 different devices for MIDI input and output.
    if (input_name == "MIDIIN2 (Ableton Push)"_L1 && output_name == "MIDIOUT2 (Ableton Push)"_L1) {
        return true;
    }
    return false;
}

bool namesMatch(QLatin1StringView input_name, QLatin1StringView output_name) {
    return namesMatchMidiPattern(input_name, output_name) ||
            namesMatchInOutPattern(input_name, output_name) ||
            namesMatchPattern(input_name, output_name);
}

QLatin1StringView trimStart(QLatin1StringView str, QLatin1StringView trim) {
    if (str.startsWith(trim, Qt::CaseInsensitive)) {
        return str.sliced(trim.length());
    }
    return str;
}

QString trimMiddle(QLatin1StringView str, QLatin1StringView trim) {
    int offset = str.indexOf(trim, 0, Qt::CaseInsensitive);
    if (offset != -1) {
        return QString::fromLatin1(str).replace(offset, trim.length(), " ");
    }
    return str;
}

} // namespace

PortMidiEnumerator::PortMidiEnumerator() {
    PmError err = Pm_Initialize();
    // Based on reading the source, it's not possible for this to fail.
    if (err != pmNoError) {
        qWarning() << "PortMidi error:" << Pm_GetErrorText(err);
    }
}

PortMidiEnumerator::~PortMidiEnumerator() {
    qDebug() << "Deleting PortMIDI devices...";
    m_devices.clear();
    PmError err = Pm_Terminate();
    // Based on reading the source, it's not possible for this to fail.
    if (err != pmNoError) {
        qWarning() << "PortMidi error:" << Pm_GetErrorText(err);
    }
}

bool shouldLinkInputToOutput(QLatin1StringView input_name,
        QLatin1StringView output_name) {
    // Early exit.
    if (input_name == output_name || namesMatchAllowableEdgeCases(input_name, output_name)) {
        return true;
    }

    // Some device drivers prepend "To" and "From" to the names of their MIDI
    // ports. If the output and input device names don't match, let's try
    // trimming those words from the start, and seeing if they then match.
    QLatin1StringView input_name_trimmed = trimStart(input_name, "from"_L1);
    QLatin1StringView output_name_trimmed = trimStart(output_name, "to"_L1);

    if (input_name_trimmed == output_name_trimmed) {
        return true;
    }

    QString input_name_stripped = trimMiddle(input_name_trimmed, " input "_L1);
    QString output_name_stripped = trimMiddle(output_name_trimmed, " output "_L1);

    if (input_name_trimmed == output_name_trimmed ||
            namesMatch(input_name, output_name) ||
            namesMatch(input_name_trimmed, output_name_trimmed) ||
            namesMatch(QLatin1StringView(input_name_trimmed.latin1()),
                    QLatin1StringView(output_name_trimmed.latin1()))) {
        return true;
    }

    return false;
}

/** Enumerate the MIDI devices
  * This method needs a bit of intelligence because PortMidi (and the underlying MIDI APIs) like to split
  * output and input into separate devices. Eg. PortMidi would tell us the Hercules is two half-duplex devices.
  * To help simplify a lot of code, we're going to aggregate these two streams into a single full-duplex device.
  */
QList<Controller*> PortMidiEnumerator::queryDevices() {
    qDebug() << "Scanning PortMIDI devices:";

    int numDevices = Pm_CountDevices();

    m_devices.clear();

    QMap<int, QLatin1StringView> unassignedOutputDevices;

    // Build a complete list of output devices for later pairing
    for (int i = 0; i < numDevices; i++) {
        const PmDeviceInfo* pDeviceInfo = Pm_GetDeviceInfo(i);
        VERIFY_OR_DEBUG_ASSERT(pDeviceInfo) {
            continue;
        }
        if (!recognizeDevice(*pDeviceInfo) || !pDeviceInfo->output) {
            continue;
        }
        qDebug() << " Found output device"
                 << "#" << i << pDeviceInfo->name;
        unassignedOutputDevices[i] = QLatin1StringView(pDeviceInfo->name);
    }

    // Search for input devices and pair them with output devices if applicable
    for (int i = 0; i < numDevices; i++) {
        const PmDeviceInfo* pDeviceInfo = Pm_GetDeviceInfo(i);
        VERIFY_OR_DEBUG_ASSERT(pDeviceInfo) {
            continue;
        }
        if (!recognizeDevice(*pDeviceInfo) || !pDeviceInfo->input) {
            // Is there a use case for output-only devices such as message
            // displays? Then this condition has to be split and
            // deviceInfo->output also needs to be checked and handled.
            continue;
        }

        qDebug() << " Found input device"
                 << "#" << i << pDeviceInfo->name;
        const PmDeviceInfo* inputDeviceInfo = pDeviceInfo;
        int inputDevIndex = i;

        //Reset our output device variables before we look for one in case we find none.
        const PmDeviceInfo* outputDeviceInfo = nullptr;
        int outputDevIndex = -1;

        //Search for a corresponding output device
        QMapIterator<int, QLatin1StringView> j(unassignedOutputDevices);
        while (j.hasNext()) {
            j.next();

            if (shouldLinkInputToOutput(inputDeviceInfo->name, j.value())) {
                outputDevIndex = j.key();
                outputDeviceInfo = Pm_GetDeviceInfo(outputDevIndex);

                unassignedOutputDevices.remove(outputDevIndex);

                qDebug() << "    Linking to output device #" << outputDevIndex << j.value();
                break;
            }
        }

        // So at this point, we either have an input-only MIDI device
        // (outputDeviceInfo == nullptr) or we've found a matching output MIDI
        // device (outputDeviceInfo != nullptr).

        //.... so create our (aggregate) MIDI device!
        m_devices.push_back(std::make_unique<PortMidiController>(inputDeviceInfo,
                outputDeviceInfo,
                inputDevIndex,
                outputDevIndex));
    }
    QList<Controller*> devices;
    devices.reserve(m_devices.size());
    for (const auto& pDevice : m_devices) {
        devices.push_back(pDevice.get());
    }

    return devices;
}
