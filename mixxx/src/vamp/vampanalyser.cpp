/*
 * vampanalyser.cpp
 *
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 */

#include "vampanalyser.h"


#include "vamp-hostsdk/PluginChannelAdapter.h"
#include "vamp-hostsdk/PluginInputDomainAdapter.h"
#include "vamp-hostsdk/PluginHostAdapter.h"
#include "vamp-hostsdk/PluginInputDomainAdapter.h"
#include "vamp-hostsdk/PluginLoader.h"
#include <QtDebug>
#include <QStringList>

using Vamp::Plugin;
using Vamp::PluginHostAdapter;

VampAnalyser::VampAnalyser()
    {
    m_pluginbuf = new CSAMPLE*[2];
    mPlugin = NULL;
}

VampAnalyser::~VampAnalyser() {
    if(m_pluginbuf!=NULL){
    delete[] m_pluginbuf;
    m_pluginbuf = NULL;
    }
    if(mPlugin!=NULL){
        delete mPlugin;
        mPlugin = NULL;
    }
}

bool VampAnalyser::Init(const QString pluginlibrary, const QString pluginid,
        const int samplerate, const int TotalSamples) {
    m_iOutput = 0;
    mRate = 0;

    m_iRemainingSamples = TotalSamples;
    m_iSampleCount = 0;
    m_iOUT = 0;

    if (samplerate <= 0.0) {
        qDebug() << "VampAnalyser: Track has incorrect samplerate";
        return false;
    }

    if (TotalSamples <= 0){
        qDebug() << "VampAnalyser: VampAnalyser cowardly refused to process an empty track";
        return false;
    }

    mRate = samplerate;
    if (mPlugin != NULL){
        delete mPlugin;
        qDebug()<<"VampAnalyser: kill plugin";
    }
    Vamp::HostExt::PluginLoader *loader =
            Vamp::HostExt::PluginLoader::getInstance();
    QStringList pluginlist = pluginid.split(":");
    if(pluginlist.size()!=2){
        qDebug()<<"VampAnalyser: got malformed pluginid: "<<pluginid;
        return false;
    }
    QString plugin = pluginlist.at(0);
    mKey = loader->composePluginKey(pluginlibrary.toStdString(),plugin.toStdString());
    int outputnumber = (pluginlist.at(1)).toInt();


    mPlugin = loader->loadPlugin(mKey, mRate,
                                 Vamp::HostExt::PluginLoader::ADAPT_ALL);

    if (!mPlugin) {
        qDebug() << "VampAnalyser: Cannot load Vamp Plug-in.";
        qDebug()<<"Please copy libmixxxminimal.so from build dir to one of the following:";
        std::vector<std::string> path = PluginHostAdapter::getPluginPath();
        for (int i=0; i<path.size(); i++){
            qDebug()<<QString::fromStdString(path[i]);
        }
        return false;
    }
    Plugin::OutputList outputs = mPlugin->getOutputDescriptors();
    if (outputs.empty()) {
        qDebug() << "VampAnalyser: Plugin has no outputs!";
        return false;
    }
    this->SelectOutput(outputnumber);
    m_iBlockSize = mPlugin->getPreferredBlockSize();

    if (m_iBlockSize == 0) {
        m_iBlockSize = 1024;
    }

    m_iStepSize = m_iBlockSize;

    if (!mPlugin->initialise(2, m_iBlockSize, m_iStepSize)){
        qDebug()<<"VampAnalyser: Cannot initialise plugin";
        return false;
    }
    m_pluginbuf[0] = new CSAMPLE[m_iBlockSize];
    m_pluginbuf[1] = new CSAMPLE[m_iBlockSize];

    return true;

}

bool VampAnalyser::Process(const CSAMPLE *pIn, const int iLen) {
    if (!mPlugin) {
        qDebug() << "VampAnalyser: Plugin not loaded";
        return false;
    }
    int iIN = 0;
    bool lastsamples = false;
    m_iRemainingSamples -= iLen;
    if(m_pluginbuf[0]==NULL||m_pluginbuf[1]==NULL){
        qDebug()<< "VampAnalyser: Buffer points to NULL";
        return false;
    }
    while (iIN < iLen / 2) {
        m_pluginbuf[0][m_iOUT] = pIn[2 * iIN]; //* 32767;
        m_pluginbuf[1][m_iOUT] = pIn[2 * iIN + 1]; //* 32767;


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
    for(int i=0; i < 2; i++){
    if(m_pluginbuf[i]){
        delete [] m_pluginbuf[i];
        m_pluginbuf[i] = NULL;
        }
    }
    m_Results.clear();
    return true;
}




bool VampAnalyser::SetParameter(const QString parameter,const double value) {


return true;
}

void VampAnalyser::SelectOutput(int outputnumber){
    Plugin::OutputList outputs = mPlugin->getOutputDescriptors();
    if (outputnumber>=0 && outputnumber<outputs.size()){
        m_iOutput = outputnumber;
        }

}

QVector <double> VampAnalyser::GetInitFramesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        if (fli->hasTimestamp) {
            Vamp::RealTime ftime0 = fli->timestamp;
            //double ltime0 = ftime0.sec + (double(ftime0.nsec)
            //        / 1000000000.0);
            vectout << (double) (Vamp::RealTime::realTime2Frame(ftime0, mRate))*2;
        }

    }
    return vectout;
}

QVector <double> VampAnalyser::GetEndFramesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        if (fli->hasDuration) {
            Vamp::RealTime ftime0 = fli->timestamp;
            Vamp::RealTime ftime1 = ftime0;
            ftime1 = ftime0 + fli->duration;
            //double ltime1 = ftime1.sec + (double(ftime1.nsec)
            //        / 1000000000.0);
            vectout << (double) (Vamp::RealTime::realTime2Frame(ftime1, mRate))*2;
        }

    }
    return vectout;

}

QVector<QString> VampAnalyser::GetLabelsVector() {
    QVector<QString> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {

        vectout << fli->label.c_str();
    }
    return vectout;

}


QVector <double> VampAnalyser::GetFirstValuesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        std::vector<float> vec = fli->values;
        if (!vec.empty())
            vectout << vec[0];

    }
    return vectout;

}

QVector <double> VampAnalyser::GetLastValuesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin(); fli
            != m_Results.end(); ++fli) {
        std::vector<float> vec = fli->values;
        if (!vec.empty())
            vectout << vec[vec.size() - 1];

    }
    return vectout;

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
