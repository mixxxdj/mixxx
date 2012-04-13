/*
 * beatmap.cpp
 *
 *  Created on: 08/dic/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QMutexLocker>

#include "track/beatmap.h"
#include "track/beatutils.h"

#define BPM_TOLERANCE 0.6f

static const int kFrameSize = 2;

inline double samplesToFrames(const double samples) {
    return floorf(samples / kFrameSize);
}

inline double framesToSamples(const double frames) {
    return frames * kFrameSize;
}

BeatMap::BeatMap(TrackPointer pTrack, const QByteArray* pByteArray)
        : QObject(),
          m_mutex(QMutex::Recursive) {
    initialize(pTrack);
    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }
}

BeatMap::BeatMap(TrackPointer pTrack, const QVector<double> beats)
        : QObject(),
          m_mutex(QMutex::Recursive) {
    initialize(pTrack);
    if (beats.size() > 0) {
        createFromBeatVector(beats);
    }
}

void BeatMap::initialize(TrackPointer pTrack) {
    m_iSampleRate = pTrack->getSampleRate();
    m_dCachedBpm = 0;
    m_dLastFrame = 0;
}

BeatMap::~BeatMap() {
}

QByteArray* BeatMap::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    // No guarantees BeatLists are made of a data type which located adjacent
    // items in adjacent memory locations.
    double* pBuffer = new double[m_signedBeatList.size()];
    for (int i = 0; i < m_signedBeatList.size(); ++i) {
        // Note that if a beat is at frame 0
        // it is the first one, so it will be always on
        if (m_signedBeatList[i].isOn)
            pBuffer[i] = m_signedBeatList[i].position;
        else
            pBuffer[i] = -m_signedBeatList[i].position;
    }
    QByteArray* pByteArray = new QByteArray((char*) pBuffer, sizeof(pBuffer[0])
            * m_signedBeatList.size());
    delete[] pBuffer;
    return pByteArray;
}

void BeatMap::readByteArray(const QByteArray* pByteArray) {
    if (pByteArray->size() % sizeof(double) != 0) {
        qDebug() << "ERROR: Could not parse BeatMap from QByteArray of size"
                << pByteArray->size();
        return;
    }
    SignedBeat beat;
    int numBeats = pByteArray->size() / sizeof(double);
    const double* pBuffer = (const double*)pByteArray->constData();

    for (int i = 0; i < numBeats; ++i) {
        if (pBuffer[i] < 0) {
            beat.position = -pBuffer[i];
            beat.isOn = false;
        } else {
            // if a beat is at frame 0
            // it will be always on, since it is the first one.
            beat.position = pBuffer[i];
            beat.isOn = true;
        }
        m_signedBeatList.append(beat);
    }
    onBeatlistChanged();
}

void BeatMap::createFromBeatVector(QVector<double> beats) {
    if (beats.isEmpty()) {
       return;
    }
    double previous_beatpos = -1;
    SignedBeat beat;

    foreach (double beatpos, beats) {
        // beatpos is in frames. Do not accept fractional frames.
        beatpos = floorf(beatpos);
        if (beatpos <= previous_beatpos || beatpos < 0) {
            qDebug() << "BeatMap::createFromeVector: beats not in increasing order or negative";
            qDebug() << "discarding beat " << beatpos;
        } else {
            beat.position = beatpos;
            beat.isOn = true;
            m_signedBeatList.append(beat);
            previous_beatpos = beatpos;
        }
    }
    onBeatlistChanged();
}

QString BeatMap::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return BEAT_MAP_VERSION;
}

QString BeatMap::getSubVersion() const {
    QMutexLocker locker(&m_mutex);
    return m_subVersion;
}

void BeatMap::setSubVersion(QString subVersion) {
    m_subVersion = subVersion;
}

bool BeatMap::isValid() const {
    return m_iSampleRate > 0 && m_signedBeatList.size() > 0;
}

double BeatMap::findNextBeat(double dSamples) const {
    return findNthBeat(dSamples, 1);
}

double BeatMap::findPrevBeat(double dSamples) const {
    return findNthBeat(dSamples, -1);
}

double BeatMap::findClosestBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double nextBeat = findNextBeat(dSamples);
    double prevBeat = findPrevBeat(dSamples);
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

double BeatMap::findNthBeat(double dSamples, int n) const {
    QMutexLocker locker(&m_mutex);
    // Reduce sample offset to a frame offset.
    dSamples = samplesToFrames(dSamples);

    SignedBeat Beat;
    Beat.position = dSamples;
    Beat.isOn = true;
    SignedBeatList::const_iterator it;

    if (!isValid() || n == 0) {
        return -1;
    }

    if (n > 0) {
        it = qLowerBound(m_signedBeatList.begin(), m_signedBeatList.end(),
                         Beat);

        // Count down until n=1
        while (it != m_signedBeatList.end()) {
            if (!it->isOn) {
                it++;
                continue;
            }
            if (n == 1) {
                // Return a sample offset
                return framesToSamples(it->position);
            }
            it++;
            n--;
        }
    } else if (n < 0) {
        it = qUpperBound(m_signedBeatList.begin(), m_signedBeatList.end(),
                         Beat);

        // Count up until n=-1
        while (it != m_signedBeatList.begin()) {
            // qUpperBound starts us off at the position just-one-past the last
            // occurence of dSamples-or-smaller in the list. In order to get the
            // last instance of dSamples-or-smaller, we decrement it by 1 before
            // touching it. The guard of this while loop guarantees this does
            // not put us before the start of the loop.
            it--;
            if (!it->isOn) {
                continue;
            }
            if (n == -1) {
                // Return a Sample Offset
                return framesToSamples(it->position);
            }
            n++;
        }
    }
    return -1;
}

void BeatMap::findBeats(double startSample, double stopSample,
                        QList<double>* pBeatsList) const {
    QMutexLocker locker(&m_mutex);
    //startSample and stopSample are sample offsets, converting them to
    //frames
    startSample = samplesToFrames(startSample);
    stopSample = samplesToFrames(stopSample);
    SignedBeat startBeat, stopBeat;
    startBeat.position = startSample;
    startBeat.isOn = true;
    stopBeat.position = stopSample;
    stopBeat.isOn = true;

    if (!isValid() || startSample > stopSample) {
        return;
    }

    SignedBeatList::const_iterator curBeat =
            qLowerBound(m_signedBeatList.begin(), m_signedBeatList.end(),
                        startBeat);
    SignedBeatList::const_iterator lastBeat =
            qUpperBound(m_signedBeatList.begin(), m_signedBeatList.end(),
                        stopBeat);

    for (; curBeat != lastBeat; curBeat++) {
        // BeatGrid::findBeats outputs a frame offset * kFrameSize, i.e. a
        // sample offset here it should be the same:
        if (curBeat->isOn) {
            pBeatsList->append(framesToSamples(curBeat->position));
        }
    }
}

bool BeatMap::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return false;
    }
    double curBeat = findNextBeat(startSample);
    if (curBeat <= stopSample) {
        return true;
    }
    return false;
}

double BeatMap::getBpm() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid())
        return -1;
    return m_dCachedBpm;
}

double BeatMap::getBpmRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid())
        return -1;
    startSample = samplesToFrames(startSample);
    stopSample = samplesToFrames(stopSample);
    SignedBeat startBeat, stopBeat;
    startBeat.position = startSample;
    startBeat.isOn = true;
    stopBeat.position = stopSample;
    stopBeat.isOn = true;
    return calculateBpm(startBeat, stopBeat);
}

void BeatMap::addBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);
    dBeatSample = samplesToFrames(dBeatSample);
    SignedBeat Beat;
    Beat.position = dBeatSample;
    Beat.isOn = true;
    SignedBeatList::iterator it = qLowerBound(m_signedBeatList.begin(),
                                              m_signedBeatList.end(), Beat);
    // Don't insert a duplicate beat. TODO(XXX) determine what epsilon to
    // consider a beat identical to another.
    if (it->position == dBeatSample)
        return;

    m_signedBeatList.insert(it, Beat);
    onBeatlistChanged();
    locker.unlock();
    emit(updated());
}

void BeatMap::removeBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);
    dBeatSample = samplesToFrames(dBeatSample);
    SignedBeat Beat;
    Beat.position = dBeatSample;
    Beat.isOn = true;
    SignedBeatList::iterator it = qLowerBound(m_signedBeatList.begin(),
                                              m_signedBeatList.end(), Beat);
    // In case there are duplicates, remove every instance of dBeatSample
    // TODO(XXX) add invariant checks against this
    // TODO(XXX) determine what epsilon to consider a beat identical to another
    while (it->position == dBeatSample) {
        it = m_signedBeatList.erase(it);
    }
    onBeatlistChanged();
    locker.unlock();
    emit(updated());
}

void BeatMap::moveBeat(double dBeatSample, double dNewBeatSample) {
    QMutexLocker locker(&m_mutex);
    dBeatSample = samplesToFrames(dBeatSample);
    dNewBeatSample = samplesToFrames(dNewBeatSample);
    SignedBeat Beat, NewBeat;
    Beat.position = dBeatSample;
    Beat.isOn = true;
    NewBeat.position = dNewBeatSample;
    NewBeat.isOn = true;
    SignedBeatList::iterator it = qLowerBound(m_signedBeatList.begin(),
                                              m_signedBeatList.end(), Beat);
    // In case there are duplicates, remove every instance of dBeatSample
    // TODO(XXX) add invariant checks against this
    // TODO(XXX) determine what epsilon to consider a beat identical to another
    while (it->position == dBeatSample) {
        NewBeat.isOn = it->isOn;
        it = m_signedBeatList.erase(it);
    }

    // Now add a beat to dNewBeatSample
    it = qLowerBound(m_signedBeatList.begin(), m_signedBeatList.end(), NewBeat);
    // TODO(XXX) beat epsilon
    if (it->position != dNewBeatSample) {
        m_signedBeatList.insert(it, NewBeat);
    }
    onBeatlistChanged();
    locker.unlock();
    emit(updated());
}

void BeatMap::translate(double dNumSamples) {
    QMutexLocker locker(&m_mutex);
    // Converting to frame offset
    dNumSamples = samplesToFrames(dNumSamples);
    if (!isValid()) {
        return;
    }

    for (SignedBeatList::iterator it = m_signedBeatList.begin();
         it != m_signedBeatList.end(); ++it) {
        double newpos = it->position + dNumSamples;
        if (newpos >= 0) {
            it->position = newpos;
        } else {
            m_signedBeatList.erase(it);
        }
    }
    onBeatlistChanged();
    locker.unlock();
    emit(updated());
}

void BeatMap::scale(double dScalePercentage) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    // Scale every beat relative to the first one.
    SignedBeat firstBeat = m_signedBeatList.first();
    for (SignedBeatList::iterator it = m_signedBeatList.begin();
         it != m_signedBeatList.end(); ++it) {
        // Need to not accrue fractional frames.
        it->position = floorf(
            (1 - dScalePercentage) * firstBeat.position +
            dScalePercentage * it->position);
    }
    onBeatlistChanged();
    locker.unlock();
    emit(updated());
}

void BeatMap::setBpm(double dBpm) {
    /*
     * One of the problems of beattracking algorithms is the so called "octave error"
     * that is, calculated bpm is a power-of-two fraction of the bpm of the track.
     * But there is more. In an experiment, it had been proved that roughly 30% of the humans
     * fail to guess the correct bpm of a track by usually reporting it as the double or one
     * half of the correct one.
     * We can interpret it in two ways:
     * On one hand, a beattracking algorithm which totally avoid the octave error does not yet exists.
     * On the other hand, even if the algorithm guesses the correct bpm,
     * 30% of the users will perceive a different bpm and likely change it.
     * In this case, we assume that calculated beat markers are correctly placed. All
     * that we have to do is to delete or add some beat markers, while leaving others
     * so that the number of the beat markers per minute matches the new bpm.
     * We are jealous of our well-guessed beats since they belong to a time-expensive analysis.
     * When requested we simply turn them off instead of deleting them, so that they can be recollected.
     * If the new provided bpm is not a power-of-two fraction, we assume that the algorithm failed
     * at all to guess the bpm. I have no idea on how to deal with this.
     * If we assume that bpm does not change along the track, i.e. if we use
     * fixed tempo approximation (see analyserbeat.*), this should coincide with the
     * method in beatgrid.cpp.
     *
     * - vittorio.
     */
    QMutexLocker locker(&m_mutex);

    if (!isValid())
        return;

    // This problem is so complicated that for now we are just going to bail and
    // scale the beatgrid exactly by the ratio indicated by the desired
    // BPM. This is a downside of using a BeatMap over a BeatGrid. rryan 4/2012
    double currentBpm = calculateBpm(m_signedBeatList.first(),
                                     m_signedBeatList.last());
    double ratio = dBpm / currentBpm;
    locker.unlock();
    scale(ratio);
}

