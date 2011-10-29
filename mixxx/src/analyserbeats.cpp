/*
 * analyservamptest.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QVector>
#include <QString>
#include <time.h>

#include "trackinfoobject.h"
#include "track/beatmatrix.h"
#include "track/beatfactory.h"
#include "analyserbeats.h"

static bool sDebug = true;


#ifdef __LINUX__
    #define VAMP_MIXXX_MINIMAL "libmixxxminimal.so"
#elif __APPLE__
    #define VAMP_MIXXX_MINIMAL "libmixxxminimal.dylib"
#else
    #define VAMP_MIXXX_MINIMAL "libmixxxminimal.dll"
#endif

#define VAMP_PLUGIN_BEAT_TRACKER_ID "qm-tempotracker:0"

AnalyserBeats::AnalyserBeats(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;
    m_iSampleRate = 44100;



}

AnalyserBeats::~AnalyserBeats(){

}
void AnalyserBeats::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {

//    if(tio->getBpm() != 0)
//        return;
    m_iSampleRate = sampleRate;
    qDebug()<<"Beat calculation started";
    m_bPass = false;
    BeatsPointer pBeats = tio->getBeats();
    //if(pBeats)
    //    m_bPass = !(pBeats->getVersion()).contains("BeatMatrix");
//    if(!m_bPass){
//        qDebug()<<"BeatMatrix already exists: calculation will not start";
//        return;
//    }

        mvamp = new VampAnalyser();
        m_bPass = mvamp->Init(VAMP_MIXXX_MINIMAL, VAMP_PLUGIN_BEAT_TRACKER_ID, sampleRate, totalSamples);
    //   m_iStartTime = clock();
}

void AnalyserBeats::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass)
        return;
    m_bPass = mvamp->Process(pIn, iLen);
}

void AnalyserBeats::finalise(TrackPointer tio) {
    if(!m_bPass)
        return;

   QVector <double> beats;
   beats = mvamp->GetInitFramesVector();

    if(!beats.isEmpty()){
        /*
         * By default Vamp does not assume a 4/4 signature.
         * This is basically a good property of Vamp, however,
         * it leads to inaccurate beat grids if a 4/4 signature is given.
         * What is the problem? Almost all modern dance music from the last decades
         * refer to 4/4 signatures. Thus, we must 'correct' the beat positions of Vamp
         */
        double corrected_global_bpm = calculateBpm(beats);
        BeatsPointer pBeats = BeatFactory::makeBeatGrid(tio, corrected_global_bpm, beats.at(0));
        tio->setBeats(pBeats);
        tio->setBpm(pBeats->getBpm());
    }
    m_bPass = mvamp->End();
    beats.clear();
    if(m_bPass)
        qDebug()<<"Beat Calculation complete";
    else
        qDebug()<<"Beat Calculation failed";
    //m_iStartTime = clock() - m_iStartTime;
    delete mvamp;
}

/*
 * This method detects the BPM given a set of beat positions.
 * We compute the avergage local BPM of by condering 8 beats
 * at a time. Internally, a sorted list of average BPM values is constructed
 * from which the statistical median is computed. This value provides
 * a pretty good guess of the global BPM value.
 */
double AnalyserBeats::calculateBpm(QVector<double> beats) const
{

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
     //mapping local bpm values to their frequencies
     QMap<QString, int> frequency_table;
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

    for(int i=N; i < beats.size(); i+=N){
        //get start and end sample of the beats
        double start_sample = beats.at(i-N);
        double end_sample = beats.at(i);

        //Time needed to count a bar (4 beats)
        double time = (end_sample - start_sample)/(m_iSampleRate * 2);
        double avg_bpm = 60*N / time;
        //add to local BPM to list
        average_bpm_list << avg_bpm;

        QString local_bpm_str = QString::number(avg_bpm,'g',6);
        //qDebug() << "Local BPM: " << avg_bpm << " StringVal: " << local_bpm_str;
        if(frequency_table.contains(local_bpm_str)){
            int newFreq = frequency_table.value(local_bpm_str) + 1;
            //Set new Frequency
            frequency_table.insert(local_bpm_str, newFreq);
        }
        else{
            frequency_table.insert(local_bpm_str, 1);
        }
        //qDebug() << "Average Bar BPM: " << avg_bpm;
    }
    //sort the list of average BPMs
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

    /*
     * Okay, let's consider the median an estimation of the BPM
     * To not soley rely on the median, we build the average
     * weighted value of all bpm values being at most
     * +-1 BPM from the median away.
     * Please note, this has improved the BPM: While relying on median only
     * we may have a derivation of about +-0.2 BPM, taking into account
     * BPM values around the median leads to derivation of +- 0.05
     * Please also note that this value refers to electronic music,
     * but to be honest, the BPM detection of Traktor and Co work best
     * with electronic music, too. But BPM detection for non-electronic
     * music isn't too bad.
     */
    QMapIterator<QString, int> i(frequency_table);
    double avg_weighted_bpm = 0.0;
    int sum = 0;
     while (i.hasNext()) {
         i.next();
         double bpmVal = i.key().toDouble();
         if(bpmVal >= median -1 && bpmVal <= median+1){
             sum += i.value();
             avg_weighted_bpm += bpmVal * i.value();
             if(sDebug)
                qDebug() << "BPM:" << bpmVal << " Frequency: " << i.value();
         }
     }
    //return median;
     if(sDebug){
         qDebug() << "Median: " << median;
         qDebug() << "Corrected Median: " << (avg_weighted_bpm / (double) sum);
     }
     return double (avg_weighted_bpm / (double) sum);


}
