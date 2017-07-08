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
struct mixxx_ctlra_accept_t;

/* A reader thread to poll the device, execute actions based on the
 * recieved events, and post them to Mixxx using ControlObjects */
class CtlraReader : public QThread
{
	Q_OBJECT
public:
	CtlraReader(struct ctlra_t *dev);
	virtual ~CtlraReader();
	void stop() {
		m_stop = 1;
	}

protected:
	void run();

private:
	struct ctlra_t *ctlra;
	QAtomicInt m_stop;
};


class CtlraEnumerator : public ControllerEnumerator {
  public:
    CtlraEnumerator();
    virtual ~CtlraEnumerator();

    QList<Controller*> queryDevices();

    // accept function, called once per device, and for hotplug arrival
    // this is public so it can be called from the static wrapper
    int accept_dev_func(struct mixxx_ctlra_accept_t*);

  private:
    QList<Controller*> m_devices;

    // This is the main ctlra instance for Mixxx. It is contained within
    // the Enumerator class as when devices are hotpluggeed, this class is
    // responsible for adding/removing that controller from the list. For
    // that reason, it makes most sense that the Enumerator is the Ctlra
    // context owner.
    struct ctlra_t *m_ctlra;

    // the number of Ctlra devices in use
    uint32_t m_num_devices;

    // The reader for the Ctlra devices
    CtlraReader* m_reader;

};

#endif
