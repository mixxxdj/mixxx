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

EngineClipping::EngineClipping(const char* group) {
}

EngineClipping::~EngineClipping() {
}

void EngineClipping::process(const CSAMPLE* pIn, CSAMPLE* pOutput, const int iBufferSize) {
    const CSAMPLE kfMaxAmp = 1.0;

    // SampleUtil clamps the buffer and if pIn and pOut are aliased will not copy.
    SampleUtil::copyClampBuffer(kfMaxAmp, -kfMaxAmp,
                                          pOutput, pIn, iBufferSize);
}
