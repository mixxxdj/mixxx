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
    m_iLoopStartSample = kNoTrigger;
    m_iLoopEndSample = kNoTrigger;

    //Create loop-in, loop-out, and reloop/exit ControlObjects
    m_pLoopInButton = new ControlPushButton(ConfigKey(_group, "loop_in"), true);
    connect(m_pLoopInButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopIn(double)));
    m_pLoopInButton->set(0);

    m_pLoopOutButton = new ControlPushButton(ConfigKey(_group, "loop_out"), true);
    connect(m_pLoopOutButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopOut(double)));
    m_pLoopOutButton->set(0);

    m_pReloopExitButton = new ControlPushButton(ConfigKey(_group, "reloop_exit"), true);
    connect(m_pReloopExitButton, SIGNAL(valueChanged(double)),
            this, SLOT(slotReloopExit(double)));
    m_pReloopExitButton->set(0);


    m_pCOLoopEnabled = new ControlObject(ConfigKey(_group, "loop_enabled"));
    m_pCOLoopEnabled->set(0.0f);

    m_pCOLoopStartPosition =
            new ControlObject(ConfigKey(_group, "loop_start_position"));
    m_pCOLoopStartPosition->set(kNoTrigger);
    connect(m_pCOLoopStartPosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopStartPos(double)));

    m_pCOLoopEndPosition =
            new ControlObject(ConfigKey(_group, "loop_end_position"));
    m_pCOLoopEndPosition->set(kNoTrigger);
    connect(m_pCOLoopEndPosition, SIGNAL(valueChanged(double)),
            this, SLOT(slotLoopEndPos(double)));
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
    if(m_bLoopingEnabled &&
       m_iLoopStartSample != kNoTrigger &&
       m_iLoopEndSample != kNoTrigger) {
        bool outsideLoop = (currentSample >= m_iLoopEndSample ||
                            currentSample <= m_iLoopStartSample);
        if (outsideLoop) {
            retval = reverse ? m_iLoopEndSample : m_iLoopStartSample;
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
    if (m_bLoopingEnabled) {
        // If we're looping, hint the loop in and loop out, in case we reverse
        // into it. We could save information from process to tell which
        // direction we're going in, but that this is much simpler, and hints
        // aren't that bad to make anyway.
        if (m_iLoopStartSample >= 0) {
            loop_hint.priority = 2;
            loop_hint.sample = m_iLoopStartSample;
            loop_hint.length = 0; // Let it issue the default length
            hintList.append(loop_hint);
        }
        if (m_iLoopEndSample >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = m_iLoopEndSample;
            loop_hint.length = -1; // Let it issue the default (backwards) length
            hintList.append(loop_hint);
        }
    } else {
        if (m_iLoopStartSample >= 0) {
            loop_hint.priority = 10;
            loop_hint.sample = m_iLoopStartSample;
            loop_hint.length = 0; // Let it issue the default length
            hintList.append(loop_hint);
        }
    }
}

void LoopingControl::slotLoopIn(double val) {
    if (val == 1.0f) {
        // set loop in position
        m_iLoopStartSample = m_iCurrentSample;
        m_pCOLoopStartPosition->set(m_iLoopStartSample);

        // Reset the loop out position if it is before the loop in so that loops
        // cannot be inverted.
        if (m_iLoopEndSample != -1 &&
            m_iLoopEndSample < m_iLoopStartSample) {
            m_iLoopEndSample = -1;
            m_pCOLoopEndPosition->set(kNoTrigger);
        }
    }
}

void LoopingControl::slotLoopOut(double val) {
    if (val == 1.0f) {

        // If the user is trying to set a loop-out before the loop in or without
        // having a loop-in, then ignore it.
        if (m_iLoopStartSample == -1 ||
            m_iCurrentSample < m_iLoopStartSample) {
            return;
        }

        //set loop out position and start looping
        m_iLoopEndSample = m_iCurrentSample;
        m_pCOLoopEndPosition->set(m_iLoopEndSample);

        if (m_iLoopStartSample != -1 &&
            m_iLoopEndSample != -1) {
            m_bLoopingEnabled = true;
            m_pCOLoopEnabled->set(1.0f);
        }
        //qDebug() << "set loop_out to " << m_iLoopStartSample;
    }
}

void LoopingControl::slotReloopExit(double val) {
    if (val == 1.0f) {
        // If we're looping, stop looping
        if (m_bLoopingEnabled) {
            m_bLoopingEnabled = false;
            m_pCOLoopEnabled->set(0.0f);
            //qDebug() << "reloop_exit looping off";
        } else {
            // If we're not looping, jump to the loop-in point and start looping
            if (m_iLoopStartSample != -1 && m_iLoopEndSample != -1 &&
                m_iLoopStartSample <= m_iLoopEndSample) {
                m_bLoopingEnabled = true;
                m_pCOLoopEnabled->set(1.0f);
            }
            //qDebug() << "reloop_exit looping on";
        }
    }
}

void LoopingControl::slotLoopStartPos(double pos) {
    int newpos = pos;
    if (newpos >= 0 && !even(newpos)) {
        newpos--;
    }
    if (pos == -1.0f) {
        m_bLoopingEnabled = false;
        m_pCOLoopEnabled->set(0.0f);
    }

    m_iLoopStartSample = newpos;

    if (m_iLoopEndSample != -1 &&
        m_iLoopEndSample < m_iLoopStartSample) {
        m_iLoopEndSample = -1;
        m_pCOLoopEndPosition->set(kNoTrigger);
    }
}

void LoopingControl::slotLoopEndPos(double pos) {
    int newpos = pos;

    if (newpos >= 0 && !even(newpos)) {
        newpos--;
    }

    // Reject if the loop-in is not set, or if this is before the start point.
    if (m_iLoopStartSample == -1 ||
        newpos < m_iLoopStartSample) {
        return;
    }

    if (pos == -1.0f) {
        m_bLoopingEnabled = false;
        m_pCOLoopEnabled->set(0.0f);
    }
    m_iLoopEndSample = newpos;
}
