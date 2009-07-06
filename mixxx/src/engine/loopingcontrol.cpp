/*
 * LoopingControl.cpp
 *
 *  Created on: Sep 23, 2008
 *      Author: asantoni
 */

#include <QtDebug>
#include <QObject>


#include "controlobject.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "engine/loopingcontrol.h"
#include "engine/enginecontrol.h"

LoopingControl::LoopingControl(const char * _group,
                               ConfigObject<ConfigValue> * _config) :
    EngineControl(_group, _config) {

	m_bLoopingEnabled = false;
	m_iLoopStartSample = 0;
	m_iLoopEndSample = 0;

    //Create loop-in, loop-out, and reloop/exit ControlObjects
    m_pLoopInButton = new ControlPushButton(ConfigKey(_group, "loop_in"), true);
    connect(m_pLoopInButton, SIGNAL(valueChanged(double)), this, SLOT(slotLoopIn(double)));
    m_pLoopInButton->set(0);

    m_pLoopOutButton = new ControlPushButton(ConfigKey(_group, "loop_out"), true);
    connect(m_pLoopOutButton, SIGNAL(valueChanged(double)), this, SLOT(slotLoopOut(double)));
    m_pLoopOutButton->set(0);

    m_pReloopExitButton = new ControlPushButton(ConfigKey(_group, "reloop_exit"), true);
    connect(m_pReloopExitButton, SIGNAL(valueChanged(double)), this, SLOT(slotReloopExit(double)));
    m_pReloopExitButton->set(0);

    m_pCOLoopStartPosition = new ControlObject(ConfigKey(_group, "loop_start_position"));
    m_pCOLoopEndPosition = new ControlObject(ConfigKey(_group, "loop_end_position"));
}

LoopingControl::~LoopingControl() {
}

double LoopingControl::process(double currentSample, double totalSamples)
{
	// From WBS: 1.2.2 In process() (or similar) function, check if playpos ==
    // end of loop (and move playpos to start of loop)

    m_iCurrentSample = currentSample;
    
    double retval = currentSample;
    if(m_bLoopingEnabled) {
        if (currentSample >= m_iLoopEndSample) {
        
            //Oh crap, the play position ticks along in buffer sizes. How do we
            //wrap when the loop end position is in the middle of a buffer?
            //Maybe this is why looping isn't implemented yet...

            // Oh well, let's hack around it!
            // People with high-latency will have sucky looping though.
            retval = m_iLoopStartSample;
        }
    } 
    
    return retval;
}

void LoopingControl::slotLoopIn(double val)
{
	if (val == 1.0f)
	{
		//set loop in position
        m_iLoopStartSample = m_iCurrentSample;
        m_pCOLoopStartPosition->set(m_iLoopStartSample);
        qDebug() << "set loop_in to " << m_iLoopStartSample;
	}
}

void LoopingControl::slotLoopOut(double val)
{
	if (val == 1.0f)
	{
		//set loop out position and start looping
        m_iLoopEndSample = m_iCurrentSample;
        m_pCOLoopEndPosition->set(m_iLoopEndSample);
		m_bLoopingEnabled = true;
        qDebug() << "set loop_out to " << m_iLoopStartSample;
	}
}

void LoopingControl::slotReloopExit(double val)
{
	if (val == 1.0f)
	{
        
		//if we're looping, stop looping
		if (m_bLoopingEnabled) {
            m_bLoopingEnabled = false;
            qDebug() << "reloop_exit looping off";
		} else {
			//if we're not looping, jump to the loop-in point and start looping
            m_bLoopingEnabled = true;
            qDebug() << "reloop_exit looping on";
		}

	}
}
