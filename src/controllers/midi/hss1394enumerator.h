#pragma once

#include <QList>
#include <memory>
#include <vector>

#include "controllers/midi/midienumerator.h"

class Controller;

// Handles discovery and enumeration of DJ controllers that appear under the
// HSS1394 cross-platform API.
class Hss1394Enumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    explicit Hss1394Enumerator();
    ~Hss1394Enumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    std::vector<std::unique_ptr<Controller>> m_devices;
};
