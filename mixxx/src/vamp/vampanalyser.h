/*
 * vampanalyser.h
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 *      Original ideas took from Audacity VampEffect class from Chris Cannam.
 */

#ifndef VAMPANALYSER_H_
#define VAMPANALYSER_H_

#include <vamp-hostsdk/PluginLoader.h>
#include <QString>
#include <QList>
#include "sampleutil.h"
#include "configobject.h"

class VampAnalyser {

public:

    VampAnalyser(Vamp::HostExt::PluginLoader::PluginKey key, int OutputNumber);
    virtual ~VampAnalyser();

    //virtual QString GetPluginName();

    //virtual QString GetPluginDescription();

    //virtual QList<QString> GetPluginParameters();

    //virtual QString GetPluginCopyright();

    //virtual QString GetMaker();

    //virtual void SetPluginParameter(QString parameter, QString value);

    bool Init(int samplerate, const int TotalSamples);

    bool Process(const CSAMPLE *pIn, const int iLen);

    bool End();

    virtual QList<QString> GetEventsLabelList();
    virtual QList<double> GetEventsStartList(bool PreferRealTime = 0);
    virtual QList<double> GetEventsEndList(bool PreferRealTime = 0);
    virtual QList<double> GetEventsValueList();

private:

    void AddFeatures(Vamp::Plugin::FeatureSet &features);

    Vamp::HostExt::PluginLoader::PluginKey mKey;
    QList<QString> m_Label;
    QList<double> m_Timestamp;
    QList<double> m_TimestampFrame;
    QList<double> m_TimestampEnd;
    QList<double> m_TimestampEndFrame;
    QList<double> m_Value;

    QString m_PluginName;
    QString m_PluginDescription;
    QString m_PluginCopyright;
    QString m_PluginMaker;
    QList<QString> m_Parameters;
    int m_iSampleCount, m_iIN, m_iOUT, m_iRemainingSamples;
    int m_iBlockSize, m_iStepSize, mRate, mOutputNumber;
    bool m_bAlreadyLoaded;
    CSAMPLE ** m_pluginbuf;
    Vamp::Plugin *mPlugin;
    Vamp::Plugin::ParameterList mParameters;

};

#endif /* VAMPANALYSER_H_ */
