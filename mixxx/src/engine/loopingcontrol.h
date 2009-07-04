/*
 * LoopingControl.h
 *
 *  Created on: Sep 23, 2008
 *      Author: alb
 */

#ifndef LOOPINGCONTROL_H_
#define LOOPINGCONTROL_H_

#include "configobject.h"

#include <QObject>

class ControlPushButton;
class ControlObject;

class LoopingControl : QObject {
    Q_OBJECT
public:
	LoopingControl(const char * _group, ConfigObject<ConfigValue> * _config);
	virtual ~LoopingControl();
	double process(double absPlayPos);
	void setCurrentSample(int currentSample);

	public slots:
	void slotLoopIn(double);
	void slotLoopOut(double);
	void slotReloopExit(double);
private:
	QString m_group;
	ConfigObject<ConfigValue>* m_pConfig;
	ControlObject* m_pCOLoopStartPosition;
	ControlObject* m_pCOLoopEndPosition;
	ControlPushButton* m_pLoopInButton;
	ControlPushButton* m_pLoopOutButton;
	ControlPushButton* m_pReloopExitButton;
	bool m_bLoopingEnabled;
	unsigned long m_iLoopEndSample;
	unsigned long m_iLoopStartSample;
	unsigned long m_iCurrentSample;

};

#endif /* LOOPINGCONTROL_H_ */
