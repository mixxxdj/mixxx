#pragma once

#include <libremidi/libremidi.hpp>

#include "controllers/controllermanager.h"
#include "controllers/midi/libremidicontroller.h"
#include "controllers/midi/midienumerator.h"
#include "preferences/usersettings.h"

/// This class handles discovery and enumeration of DJ controllers that appear
/// under the Libremidi cross-platform API.
class LibremidiEnumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    LibremidiEnumerator(UserSettingsPointer pConfig, ControllerManager* pControllerManager);

    QList<Controller*> queryDevices() override;

  private:
    Controller* addDevice(const libremidi::input_port* pInputPort,
            const libremidi::output_port* pOutputPort);
    Controller* addInput(const libremidi::input_port& port);
    Controller* addOutput(const libremidi::output_port& port);
    void removeInput(const libremidi::input_port& port);
    void removeOutput(const libremidi::output_port& port);

    libremidi::observer m_observer;
    std::vector<std::unique_ptr<LibremidiController>> m_devices;
    UserSettingsPointer m_pConfig;
    ControllerManager* m_pControllerManager;
};

// For testing.
bool libremidiShouldLinkInputToOutput(const QString& input_name,
        const QString& output_name);
