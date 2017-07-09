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
