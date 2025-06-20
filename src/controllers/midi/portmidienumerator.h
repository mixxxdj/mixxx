#pragma once

#include "controllers/midi/midienumerator.h"
#include "preferences/usersettings.h"

/// This class handles discovery and enumeration of DJ controllers that appear under the PortMIDI cross-platform API.
class PortMidiEnumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    PortMidiEnumerator(UserSettingsPointer pConfig);
    ~PortMidiEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
    UserSettingsPointer m_pConfig;
};

// For testing.
bool shouldLinkInputToOutput(const QString& input_name,
        const QString& output_name);
