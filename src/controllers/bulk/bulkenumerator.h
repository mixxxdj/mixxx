#pragma once

#include "controllers/controllerenumerator.h"

struct libusb_context;

/// Locate supported USB bulk controllers
class BulkEnumerator : public ControllerEnumerator {
  public:
    explicit BulkEnumerator(UserSettingsPointer pConfig);
    virtual ~BulkEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;
    libusb_context* m_context;
    UserSettingsPointer m_pConfig;
};
