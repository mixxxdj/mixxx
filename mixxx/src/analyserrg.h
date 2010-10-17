/*
 * analyserrg.h
 *
 *  Created on: 13/ott/2010
 *      Author: vittorio
 */

#ifndef ANALYSERRG_H_
#define ANALYSERRG_H_

/*#include <iostream>
#include <math.h>*/
#include "analyser.h"

class AnalyserGain : public Analyser {

  public:
    AnalyserGain();
    void initialise(TrackPointer tio, int sampleRate, int totalSamples);
    void process(const CSAMPLE *pIn, const int iLen);
    void finalise(TrackPointer tio);

  private:
    //int m_iStartTime;


};


#endif /* ANALYSERRG_H_ */
