#pragma once

#include "controllers/midi/midienumerator.h"

// Handles discovery and enumeration of DJ controllers that appear under the
// HSS1394 cross-platform API.
class Hss1394Enumerator : public MidiEnumerator {
    Q_OBJECT
  public:
    explicit Hss1394Enumerator(UserSettingsPointer pConfig);
    virtual ~Hss1394Enumerator();

    QList<Controller*> queryDevices();

  private:
    UserSettingsPointer m_pConfig;
    QList<Controller*> m_devices;
};
