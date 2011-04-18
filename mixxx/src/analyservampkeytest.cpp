/*
 * analyservamptest.cpp
 *
 *  Created on: 16/mar/2011
 *      Author: vittorio
 */

#include <QtDebug>
#include <QList>
#include <QVector>
#include <QString>
#include <time.h>
#include <math.h>
#include "analyservampkeytest.h"

AnalyserVampKeyTest::AnalyserVampKeyTest(ConfigObject<ConfigValue> *_config) {
    m_pConfigAVT = _config;
    m_bPass = 0;
    mvamp = new VampAnalyser();

}

AnalyserVampKeyTest::~AnalyserVampKeyTest(){
    delete mvamp;
}
void AnalyserVampKeyTest::initialise(TrackPointer tio, int sampleRate,
        int totalSamples) {
    //usage mvamp->Init(plugin key, output number, samplerate, totalsamples);
    //tested key detection features with vamp-plugins:
   // qm-vamp-plugins:qm-keydetector (GPLed)
    m_bPass = mvamp->Init("qm-subset:qm-keydetector",2,sampleRate, totalSamples);
    if (!m_bPass)
        qDebug() << "Failed to init";


    //   m_iStartTime = clock();
}

void AnalyserVampKeyTest::process(const CSAMPLE *pIn, const int iLen) {
    if(!m_bPass) return;
    m_bPass = mvamp->Process(pIn, iLen);


}

void AnalyserVampKeyTest::finalise(TrackPointer tio) {
    if(!m_bPass) return;
    QVector <QString> labels;
        if(mvamp->GetInitFramesVector(&m_frames)){
               if(mvamp->GetFirstValuesVector(&m_keys))
                   mvamp->GetLabelsVector(&labels);
               for (int i=0; i<m_frames.size(); i++){
                   qDebug()<<"Key changes to "<< m_keys[i] <<"("<<labels[i]<<")" << " at frame "<< m_frames[i];
               }
           }
           m_frames.clear();
           m_keys.clear();
    m_bPass = mvamp->End();
    qDebug()<<"Key detection complete";

    //m_iStartTime = clock() - m_iStartTime;
}

