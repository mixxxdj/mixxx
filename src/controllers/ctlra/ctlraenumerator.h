/**
* @file ctlraenumerator.h
* @author Harry van Haaren harryhaaren@gmail.com
* @date Thu Dec 22 2016
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#ifndef CTLRAENUMERATOR_H
#define CTLRAENUMERATOR_H

#include "controllers/controllerenumerator.h"

// foward declaration only
struct ctlra_t;

class CtlraEnumerator : public ControllerEnumerator {
  public:
    CtlraEnumerator();
    virtual ~CtlraEnumerator();

    QList<Controller*> queryDevices();

  private:
    QList<Controller*> m_devices;

    // This is the main ctlra instance for Mixxx. It is contained within
    // the Enumerator class as when devices are hotpluggeed, this class is
    // responsible for adding/removing that controller from the list. For
    // that reason, it makes most sense that the Enumerator is the Ctlra
    // context owner.
    struct ctlra_t *ctlra;
};

#endif
