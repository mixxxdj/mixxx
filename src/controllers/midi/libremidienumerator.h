#pragma once

#include <libremidi/libremidi.hpp>

#include "controllers/midi/midienumerator.h"
#include "preferences/usersettings.h"

/// This class handles discovery and enumeration of DJ controllers that appear
/// under the Libremidi cross-platform API.
class LibremidiEnumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    LibremidiEnumerator(UserSettingsPointer pConfig);
    ~LibremidiEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    libremidi::observer obs;
    std::vector<libremidi::input_port> input_ports;
    std::vector<libremidi::output_port> output_ports;
    QList<Controller*> m_devices;
    UserSettingsPointer m_pConfig;
};

// For testing.
bool libremidiShouldLinkInputToOutput(const QString& input_name,
        const QString& output_name);
