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

CtlraController::CtlraController()
{
	setDeviceName("Ctlra");
	// Copy required variables from deviceInfo, which will be freed after
	// this class is initialized by caller.
	m_preset.setDeviceId("Ctlra Device");
	m_preset.setName("Scriptable Device Backend");
	m_preset.setAuthor("OpenAV");
	m_preset.setDescription("Ctlra library supports scripting of devices"
	                        ", so if your device is supported by Ctlra"
	                        ", you can script your own controls to Mixxx"
	                        " parameters.");

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
