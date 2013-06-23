/**
* @file controllerenumerator.h
* @author Sean Pappalardo spappalardo@mixxx.org
* @date Sat Apr 30 2011
* @brief Base class handling discovery and enumeration of DJ controllers.
*
* This class handles discovery and enumeration of DJ controllers and
*   must be inherited by a class that implements it on some API.
*/

#ifndef CONTROLLERENUMERATOR_H
#define CONTROLLERENUMERATOR_H

#include "controllers/controller.h"

class ControllerEnumerator : public QObject {
    Q_OBJECT
  public:
    ControllerEnumerator();
    // In this function, the inheriting class must delete the Controllers it
    // creates
    virtual ~ControllerEnumerator();

    virtual QList<Controller*> queryDevices() = 0;

    // Sub-classes return true here if their devices must be polled to get data
    // from the controler.
    virtual bool needPolling() {
        return false;
    }
};

#endif
