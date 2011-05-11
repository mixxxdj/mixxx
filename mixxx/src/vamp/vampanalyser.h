/*
 * vampanalyser.h
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 *      Original ideas taken from Audacity VampEffect class from Chris Cannam.
 */

#ifndef VAMPANALYSER_H_
#define VAMPANALYSER_H_

#include "vamp-hostsdk/PluginLoader.h"
#include "vamp-hostsdk/vamp-hostsdk.h"
#include "vamp-hostsdk/Plugin.h"
#include <QString>
#include <QList>
#include <QVector>
#include "sampleutil.h"



class VampAnalyser {

public:

    VampAnalyser();

    ~VampAnalyser();

    //bool Init(const Vamp::HostExt::PluginLoader::PluginKey key, int outputnumber,
    //        const int samplerate, const int TotalSamples);

    bool Init(const QString pluginlibrary, const QString pluginid,
                const int samplerate, const int TotalSamples);

    bool Process(const CSAMPLE *pIn, const int iLen);

    bool End();

    bool SetParameter (const QString parameter,const double value);



    QVector <double> GetInitFramesVector();

    QVector <double> GetEndFramesVector();

    QVector<QString> GetLabelsVector ();

    QVector <double> GetFirstValuesVector ();

    QVector <double> GetLastValuesVector ();

//   QVector <double> GetMeanValueVector ( int FromOutput);
//
//   QVector <double> GetMinValueVector ( int FromOutput);
//
//    QVector <double> GetMaxValueVector ( int FromOutput);






private:

    //void AddFeatures(Vamp::Plugin::FeatureSet &features);

    void SelectOutput (const int outputnumber);

    Vamp::HostExt::PluginLoader::PluginKey mKey;

    int m_iSampleCount, m_iOUT, m_iRemainingSamples,
        m_iBlockSize, m_iStepSize, mRate, m_iOutput;
    CSAMPLE ** m_pluginbuf;
    Vamp::Plugin *mPlugin;
    Vamp::Plugin::ParameterList mParameters;
    Vamp::Plugin::FeatureList m_Results;
};

#endif /* VAMPANALYSER_H_ */
