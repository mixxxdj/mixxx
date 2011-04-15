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

    VampAnalyser(Vamp::HostExt::PluginLoader::PluginKey key);
    virtual ~VampAnalyser();


    bool Init(int samplerate, const int TotalSamples);

    virtual bool Process(const CSAMPLE *pIn, const int iLen);

    bool End();

    virtual bool SetParameter (QString parameter, double value);

    virtual void SelectOutput (int outputnumber);

    virtual bool GetInitFramesVector( QVector<double>* vectout );

    virtual bool GetEndFramesVector( QVector<double>* vectout );

    virtual bool GetLabelsVector ( QVector<QString>* vectout);

    virtual bool GetFirstValuesVector ( QVector<double>* vectout);

    virtual bool GetLastValuesVector ( QVector<double>* vectout);

//    virtual QVector <double> GetMeanValueVector ( int FromOutput);
//
//    virtual QVector <double> GetMinValueVector ( int FromOutput);
//
//    virtual QVector <double> GetMaxValueVector ( int FromOutput);






private:

    void AddFeatures(Vamp::Plugin::FeatureSet &features);

    Vamp::HostExt::PluginLoader::PluginKey mKey;

    int m_iSampleCount, m_iOUT, m_iRemainingSamples,
        m_iBlockSize, m_iStepSize, mRate, m_iOutput;
    CSAMPLE ** m_pluginbuf;
    Vamp::Plugin *mPlugin;
    Vamp::Plugin::ParameterList mParameters;
    QVector <double> m_InitFrameVector;
    QVector <double> m_EndFrameVector;
    QVector <QString> m_LabelsVector;
    QVector <double> m_FirstValueVector;
    QVector <double> m_LastValueVector;
    QVector <double> m_MeanValueVector;
    QVector <double> m_MinValueVector;
    Vamp::Plugin::FeatureList m_Results;
};

#endif /* VAMPANALYSER_H_ */
