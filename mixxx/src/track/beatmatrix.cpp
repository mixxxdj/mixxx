#include <QtDebug>
#include <QMutexLocker>

#include "track/beatmatrix.h"
#include "beattools.h"

BeatMatrix::BeatMatrix(TrackPointer pTrack, const QByteArray* pByteArray)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iSampleRate(pTrack->getSampleRate()) {
    connect(pTrack.data(), SIGNAL(bpmUpdated(double)),
                this, SLOT(slotTrackBpmUpdated(double)),
                Qt::DirectConnection);
        slotTrackBpmUpdated(pTrack->getBpm());

    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }
}

BeatMatrix::~BeatMatrix() {
}

unsigned int BeatMatrix::numBeats() const {
    return m_beatList.size();
}

QByteArray* BeatMatrix::toByteArray() const {
    QMutexLocker locker(&m_mutex);
    // No guarantees BeatLists are made of a data type which located adjacent
    // items in adjacent memory locations.
    double* pBuffer = new double[m_beatList.size()];
    for (int i = 0; i < m_beatList.size(); ++i) {
        pBuffer[i] = m_beatList[i];
    }
    QByteArray* pByteArray = new QByteArray((char*)pBuffer, sizeof(pBuffer[0]) * m_beatList.size());
    delete [] pBuffer;
    return pByteArray;
}

void BeatMatrix::readByteArray(const QByteArray* pByteArray) {
    if (pByteArray->size() % sizeof(double) != 0) {
        qDebug() << "ERROR: Could not parse BeatMatrix from QByteArray of size" << pByteArray->size();
        return;
    }

    int numBeats = pByteArray->size() / sizeof(double);
    double* pBuffer = (double*)pByteArray->data();
    for (int i = 0; i < numBeats; ++i) {
        m_beatList.append(pBuffer[i]);
    }
}

void BeatMatrix::createFromVector(QVector <double> beats){
    QMutexLocker locker(&m_mutex);
    double previous_beat = -1;
    QVectorIterator<double> i(beats);
    while (i.hasNext()){
        double beat = i.next();
        if(beat<=previous_beat){
            qDebug()<<"BeatMatrix::createFromeVector: beats not in increasing order";
            locker.unlock();
            return;
        }
        else{
            m_beatList.append(beat);
            previous_beat = beat;
        }
    }
    locker.unlock();
    emit(updated());
}

QString BeatMatrix::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return "BeatMatrix-1.0";
}

// internal use only
bool BeatMatrix::isValid() const {
    return m_iSampleRate > 0 && m_beatList.size() > 0;
}

double BeatMatrix::findNextBeat(double dSamples) const {
    return findNthBeat(dSamples, 1);
}

double BeatMatrix::findPrevBeat(double dSamples) const {
    return findNthBeat(dSamples, -1);
}

double BeatMatrix::findClosestBeat(double dSamples) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }
    double nextBeat = findNextBeat(dSamples);
    double prevBeat = findPrevBeat(dSamples);
    return (nextBeat - dSamples > dSamples - prevBeat) ? prevBeat : nextBeat;
}

double BeatMatrix::findNthBeat(double dSamples, int n) const {
    QMutexLocker locker(&m_mutex);
    // Reduce the Sample Offset to a frame offset.
    dSamples = floorf(dSamples/2);

    BeatList::const_iterator it;
    int i;

    if (!isValid() || n == 0) {
        return -1;
    }

    if (n > 0) {
        it = qLowerBound(m_beatList.begin(), m_beatList.end(), dSamples);

        // Count down until n=1
        while (it != m_beatList.end()) {
            if (n == 1) {
                // Return a Sample Offset
                return (*it * 2);
            }
            it++; n--;
        }
    }
    else if (n < 0) {
        it = qUpperBound(m_beatList.begin(), m_beatList.end(), dSamples);

        // Count up until n=-1
        while (it != m_beatList.begin()) {
            // qUpperBound starts us off at the position just-one-past the last
            // occurence of dSamples-or-smaller in the list. In order to get the
            // last instance of dSamples-or-smaller, we decrement it by 1 before
            // touching it. The guard of this while loop guarantees this does
            // not put us before the start of the loop.
            it--;
            if (n == -1) {
                // Return a Sample Offset
                return ( *it * 2);
            }
            n++;
        }
    }

    return -1;
}

