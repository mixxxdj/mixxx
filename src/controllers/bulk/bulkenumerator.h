#pragma once

#include "controllers/controllerenumerator.h"

struct libusb_context;

/// Locate supported USB bulk controllers
class BulkEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    explicit BulkEnumerator();
    virtual ~BulkEnumerator();

    QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
#ifndef __ANDROID__
    libusb_context* m_pContext;
#endif
};
