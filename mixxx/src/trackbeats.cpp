#include <QtDebug>

#include "defs.h"
#include "trackbeats.h"
#include "trackinfoobject.h"


TrackBeats::TrackBeats(TrackPointer tio) : m_qMutex(QMutex::Recursive)
{
    m_iSampleRate = tio->getSampleRate();
}

TrackBeats::~TrackBeats()
{
}

void TrackBeats::addBeatSeconds(double beat)
{
    QMutexLocker lock(&m_qMutex);
    addBeatSample((int)round(beat * m_iSampleRate));
}

bool TrackBeats::hasBeatsSamples(double bgnRange, double endRange) const
{
    QMutexLocker lock(&m_qMutex);
    QMapIterator<int, int> iter(m_beats);
    int index = sampleIndex(start);


    if (iter.findNext(m_beatIndex.value(index)))
    {
        do {
            if ((iter.value() >= start) && (iter.value() <= stop))
                 return true;
            
            iter.next();
        } while((iter.hasNext()) && (iter.value() <= stop));
    }
    
    return false;
}
    
double TrackBeats::findNextBeatSeconds(double beat) const
{
    QMutexLocker lock(&m_qMutex);
    int sample = (int) round(beat * m_iSampleRate);
    return findNextBeatSample(sample) / m_iSampleRate;
}

double TrackBeats::findPrevBeatSeconds(double beat) const
{
    QMutexLocker lock(&m_qMutex);
    int sample = (int) round(beat * m_iSampleRate);
    return findPrevBeatSample(sample) / m_iSampleRate;
}

QList<double> TrackBeats::findBeatsSeconds(double start, double stop) const
{
    QMutexLocker lock(&m_qMutex);
    QList<double> ret;
    QList<int> samples;
    int begin = round(start * m_iSampleRate);
    int end = round(stop * m_iSampleRate);
    int i;
    
    
    samples = findBeatsSamples(begin, end);
    for (i = 0; i < samples.size(); i++)
    {
        //qDebug() << "Sample:" << samples.at(i) << "SampleRate:" << m_iSampleRate
        //            << "Seconds:" << ((double)samples.at(i) / (double)m_iSampleRate);
        
        ret.append((double)samples.at(i) / (double)m_iSampleRate);
    }
    
    return ret;
}

int TrackBeats::getBeatCount() const
{
    QMutexLocker lock(&m_qMutex);
    return m_beats.size();
}

void TrackBeats::dumpBeats()
{
    QMutexLocker lock(&m_qMutex);
    QMapIterator<int, int> iter(m_beats);
    
    do {
        iter.next();
        qDebug() << "TrackBeat Sample:" << iter.value();
        
    } while(iter.hasNext());
}

int TrackBeats::sampleIndex(int sample) const
{
    QMutexLocker lock(&m_qMutex);
    return (int) round(sample / (m_iSampleRate * 10));
}

void TrackBeats::addBeatSample(int sample)
{
    QMutexLocker lock(&m_qMutex);
    int index = sampleIndex(sample);
    
    
    if ((m_beatIndex.size()-1) < index ) 
    {
        int i;
        
        
        for (i = m_beatIndex.size() - 1; i < index; i++ )
            m_beatIndex.append(sample);
        
        m_beatIndex.append(sample);
    }
    
    m_beats[sample] = sample;
}

int TrackBeats::findNextBeatSample(int sample) const
{
    QMutexLocker lock(&m_qMutex);
    QMapIterator<int, int> iter(m_beats);
    int index = sampleIndex(sample);
    
    
    if (m_beatIndex.size() > index)
    {
        // make sure we dont trip on unindex'ed areas (-1)..
        iter.findNext(m_beatIndex.value(index));
        do {
            iter.next();
        } while((iter.hasNext()) && (iter.value() <= sample));
        
        return iter.value();
    }
    
    return -1;
}

int TrackBeats::findBeatOffsetSamples(int sample, int offset) const
{
    QMutexLocker lock(&m_qMutex);
    QMapIterator<int, int> iter(m_beats);
    int index = sampleIndex(sample);
    int i;
    
    
    if (m_beatIndex.size() < index)
        return -1;
    
    
    iter.findNext(m_beatIndex.value(index));
    do {
        iter.next();
    } while((iter.hasNext()) && (iter.value() <= sample));
    
    // Backup one just to be before the marker
    if ((iter.hasPrevious()) && (iter.value() > sample))
        iter.previous();
    
    // Find the offset from the current beat
    if ( offset > 0 )
    {
        for (i = 0; i < offset && iter.hasNext(); i++)
            iter.next();
    }
    else if ( offset < 0 )
    {
        for (i = offset * -1; i > 0 && iter.hasPrevious(); i--)
            iter.previous();
    }
    
    return iter.value();
}

int TrackBeats::findPrevBeatSample(int sample) const
{
    QMutexLocker lock(&m_qMutex);
    QMapIterator<int, int> iter(m_beats);
    int index = sampleIndex(sample);
    
    if (m_beatIndex.size() > index)
    {
        iter.findNext(m_beatIndex.value(index));
        do {
            iter.previous();
        } while((iter.hasPrevious()) && (iter.value() >= sample));
        
        return iter.value();
    }
    
    return -1;
}

QList<int> TrackBeats::findBeatsSamples(int start, int stop) const
{
    QMutexLocker lock(&m_qMutex);
    QList<int> ret;
    QMapIterator<int, int> iter(m_beats);
    int index = sampleIndex(start);
    
    
    if (iter.findNext(m_beatIndex.value(index)))
    {
        do {
            if ((iter.value() >= start) && (iter.value() <= stop))
                 ret.append(iter.value());
            
            iter.next();
        } while((iter.hasNext()) && (iter.value() <= stop));
    }
    
    return ret;
}

QByteArray *TrackBeats::serializeToBlob()
{
    QMutexLocker lock(&m_qMutex);
    QByteArray *blob;
    int *buffer = new int[getBeatCount()];
    int *ptr = buffer;
    QMapIterator<int, int> iter(m_beats);
    
    
    iter.next();
    
    while ( iter.hasNext())
    {
        *ptr++ = iter.value();
        iter.next();
    }
    
    blob = new QByteArray((char *)buffer, getBeatCount() * sizeof(int));
    delete []buffer;

    return blob;
}

void TrackBeats::unserializeFromBlob(QByteArray *blob)
{
    QMutexLocker lock(&m_qMutex);
    int *ptr = (int *)blob->constData();
    int i;
    
    
    for (i = blob->size() / sizeof(int); --i; ptr++)
    {
        addBeatSample(*ptr);
    }
}