void BeatMatrix::findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const {
    QMutexLocker locker(&m_mutex);
    //startSample and stopSample are sample offsets:
    startSample = floorf(startSample/2);
    stopSample = floorf(stopSample/2);
    if (!isValid() || startSample > stopSample) {
       return;
    }
    BeatList::const_iterator curBeat = qLowerBound(m_beatList.begin(),
                                                   m_beatList.end(),
                                                   startSample);
    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
                                                    m_beatList.end(),
                                                    stopSample);

    for (; curBeat != stopBeat; curBeat++) {
        //BeatGrid::findBeats outputs a frame offset * kFrameSize, i.e. a sample offset
        //here it should be the same:
        pBeatsList->append(*curBeat * 2);
    }
}

bool BeatMatrix::hasBeatInRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return false;
    }
    BeatList::const_iterator startBeat = qLowerBound(m_beatList.begin(),
                                                     m_beatList.end(),
                                                     startSample);
    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
                                                    m_beatList.end(),
                                                    stopSample);

    if (startBeat != stopBeat)
        return true;
    return false;
}

double BeatMatrix::getBpm() const {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return -1;
    }

    // TODO(XXX) not actually correct. We need the true song length.
    //    double startSample = *m_beatList.begin();
    //    double stopSample = *(m_beatList.end()-1);
    //    double songDurationMinutes =
    //            (stopSample - startSample) / (60.0f * m_iSampleRate);
    //    return m_beatList.size() / songDurationMinutes;
    //
#ifdef __VAMP__
    //    statistical approach from Tobias Rafreider should work here too.
    return BeatTools::calculateBpm(m_beatList.toVector(),m_iSampleRate,0,9999);

#else

    double startSample = *m_beatList.begin();
    double stopSample = *(m_beatList.end()-1);
    double songDurationMinutes =
            (stopSample - startSample) / (60.0f * m_iSampleRate);
    return m_beatList.size() / songDurationMinutes;

#endif
}

double BeatMatrix::getBpmRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return -1;
    }


    BeatList::const_iterator startBeat = qLowerBound(m_beatList.begin(),
                                                     m_beatList.end(),
                                                     startSample);
    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
                                                    m_beatList.end(),
                                                    stopSample);
#ifdef __VAMP__
    QVector<double> beatvect;
    for (; startBeat != stopBeat; startBeat++) {
        beatvect.append(*startBeat);
    }
    // Statistical approach works better if we have more than 8 samples:
    if (beatvect.size()<8){
        double rangeDurationMinutes =
                (stopSample - startSample) / ( 60.0f * m_iSampleRate) ;
        // Subtracting returns the number of beats between the samples referred to
        // by the start and end.
        double beatsInRange = stopBeat - startBeat;

        return beatsInRange / rangeDurationMinutes;
    }

    return BeatTools::calculateBpm(beatvect,m_iSampleRate,0,9999);
#else
    double rangeDurationMinutes =
                   (stopSample - startSample) / ( 60.0f * m_iSampleRate) ;
           // Subtracting returns the number of beats between the samples referred to
           // by the start and end.
           double beatsInRange = stopBeat - startBeat;

           return beatsInRange / rangeDurationMinutes;
#endif

}

void BeatMatrix::addBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);

    BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                        m_beatList.end(),
                                        dBeatSample);
    // Don't insert a duplicate beat. TODO(XXX) determine what epsilon to
    // consider a beat identical to another.
    if (*it == dBeatSample)
        return;

    m_beatList.insert(it, dBeatSample);
    locker.unlock();
    emit(updated());
}

