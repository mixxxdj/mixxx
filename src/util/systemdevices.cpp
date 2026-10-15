#include "util/systemdevices.h"

#include <QFile>

#ifdef Q_OS_WIN
#include <windows.h>

#include <mmsystem.h>
#endif

#ifdef Q_OS_LINUX
#include <QRegularExpression>
#endif

namespace mixxx {

QStringList systemConnectedDeviceNames() {
    QStringList names;

#ifdef Q_OS_WIN
    // The Windows MIDI devices. Reading them does not initialize the MIDI
    // backend Mixxx uses for controllers, so it cannot disturb it.
    const UINT midiInputCount = midiInGetNumDevs();
    for (UINT i = 0; i < midiInputCount; ++i) {
        MIDIINCAPS caps;
        if (midiInGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            names.append(QString::fromWCharArray(caps.szPname));
        }
    }
    const UINT midiOutputCount = midiOutGetNumDevs();
    for (UINT i = 0; i < midiOutputCount; ++i) {
        MIDIOUTCAPS caps;
        if (midiOutGetDevCaps(i, &caps, sizeof(caps)) == MMSYSERR_NOERROR) {
            names.append(QString::fromWCharArray(caps.szPname));
        }
    }
#elif defined(Q_OS_LINUX)
    // The ALSA sound cards, e.g.
    // " 2 [ProII          ]: USB-Audio - MixTrack Pro II at usb-0000:00:14.0-1"
    QFile cards(QStringLiteral("/proc/asound/cards"));
    if (cards.open(QIODevice::ReadOnly | QIODevice::Text)) {
        while (!cards.atEnd()) {
            const QString line = QString::fromLocal8Bit(cards.readLine());
            if (!line.contains(QLatin1String("USB-Audio")) &&
                    !line.contains(QLatin1String(": USB"))) {
                continue;
            }
            const QString name = line.section(QLatin1Char('-'), 1).trimmed();
            if (!name.isEmpty()) {
                names.append(name);
            }
        }
    }
#endif

    names.removeDuplicates();
    names.sort();
    return names;
}

} // namespace mixxx
