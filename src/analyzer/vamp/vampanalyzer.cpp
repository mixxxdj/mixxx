/*
 * vampanalyzer.cpp
 *
 *  Created on: 14/mar/2011
 *      Author: Vittorio Colao
 */
#include "analyzer/vamp/vampanalyzer.h"

#include <QDir>
#include <QtDebug>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QStringList>

#ifdef __WINDOWS__
    #include <windows.h>
    #define PATH_SEPARATOR ";"
#else
    #define PATH_SEPARATOR ":"
#endif

using Vamp::Plugin;
using Vamp::PluginHostAdapter;

void VampAnalyzer::initializePluginPaths() {
    const char* pVampPath = getenv("VAMP_PATH");
    QString vampPath = "";
    if (pVampPath) {
        vampPath = QString(pVampPath);
    }

    // TODO(XXX) use correct split separator here.
    QStringList pathElements = vampPath.length() > 0 ? vampPath.split(PATH_SEPARATOR)
            : QStringList();

    const QString dataLocation = QDesktopServices::storageLocation(
            QDesktopServices::DataLocation);
    const QString applicationPath = QCoreApplication::applicationDirPath();

#ifdef __WINDOWS__
    QDir winVampPath(applicationPath);
    if (winVampPath.cd("plugins") && winVampPath.cd("vamp")) {
        pathElements << winVampPath.absolutePath().replace("/","\\");
    } else {
        qDebug() << winVampPath.absolutePath() << "does not exist!";
    }
#elif __APPLE__
    // Location within the OS X bundle that we store plugins.
    // blah/Mixxx.app/Contents/MacOS/
    QDir bundlePluginDir(applicationPath);
    if (bundlePluginDir.cdUp() && bundlePluginDir.cd("PlugIns")) {
        pathElements << bundlePluginDir.absolutePath();
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("osx32_build") && developer32Root.cd("vamp-plugins")) {
        pathElements << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("osx64_build") && developer64Root.cd("vamp-plugins")) {
        pathElements << developer64Root.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("Plugins") && dataPluginDir.cd("vamp")) {
        pathElements << dataPluginDir.absolutePath();
    }
#elif __LINUX__
    QDir libPath(UNIX_LIB_PATH);
    if (libPath.cd("plugins") && libPath.cd("vamp")) {
        pathElements << libPath.absolutePath();
    }

    QDir dataPluginDir(dataLocation);
    if (dataPluginDir.cd("plugins") && dataPluginDir.cd("vamp")) {
        pathElements << dataPluginDir.absolutePath();
    }

    // For people who build from source.
    QDir developer32Root(applicationPath);
    if (developer32Root.cd("lin32_build") && developer32Root.cd("vamp-plugins")) {
        pathElements << developer32Root.absolutePath();
    }
    QDir developer64Root(applicationPath);
    if (developer64Root.cd("lin64_build") && developer64Root.cd("vamp-plugins")) {
        pathElements << developer64Root.absolutePath();
    }
#endif

    QString newPath = pathElements.join(PATH_SEPARATOR);
    qDebug() << "Setting VAMP_PATH to: " << newPath;
    QByteArray newPathBA = newPath.toLocal8Bit();
#ifndef __WINDOWS__
    setenv("VAMP_PATH", newPathBA.constData(), 1);
#else
    QString winpath = "VAMP_PATH=" + newPath;
    QByteArray winpathBA = winpath.toLocal8Bit();
    putenv(winpathBA.constData());
#endif
}

VampAnalyzer::VampAnalyzer()
    : m_iSampleCount(0),
      m_iOUT(0),
      m_iRemainingSamples(0),
      m_iBlockSize(0),
      m_iStepSize(0),
      m_rate(0),
      m_iOutput(0),
      m_pluginbuf(new CSAMPLE*[2]),
      m_plugin(NULL),
      m_bDoNotAnalyseMoreSamples(false),
      m_FastAnalysisEnabled(false),
      m_iMaxSamplesToAnalyse(0) {
}

VampAnalyzer::~VampAnalyzer() {
    delete[] m_pluginbuf;
    delete m_plugin;
}