void BeatMap::onBeatlistChanged() {
    if (!isValid()) {
        m_dLastFrame = 0;
        m_dCachedBpm = 0;
        return;
    }
    m_dLastFrame = m_signedBeatList.last().position;
    SignedBeat startBeat = m_signedBeatList.first();
    SignedBeat stopBeat =  m_signedBeatList.last();
    m_dCachedBpm = calculateBpm(startBeat, stopBeat);
}

double BeatMap::calculateBpm(SignedBeat startBeat, SignedBeat stopBeat) const {
    double startSample = startBeat.position;
    double stopSample = stopBeat.position;

    if (startSample > stopSample) {
        return -1;
    }

    SignedBeatList::const_iterator curBeat =
            qLowerBound(m_signedBeatList.begin(), m_signedBeatList.end(),
                        startBeat);
    SignedBeatList::const_iterator lastBeat =
            qUpperBound(m_signedBeatList.begin(), m_signedBeatList.end(),
                        stopBeat);

    QVector<double> beatvect;
    for (; curBeat != lastBeat; curBeat++) {
        SignedBeat beat = *curBeat;
        if (beat.isOn)
            beatvect.append(beat.position);
    }

    if (beatvect.size() == 0)
        return -1;

    // If we have less than 8 samples, just divide the # of beats by the
    // duration in minutes.
    if (beatvect.size() < 8) {
        double rangeDurationMinutes =
                (stopSample - startSample) / ( 60.0f * m_iSampleRate);
        // Subtracting returns the number of beats between the samples referred to
        // by the start and end.
        double beatsInRange = beatvect.size();
        return beatsInRange / rangeDurationMinutes;
    }

    // Statistical approach works better if we have more than 8 samples:
    return BeatUtils::calculateBpm(beatvect, m_iSampleRate, 0, 9999);
}

