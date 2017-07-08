/**
* @file ctlraenumerator.cpp
* @author Harry van Haaren harryhaaren@gmail.com
* @date Thu Dec 22 2016
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#include "controllers/ctlra/ctlraenumerator.h"

#include "control/controlobject.h"

#include "controllers/ctlra/ctlra.h"

/* A reader thread to poll the device, execute actions based on the
 * recieved events, and post them to Mixxx using ControlObjects */
class CtlraReader : public QThread
{
	Q_OBJECT
public:
	CtlraReader(struct ctlra_t *dev);
	virtual ~CtlraReader();

	void stop()
	{
		m_stop = 1;
	}

protected:
	void run();

private:
	struct ctlra_t *ctlra;
	QAtomicInt m_stop;
};


// Hide these typedefs from the header file by passing a struct* instead
struct mixxx_ctlra_accept_t {
	const struct ctlra_dev_info_t* info;
	ctlra_event_func* event_func;
	ctlra_feedback_func* feedback_func;
	ctlra_remove_dev_func* remove_func;
	void** userdata_for_event_func;
};

static int mixxx_accept_dev_func(const struct ctlra_dev_info_t *info,
                    ctlra_event_func *event_func,
                    ctlra_feedback_func *feedback_func,
                    ctlra_remove_dev_func *remove_func,
                    void **userdata_for_event_func,
                    void *userdata)
{
	CtlraEnumerator *ce = (CtlraEnumerator*)userdata;

	// hide functions in struct, avoids polluting CtlraEnumerator
	// header with all of the Ctlra functions and typedefs
	struct mixxx_ctlra_accept_t accept;
	accept.info = info;
	accept.event_func = event_func;
	accept.feedback_func = feedback_func;
	accept.remove_func = remove_func;
	accept.userdata_for_event_func = userdata_for_event_func;

	return ce->accept_dev_func(&accept);
}


int CtlraEnumerator::accept_dev_func(struct mixxx_ctlra_accept_t *a)
{
	printf("mixxx-ctlra accepting %s %s\n",
	       a->info->vendor,
	       a->info->device);
	return 1;
}

CtlraEnumerator::CtlraEnumerator() : ControllerEnumerator()
{
	qDebug() << "CtlraEnumerator\n";

	struct ctlra_create_opts_t opts;
	opts.flags_usb_no_own_context = 1;
	m_ctlra = ctlra_create(&opts);
	if(m_ctlra == 0) {
		printf("Ctlra error creating context!\n");
		return;
	}
}

CtlraEnumerator::~CtlraEnumerator()
{
	qDebug() << "Deleting Ctlra devices...";
	while (m_devices.size() > 0) {
		delete m_devices.takeLast();
	}

	ctlra_exit(m_ctlra);
}

QList<Controller*> CtlraEnumerator::queryDevices()
{
	// probe for devices, the accept_func is called once per device
	m_num_devices = ctlra_probe(m_ctlra, mixxx_accept_dev_func, this);

	return m_devices;
}