bool VampAnalyzer::Init(const QString pluginlibrary, const QString pluginid,
                        const int samplerate, const int TotalSamples, bool bFastAnalysis) {
    m_iRemainingSamples = TotalSamples;
    m_rate = samplerate;

    if (samplerate <= 0.0) {
        qDebug() << "VampAnalyzer: Track has non-positive samplerate";
        return false;
    }

    if (TotalSamples <= 0) {
        qDebug() << "VampAnalyzer: Track has non-positive # of samples";
        return false;
    }

    if (m_plugin != NULL) {
        delete m_plugin;
        m_plugin = NULL;
        qDebug() << "VampAnalyzer: kill plugin";
    }

    VampPluginLoader *loader = VampPluginLoader::getInstance();
    QStringList pluginlist = pluginid.split(":");
    if (pluginlist.size() != 2) {
        qDebug() << "VampAnalyzer: got malformed pluginid: " << pluginid;
        return false;
    }

    bool isNumber = false;
    int outputnumber = (pluginlist.at(1)).toInt(&isNumber);
    if (!isNumber) {
        qDebug() << "VampAnalyzer: got malformed pluginid: " << pluginid;
        return false;
    }

    QString plugin = pluginlist.at(0);
    m_key = loader->composePluginKey(pluginlibrary.toStdString(),
                                     plugin.toStdString());
    m_plugin = loader->loadPlugin(m_key, m_rate,
                                  Vamp::HostExt::PluginLoader::ADAPT_ALL_SAFE);

    if (!m_plugin) {
        qDebug() << "VampAnalyzer: Cannot load Vamp Plug-in.";
        qDebug() << "Please copy libmixxxminimal.so from build dir to one of the following:";

        std::vector<std::string> path = PluginHostAdapter::getPluginPath();
        for (unsigned int i = 0; i < path.size(); i++) {
            qDebug() << QString::fromStdString(path[i]);
        }
        return false;
    }
    Plugin::OutputList outputs = m_plugin->getOutputDescriptors();
    if (outputs.empty()) {
        qDebug() << "VampAnalyzer: Plugin has no outputs!";
        return false;
    }
    SelectOutput(outputnumber);

    m_iBlockSize = m_plugin->getPreferredBlockSize();
    qDebug() << "Vampanalyzer BlockSize: " << m_iBlockSize;
    if (m_iBlockSize == 0) {
        // A plugin that can handle any block size may return 0. The final block
        // size will be set in the initialize() call. Since 0 means it is
        // accepting any size, 1024 should be good
        m_iBlockSize = 1024;
        qDebug() << "Vampanalyzer: setting m_iBlockSize to 1024";
    }

    m_iStepSize = m_plugin->getPreferredStepSize();
    qDebug() << "Vampanalyzer StepSize: " << m_iStepSize;
    if (m_iStepSize == 0 || m_iStepSize > m_iBlockSize) {
        // A plugin may return 0 if it has no particular interest in the step
        // size. In this case, the host should make the step size equal to the
        // block size if the plugin is accepting input in the time domain. If
        // the plugin is accepting input in the frequency domain, the host may
        // use any step size. The final step size will be set in the
        // initialize() call.
        m_iStepSize = m_iBlockSize;
        qDebug() << "Vampanalyzer: setting m_iStepSize to" << m_iStepSize;
    }

    if (!m_plugin->initialise(2, m_iStepSize, m_iBlockSize)) {
        qDebug() << "VampAnalyzer: Cannot initialize plugin";
        return false;
    }
    // Here we are using m_iBlockSize: it cannot be 0
    m_pluginbuf[0] = new CSAMPLE[m_iBlockSize];
    m_pluginbuf[1] = new CSAMPLE[m_iBlockSize];
    m_FastAnalysisEnabled = bFastAnalysis;
    if (m_FastAnalysisEnabled) {
        qDebug() << "Using fast analysis methods for BPM and Replay Gain.";
        m_iMaxSamplesToAnalyse = 120 * m_rate; //only consider the first minute
    }
    return true;
}

