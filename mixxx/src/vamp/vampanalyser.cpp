/*
 * vampanalyser.cpp
 *
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 */

#include "vampanalyser.h"

#include <vamp-hostsdk/Plugin.h>
#include <vamp-hostsdk/PluginChannelAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>
#include <QtDebug>

VampAnalyser::VampAnalyser(Vamp::HostExt::PluginLoader::PluginKey key,
        int OutputNumber) :
        mKey(key), mOutputNumber(OutputNumber) {
    m_bAlreadyLoaded=false;
    m_pluginbuf = new CSAMPLE*[2];
}

VampAnalyser::~VampAnalyser() {
    delete [] m_pluginbuf;
    m_pluginbuf = NULL;
    delete mPlugin;
    mPlugin = 0;
    qDebug()<<"Deleting Vamp Analyser";
}

bool VampAnalyser::Init(int samplerate, int TotalSamples) {


    m_iRemainingSamples = TotalSamples;
    mRate = samplerate;
    m_iSampleCount = 0;
    m_iOUT = 0;
    if (!m_Label.isEmpty())
        m_Label.clear();
    if (!m_Timestamp.isEmpty())
        m_Timestamp.clear();
    if (!m_TimestampEnd.isEmpty())
        m_TimestampEnd.clear();
    if (!m_TimestampFrame.isEmpty())
        m_TimestampFrame.clear();
    if (!m_TimestampEndFrame.isEmpty())
        m_TimestampEndFrame.clear();
    if (!m_Value.isEmpty())
        m_Value.clear();
    if (!m_Parameters.isEmpty())
        m_Parameters.clear();

    if (mRate <= 0.0) {
        qDebug() << "Track has incorrect samplerate";
        return false;
    }

    if (mPlugin)
        delete mPlugin;
    mPlugin = 0;

    Vamp::HostExt::PluginLoader *loader =
            Vamp::HostExt::PluginLoader::getInstance();
    mPlugin = loader->loadPlugin(mKey, mRate,
                                 Vamp::HostExt::PluginLoader::ADAPT_ALL);

    if (!mPlugin) {
        qDebug() << "Failed to load Vamp Plug-in.";
        return false;
    }
    m_iBlockSize = mPlugin->getPreferredBlockSize();

    if (m_iBlockSize == 0) {
        m_iBlockSize = 1024;
    }

    mPlugin->reset();
    m_iStepSize = m_iBlockSize;

    if (!mPlugin->initialise(2, m_iBlockSize, m_iStepSize))
        return false;

        m_pluginbuf[0] = new CSAMPLE[m_iBlockSize];
        m_pluginbuf[1] = new CSAMPLE[m_iBlockSize];
        return true;


}

bool VampAnalyser::Process(const CSAMPLE *pIn, const int iLen) {
    if (!mPlugin) {
        qDebug() << "Error in process: Plugin not loaded";
        return false;
    }
    m_iIN = 0;
    m_iRemainingSamples -= iLen;

    while (m_iIN < iLen / 2) {
        m_pluginbuf[0][m_iOUT] = pIn[2 * m_iIN] * 32767;
        m_pluginbuf[1][m_iOUT] = pIn[2 * m_iIN + 1] * 32767;
        m_iOUT++;
        m_iIN++;
        if (m_iRemainingSamples <= 0 && m_iIN == iLen / 2) {
            while (m_iOUT < m_iBlockSize) {
                m_pluginbuf[0][m_iOUT] = 0;
                m_pluginbuf[1][m_iOUT] = 0;
                m_iOUT++;
            }
        }
        if (m_iOUT == m_iBlockSize) {

            Vamp::RealTime timestamp =
                    Vamp::RealTime::frame2RealTime(m_iSampleCount, mRate);

            Vamp::Plugin::FeatureSet features = mPlugin->process(m_pluginbuf,
                                                                 timestamp);
            AddFeatures(features);
            m_iSampleCount += m_iBlockSize;
            m_iOUT = 0;
        }
    }
    return true;
}

bool VampAnalyser::End() {
    Vamp::Plugin::FeatureSet features = mPlugin->getRemainingFeatures();
    AddFeatures(features);
    delete [] m_pluginbuf[0];
    delete [] m_pluginbuf[1];
    m_pluginbuf[0] = NULL;
    m_pluginbuf[1] = NULL;

    return true;
}

void VampAnalyser::AddFeatures(Vamp::Plugin::FeatureSet &features) {

    for (Vamp::Plugin::FeatureList::iterator fli =
            features[mOutputNumber].begin(); fli
            != features[mOutputNumber].end(); ++fli) {
        QString label = fli->label.c_str();
        m_Label << label;

        Vamp::RealTime ftime0 = fli->timestamp;
        double ltime0 = ftime0.sec + (double(ftime0.nsec) / 1000000000.0);
        m_Timestamp << (double) (ltime0);
        m_TimestampFrame << (double) (Vamp::RealTime::realTime2Frame(ftime0,
                mRate));

        Vamp::RealTime ftime1 = ftime0;
        if (fli->hasDuration)
            ftime1 = ftime0 + fli->duration;
        double ltime1 = ftime1.sec + (double(ftime1.nsec) / 1000000000.0);
        m_TimestampEnd << (double) (ltime1);
        m_TimestampEndFrame << (double) (Vamp::RealTime::realTime2Frame(ftime1,
                mRate));

        double value;
        if (fli->values.empty()) {
            value = 0;
        } else {
            value = (double) (*fli->values.begin());
        }
        m_Value << value;
    }
}

QList<QString> VampAnalyser::GetEventsLabelList() {
    return m_Label;
}

QList<double> VampAnalyser::GetEventsStartList(bool PreferRealTime) {
    if (PreferRealTime)
        return m_Timestamp;
    else
        return m_TimestampFrame;
}

QList<double> VampAnalyser::GetEventsEndList(bool PreferRealTime) {
    if (PreferRealTime)
        return m_TimestampEnd;
    else
        return m_TimestampEndFrame;
}

QList<double> VampAnalyser::GetEventsValueList() {
    return m_Value;
}
