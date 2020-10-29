/***************************************************************************
                          enginedelay.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                :
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "enginedelay.h"

#include "control/controlproxy.h"
#include "control/controlpotmeter.h"
#include "engine/engine.h"
#include "util/assert.h"
#include "util/sample.h"

namespace {
constexpr double kdMaxDelayPot = 500;
const int kiMaxDelay = static_cast<int>((kdMaxDelayPot + 8) / 1000 *
        mixxx::audio::SampleRate::kValueMax * mixxx::kEngineChannelCount);
} // anonymous namespace

EngineDelay::EngineDelay(const QString& group, ConfigKey delayControl, bool bPersist)
        : m_iDelayPos(0),
          m_iDelay(0) {
    m_pDelayBuffer = SampleUtil::alloc(kiMaxDelay);
    SampleUtil::clear(m_pDelayBuffer, kiMaxDelay);
    m_pDelayPot = new ControlPotmeter(delayControl, 0, kdMaxDelayPot, false, true, false, bPersist);
    m_pDelayPot->setDefaultValue(0);
    connect(m_pDelayPot, &ControlObject::valueChanged, this,
            &EngineDelay::slotDelayChanged, Qt::DirectConnection);

    m_pSampleRate = new ControlProxy(group, "samplerate", this);
    m_pSampleRate->connectValueChanged(this, &EngineDelay::slotDelayChanged, Qt::DirectConnection);
}

EngineDelay::~EngineDelay() {
    SampleUtil::free(m_pDelayBuffer);
    delete m_pDelayPot;
}

void EngineDelay::slotDelayChanged() {
    double newDelay = m_pDelayPot->get();
    double sampleRate = m_pSampleRate->get();

    m_iDelay = (int)(sampleRate * newDelay / 1000);
    m_iDelay *= 2;
    if (m_iDelay > (kiMaxDelay - 2)) {
        m_iDelay = (kiMaxDelay - 2);
    }
    if (m_iDelay <= 0) {
        // We start bypassing, so clear buffer, to avoid noise in case of re-enable delay
        SampleUtil::clear(m_pDelayBuffer, kiMaxDelay);
    }
}


void EngineDelay::process(CSAMPLE* pInOut, const int iBufferSize) {
    if (m_iDelay > 0) {
        int iDelaySourcePos = (m_iDelayPos + kiMaxDelay - m_iDelay) % kiMaxDelay;

        VERIFY_OR_DEBUG_ASSERT(iDelaySourcePos >= 0) {
            return;
        }
        VERIFY_OR_DEBUG_ASSERT(iDelaySourcePos <= kiMaxDelay) {
            return;
        }

        for (int i = 0; i < iBufferSize; ++i) {
            // put sample into delay buffer:
            m_pDelayBuffer[m_iDelayPos] = pInOut[i];
            m_iDelayPos = (m_iDelayPos + 1) % kiMaxDelay;

            // Take delayed sample from delay buffer and copy it to dest buffer:
            pInOut[i] = m_pDelayBuffer[iDelaySourcePos];
            iDelaySourcePos = (iDelaySourcePos + 1) % kiMaxDelay;
        }
    }
}

void EngineDelay::setDelay(double newDelay) {
    m_pDelayPot->set(newDelay);
}
