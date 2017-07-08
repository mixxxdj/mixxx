/**
* @file ctlraenumerator.cpp
* @author Harry van Haaren harryhaaren@gmail.com
* @date Thu Dec 22 2016
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#include "controllers/ctlra/ctlraenumerator.h"

#include "control/controlproxy.h"

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
	static const char* grid_pressed[] = { " X ", "   " };
	struct ctlra_dev_info_t info;
	ctlra_dev_get_info(dev, &info);

	for(uint32_t i = 0; i < num_events; i++) {
		struct ctlra_event_t *e = events[i];
		const char *pressed = 0;
		const char *name = 0;
		switch(e->type) {
		case CTLRA_EVENT_BUTTON:
			name = ctlra_info_get_name(&info, CTLRA_EVENT_BUTTON,
			                           e->button.id);
			printf("[%s] button %s (%d)\n",
			       e->button.pressed ? " X " : "   ",
			       name, e->button.id);
			break;

		case CTLRA_EVENT_ENCODER:
			name = ctlra_info_get_name(&info, CTLRA_EVENT_ENCODER,
			                           e->encoder.id);
			printf("[%s] encoder %s (%d)\n",
			       e->encoder.delta > 0 ? " ->" : "<- ",
			       name, e->button.id);
			break;

		case CTLRA_EVENT_SLIDER:
			name = ctlra_info_get_name(&info, CTLRA_EVENT_SLIDER,
			                           e->slider.id);
			printf("[%03d] slider %s (%d)\n",
			       (int)(e->slider.value * 100.f),
			       name, e->slider.id);
			if(e->slider.id == 0) {
				ControlProxy("[Master]", "crossfader")
					.set((e->slider.value * 2) - 1);
			}
			break;

		case CTLRA_EVENT_GRID:
			name = ctlra_info_get_name(&info, CTLRA_EVENT_GRID,
			                           e->grid.id);
			if(e->grid.flags & CTLRA_EVENT_GRID_FLAG_BUTTON) {
				pressed = grid_pressed[e->grid.pressed];
			} else {
				pressed = "---";
			}
			printf("[%s] grid %d", pressed ? " X " : "   ", e->grid.pos);
			if(e->grid.flags & CTLRA_EVENT_GRID_FLAG_PRESSURE)
				printf(", pressure %1.3f", e->grid.pressure);
			printf("\n");
			break;
		default:
			break;
		};
	}
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
