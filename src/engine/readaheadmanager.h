// readaheadmanager.h
// Created 8/2/2009 by RJ Ryan (rryan@mit.edu)

#ifndef READAHEADMANGER_H
#define READAHEADMANGER_H

#include <QLinkedList>
#include <QList>
#include <QPair>

#include "util/types.h"
#include "util/math.h"
#include "cachingreader.h"

class LoopingControl;
class RateControl;
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
    explicit ReadAheadManager(); // Only for testing: ReadAheadManagerMock
    explicit ReadAheadManager(CachingReader* reader,
                              LoopingControl* pLoopingControl);
    virtual ~ReadAheadManager();

    // Call this method to fill buffer with requested_samples out of the
    // lookahead buffer. Provide rate as dRate so that the manager knows the
    // direction the audio is progressing in. Returns the total number of
    // samples read into buffer. Note that it is very common that the total
    // samples read is less than the requested number of samples.
    virtual int getNextSamples(double dRate, CSAMPLE* buffer,
                               int requested_samples);


    // Used to add a new EngineControls that ReadAheadManager will use to decide
    // which samples to return.
    void addLoopingControl();
    void addRateControl(RateControl* pRateControl);

    // Get the current read-ahead position in samples.
    virtual inline int getPlaypos() const {
        return m_iCurrentPosition;
    }

    virtual void notifySeek(int iSeekPosition);

    // hintReader allows the ReadAheadManager to provide hints to the reader to
    // indicate that the given portion of a song is about to be read.
    virtual void hintReader(double dRate, HintVector* hintList);

    virtual int getEffectiveVirtualPlaypositionFromLog(double
            currentVirtualPlayposition,
            double numConsumedSamples);

    virtual void setReader(CachingReader* pReader) {
        m_pReader = pReader;
    }

  private:
    // An entry in the read log indicates the virtual playposition the read
    // began at and the virtual playposition it ended at.
    struct ReadLogEntry {
        double virtualPlaypositionStart;
        double virtualPlaypositionEndNonInclusive;

        ReadLogEntry(double virtualPlaypositionStart,
                     double virtualPlaypositionEndNonInclusive) {
            this->virtualPlaypositionStart = virtualPlaypositionStart;
            this->virtualPlaypositionEndNonInclusive =
                virtualPlaypositionEndNonInclusive;
        }

        bool direction() const {
            // NOTE(rryan): We try to avoid 0-length ReadLogEntry's when
            // possible but they have happened in the past. We treat 0-length
            // ReadLogEntry's as forward reads because this prevents them from
            // being interpreted as a seek in the common case.
            return virtualPlaypositionStart <= virtualPlaypositionEndNonInclusive;
        }

        double length() const {
            return fabs(virtualPlaypositionEndNonInclusive -
                        virtualPlaypositionStart);
        }

        // Moves the start position forward or backward (depending on
        // direction()) by numSamples. Returns the total number of samples
        // consumed. Caller should check if length() is 0 after consumption in
        // order to expire the ReadLogEntry.
        double consume(double numSamples) {
            double available = math_min(numSamples, length());
            virtualPlaypositionStart += (direction() ? 1 : -1) * available;
            return available;
        }

        bool merge(const ReadLogEntry& other) {
            // Allow 0-length ReadLogEntry's to merge regardless of their
            // direction if they have the right start point.
            if ((other.length() == 0 || direction() == other.direction()) &&
                    virtualPlaypositionEndNonInclusive == other.virtualPlaypositionStart) {
                virtualPlaypositionEndNonInclusive =
                    other.virtualPlaypositionEndNonInclusive;
                return true;
            }
            return false;
        }
    };

    // virtualPlaypositionEnd is the first sample in the direction that was read
    // that was NOT read as part of this log entry. This is to simplify the
    void addReadLogEntry(double virtualPlaypositionStart,
                         double virtualPlaypositionEndNonInclusive);

    LoopingControl* m_pLoopingControl;
    RateControl* m_pRateControl;
    QLinkedList<ReadLogEntry> m_readAheadLog;
    int m_iCurrentPosition;
    CachingReader* m_pReader;
    CSAMPLE* m_pCrossFadeBuffer;
};

#endif // READAHEADMANGER_H
