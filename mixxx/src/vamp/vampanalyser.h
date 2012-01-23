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
#include "vamppluginloader.h"
#include "sampleutil.h"

class VampAnalyser {
  public:
    VampAnalyser();
    virtual ~VampAnalyser();

    bool Init(const QString pluginlibrary, const QString pluginid,
              const int samplerate, const int TotalSamples);
    bool Process(const CSAMPLE *pIn, const int iLen);
    bool End();
    bool SetParameter(const QString parameter, const double value);

    QVector <double> GetInitFramesVector();
    QVector <double> GetEndFramesVector();
    QVector<QString> GetLabelsVector();
    QVector <double> GetFirstValuesVector();
    QVector <double> GetLastValuesVector();

    // Initialize the VAMP_PATH environment variable to point to the default
    // places that Mixxx VAMP plugins are deployed on installation. If a
    // VAMP_PATH environment variable is already set by the user, then this
    // method appends to that.
    static void initializePluginPaths();

  private:
    void SelectOutput(const int outputnumber);

    Vamp::HostExt::PluginLoader::PluginKey mKey;
    int m_iSampleCount, m_iOUT, m_iRemainingSamples,
        m_iBlockSize, m_iStepSize, mRate, m_iOutput;
    CSAMPLE ** m_pluginbuf;
    Vamp::Plugin *mPlugin;
    Vamp::Plugin::ParameterList mParameters;
    Vamp::Plugin::FeatureList m_Results;
};

#endif /* VAMPANALYSER_H_ */
