/*
 * beattools.cpp
 *
 *  Created on: 30/nov/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QString>
#include <QList>
#include <QMap>
#include <math.h>
#define BPM_ERROR 0.05f //we are generous and assume the global_BPM to be at most 0.12 BPM far away from the correct one

#include "beattools.h"

static bool sDebug = false;

double BeatTools::calculateBpm(QVector<double> beats, int SampleRate, int min_bpm, int max_bpm){


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
    int max_frequency = 0;
    double most_freq_bpm = 0;
    double avg_bpm = 0;
    for(int i=N; i < beats.size(); i+=N){
        //get start and end sample of the beats
        double start_sample = beats.at(i-N);
        double end_sample = beats.at(i);

        //Time needed to count a bar (4 beats)
        double time = (end_sample - start_sample)/(SampleRate);
        avg_bpm = 60*N / time;

        if(avg_bpm < min_bpm)
        {
            avg_bpm *= 2;
        }
        if(avg_bpm > max_bpm){
            avg_bpm /=2;
        }


        //add to local BPM to list
        average_bpm_list << avg_bpm;

        QString local_bpm_str = QString::number(avg_bpm,'g',6);
        //qDebug() << "Local BPM: " << avg_bpm << " StringVal: " << local_bpm_str;
        if(frequency_table.contains(local_bpm_str)){
            int newFreq = frequency_table.value(local_bpm_str) + 1;
            //Set new Frequency
            frequency_table.insert(local_bpm_str, newFreq);

            if(newFreq > max_frequency){
                max_frequency = newFreq;
                most_freq_bpm = avg_bpm;
            }

        }
        else{
            frequency_table.insert(local_bpm_str, 1);
        }
        //qDebug() << "Average Bar BPM: " << avg_bpm;
    }
    //sort the list of average BPMs
    qSort(average_bpm_list);


    /*
     * compute the median (bpm)
     * see http://en.wikipedia.org/wiki/Median#The_sample_median
     */
    double median = 0;
    if(average_bpm_list.size()==0)
        return avg_bpm;
    if(average_bpm_list.size() % 2 == 0){
        //build avg of {(n)/2} and {(n)/2}+1 in the sorted list
        int item_position = (average_bpm_list.size())/2;
        //when the list size is eval
        double item_value1 = average_bpm_list.at(item_position);
        double item_value2 = average_bpm_list.at(item_position + 1);
        median = (item_value1 + item_value2)/2;

    }
    else
    {
        //find the {(n+1)/2} th item in the sorted list
        int item_position = (average_bpm_list.size() + 1)/2;
        median = average_bpm_list.at(item_position);
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
    qDebug() << "BPM range between " << min_bpm << " and " << max_bpm;
    qDebug() << "Max BPM Frequency=" << most_freq_bpm;
    int sum = 0;
     while (i.hasNext()) {
         i.next();
         double bpmVal = i.key().toDouble();

         if( (bpmVal >= median -1.0) && (bpmVal <= median+1.0)){
             sum += i.value();
             avg_weighted_bpm += bpmVal * i.value();
             if(sDebug)
                qDebug() << "BPM:" << bpmVal << " Frequency: " << i.value();
         }
     }
     double global_bpm = (avg_weighted_bpm / (double) sum);
    //return median;
     if(sDebug){
         qDebug() << "Sum of frequencies: " << sum;
         qDebug() << "Statistical median BPM: " << median;
         qDebug() << "Weighted Avg of values around median of +- 1 BPM " << global_bpm;
     }
     /*
      * Although we have a minimal deviation of about +- 0.05 BPM units
      * compared to Traktor, this deviation may cause the beat grid to
      * look unaligned, especially at the end of a track.
      * Let's try to get the BPM 'perfect' :-)
      *
      * Idea: Iterate over the original VAMP beat set where
      * some detected beats may be wrong:
      * The Vamp beat is considered as correct if:
      *  - the beat position can be approached by a beat grid obtained by the global BPM
      *
      * If the beat turns out correct, we can compute the error in BPM units.
      * E.g., we can check the Vamp beat position after 60 seconds. Ideally,
      * the approached beat is just a couple of samples away, i.e., not worse than 0.05 BPM units.
      * The distance between these two samples can be used for BPM error correction.
      */


     double perfect_bpm = global_bpm;
     // Calculate beat length as sample offsets based on our estimated 'global_bpm'
     double dBeatLength = (60.0 * SampleRate  / global_bpm);
     double reference_beat = 0;

     if(beats.size() > 0)
         reference_beat = beats[0]; //we assume the first beat to be wrong


     for(int i=N; i < beats.size(); i+=N){
         //get start and end sample of the beats
         double beat_start = beats.at(i-N);
         double beat_end = beats.at(i);

         //Time needed to count a bar (N beats)
         double time = (beat_end - beat_start)/(SampleRate);
         double local_bpm = 60*N / time;
         //qDebug() << "Local BPM beat " << i << ": " << local_bpm;

         if(local_bpm <= global_bpm + BPM_ERROR && local_bpm >= global_bpm - BPM_ERROR){

             /*
              * Let's try to replace 'reference_beat_end' by approaching the beat using the global BPM
              */
               double estimated_beat = beat_start + N * dBeatLength;
               //compute the beat number
               double dBeatPos = (estimated_beat / dBeatLength);
               double iBeatPos =  floor(dBeatPos + 0.5); //rounding up or down

               //get BPM for estimated_beat
               double time2 = (estimated_beat - reference_beat)/(SampleRate);
               double exact_BPM = 60 * iBeatPos / time2;

               if(exact_BPM  <= global_bpm + BPM_ERROR && exact_BPM >= global_bpm - BPM_ERROR){
                   if(sDebug)
                       qDebug() << "Vamp beat " << i<< " might be correct: " << dBeatPos << " = " << exact_BPM << "BPM"  ;
                   perfect_bpm = exact_BPM;
               }


         }
     }

      //last guess to make BPM more accurate: rounding values like 127.96 or 128.01 to 128.0
     double rounded_bpm = floor(perfect_bpm+0.5);
     double bpm_diff = rounded_bpm - perfect_bpm;
     if(sDebug){
         qDebug() << "Perfect BPM=" << perfect_bpm;
         qDebug() << "Rounded Perfect BPM=" << rounded_bpm;
         qDebug() << "Rounded difference=" << fabs(bpm_diff);
         qDebug() << "Perform rounding=" << ((fabs(bpm_diff) <= BPM_ERROR)? true: false);
     }

     return (fabs(bpm_diff) <= BPM_ERROR)? rounded_bpm : perfect_bpm;

}

