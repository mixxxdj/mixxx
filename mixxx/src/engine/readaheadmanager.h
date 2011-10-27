// readaheadmanager.h
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#ifndef READAHEADMANGER_H
#define READAHEADMANGER_H

#include <QList>
#include <QMutex>
#include <QPair>

#include "defs.h"

struct Hint;
class EngineControl;
class CachingReader;

// ReadAheadManager is a tool for keeping track of the engine's current position
// in a file. In the case that the engine needs to read ahead of the current
// play position (for example, to feed more samples into a library like
// SoundTouch) then this will keep track of how many samples the engine has
// consumed. The getNextSamples() method encapsulates the logic of determining
// whether to take a loop or jump into a single method. Whenever the Engine
// seeks or the current play position is invalidated somehow, the Engine must
// call notifySeek to inform the ReadAheadManager to reset itself to the seek
// point.
class ReadAheadManager {
  public:
    explicit ReadAheadManager(CachingReader* reader);
    virtual ~ReadAheadManager();

    // Call this method to fill buffer with requested_samples out of the
    // lookahead buffer. Provide rate as dRate so that the manager knows the
    // direction the audio is progressing in. Returns the total number of
    // samples read into buffer. Note that it is very common that the total
    // samples read is less than the requested number of samples.
    virtual int getNextSamples(double dRate, CSAMPLE* buffer, int requested_samples);

    // Used to add a new EngineControl that ReadAheadManager will use to decide
    // which samples to return.
    virtual void addEngineControl(EngineControl* control);

    // Notify the ReadAheadManager that the current playposition has
    // changed. Units are stereo samples.
    virtual void setNewPlaypos(int iNewPlaypos);

    // Get the current read-ahead position in stereo samples.
    virtual inline int getPlaypos() const {
        return m_iCurrentPosition;
    }

    virtual void notifySeek(int iSeekPosition);

    // hintReader allows the ReadAheadManager to provide hints to the reader to
    // indicate that the given portion of a song is about to be read.
    virtual void hintReader(double dRate, QList<Hint>& hintList,
                            int iSamplesPerBuffer);

  private:
    QList<EngineControl*> m_sEngineControls;
    int m_iCurrentPosition;
    CachingReader* m_pReader;
};

#endif // READAHEADMANGER_H
