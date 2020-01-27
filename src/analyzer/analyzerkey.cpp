#include "analyzer/analyzerkey.h"

#include <QVector>
#include <QtDebug>

#include "analyzer/constants.h"
#include "analyzer/plugins/analyzerqueenmarykey.h"
#include "proto/keys.pb.h"
#include "track/keyfactory.h"

// static
QList<mixxx::AnalyzerPluginInfo> AnalyzerKey::availablePlugins() {
    QList<mixxx::AnalyzerPluginInfo> analyzers;
    // First one below is the default
    analyzers.push_back(mixxx::AnalyzerQueenMaryKey::pluginInfo());
    return analyzers;
}

AnalyzerKey::AnalyzerKey(KeyDetectionSettings keySettings)
        : m_keySettings(keySettings),
          m_iSampleRate(0),
          m_iTotalSamples(0),
          m_iMaxSamplesToProcess(0),
          m_iCurrentSample(0),
          m_bPreferencesKeyDetectionEnabled(true),
          m_bPreferencesFastAnalysisEnabled(false),
          m_bPreferencesReanalyzeEnabled(false) {
}

bool AnalyzerKey::initialize(TrackPointer tio, int sampleRate, int totalSamples) {
    if (totalSamples == 0) {
        return false;
    }

    m_bPreferencesKeyDetectionEnabled = m_keySettings.getKeyDetectionEnabled();
    if (!m_bPreferencesKeyDetectionEnabled) {
        qDebug() << "Key detection is deactivated";
        return false;
    }

    m_bPreferencesFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    m_bPreferencesReanalyzeEnabled = m_keySettings.getReanalyzeWhenSettingsChange();

    if (AnalyzerKey::availablePlugins().size() > 0) {
        m_pluginId = AnalyzerKey::availablePlugins().at(0).id; // first is default
        QString pluginId = m_keySettings.getKeyPluginId();
        for (const auto& info : AnalyzerKey::availablePlugins()) {
            if (info.id == pluginId) {
                m_pluginId = pluginId; // configured Plug-In available
                break;
            }
        }
    }

    qDebug() << "AnalyzerKey preference settings:"
             << "\nPlugin:" << m_pluginId
             << "\nRe-analyze when settings change:" << m_bPreferencesReanalyzeEnabled
             << "\nFast analysis:" << m_bPreferencesFastAnalysisEnabled;

    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;
    // In fast analysis mode, skip processing after
    // kFastAnalysisSecondsToAnalyze seconds are analyzed.
    if (m_bPreferencesFastAnalysisEnabled) {
        m_iMaxSamplesToProcess = mixxx::kFastAnalysisSecondsToAnalyze * m_iSampleRate * mixxx::kAnalysisChannels;
    } else {
        m_iMaxSamplesToProcess = m_iTotalSamples;
    }
    m_iCurrentSample = 0;

    // if we can't load a stored track reanalyze it
    bool bShouldAnalyze = shouldAnalyze(tio);

    DEBUG_ASSERT(!m_pPlugin);
    if (bShouldAnalyze) {
        if (m_pluginId == mixxx::AnalyzerQueenMaryKey::pluginInfo().id) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryKey>();
        } else {
            // This must not happen, because we have already verified above
            // that the PlugInId is valid
            DEBUG_ASSERT(false);
        }

        if (m_pPlugin) {
            if (m_pPlugin->initialize(sampleRate)) {
                qDebug() << "Key calculation started with plugin" << m_pluginId;
            } else {
                qDebug() << "Key calculation will not start.";
                m_pPlugin.reset();
                bShouldAnalyze = false;
            }
        } else {
            bShouldAnalyze = false;
        }
    }
    return bShouldAnalyze;
}

bool AnalyzerKey::shouldAnalyze(TrackPointer tio) const {
    bool bPreferencesFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    QString pluginID = m_keySettings.getKeyPluginId();

    const Keys keys(tio->getKeys());
    if (keys.isValid()) {
        QString version = keys.getVersion();
        QString subVersion = keys.getSubVersion();

        QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
                pluginID, bPreferencesFastAnalysisEnabled);
        QString newVersion = KeyFactory::getPreferredVersion();
        QString newSubVersion = KeyFactory::getPreferredSubVersion(extraVersionInfo);

        if (version == newVersion && subVersion == newSubVersion) {
            // If the version and settings have not changed then if the world is
            // sane, re-analyzing will do nothing.
            qDebug() << "Keys version/sub-version unchanged since previous analysis. Not analyzing.";
            return false;
        }
        if (!m_bPreferencesReanalyzeEnabled) {
            qDebug() << "Track has previous key detection result that is not up"
                     << "to date with latest settings but user preferences"
                     << "indicate we should not re-analyze it.";
            return false;
        }
    }
    return true;
}

bool AnalyzerKey::processSamples(const CSAMPLE *pIn, const int iLen) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return false;
    }

    m_iCurrentSample += iLen;
    if (m_iCurrentSample > m_iMaxSamplesToProcess) {
        return true; // silently ignore remaining samples
    }

    return m_pPlugin->processSamples(pIn, iLen);
}

void AnalyzerKey::cleanup() {
    m_pPlugin.reset();
}

void AnalyzerKey::storeResults(TrackPointer tio) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return;
    }

    if (!m_pPlugin->finalize()) {
        qWarning() << "Key detection failed";
        return;
    }

    KeyChangeList key_changes = m_pPlugin->getKeyChanges();
    QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
            m_pluginId, m_bPreferencesFastAnalysisEnabled);
    Keys track_keys = KeyFactory::makePreferredKeys(
            key_changes, extraVersionInfo, m_iSampleRate, m_iTotalSamples);
    tio->setKeys(track_keys);
}

// static
QHash<QString, QString> AnalyzerKey::getExtraVersionInfo(
        QString pluginId, bool bPreferencesFastAnalysis) {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}
