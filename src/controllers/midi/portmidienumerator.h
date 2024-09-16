#pragma once

#include <QLatin1String>
#include <QList>
#include <memory>
#include <vector>

#include "controllers/midi/midienumerator.h"

class Controller;
class PortMidiController;

/// This class handles discovery and enumeration of DJ controllers that appear under the PortMIDI cross-platform API.
class PortMidiEnumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    PortMidiEnumerator();
    ~PortMidiEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    std::vector<std::unique_ptr<PortMidiController>> m_devices;
};

// For testing.
bool shouldLinkInputToOutput(QLatin1String input_name,
        QLatin1String output_name);
