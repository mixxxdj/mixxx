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
#include <vamp-hostsdk/vamp-hostsdk.h>
#include <QtDebug>

using Vamp::Plugin;


VampAnalyser::VampAnalyser(Vamp::HostExt::PluginLoader::PluginKey key) :
        mKey(key){
   m_pluginbuf = new CSAMPLE*[2];
   mPlugin = 0;
   mRate = 0;
}

VampAnalyser::~VampAnalyser() {
    delete [] m_pluginbuf;
    m_pluginbuf = NULL;
    delete mPlugin;
    mPlugin = 0;

}

bool VampAnalyser::Init(int samplerate, int TotalSamples) {


    m_iRemainingSamples = TotalSamples;

    m_iSampleCount = 0;
    m_iOUT = 0;


    if (samplerate <= 0.0) {
        qDebug() << "Track has incorrect samplerate";
        return false;
    }


       mRate = samplerate;
       if(mPlugin) delete mPlugin;
       mPlugin = 0;
       Vamp::HostExt::PluginLoader *loader =
                   Vamp::HostExt::PluginLoader::getInstance();
       mPlugin = loader->loadPlugin(mKey, mRate,
                                        Vamp::HostExt::PluginLoader::ADAPT_ALL);




    if (!mPlugin) {
        qDebug() << "Failed to load Vamp Plug-in.";
        return false;
    }
    Plugin::OutputList outputs = mPlugin->getOutputDescriptors();
    if(outputs.empty()){
        qDebug()<<"Plugin has no outputs!";
        return false;
    }
    else
    {
        m_iOutputs = outputs.size();
        outputs.clear();
    }

    m_iBlockSize = mPlugin->getPreferredBlockSize();

    if (m_iBlockSize == 0) {
        m_iBlockSize = 1024;
    }

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
    int iIN = 0;
    bool lastsamples = false;
    m_iRemainingSamples -= iLen;

    while (iIN < iLen / 2) {
        m_pluginbuf[0][m_iOUT] = pIn[2 * iIN] * 32767;
        m_pluginbuf[1][m_iOUT] = pIn[2 * iIN + 1] * 32767;
        m_iOUT++;
        iIN++;
        if (m_iRemainingSamples <= 0 && iIN == iLen / 2) {
            lastsamples = true;
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
            if (lastsamples){
                Vamp::Plugin::FeatureSet features = mPlugin->getRemainingFeatures();
                AddFeatures(features);
            }
            m_iSampleCount += m_iBlockSize;
            m_iOUT = 0;
        }
    }
    return true;
}

bool VampAnalyser::End() {

    delete [] m_pluginbuf[0];
    delete [] m_pluginbuf[1];
    m_pluginbuf[0] = NULL;
    m_pluginbuf[1] = NULL;
    m_Results.clear();
    return true;
}

void VampAnalyser::AddFeatures(Vamp::Plugin::FeatureSet &features) {

    for (int k = 0; k < m_iOutputs; k++) {
        for (Vamp::Plugin::FeatureList::iterator fli =
                features[k].begin(); fli
                != features[k].end(); ++fli) {
            VampPluginEvent v;

            v.isFromOutput = k;

            if (fli->hasTimestamp) {
                v.hasStartingFrame = true;
                Vamp::RealTime ftime0 = fli->timestamp;
                //double ltime0 = ftime0.sec + (double(ftime0.nsec)
                //        / 1000000000.0);
                v.StartingFrame
                = (double) (Vamp::RealTime::realTime2Frame(ftime0,
                        mRate));
                if (fli->hasDuration) {
                    Vamp::RealTime ftime1 = ftime0;
                    ftime1 = ftime0 + fli->duration;
                    //double ltime1 = ftime1.sec + (double(ftime1.nsec)
                    //        / 1000000000.0);
                    v.EndingFrame
                    = (double) (Vamp::RealTime::realTime2Frame(
                            ftime1,
                            mRate));
                } else {
                    v.hasEndingFrame = false;
                    v.EndingFrame = 0;
                }
            } else {
                v.hasStartingFrame = false;
                v.StartingFrame = 0;
                v.hasEndingFrame = false;
                v.EndingFrame = 0;
            }

            std::vector<float> vec = fli->values;
            if (vec.empty()) {
                v.VectorValuesSize = 0;
            } else {
                v.VectorValuesSize = vec.size();
                v.Values.reserve(v.VectorValuesSize);
                v.Values = QVector<float>::fromStdVector(vec);
            }



            v.Label = fli->label.c_str();
            m_Results << v;
        }
    }

}

VampPluginEventList VampAnalyser::GetResults() {
    return m_Results;
}

