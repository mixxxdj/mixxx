#pragma once

#include "controllers/controllerenumerator.h"
#include <QtCore/QSharedPointer>
#include "preferences/usersettings.h"

struct libusb_context;

/// Locate supported USB bulk controllers
class BulkEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    explicit BulkEnumerator(UserSettings *pConfig);
    virtual ~BulkEnumerator();

    QList<Controller*> queryDevices() override;

  private:
    QList<Controller*> m_devices;
    libusb_context *m_context;
    UserSettings *m_pConfig;
};
