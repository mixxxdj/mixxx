#include <QtDebug>
#include <QList>
#include <QSharedPointer>
#include "trackinfoobject.h"


#ifndef __TRACK_BEATS_H__
#define __TRACK_BEATS_H__

class TrackBeats;
typedef QSharedPointer<TrackBeats> TrackBeatsPointer;

class TrackBeats {
public:
    TrackBeats(TrackPointer);
    virtual ~TrackBeats();
    
    void addBeatSeconds(double);
    
    double findNextBeatSeconds(double) const;
    double findPrevBeatSeconds(double) const;
    QList<double>* findBeatsSeconds(double, double) const;
    
    int getBeatCount() const;
    void dumpBeats();
    
    void addBeatSample(int);
    int findPrevBeatSample(int) const;
    int findNextBeatSample(int) const;
    int findBeatOffsetSamples(int sample, int offset) const;
    bool hasBeatsSamples(double bgnRange, double endRange) const;
    QList<int>* findBeatsSamples(int, int) const;
    QByteArray *serializeToBlob();
    void unserializeFromBlob(QByteArray *blob);
    
private:
    /** Find the Samples Index */
    int sampleIndex(int) const;
    
    /** Sample Rate for this song */
    int m_iSampleRate;
    /** Duration of the entire song in seconds */
    double m_dDuration;
    /** 10 second index (in samples) */
    QList<int> m_beatIndex;
    /** Map of all the beats in samples */
    QMap<int, int> m_beats;
    /** Pointer to the related Track */
    TrackPointer m_track;
    /** Mutex protecting access to object */
    mutable QMutex m_qMutex;
};

#endif
