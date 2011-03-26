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

    //"pluginID", OutputNumber
    //tested beat tracking features with vamp-plugins:
    //"vamp-aubio:aubiotempo", 0 (GPLed)
    //"qm-vamp-plugins:qm-barbeattracker", 0 (best but closed source)
    mvamp = new VampAnalyser("vamp-aubio:aubiotempo", 0);
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
    m_bPass = mvamp->End();
    collect = mvamp->GetEventsStartList();
    BeatMatrix* BeatMat = new BeatMatrix(tio);
    for (int i = 0; i < collect.size(); ++i) {
        BeatMat->addBeat(collect[i] * 2);
    }
    tio->setBeats(BeatsPointer(BeatMat));
    if(!collect.isEmpty()) collect.clear();
    //m_iStartTime = clock() - m_iStartTime;
}

