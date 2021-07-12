#include "controllers/midi/portmidienumerator.h"

#include <portmidi.h>

#include <QRegExp>

#include "controllers/midi/portmidicontroller.h"
#include "moc_portmidienumerator.cpp"
#include "util/cmdlineargs.h"

namespace {

const auto kMidiThroughPortPrefix = QLatin1String("MIDI Through Port");

bool recognizeDevice(const PmDeviceInfo& deviceInfo) {
    // In developer mode we show the MIDI Through Port, otherwise ignore it
    // since it routinely causes trouble.
    return CmdlineArgs::Instance().getDeveloper() ||
            !QLatin1String(deviceInfo.name)
                     .startsWith(kMidiThroughPortPrefix, Qt::CaseInsensitive);
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
    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }
    PmError err = Pm_Terminate();
    // Based on reading the source, it's not possible for this to fail.
    if (err != pmNoError) {
        qWarning() << "PortMidi error:" << Pm_GetErrorText(err);
    }
}

bool namesMatchMidiPattern(const QString& input_name,
        const QString& output_name) {
    // Some platforms format MIDI device names as "deviceName MIDI ###" where
    // ### is the instance # of the device. Therefore we want to link two
    // devices that have an equivalent "deviceName" and ### section.
    QRegExp deviceNamePattern("^(.*) MIDI (\\d+)( .*)?$");

    int inputMatch = deviceNamePattern.indexIn(input_name);
    if (inputMatch == 0) {
        QString inputDeviceName = deviceNamePattern.cap(1);
        QString inputDeviceIndex = deviceNamePattern.cap(2);
        int outputMatch = deviceNamePattern.indexIn(output_name);
        if (outputMatch == 0) {
            QString outputDeviceName = deviceNamePattern.cap(1);
            QString outputDeviceIndex = deviceNamePattern.cap(2);
            if (outputDeviceName.compare(inputDeviceName, Qt::CaseInsensitive) == 0 &&
                outputDeviceIndex == inputDeviceIndex) {
                return true;
            }
        }
    }
    return false;
}

bool namesMatchInOutPattern(const QString& input_name,
        const QString& output_name) {
    QString basePattern = "^(.*) %1 (\\d+)( .*)?$";
    QRegExp inputPattern(basePattern.arg("in"));
    QRegExp outputPattern(basePattern.arg("out"));

    int inputMatch = inputPattern.indexIn(input_name);
    if (inputMatch == 0) {
        QString inputDeviceName = inputPattern.cap(1);
        QString inputDeviceIndex = inputPattern.cap(2);
        int outputMatch = outputPattern.indexIn(output_name);
        if (outputMatch == 0) {
            QString outputDeviceName = outputPattern.cap(1);
            QString outputDeviceIndex = outputPattern.cap(2);
            if (outputDeviceName.compare(inputDeviceName, Qt::CaseInsensitive) == 0 &&
                outputDeviceIndex == inputDeviceIndex) {
                return true;
            }
        }
    }
    return false;
}

bool namesMatchPattern(const QString& input_name,
        const QString& output_name) {
    // This is a broad pattern that matches a text blob followed by a numeral
    // potentially followed by non-numeric text. The non-numeric requirement is
    // meant to avoid corner cases around devices with names like "Hercules RMX
    // 2" where we would potentially confuse the number in the device name as
    // the ordinal index of the device.
    QRegExp deviceNamePattern("^(.*) (\\d+)( [^0-9]+)?$");

    int inputMatch = deviceNamePattern.indexIn(input_name);
    if (inputMatch == 0) {
        QString inputDeviceName = deviceNamePattern.cap(1);
        QString inputDeviceIndex = deviceNamePattern.cap(2);
        int outputMatch = deviceNamePattern.indexIn(output_name);
        if (outputMatch == 0) {
            QString outputDeviceName = deviceNamePattern.cap(1);
            QString outputDeviceIndex = deviceNamePattern.cap(2);
            if (outputDeviceName.compare(inputDeviceName, Qt::CaseInsensitive) == 0 &&
                outputDeviceIndex == inputDeviceIndex) {
                return true;
            }
        }
    }
    return false;
}