void BeatMatrix::removeBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);
    BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                        m_beatList.end(), dBeatSample);

    // In case there are duplicates, remove every instance of dBeatSample
    // TODO(XXX) add invariant checks against this
    // TODO(XXX) determine what epsilon to consider a beat identical to another
    while (*it == dBeatSample) {
        it = m_beatList.erase(it);
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::moveBeat(double dBeatSample, double dNewBeatSample) {
    QMutexLocker locker(&m_mutex);

    BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                        m_beatList.end(), dBeatSample);

    // Remove all beats from dBeatSample
    while (*it == dBeatSample) {
        it = m_beatList.erase(it);
    }

    // Now add a beat to dNewBeatSample
    it = qLowerBound(m_beatList.begin(),
                     m_beatList.end(), dNewBeatSample);

    // TODO(XXX) beat epsilon
    if (*it != dNewBeatSample) {
        m_beatList.insert(it, dNewBeatSample);
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::translate(double dNumSamples) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }

    for (BeatList::iterator it = m_beatList.begin();
         it != m_beatList.end(); ++it) {
        *it += dNumSamples;
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::scale(double dScalePercentage) {
    QMutexLocker locker(&m_mutex);
    if (!isValid()) {
        return;
    }
    for (BeatList::iterator it = m_beatList.begin();
         it != m_beatList.end(); ++it) {
        *it *= dScalePercentage;
    }
    locker.unlock();
    emit(updated());
}

void BeatMatrix::slotTrackBpmUpdated(double dBpm) {

    // TODO(XXX) How do we handle this?
    /*
     * Idea: If we assume that beattracking is or totally precise or totally wrong,
     * then the main problem is that the same track can be perceived as played at 2*bpm or as played at bpm/2,
     * even if the result from the beattracker is totally precise.
     * The idea is:
     * if the requested bpm variation is "little", we may interpret it as an error from the beattracking
     * algorithm, that is we have to scale the beatmatrix by calling BeatMatrix::scale.
     * In this case we cannot even try to identify the first beat (it is probably wrong too)
     * and the user should try to manually realign the grid.
     * If the requested bpm is the double of the original, we should put a beat in the middle
     * of every two beats.
     * If the requested bpm is half the original one, we should delete every beat
     * which is in a multiple-of-two position (here we assume that the first beat is correct).
     * We can combine this steps to handle any bpm.
     * Note that a 4/4 tempo can be perceived as a 6/4 one too. This is a rare condition.
     *
     * If we assume that bpm does not change along the track, i.e. if we use
     * fixed tempo approximation (see analyserbeat.*), this should coincide with the
     * method in beatgrid.cpp.
     * For a variable bpm track, that is without fixed tempo approximation,
     * we "lose" the correct positioning if changing and reverting bpms.
     * Does not work when tapping via the skin. Is it implemented?
     *
     * Here it follows a proof of concept.
     * - vittorio.
     */

    double dtrack_Bpm = getBpm();
    if (dtrack_Bpm == 0 || dBpm == 0 || !isValid() || fabs(dtrack_Bpm - dBpm)
            < 0.5)
        return;
    //QMutexLocker locker(&m_mutex);
    QList<double> temp_beatList;

    while (dBpm >= (dtrack_Bpm * 2) - 0.5) {
        temp_beatList = m_beatList;
        for (int i = 0; i < temp_beatList.size() - 2; i++) {
            addBeat(floor(temp_beatList.at(i) + temp_beatList.at(i + 1)) / 2);
        }
        dtrack_Bpm *= 2;
    }

    while (dBpm <= (dtrack_Bpm / 2) + 0.5) {
        temp_beatList = m_beatList;
        for (int i = 1; i < temp_beatList.size() - 2; i += 2) {
            removeBeat(temp_beatList.at(i));
        }
        dtrack_Bpm /= 2;
    }

    scale(dtrack_Bpm / dBpm);
    dtrack_Bpm = dBpm;


//    locker.unlock();
//    emit(updated());

}
