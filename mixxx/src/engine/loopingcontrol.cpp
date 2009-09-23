// loopingcontrol.cpp
// Created on Sep 23, 2008
// Author: asantoni, rryan

#include <QtDebug>
#include <QObject>

#include "controlobject.h"
#include "configobject.h"
#include "controlpushbutton.h"
#include "cachingreader.h"
#include "engine/loopingcontrol.h"
#include "engine/enginecontrol.h"
#include "mathstuff.h"

LoopingControl::LoopingControl(const char * _group,
                               ConfigObject<ConfigValue> * _config)
        : EngineControl(_group, _config) {

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

double LoopingControl::process(const double dRate,
                               const double currentSample,
                               const double totalSamples,
                               const int iBufferSize) {
    m_iCurrentSample = currentSample;
    if (!even(m_iCurrentSample))
        m_iCurrentSample--;

    bool reverse = dRate < 0;

    double retval = currentSample;
    if(m_bLoopingEnabled) {
        if (reverse) {
            if (currentSample <= m_iLoopStartSample)
                retval = m_iLoopEndSample;
        } else {
            if (currentSample >= m_iLoopEndSample)
                retval = m_iLoopStartSample;
        }
    }

    return retval;
}

double LoopingControl::nextTrigger(const double dRate,
                                   const double currentSample,
                                   const double totalSamples,
                                   const int iBufferSize) {
    bool bReverse = dRate < 0;

    if(m_bLoopingEnabled) {
        if (bReverse)
            return m_iLoopStartSample;
        else
            return m_iLoopEndSample;
    }
    return kNoTrigger;
}

double LoopingControl::getTrigger(const double dRate,
                                  const double currentSample,
                                  const double totalSamples,
                                  const int iBufferSize) {
    bool bReverse = dRate < 0;

    if(m_bLoopingEnabled) {
        if (bReverse)
            return m_iLoopEndSample;
        else
            return m_iLoopStartSample;
    }
    return kNoTrigger;
}

void LoopingControl::hintReader(QList<Hint>& hintList) {
    Hint loop_hint;
    // If the loop is enabled, then this is high priority because we will loop
    // sometime potentially very soon! The current audio itself is priority 1,
    // but we will issue ourselves at priority 2.
    loop_hint.priority = 2 : 10;
    if (m_bLoopingEnabled) {
        // If we're looping, hint the loop in and loop out, in case we reverse
        // into it. We could save information from process to tell which
        // direction we're going in, but that this is much simpler, and hints
        // aren't that bad to make anyway.
        loop_hint.priority = 2;
        loop_hint.sample = m_iLoopStartSample;
        loop_hint.length = 0; // Let it issue the default length
        hintList.append(loop_hint);
        loop_hint.priority = 10;
        loop_hint.sample = m_iLoopEndSample;
        loop_hint.length = -1; // Let it issue the default (backwards) length
        hintList.append(loop_hint);
    } else {
        loop_hint.priority = 10;
        loop_hint.sample = m_iLoopStartSample;
        loop_hint.length = 0; // Let it issue the default length
        hint_list.append(loop_hint);
    }
}

void LoopingControl::slotLoopIn(double val) {
    if (val == 1.0f) {
        // set loop in position
        m_iLoopStartSample = m_iCurrentSample;
        m_pCOLoopStartPosition->set(m_iLoopStartSample);
        qDebug() << "set loop_in to " << m_iLoopStartSample;
    }
}

void LoopingControl::slotLoopOut(double val) {
    if (val == 1.0f) {
        //set loop out position and start looping
        m_iLoopEndSample = m_iCurrentSample;
        m_pCOLoopEndPosition->set(m_iLoopEndSample);
        m_bLoopingEnabled = true;
        qDebug() << "set loop_out to " << m_iLoopStartSample;
    }
}

void LoopingControl::slotReloopExit(double val) {
    if (val == 1.0f) {
        // If we're looping, stop looping
        if (m_bLoopingEnabled) {
            m_bLoopingEnabled = false;
            qDebug() << "reloop_exit looping off";
        } else {
            // If we're not looping, jump to the loop-in point and start looping
            m_bLoopingEnabled = true;
            qDebug() << "reloop_exit looping on";
        }
    }
}
