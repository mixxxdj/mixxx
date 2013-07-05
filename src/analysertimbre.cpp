#include <QtDebug>
#include <QVector>
#include <QHash>

#include "analysertimbre.h"
#include "track/timbre_preferences.h"
#include "track/timbrefactory.h"


AnalyserTimbre::AnalyserTimbre(ConfigObject<ConfigValue> *pConfig)
    : m_pConfig(pConfig),
      m_pVamp(NULL),
      m_iSampleRate(0),
      m_iTotalSamples(0),
      m_bShouldAnalyze(false),
      m_bPreferencesFastAnalysis(false) {
}
AnalyserTimbre::~AnalyserTimbre() {
}

bool AnalyserTimbre::initialise(TrackPointer tio, int sampleRate, int totalSamples) {
    m_bShouldAnalyze = false;
    if (totalSamples == 0) {
        return false;
    }
    bool bPreferencesTimbreAnalysisEnabled = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey(TIMBRE_CONFIG_KEY, TIMBRE_ANALYSIS_ENABLED)).toInt());

    if (!bPreferencesTimbreAnalysisEnabled) {
        qDebug() << "Timbre analysis is deactivated";
        m_bShouldAnalyze = false;
        return false;
    }
    QString library = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_PLUGIN_ID));

    m_pluginId = pluginID;
    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;

    m_bShouldAnalyze = !checkStoredTimbre(tio);

    if (!m_bShouldAnalyze) {
        qDebug() << "Timbre model calculation will not start.";
        return false;
    }

    qDebug() << "Timbre analysis started with plugin" << pluginID;

    m_pVamp = new VampAnalyser(m_pConfig);
    m_bShouldAnalyze = m_pVamp->Init(library, pluginID, m_iSampleRate, totalSamples,
                                     m_bPreferencesFastAnalysis);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
    return m_bShouldAnalyze;
}

void AnalyserTimbre::process(const CSAMPLE *pIn, const int iLen) {
    if (!m_bShouldAnalyze || m_pVamp == NULL)
        return;
    m_bShouldAnalyze = m_pVamp->Process(pIn, iLen);
    if (!m_bShouldAnalyze) {
        delete m_pVamp;
        m_pVamp = NULL;
    }
}
bool AnalyserTimbre::loadStored(TrackPointer tio) const {
    QString library = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
        ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYSER_TIMBRE_PLUGIN_ID));
    if (library.isEmpty() || library.isNull())
        library = "libmixxxminimal";

    if (pluginID.isEmpty() || pluginID.isNull())
        pluginID = VAMP_ANALYSER_TIMBRE_DEFAULT_PLUGIN_ID;

    return checkStoredTimbre(tio);
}

void AnalyserTimbre::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    delete m_pVamp;
    m_pVamp = NULL;
}
void AnalyserTimbre::finalise(TrackPointer tio) {
    if (!m_bShouldAnalyze || m_pVamp == NULL) {
        return;
    }

    // Call End() here, because the number of total samples may have been
    // estimated incorrectly.
    bool success = m_pVamp->End();
    qDebug() << "Timbre analysis" << (success ? "complete" : "failed");

    QVector<double> timbreVector = m_pVamp->GetValuesVector();
    delete m_pVamp;
    m_pVamp = NULL;

    qDebug() << "Timbre vector size:" << timbreVector.size();

    QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
        m_pluginId, m_bPreferencesFastAnalysis);

    TimbrePointer pTimbre = TimbreFactory::makePreferredTimbreModel(tio,
                                                                    timbreVector,
                                                                    extraVersionInfo,
                                                                    m_iSampleRate,
                                                                    m_iTotalSamples);
    tio->setTimbre(pTimbre);
}

QHash<QString, QString> AnalyserTimbre::getExtraVersionInfo(
    QString pluginId, bool bPreferencesFastAnalysis) {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}

bool AnalyserTimbre::checkStoredTimbre(TrackPointer tio) const {
    bool isStored = false;
    TimbrePointer pTimbre = tio->getTimbre();
    if (pTimbre) {
        QString version = pTimbre->getVersion();
        QString subVersion = pTimbre->getSubVersion();
        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            m_pluginId, m_bPreferencesFastAnalysis);
        QString newVersion = TimbreFactory::getPreferredVersion();
        QString newSubVersion = TimbreFactory::getPreferredSubVersion(extraVersionInfo);

        if (version == newVersion && subVersion == newSubVersion) {
            qDebug() << "Timbre version/sub-version unchanged since previous analysis. Not analyzing.";
            isStored = true;
        } else {
            qDebug() << "Track has previous timbre model that is not up"
                     << "to date with latest settings, but user preferences"
                     << "indicate we should not re-analyze it.";
            isStored = true;
        }
    } else {
        // If we got here, we want to analyze this track.
        isStored = false;
    }
    return isStored;
}
