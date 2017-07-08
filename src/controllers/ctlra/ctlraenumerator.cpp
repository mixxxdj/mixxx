/**
* @file ctlraenumerator.cpp
* @author Harry van Haaren harryhaaren@gmail.com
* @date Thu Dec 22 2016
* @brief This class handles discovery and enumeration of DJ controllers
* supported by the Ctlra library as developed by OpenAV.
*/

#include "controllers/ctlra/ctlraenumerator.h"

#include "control/controlobject.h"

//#include "controllers/ctlra/ctlra.h"
//#include "controllers/ctlra/ctlracontroller.h"

#include <iostream>

CtlraEnumerator::CtlraEnumerator() : ControllerEnumerator()
{
	qDebug() << "CtlraEnumerator\n";
}

CtlraEnumerator::~CtlraEnumerator()
{
	qDebug() << "Deleting Ctlra devices...";
	while (m_devices.size() > 0) {
		delete m_devices.takeLast();
	}
}

QList<Controller*> CtlraEnumerator::queryDevices()
{
	qDebug() << "CtlraEnumerator queryDevices()";
	std::cout << "\n\n\nCtlraEnumerator queryDevices()\n\n\n";

	// Move this to accept_func
	//m_devices.push_back( new CtlraController() );

	return m_devices;
}
