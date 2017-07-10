/**
  * @file ctlracontroller.cpp
  * @author Harry van Haaren harryhaaren@gmail.com
  * @date Thu Dec 22 2016
  * @brief Ctlra backend implementation
  *
  */

#include <wchar.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "controllers/ctlra/ctlracontroller.h"
#include "util/path.h" // for PATH_MAX on Windows
#include "controllers/defs_controllers.h"
#include "util/compatibility.h"
#include "util/trace.h"
#include "controllers/controllerdebug.h"
#include "util/time.h"

#include "control/controlobject.h"
#include "control/controlproxy.h"

#include "ctlra.h"

void CtlraController::event_func(struct ctlra_dev_t* dev,
				 uint32_t num_events,
				 struct ctlra_event_t** events)
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
				// TODO: optimize to avoid repeated
				// lookups of ControlProxy
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

void CtlraController::feedback_func(struct ctlra_dev_t* dev)
{
	// TODO: optimize to avoid repeated lookups of ControlProxy
	int cue_indicator = ControlProxy("[Channel1]","cue_indicator").get();
	uint32_t cue_led_1 = cue_indicator ? 0xffffffff : 0x11111111;
	ctlra_dev_light_set(dev, 0, cue_led_1);
	ctlra_dev_light_flush(dev, 1);
}

CtlraController::CtlraController(const struct ctlra_dev_info_t* info)
{
	setDeviceName(info->device);
	// Copy required variables from deviceInfo, which will be freed after
	// this class is initialized by caller.
	m_preset.setDeviceId(info->serial);
	char buf[64];
	snprintf(buf, sizeof(buf), "%s %s\n", info->vendor, info->device);
	m_preset.setName(buf);
	m_preset.setAuthor("OpenAV");
	m_preset.setDescription("Ctlra controller instance: events need "
                                "mapping, and is currently no use (yet :)");
	open();
}

CtlraController::~CtlraController()
{
	close();
}

int CtlraController::open()
{
	if (isOpen()) {
		qDebug() << "Ctlra device already open";
		return -1;
	}

	setOpen(true);
	startEngine();

	return 0;
}

int CtlraController::close()
{
	if (!isOpen()) {
		qDebug() << "Ctlra device already closed";
		return -1;
	}

	// Stop controller engine here to ensure it's done before the device is closed
	//  incase it has any final parting messages
	stopEngine();

	// Close device
	controllerDebug("Ctlra Closing device - done");

	setOpen(false);

	return 0;
}
