/**
 * @file portmidienumerator.h
 * @author Sean Pappalardo spappalardo@mixxx.org
 * @date Thu 15 Mar 2012
 * @brief This class handles discovery and enumeration of DJ controllers that appear under the PortMIDI cross-platform API.
 */

#ifndef PORTMIDIENUMERATOR_H
#define PORTMIDIENUMERATOR_H

#include "controllers/midi/midienumerator.h"

class PortMidiEnumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    explicit PortMidiEnumerator(UserSettingsPointer pConfig);
    virtual ~PortMidiEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;
    UserSettingsPointer m_pConfig;
};

// For testing.
bool shouldLinkInputToOutput(const QString input_name,
                             const QString output_name);

#endif
