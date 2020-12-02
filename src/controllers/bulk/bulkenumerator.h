/**
* @file bulkenumerator.h
* @author Neale Pickett  neale@woozle.org
* @date Thu Jun 28 2012
* @brief Locate supported USB bulk controllers
*/

#ifndef BULKENUMERATOR_H
#define BULKENUMERATOR_H

#include <libusb.h>

#include <QList>

#include "controllers/controllerenumerator.h"
#include "preferences/usersettings.h"

struct libusb_context;
class Controller;

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

#endif
