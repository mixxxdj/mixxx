/*
 * analyservamptest.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QList>
#include <QString>
#include <time.h>
#include <math.h>

#include "trackinfoobject.h"
#include "analyservamptest.h"
#include "track/beatmatrix.h"

AnalyserVampTest::AnalyserVampTest(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;

    //"pluginID"
    //tested beat tracking features with vamp-plugins:
    //"vamp-aubio:aubiotempo"(GPLed)
    //"qm-vamp-plugins:qm-barbeattracker"
    //"qm-vamp-plugins:qm-tempotracker" (both closed source even if qm-vamp-plugins may
    //be used for any purpose, or redistributed for non-commercial purposes only
    //as stated here: http://isophonics.net/QMVampPlugins)
    mvamp = new VampAnalyser("vamp-aubio:aubiotempo");

}

AnalyserVampTest::~AnalyserVampTest(){
    delete mvamp;
}
void AnalyserVampTest::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {
    m_bPass = mvamp->Init(sampleRate, totalSamples);
    if (!m_bPass)
        qDebug() << "Failed to init";

    //   m_iStartTime = clock();
}

void AnalyserVampTest::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass) return;
    m_bPass = mvamp->Process(pIn, iLen);
}

void AnalyserVampTest::finalise(TrackPointer tio) {
    if(!m_bPass) return;

    VampPluginEventList collect = mvamp->GetResults();
    float bpm = 0;
    int count = 0;
    BeatMatrix* BeatMat = new BeatMatrix(tio);
    for (int i = 0; i < collect.size(); ++i) {
        if(!(collect[i]).isFromOutput)
                BeatMat->addBeat((collect[i]).StartingFrame * 2);
        if((collect[i]).isFromOutput == 2){
            bpm += collect[i].Values[0];
            count++;
        }
    }
    if(count)qDebug()<<"bpm: "<< bpm/count;
    tio->setBeats(BeatsPointer(BeatMat));
    m_bPass = mvamp->End();
    if(!collect.isEmpty()) collect.clear();
    //m_iStartTime = clock() - m_iStartTime;
}

