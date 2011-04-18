/*
 * vampanalyser.cpp
 *
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 */

#include "vampanalyser.h"


#include <vamp-hostsdk/PluginChannelAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>
#include <QtDebug>

using Vamp::Plugin;

VampAnalyser::VampAnalyser()
    {
    m_pluginbuf = new CSAMPLE*[2];
}

VampAnalyser::~VampAnalyser() {
    delete[] m_pluginbuf;
    m_pluginbuf = NULL;
    delete mPlugin;
    mPlugin = 0;
}

bool VampAnalyser::Init(const Vamp::HostExt::PluginLoader::PluginKey key, int outputnumber,
        const int samplerate,const int TotalSamples) {

    mKey = key;
    m_iOutput = 0;
    mPlugin = 0;
    mRate = 0;

    m_iRemainingSamples = TotalSamples;
    m_iSampleCount = 0;
    m_iOUT = 0;

    if (samplerate <= 0.0) {
        qDebug() << "Track has incorrect samplerate";
        return false;
    }

    if (TotalSamples <= 0){
        qDebug() << "VampAnalyser cowardly refused to process an empty track";
        return false;
    }

    mRate = samplerate;
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
    Plugin::OutputList outputs = mPlugin->getOutputDescriptors();
    if (outputs.empty()) {
        qDebug() << "Plugin has no outputs!";
        return false;
    }
    this->SelectOutput(outputnumber);
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
            m_Results.insert( m_Results.end(), features[m_iOutput].begin(),features[m_iOutput].end());
            if (lastsamples) {
                Vamp::Plugin::FeatureSet features =
                        mPlugin->getRemainingFeatures();
                m_Results.insert( m_Results.end(), features[m_iOutput].begin(),features[m_iOutput].end());
            }
            m_iSampleCount += m_iBlockSize;
            m_iOUT = 0;
        }
    }
    return true;
}

bool VampAnalyser::End() {

    delete[] m_pluginbuf[0];
    delete[] m_pluginbuf[1];
    m_pluginbuf[0] = NULL;
    m_pluginbuf[1] = NULL;
    m_Results.clear();
    return true;
}

//void VampAnalyser::AddFeatures(Vamp::Plugin::FeatureSet &features, VampDataFromOutput vectout) {

//    for (int k = 0; k < m_iOutputs; k++) {
//        for (Vamp::Plugin::FeatureList::iterator fli = features[k].begin(); fli
//        != features[k].end(); ++fli) {
//
//            if (fli->hasTimestamp) {
//
//                Vamp::RealTime ftime0 = fli->timestamp;
//                //double ltime0 = ftime0.sec + (double(ftime0.nsec)
//                //        / 1000000000.0);
//
//                vectout[k].initframe  << (double) (Vamp::RealTime::realTime2Frame(ftime0,
//                        mRate));
//                if (fli->hasDuration) {
//                    Vamp::RealTime ftime1 = ftime0;
//                    ftime1 = ftime0 + fli->duration;
//                    //double ltime1 = ftime1.sec + (double(ftime1.nsec)
//                    //        / 1000000000.0);
//                    vectout[k].endframe
//                            << (double) (Vamp::RealTime::realTime2Frame(ftime1,
//                                    mRate));
//                }
//            }
//            vectout[k].label << fli->label.c_str();
//
//
//            std::vector<float> vec = fli->values;
//            if (!vec.empty()) {
//                vectout[k].firstvalue << vec[0];
//                vectout[k].lastvalue << vec[vec.size()-1];
//            }
//        }
//    }
//
//}


bool VampAnalyser::SetParameter(const QString parameter,const double value) {
return true;
}

void VampAnalyser::SelectOutput(int outputnumber){
    Plugin::OutputList outputs = mPlugin->getOutputDescriptors();
    if (outputnumber>=0 && outputnumber<outputs.size()){
        m_iOutput = outputnumber;
        }

}

bool VampAnalyser::GetInitFramesVector( QVector<double>* vectout) {

    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        if (fli->hasTimestamp) {
            Vamp::RealTime ftime0 = fli->timestamp;
            //double ltime0 = ftime0.sec + (double(ftime0.nsec)
            //        / 1000000000.0);
            *(vectout) << (double) (Vamp::RealTime::realTime2Frame(ftime0, mRate))*2;
        }

    }
    return true;
}

bool VampAnalyser::GetEndFramesVector( QVector<double>* vectout) {

    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        if (fli->hasDuration) {
            Vamp::RealTime ftime0 = fli->timestamp;
            Vamp::RealTime ftime1 = ftime0;
            ftime1 = ftime0 + fli->duration;
            //double ltime1 = ftime1.sec + (double(ftime1.nsec)
            //        / 1000000000.0);
            *(vectout) << (double) (Vamp::RealTime::realTime2Frame(ftime1, mRate))*2;
        }

    }
    return true;

}

bool VampAnalyser::GetLabelsVector( QVector<QString>* vectout) {

    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {

        *(vectout) << fli->label.c_str();
    }
    return true;

}


bool VampAnalyser::GetFirstValuesVector( QVector<double>* vectout) {

    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        std::vector<float> vec = fli->values;
        if (!vec.empty())
            *(vectout) << vec[0];

    }
    return true;

}

bool VampAnalyser::GetLastValuesVector( QVector<double>* vectout) {

    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        std::vector<float> vec = fli->values;
        if (!vec.empty())
            *(vectout) << vec[vec.size() - 1];

    }
    return true;

}

//bool GetMeanValuesVector ( int FromOutput , QVector <double> vectout ){
//    if( FromOutput>m_iOutputs || FromOutput < 0) return false;
//        for (Vamp::Plugin::FeatureList::iterator fli =
//                        features[k].begin(); fli
//                        != features[k].end(); ++fli){
//
//        }
//
//}
//
//bool GetMinValuesVector ( int FromOutput , QVector <double> vectout ){
//    if( FromOutput>m_iOutputs || FromOutput < 0) return false;
//        for (Vamp::Plugin::FeatureList::iterator fli =
//                        features[k].begin(); fli
//                        != features[k].end(); ++fli){
//
//        }
//
//}
//
//bool GetMaxValuesVector ( int FromOutput , QVector <double> vectout ){
//    if( FromOutput>m_iOutputs || FromOutput < 0) return false;
//        for (Vamp::Plugin::FeatureList::iterator fli =
//                        features[k].begin(); fli
//                        != features[k].end(); ++fli){
//
//        }
//
//}
