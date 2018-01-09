#include "analyzer/vamp/vampanalyzer.h"


VampAnalyzer::VampAnalyzer()
    : m_iOutput(0),
      m_iBlockSize(0),
      m_iStepSize(0),
      m_iSampleCount(0),
      m_iOUT(0),
      m_iRemainingSamples(0),
      m_rate(0),
      m_bDoNotAnalyseMoreSamples(false),
      m_FastAnalysisEnabled(false),
      m_iMaxSamplesToAnalyse(0) {
    m_pluginbuf[0] = nullptr;
    m_pluginbuf[1] = nullptr;
}

VampAnalyzer::~VampAnalyzer() {
    delete[] m_pluginbuf[0];
    delete[] m_pluginbuf[1];
}

bool VampAnalyzer::Init(const QString pluginlibrary, const QString pluginId,
                        const int samplerate, const int totalSamples, bool bFastAnalysis) {
    if (samplerate <= 0.0) {
        qWarning() << "VampAnalyzer: Track has non-positive samplerate" << samplerate;
        return false;
    }

    if (totalSamples <= 0) {
        qWarning() << "VampAnalyzer: Track has non-positive # of samples" << totalSamples;
        return false;
    }

    QStringList pluginList = pluginId.split(":");
    if (pluginList.size() != 2) {
        qWarning() << "VampAnalyzer: got malformed pluginId: " << pluginId;
        return false;
    }

    bool isOutputNumber = false;
    int outputNumber = pluginList.at(1).toInt(&isOutputNumber);
    if (!isOutputNumber) {
        qWarning() << "VampAnalyzer: got malformed pluginId: " << pluginId;
        return false;
    }

    const auto pluginKey =
            mixxx::VampPluginAdapter::composePluginKey(
                    pluginlibrary.toStdString(),
                    pluginList.at(0).toStdString());
    m_pluginAdapter.loadPlugin(
            pluginKey,
            samplerate,
            Vamp::HostExt::PluginLoader::ADAPT_ALL_SAFE);
    if (!m_pluginAdapter) {
        qWarning() << "VampAnalyzer: Cannot load Vamp Plug-in.";
        qWarning() << "Please copy libmixxxminimal.so from build dir to one of the following:";
        std::vector<std::string> path = Vamp::PluginHostAdapter::getPluginPath();
        for (unsigned int i = 0; i < path.size(); i++) {
            qWarning() << QString::fromStdString(path[i]);
        }
        return false;
    }

    const auto outputs = m_pluginAdapter.getOutputDescriptors();
    if (outputs.empty()) {
        qWarning() << "VampAnalyzer: Plugin has no outputs!";
        return false;
    }
    if (outputNumber >= 0 && outputNumber < int(outputs.size())) {
        m_iOutput = outputNumber;
    } else {
        qWarning() << "VampAnalyzer: Invalid output number!";
        return false;
    }

    m_iBlockSize = m_pluginAdapter.getPreferredBlockSize();
    qDebug() << "VampAnalyzer BlockSize: " << m_iBlockSize;
    if (m_iBlockSize == 0) {
        // A plugin that can handle any block size may return 0. The final block
        // size will be set in the initialize() call. Since 0 means it is
        // accepting any size, 1024 should be good
        m_iBlockSize = 1024;
        qDebug() << "VampAnalyzer: setting block size to" << m_iBlockSize;
    }

    m_iStepSize = m_pluginAdapter.getPreferredStepSize();
    qDebug() << "VampAnalyzer StepSize: " << m_iStepSize;
    if (m_iStepSize == 0 || m_iStepSize > m_iBlockSize) {
        // A plugin may return 0 if it has no particular interest in the step
        // size. In this case, the host should make the step size equal to the
        // block size if the plugin is accepting input in the time domain. If
        // the plugin is accepting input in the frequency domain, the host may
        // use any step size. The final step size will be set in the
        // initialize() call.
        m_iStepSize = m_iBlockSize;
        qDebug() << "VampAnalyzer: setting step size to" << m_iStepSize;
    }

    if (!m_pluginAdapter.initialise(2, m_iStepSize, m_iBlockSize)) {
        qWarning() << "VampAnalyzer: Cannot initialize plugin";
        return false;
    }

    m_iRemainingSamples = totalSamples;
    m_rate = samplerate;

    // Here we are using m_iBlockSize: it cannot be 0
    delete[] m_pluginbuf[0];
    delete[] m_pluginbuf[1];
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
    if (!m_pluginAdapter) {
        qWarning() << "VampAnalyzer: Plugin not loaded";
        return false;
    }

    if (m_pluginbuf[0] == NULL || m_pluginbuf[1] == NULL) {
        qWarning() << "VampAnalyzer: Buffer points to NULL";
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
                    m_pluginAdapter.process(m_pluginbuf, timestamp);

            m_results.insert(m_results.end(), features[m_iOutput].begin(),
                             features[m_iOutput].end());

            if (lastsamples) {
                Vamp::Plugin::FeatureSet features =
                        m_pluginAdapter.getRemainingFeatures();
                m_results.insert(m_results.end(), features[m_iOutput].begin(),
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
        Vamp::Plugin::FeatureSet features =
                m_pluginAdapter.getRemainingFeatures();
        m_results.insert(m_results.end(), features[m_iOutput].begin(),
                         features[m_iOutput].end());
    }
    // Clearing buffer arrays
    for (int i = 0; i < 2; i++) {
        delete[] m_pluginbuf[i];
        m_pluginbuf[i] = nullptr;
    }
    return true;
}

bool VampAnalyzer::SetParameter(const QString parameter, const double value) {
    Q_UNUSED(parameter);
    Q_UNUSED(value);
    return true;
}

QVector<double> VampAnalyzer::GetInitFramesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_results.begin();
         fli != m_results.end(); ++fli) {
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
    for (Vamp::Plugin::FeatureList::iterator fli = m_results.begin();
         fli != m_results.end(); ++fli) {
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
    for (Vamp::Plugin::FeatureList::iterator fli = m_results.begin();
         fli != m_results.end(); ++fli) {
        vectout << fli->label.c_str();
    }
    return vectout;
}

QVector<double> VampAnalyzer::GetFirstValuesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_results.begin();
         fli != m_results.end(); ++fli) {
        std::vector<float> vec = fli->values;
        if (!vec.empty())
            vectout << vec[0];
    }
    return vectout;
}

QVector<double> VampAnalyzer::GetLastValuesVector() {
    QVector<double> vectout;
    for (Vamp::Plugin::FeatureList::iterator fli = m_results.begin();
         fli != m_results.end(); ++fli) {
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
