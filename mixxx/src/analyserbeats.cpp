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
    QString library = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatLibrary"));
    QString pluginID = m_pConfigAVT->getValueString(ConfigKey("[Vamp]","AnalyserBeatPluginID"));

        mvamp = new VampAnalyser();
        m_bPass = mvamp->Init(library, pluginID, sampleRate, totalSamples);
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
    /*
     * By default Vamp does not assume a 4/4 signature.
     * This is basically a good property of Vamp, however,
     * it leads to inaccurate beat grids if a 4/4 signature is given.
     * What is the problem? Almost all modern dance music from the last decades
     * refer to 4/4 signature. Thus, we must 'correct' the beat positions of Vamp
     *
     *
     */
    if(!beats.isEmpty()){
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
