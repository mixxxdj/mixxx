//
// C++ Implementation: %{MODULE}
//
// Description:
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "enginerealsearch.h"
#include "enginebuffer.h"
#include "configobject.h"
#include "controlobject.h"

EngineRealSearch::EngineRealSearch(const char *group, EngineBuffer *pBuffer) : EngineObject()
{
    m_pBuffer = pBuffer;
    
    // Search rate. Rate used when searching in sound.
    m_pRateSearch = ControlObject::getControl(ConfigKey(group, "rateSearch")); 
    
    // Control if RealSearch is enabled
    m_pRealSearch = ControlObject::getControl(ConfigKey(group, "realSearch")); 
}

EngineRealSearch::~EngineRealSearch()
{
}

void EngineRealSearch::process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize)
{
    if (pIn!=pOut)
    {    
        CSAMPLE *pOutput = (CSAMPLE *)pOut;
        memcpy(pOutput, pIn, sizeof(CSAMPLE) * iBufferSize);
    }
    
    // If real search is enabled and a search is being performed...
    if (m_pRealSearch->get()==1. && m_pRateSearch->get()!=0.)
    {
        // Update virtual file position
        
        // Seek if necessary
    }
}

