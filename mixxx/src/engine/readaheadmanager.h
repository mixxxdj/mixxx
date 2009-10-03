// readaheadmanager.h
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#ifndef __READAHEADMANGER_H__
#define __READAHEADMANGER_H__

#include <QList>
#include <QMutex>
#include <QPair>

#include "defs.h"

class EngineControl;
class CachingReader;

class ReadAheadManager {
public:
    explicit ReadAheadManager(CachingReader* reader);
    virtual ~ReadAheadManager();
    
    // Call this method to fill buffer with requested_samples out of the
    // lookahead buffer. Provide rate as dRate so that the manager knows the
    // direction the audio is progressing in. Returns the total number of
    // samples read into buffer. Note that it is very common that the total
    // samples read is less than the requested number of samples.
    int getNextSamples(double dRate, CSAMPLE* buffer, int requested_samples);

    // Used to add a new EngineControl that ReadAheadManager will use to decide
    // which samples to return.
    void addEngineControl(EngineControl* control);

    // Notify the ReadAheadManager that the current playposition has changed
    void setNewPlaypos(int iNewPlaypos);

    void notifySeek(int iSeekPosition);
private:
    QPair<int, double> getSoonestTrigger(double dRate, int iCurrentSample);
    
    QList<EngineControl*> m_sEngineControls;
    int m_iCurrentPosition;
    CachingReader* m_pReader;
};

#endif // __READAHEADMANGER_H__