double BeatTools::calculateOffset(const QVector<double> beats1, const QVector<double> beats2
        , const int SampleRate, int min_bpm, int max_bpm) {
    /*
     * Here we compare to beats vector and try to determine the best offset
     * based on the occurences, i.e. by assuming that the almost correct beats
     * are more than the "false" ones.
     */
    double bpm1 = calculateBpm(beats1, SampleRate, min_bpm, max_bpm);
    double beatlength1 = (60.0 * SampleRate / bpm1);
    int MaxFreq = 1;
    double BestOffset = beats1.at(0) - beats2.at(0);
    double offset = floor(-beatlength1 / 2);
    while (offset < (beatlength1 / 2) )
    {
        double freq = 0;
        for (int i = 0; i < beats2.size(); i+=4)
        {
            QVector<double>::const_iterator it;
            it = qUpperBound(beats1.begin(), beats1.end(), beats2.at(i));
            if (fabs(*it - beats2.at(i)-offset) <= .02 * (60*SampleRate/bpm1))
                freq++;
        }
        if (freq > MaxFreq)
        {
            MaxFreq = freq;
            BestOffset = offset;
        }
        offset++;
    }

    if (sDebug) {
        qDebug() << "Best offset " << BestOffset << " guarantees that "
                << MaxFreq << " over " << beats1.size()/4
                << " beats almost coincides. ";
    }

    return floor(BestOffset + (.02 * (60*SampleRate/bpm1)));
}
