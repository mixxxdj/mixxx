#include <QtDebug>
#include <QMutexLocker>

#include "track/beatmatrix.h"

BeatMatrix::BeatMatrix(TrackPointer pTrack, const QByteArray* pByteArray)
        : QObject(),
          m_mutex(QMutex::Recursive),
          m_iSampleRate(pTrack->getSampleRate()) {
    if (pByteArray != NULL) {
        readByteArray(pByteArray);
    }
}

BeatMatrix::~BeatMatrix() {
}

unsigned int BeatMatrix::numBeats() const {
    return m_beatList.size();
}

double BeatMatrix::calculateBpm(double dStartSample, double dEndSample) const {

    /*
     * Let's compute the average local
     * BPM for N subsequent beats.
     * The average BPMs are
     * added to a list from which the statistical
     * median is computed
     *
     * N=8 seems to work great; We coincide with Traktor's
     * BPM value in many case but not worse than +-0.2 BPM
     */
     QList<double> average_bpm_list;
     const int N = 8; //computes the avg BPM from N subsequent beats --> should be a DEFINE later on


    /*
     * Just to demonstrate how you would count the beats manually
     *
     *    Beat numbers:   1  2  3  4   5  6  7  8    9
     *    Beat positions: ?  ?  ?  ?  |?  ?  ?  ?  | ?
     *
     * Usually one measures the time of N beats. One stops
     * the timer just before the (N+1)th beat begins.
     * The BPM is then computed by 60*N/<time needed to count N beats (in seconds)>
     *
     * Although beat tracking through Vamp is promising,
     * the local average BPM of 4 beats varies frequently by +-2 BPM.
     * Somtimes there N subsequent beats in the grid that are computed wrongly by Vamp.
     * Their local BPMs can be considered as outliers which would influence the
     * BPM computation negatively.
     * Therefore, the average local BPM is build for every N subsequent beats.
     * The average BPM values are added to a sorted list from which
     * the central element is choosen in order to compute the median.
     *
     */

    for(int i=N; i < m_beatList.size(); i+=N){
        //get start and end sample of the beats
        double start_sample = m_beatList.at(i-N);
        double end_sample = m_beatList.at(i);

        //Time needed to count a bar (4 beats)
        double time = (end_sample - start_sample)/(m_iSampleRate * 2);
        double avg_bpm = 60*N / time;
        //add to local BPM to list
        average_bpm_list << avg_bpm;
        //qDebug() << "Average Bar BPM: " << avg_bpm;
    }
    //sort the least of average BPMs
    qSort(average_bpm_list);


    //find the {(n+1)/2} th item in the sorted list
    int item_position = (average_bpm_list.size() + 1)/2;
    double median;

    /*
     * compute the median (bpm)
     * see http://en.wikipedia.org/wiki/Median#The_sample_median
     */
    if(average_bpm_list.size() % 2 == 0){
        //when the list size is eval
        double item_value1 = average_bpm_list.at(item_position/2);
        double item_value2 = average_bpm_list.at((item_position/2) + 1);
        median = item_value1 + (item_value1 - item_value2)/2;

    }
    else
    {
        median = average_bpm_list.at(item_position/2);

    }

    return median;

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


QString BeatMatrix::getVersion() const {
    QMutexLocker locker(&m_mutex);
    return "BeatMatrix-1.0";
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
    //dSamples = floorf(dSamples/2);
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
                return (*it);// * 2);
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
                return (*it);// * 2);
            }
            n++;
        }
    }

    return -1;
}

void BeatMatrix::findBeats(double startSample, double stopSample, QList<double>* pBeatsList) const {
    QMutexLocker locker(&m_mutex);
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
        pBeatsList->append(*curBeat);
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
    double startSample = *m_beatList.begin();
    double stopSample = *(m_beatList.end()-1);
//    double songDurationMinutes =
//            (stopSample - startSample) / (2 * 60.0f * m_iSampleRate);
//    return m_beatList.size() / songDurationMinutes;
    double bpm = calculateBpm(startSample, stopSample);
    return bpm;

}

double BeatMatrix::getBpmRange(double startSample, double stopSample) const {
    QMutexLocker locker(&m_mutex);
    if (!isValid() || startSample > stopSample) {
        return -1;
    }

//    BeatList::const_iterator startBeat = qLowerBound(m_beatList.begin(),
//                                               m_beatList.end(),
//                                               startSample);
//    BeatList::const_iterator stopBeat = qUpperBound(m_beatList.begin(),
//                                                    m_beatList.end(),
//                                                    stopSample);
//
//    double rangeDurationMinutes =
//            (stopSample - startSample) / (2 * 60.0f * m_iSampleRate);
//    //double rangeDurationMinutes =
//    //            (m_beatList[stopBeat] - m_beatList[startBeat]) / (2 * 60.0f * m_iSampleRate);
//    // Subtracting returns the number of beats between the samples referred to
//    // by the start and end.
//    double beatsInRange = stopBeat - startBeat;
//
//    return beatsInRange / rangeDurationMinutes;
    double bpm = calculateBpm(startSample, stopSample);
    return bpm;
}

void BeatMatrix::addBeat(double dBeatSample) {
    QMutexLocker locker(&m_mutex);


        BeatList::iterator it = qLowerBound(m_beatList.begin(),
                                            m_beatList.end(),
                                            dBeatSample);
        // Don't insert a duplicate beat. TODO(XXX) determine what epsilon to
        // consider a beat identical to another.
        //
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
    //QMutexLocker locker(&m_mutex);
    // TODO(XXX) How do we handle this?
}
