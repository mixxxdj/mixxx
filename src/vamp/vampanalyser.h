/*
 * vampanalyser.h
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 *      Original ideas taken from Audacity VampEffect class from Chris Cannam.
 */

#ifndef VAMPANALYSER_H_
#define VAMPANALYSER_H_

#include <QString>
#include <QList>
#include <QVector>
#include <vamp-hostsdk/vamp-hostsdk.h>

#include "vamp/vamppluginloader.h"
#include "sampleutil.h"
#include "configobject.h"

class VampAnalyser {
  public:
    VampAnalyser();
    virtual ~VampAnalyser();

    bool Init(const QString pluginlibrary, const QString pluginid,
              const int samplerate, const int TotalSamples, bool bFastAnalysis);
    bool Process(const CSAMPLE *pIn, const int iLen);
    bool End();
    bool SetParameter(const QString parameter, const double value);

    QVector<double> GetInitFramesVector();
    QVector<double> GetEndFramesVector();
    QVector<QString> GetLabelsVector();
    QVector<double> GetFirstValuesVector();
    QVector<double> GetLastValuesVector();

    // Initialize the VAMP_PATH environment variable to point to the default
    // places that Mixxx VAMP plugins are deployed on installation. If a
    // VAMP_PATH environment variable is already set by the user, then this
    // method appends to that.
    static void initializePluginPaths();
    void SelectOutput(const int outputnumber);

  private:
    Vamp::HostExt::PluginLoader::PluginKey m_key;
    int m_iSampleCount, m_iOUT, m_iRemainingSamples,
        m_iBlockSize, m_iStepSize, m_rate, m_iOutput;
    CSAMPLE ** m_pluginbuf;
    Vamp::Plugin *m_plugin;
    Vamp::Plugin::ParameterList mParameters;
    Vamp::Plugin::FeatureList m_Results;

    bool m_bDoNotAnalyseMoreSamples;
    bool m_FastAnalysisEnabled;
    int m_iMaxSamplesToAnalyse;
};

#endif /* VAMPANALYSER_H_ */