bool namesMatchAllowableEdgeCases(const QString& input_name,
        const QString& output_name) {
    // Mac OS 10.12 & Korg Kaoss DJ 1.6:
    // Korg Kaoss DJ has input 'KAOSS DJ CONTROL' and output 'KAOSS DJ SOUND'.
    // This means it doesn't pass the shouldLinkInputToOutput test. Without an
    // output linked, the MIDI output for the device fails, as the device is
    // NULL in PortMidiController
    if (input_name == "KAOSS DJ CONTROL" && output_name == "KAOSS DJ SOUND") {
        return true;
    }
    return false;
}

bool shouldLinkInputToOutput(const QString& input_name,
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
        int offset = input_name_stripped.indexOf(" input ", 0,
                                                 Qt::CaseInsensitive);
        if (offset != -1) {
            input_name_stripped = input_name_stripped.replace(offset, 7, " ");
        }

        // Ignore " output " text in the device names
        offset = output_name_stripped.indexOf(" output ", 0,
                                              Qt::CaseInsensitive);
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

/** Enumerate the MIDI devices
  * This method needs a bit of intelligence because PortMidi (and the underlying MIDI APIs) like to split
  * output and input into separate devices. Eg. PortMidi would tell us the Hercules is two half-duplex devices.
  * To help simplify a lot of code, we're going to aggregate these two streams into a single full-duplex device.
  */
QList<Controller*> PortMidiEnumerator::queryDevices() {
    qDebug() << "Scanning PortMIDI devices:";

    int iNumDevices = Pm_CountDevices();

    QListIterator<Controller*> dev_it(m_devices);
    while (dev_it.hasNext()) {
        delete dev_it.next();
    }

    m_devices.clear();

    const PmDeviceInfo* inputDeviceInfo = nullptr;
    const PmDeviceInfo* outputDeviceInfo = nullptr;
    int inputDevIndex = -1;
    int outputDevIndex = -1;
    QMap<int, QString> unassignedOutputDevices;

    // Build a complete list of output devices for later pairing
    for (int i = 0; i < iNumDevices; i++) {
        const PmDeviceInfo* pDeviceInfo = Pm_GetDeviceInfo(i);
        VERIFY_OR_DEBUG_ASSERT(pDeviceInfo) {
            continue;
        }
        if (!recognizeDevice(*pDeviceInfo) || !pDeviceInfo->output) {
            continue;
        }
        qDebug() << " Found output device"
                 << "#" << i << pDeviceInfo->name;
        QString deviceName = pDeviceInfo->name;
        unassignedOutputDevices[i] = deviceName;
    }

    // Search for input devices and pair them with output devices if applicable
    for (int i = 0; i < iNumDevices; i++) {
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
        inputDeviceInfo = pDeviceInfo;
        inputDevIndex = i;

        //Reset our output device variables before we look for one in case we find none.
        outputDeviceInfo = nullptr;
        outputDevIndex = -1;

        //Search for a corresponding output device
        QMapIterator<int, QString> j(unassignedOutputDevices);
        while (j.hasNext()) {
            j.next();

            QString deviceName = inputDeviceInfo->name;
            QString outputName = QString(j.value());

            if (shouldLinkInputToOutput(deviceName, outputName)) {
                outputDevIndex = j.key();
                outputDeviceInfo = Pm_GetDeviceInfo(outputDevIndex);

                unassignedOutputDevices.remove(outputDevIndex);

                qDebug() << "    Linking to output device #" << outputDevIndex << outputName;
                break;
            }
        }

        // So at this point, we either have an input-only MIDI device
        // (outputDeviceInfo == NULL) or we've found a matching output MIDI
        // device (outputDeviceInfo != NULL).

        //.... so create our (aggregate) MIDI device!
        PortMidiController* currentDevice =
                new PortMidiController(inputDeviceInfo,
                        outputDeviceInfo,
                        inputDevIndex,
                        outputDevIndex);
        m_devices.push_back(currentDevice);
    }
    return m_devices;
}
