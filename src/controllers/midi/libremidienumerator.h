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

  signals:
    void deviceAdded(Controller* pController);
    void deviceRemoved(Controller* pController);
    void deviceInputAdded(Controller* pController);

  private:
    Controller* addDevice(const libremidi::input_port* pInputPort,
            const libremidi::output_port* pOutputPort);
    void inputAdded(const libremidi::input_port& port, bool notify);
    void inputRemoved(const libremidi::input_port& port);
    void outputAdded(const libremidi::output_port& port, bool notify);
    void outputRemoved(const libremidi::output_port& port);

    libremidi::observer m_observer;
    std::vector<std::unique_ptr<LibremidiController>> m_devices;
    UserSettingsPointer m_pConfig;
    ControllerManager* m_pControllerManager;
    std::mutex m_mutex;
};

// For testing.
bool libremidiShouldLinkInputToOutput(const QString& input_name,
        const QString& output_name);