bool VampAnalyzer::Process(const CSAMPLE *pIn, const int iLen) {
    if (!m_plugin) {
        qDebug() << "VampAnalyzer: Plugin not loaded";
        return false;
    }

    if (m_pluginbuf[0] == NULL || m_pluginbuf[1] == NULL) {
        qDebug() << "VampAnalyzer: Buffer points to NULL";
        return false;
    }

    if (m_bDoNotAnalyseMoreSamples) {
        return true;
    }

    int iIN = 0;
    bool lastsamples = false;
    m_iRemainingSamples -= iLen;

    while (iIN < iLen / 2) { //4096
        m_pluginbuf[0][m_iOUT] = pIn[2 * iIN]; //* 32767;
        m_pluginbuf[1][m_iOUT] = pIn[2 * iIN + 1]; //* 32767;

        m_iOUT++;
        iIN++;

        /*
         * Note 'm_iRemainingSamples' is initialized with
         * the number of total samples.
         * Thus, 'm_iRemainingSamples' will only become <= 0
         * if the number of total samples --which may be incorrect--
         * is correct.
         *
         * The following if-block works under optimal conditions
         * If the total number of samples is incorrect
         * VampAnalyzer:End() handles it.
         */
        if (m_iRemainingSamples <= 0 && iIN == iLen / 2) {
            lastsamples = true;
            //qDebug() << "LastSample reached";
            while (m_iOUT < m_iBlockSize) {
                m_pluginbuf[0][m_iOUT] = 0;
                m_pluginbuf[1][m_iOUT] = 0;
                m_iOUT++;
            }
        }
        if (m_iOUT == m_iBlockSize) { //Blocksize 1024
            //qDebug() << "VAMP Block size reached";
            //qDebug() << "Ramaining samples=" << m_iRemainingSamples;
            Vamp::RealTime timestamp =
                    Vamp::RealTime::frame2RealTime(m_iSampleCount, m_rate);

            Vamp::Plugin::FeatureSet features =
                    m_plugin->process(m_pluginbuf, timestamp);

            m_Results.insert(m_Results.end(), features[m_iOutput].begin(),
                             features[m_iOutput].end());

            if (lastsamples) {
                Vamp::Plugin::FeatureSet features =
                        m_plugin->getRemainingFeatures();
                m_Results.insert(m_Results.end(), features[m_iOutput].begin(),
                                 features[m_iOutput].end());
            }

            m_iSampleCount += m_iBlockSize;
            m_iOUT = 0;

            // The step size indicates preferred distance in sample frames
            // between successive calls to process(). To obey the step-size, we
            // move (m_iBlockSize - m_iStepSize) samples from m_iStepSize'th
            // position to 0.
            while (m_iOUT < (m_iBlockSize - m_iStepSize)) {
                CSAMPLE lframe = m_pluginbuf[0][m_iOUT + m_iStepSize];
                CSAMPLE rframe = m_pluginbuf[1][m_iOUT + m_iStepSize];
                m_pluginbuf[0][m_iOUT] = lframe;
                m_pluginbuf[1][m_iOUT] = rframe;
                m_iOUT++;
            }

            // If a track has a duration of more than our set limit, do not
            // analyse more.
            if (m_iMaxSamplesToAnalyse > 0 && m_iSampleCount >= m_iMaxSamplesToAnalyse) {
                m_bDoNotAnalyseMoreSamples = true;
                m_iRemainingSamples = 0;
            }
        }
    }
    return true;
}

bool VampAnalyzer::End() {
    // If the total number of samples has been estimated incorrectly
    if (m_iRemainingSamples > 0) {
        Vamp::Plugin::FeatureSet features = m_plugin->getRemainingFeatures();
        m_Results.insert(m_Results.end(), features[m_iOutput].begin(),
                         features[m_iOutput].end());
    }
    // Clearing buffer arrays
    for (int i = 0; i < 2; i++) {
        if (m_pluginbuf[i]) {
            delete [] m_pluginbuf[i];
            m_pluginbuf[i] = NULL;
        }
    }
    return true;
}

bool VampAnalyzer::SetParameter(const QString parameter, const double value) {
    Q_UNUSED(parameter);
    Q_UNUSED(value);
    return true;
}

void VampAnalyzer::SelectOutput(const int outputnumber) {
    Plugin::OutputList outputs = m_plugin->getOutputDescriptors();
    if (outputnumber >= 0 && outputnumber < int(outputs.size())) {
        m_iOutput = outputnumber;
    }
}

QVector<double> VampAnalyzer::GetInitFramesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin();
         fli != m_Results.end(); ++fli) {
        if (fli->hasTimestamp) {
            Vamp::RealTime ftime0 = fli->timestamp;
            //double ltime0 = ftime0.sec + (double(ftime0.nsec)
            //        / 1000000000.0);
            vectout << static_cast<double>(
                Vamp::RealTime::realTime2Frame(ftime0, m_rate));
        }
    }
    return vectout;
}

QVector<double> VampAnalyzer::GetEndFramesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin();
         fli != m_Results.end(); ++fli) {
        if (fli->hasDuration) {
            Vamp::RealTime ftime0 = fli->timestamp;
            Vamp::RealTime ftime1 = ftime0 + fli->duration;
            //double ltime1 = ftime1.sec + (double(ftime1.nsec)
            //        / 1000000000.0);
            vectout << static_cast<double>(
                Vamp::RealTime::realTime2Frame(ftime1, m_rate));
        }
    }
    return vectout;
}

QVector<QString> VampAnalyzer::GetLabelsVector() {
    QVector<QString> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin();
         fli != m_Results.end(); ++fli) {
        vectout << fli->label.c_str();
    }
    return vectout;
}

QVector<double> VampAnalyzer::GetFirstValuesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin();
         fli != m_Results.end(); ++fli) {
        std::vector<float> vec = fli->values;
        if (!vec.empty())
            vectout << vec[0];
    }
    return vectout;
}

QVector<double> VampAnalyzer::GetLastValuesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_Results.begin();
         fli != m_Results.end(); ++fli) {
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
