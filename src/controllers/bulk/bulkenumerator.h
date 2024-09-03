#pragma once

#include <memory>
#include <vector>

#include "controllers/controllerenumerator.h"
#include "preferences/usersettings.h"

struct libusb_context;
struct BulkController;

/// Locate supported USB bulk controllers
class BulkEnumerator : public ControllerEnumerator {
    Q_OBJECT
  public:
    explicit BulkEnumerator(UserSettingsPointer pConfig);
    ~BulkEnumerator() override;

    QList<Controller*> queryDevices() override;

  private:
    std::unique_ptr<libusb_context, void (*)(libusb_context*)> m_context;
    std::vector<std::unique_ptr<BulkController>> m_devices;
    UserSettingsPointer m_pConfig;
};
