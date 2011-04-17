/*
 * vampanalyser.h
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 *      Original ideas taken from Audacity VampEffect class from Chris Cannam.
 */

#ifndef VAMPANALYSER_H_
#define VAMPANALYSER_H_

#include <vamp-hostsdk/PluginLoader.h>
#include <vamp-hostsdk/vamp-hostsdk.h>
#include <vamp-hostsdk/Plugin.h>
#include <QString>
#include <QList>
#include <QVector>
#include "sampleutil.h"



class VampAnalyser {

public:

    VampAnalyser(const Vamp::HostExt::PluginLoader::PluginKey key);
    ~VampAnalyser();


    bool Init(const int samplerate, const int TotalSamples);

    bool Process(const CSAMPLE *pIn, const int iLen);

    bool End();

    bool SetParameter (const QString parameter,const double value);

    void SelectOutput (const int outputnumber);

    bool GetInitFramesVector( QVector<double>* vectout );

    bool GetEndFramesVector( QVector<double>* vectout );

    bool GetLabelsVector ( QVector<QString>* vectout);

    bool GetFirstValuesVector ( QVector<double>* vectout);

    bool GetLastValuesVector ( QVector<double>* vectout);

//   QVector <double> GetMeanValueVector ( int FromOutput);
//
//   QVector <double> GetMinValueVector ( int FromOutput);
//
//    QVector <double> GetMaxValueVector ( int FromOutput);






private:

    void AddFeatures(Vamp::Plugin::FeatureSet &features);

    Vamp::HostExt::PluginLoader::PluginKey mKey;

    int m_iSampleCount, m_iOUT, m_iRemainingSamples,
        m_iBlockSize, m_iStepSize, mRate, m_iOutput;
    CSAMPLE ** m_pluginbuf;
    Vamp::Plugin *mPlugin;
    Vamp::Plugin::ParameterList mParameters;
//    QVector <double> m_InitFrameVector;
//    QVector <double> m_EndFrameVector;
//    QVector <QString> m_LabelsVector;
//    QVector <double> m_FirstValueVector;
//    QVector <double> m_LastValueVector;
//    QVector <double> m_MeanValueVector;
//    QVector <double> m_MinValueVector;
    Vamp::Plugin::FeatureList m_Results;
};

#endif /* VAMPANALYSER_H_ */
