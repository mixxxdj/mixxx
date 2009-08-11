// NOTE: CoreMIDI must be defined first to avoid qDebug macro/function conflict
//       with Qt headers.
#ifdef __COREMIDI__
  #include "midiobjectcoremidi.h"
#endif

#include "mididevicehandler.h"
#include "midiobject.h"

#ifdef __C_METRICS__
#include <cmetrics.h>
#include "defs_mixxxcmetrics.h"
#endif

#ifdef __ALSAMIDI__
  #include "midiobjectalsa.h"
#endif

#ifdef __ALSASEQMIDI__
  #include "midiobjectalsaseq.h"
#endif

#ifdef __PORTMIDI__
  #include "midiobjectportmidi.h"
#endif


#ifdef __OSSMIDI__
  #include "midiobjectoss.h"
#endif

#ifdef __WINMIDI__
  #include "midiobjectwin.h"
#endif

// #ifdef __LINUX__
// #include "mouselinux.h"
// #endif
// #ifdef __LINUX__
// #include "powermatelinux.h"
// #endif
// #ifdef __WINDOWS__
// #include "powermatewin.h"
// #endif
// #include "hercules.h"
// #ifdef __LINUX__
// #include "herculeslinux.h"
// #endif

#include "midiobjectnull.h"

MidiDeviceHandler::MidiDeviceHandler() {
    // Open midi
    m_pMidi = 0;
#ifdef __PORTMIDI__
    m_pMidi = new MidiObjectPortMidi();
    return;
#endif
#ifdef __ALSAMIDI__
    m_pMidi = new MidiObjectALSA();
#endif
#ifdef __ALSASEQMIDI__
    m_pMidi = new MidiObjectALSASeq();
#endif
#ifdef __COREMIDI__
    m_pMidi = new MidiObjectCoreMidi();
#endif
#ifdef __OSSMIDI__
    m_pMidi = new MidiObjectOSS();
#endif
#ifdef __WINMIDI__
    m_pMidi = new MidiObjectWin();
#endif

    if (m_pMidi == 0)
        m_pMidi = new MidiObjectNull();
}

MidiObject* MidiDeviceHandler::getMidiPtr() {
	return m_pMidi;
}
