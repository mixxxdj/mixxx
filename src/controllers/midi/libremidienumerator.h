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
    LibremidiEnumerator(UserSettingsPointer pConfig, ControllerManager* manager);

    QList<Controller*> queryDevices() override {
        return QList<Controller*>();
    }

  signals:
    void deviceAdded(Controller* controller);
    void deviceRemoved(Controller* controller);

  private:
    Controller* addPort();
    void addDevice(const libremidi::input_port* inputPort,
            const libremidi::output_port* outputPort);
    void inputAdded(const libremidi::input_port& port);
    void inputRemoved(const libremidi::input_port& port);
    void outputAdded(const libremidi::output_port& port);
    void outputRemoved(const libremidi::output_port& port);

    libremidi::observer m_observer;
    std::vector<std::unique_ptr<LibremidiController>> m_devices;
    UserSettingsPointer m_pConfig;
    ControllerManager* m_pControllerManager;
    mutable QMutex m_mutex;
};

// For testing.
bool libremidiShouldLinkInputToOutput(const QString& input_name,
        const QString& output_name);
