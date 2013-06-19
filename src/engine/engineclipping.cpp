/***************************************************************************
                          engineclipping.cpp  -  description
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

#include "engineclipping.h"
#include "controlpotmeter.h"
#include "sampleutil.h"

EngineClipping::EngineClipping(const char * group)
{
    //Used controlpotmeter as the example used it :/ perhaps someone with more knowledge could use something more suitable...
    m_ctrlClipping = new ControlPotmeter(ConfigKey(group, "PeakIndicator"), 0., 1.);
    m_ctrlClipping->set(0);
    m_duration = 0;
}

EngineClipping::~EngineClipping()
{
    delete m_ctrlClipping;
}

void EngineClipping::process(const CSAMPLE * pIn, const CSAMPLE * pOut, const int iBufferSize)
{
    static const FLOAT_TYPE kfMaxAmp = 32767.;
    // static const FLOAT_TYPE kfClip = 0.8*kfMaxAmp;

    CSAMPLE * pOutput = (CSAMPLE *)pOut;
    // SampleUtil clamps the buffer and if pIn and pOut are aliased will not copy.
    clipped = SampleUtil::copyClampBuffer(kfMaxAmp, -kfMaxAmp,
                                          pOutput, pIn, iBufferSize);

    if (clipped) {
        if (m_ctrlClipping->get() != 1.) {
            m_ctrlClipping->set(1.);
        }
        m_duration = 20;
    }

    if (m_duration == 0) {
        if (m_ctrlClipping->get() == 1.) {
            m_ctrlClipping->set(0.);
        }
    }
    else {
        m_duration--;
    }


}

//returns true if the last buffer processed clipped
bool EngineClipping::hasClipped()
{
    return clipped;
}
