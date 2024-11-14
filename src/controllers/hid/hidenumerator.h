#pragma once

#include "controllers/controllerenumerator.h"
#include "controllers/hid/hiddevice.h"

/// This class handles discovery and enumeration of DJ controllers that use the
/// USB-HID protocol.
class HidEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    explicit HidEnumerator(UserSettingsPointer pConfig);
    ~HidEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    UserSettingsPointer m_pConfig;
    QList<Controller*> m_devices;
    mixxx::hid::HidUsageTables m_hidUsageTable;
};
