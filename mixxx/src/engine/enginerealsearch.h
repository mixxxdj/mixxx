//
// C++ Interface: %{MODULE}
//
// Description: 
//
//
// Author: %{AUTHOR} <%{EMAIL}>, (C) %{YEAR}
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ENGINEREALSEARCH_H
#define ENGINEREALSEARCH_H

#include <engineobject.h>

class EngineBuffer;
class ControlObject;

/**
@author Tue Haste Andersen
*/
class EngineRealSearch : public EngineObject
{
public:
    EngineRealSearch(const char *group, EngineBuffer *pBuffer);
    ~EngineRealSearch();
    
    void process(const CSAMPLE *pIn, const CSAMPLE *pOut, const int iBufferSize);

private:
    EngineBuffer *m_pBuffer;
    ControlObject *m_pRateSearch, *m_pRealSearch;
};

#endif
