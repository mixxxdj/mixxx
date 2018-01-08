/*
 * vampanalyzer.h
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 *      Original ideas taken from Audacity VampEffect class from Chris Cannam.
 */

#ifndef ANALYZER_VAMP_VAMPANALYZER_H
#define ANALYZER_VAMP_VAMPANALYZER_H

#include <QString>
#include <QVector>

#include <vamp-hostsdk/vamp-hostsdk.h>

#include "preferences/usersettings.h"
#include "util/sample.h"

namespace mixxx {

class VampPluginLoader;

}

class VampAnalyzer {
  public:
    VampAnalyzer();
    virtual ~VampAnalyzer();

    bool Init(const QString pluginlibrary, const QString pluginid,
              const int samplerate, const int totalSamples, bool bFastAnalysis);
    bool Process(const CSAMPLE *pIn, const int iLen);
    bool End();
    bool SetParameter(const QString parameter, const double value);

    QVector<double> GetInitFramesVector();
    QVector<double> GetEndFramesVector();
    QVector<QString> GetLabelsVector();
    QVector<double> GetFirstValuesVector();
    QVector<double> GetLastValuesVector();

  private:
    friend class mixxx::VampPluginLoader;
    Vamp::HostExt::PluginLoader::PluginKey m_key;
    Vamp::Plugin *m_plugin;
    int m_iOutput;
    int m_iBlockSize;
    int m_iStepSize;

    int m_iSampleCount, m_iOUT, m_iRemainingSamples,
        m_rate;
    CSAMPLE* m_pluginbuf[2];

    bool m_bDoNotAnalyseMoreSamples;
    bool m_FastAnalysisEnabled;
    int m_iMaxSamplesToAnalyse;

    Vamp::Plugin::FeatureList m_results;
};

#endif /* ANALYZER_VAMP_VAMPANALYZER_H */
