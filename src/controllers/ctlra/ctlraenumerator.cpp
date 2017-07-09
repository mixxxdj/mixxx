/**
* @file ctlraenumerator.cpp
* @author Harry van Haaren harryhaaren@gmail.com
* @date Thu Dec 22 2016
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#include "controllers/ctlra/ctlraenumerator.h"

#include "control/controlproxy.h"
#include "controllers/ctlra/ctlracontroller.h"

#include "controllers/ctlra/ctlra.h"

CtlraReader::CtlraReader(struct ctlra_t *c)
	: QThread(),
	  ctlra(c)
{
}

CtlraReader::~CtlraReader()
{
}

void CtlraReader::run()
{
	m_stop = 0;
	while (load_atomic(m_stop) == 0) {
		ctlra_idle_iter(ctlra);
		usleep(5 * 1000);
	}
}

// Hide these typedefs from the header file by passing a struct* instead
struct mixxx_ctlra_accept_t {
	const struct ctlra_dev_info_t* info;
	ctlra_event_func* event_func;
	ctlra_feedback_func* feedback_func;
	ctlra_remove_dev_func* remove_func;
	void** userdata_for_event_func;
};

static void
mixxx_event_func(struct ctlra_dev_t* dev, uint32_t num_events,
                 struct ctlra_event_t** events, void *userdata)
{
	// Cast to the pointer as was registered with Ctlra library in
	// CtlraEnumerator::accept_dev_func. This allows us to look up the
	// CtlraController class that the event originates from, and handle
	// the event specifically for that device instance.
	CtlraController* c = (CtlraController*)userdata;
	c->event_func(dev, num_events, events);
}

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
	*a->event_func = mixxx_event_func;

	// here we add the CtlraController instance to the GUI
	CtlraController *c = new CtlraController(a->info);
	m_devices.push_back(c);

	// pass the CtlraController as the userdata pointer to the event
	// callback functions as registered with Ctlra library. This allows
	// easy lookup of the CtlraController metadata when events arrive
	*a->userdata_for_event_func = (void*)c;
	printf("controller instance = %p\n", c);

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

	m_reader = new CtlraReader(m_ctlra);
	if(m_reader == nullptr) {
		printf("CtlraEnumerator error creating m_reader!\n");
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

	// Controller input needs to be prioritized since it can affect the
	// audio directly, like when scratching
	m_reader->start(QThread::HighPriority);

	return m_devices;
}
