#include "analyzerkey.h"

#include <QtDebug>
#include <QVector>

#include "analyzer/plugins/analyzerqueenmarykey.h"
#include "proto/keys.pb.h"
#include "track/keyfactory.h"

// static
QList<AnalyzerPluginInfo> AnalyzerKey::availablePlugins() {
    QList<AnalyzerPluginInfo> analyzers;
    analyzers.push_back(AnalyzerQueenMaryKey::pluginInfo());
    return analyzers;
}

AnalyzerKey::AnalyzerKey(KeyDetectionSettings keySettings)
        : m_keySettings(keySettings),
          m_iSampleRate(0),
          m_iTotalSamples(0),
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
    m_pluginId = m_keySettings.getKeyPluginId();

    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;

    // if we can't load a stored track reanalyze it
    bool bShouldAnalyze = !isDisabledOrLoadStoredSuccess(tio);

    if (bShouldAnalyze) {
        if (m_pluginId == AnalyzerQueenMaryKey::pluginInfo().id) {
            m_pPlugin.reset(new AnalyzerQueenMaryKey());
        } else {
            // Default to our built-in key detector.
            m_pPlugin.reset(new AnalyzerQueenMaryKey());
        }
        bShouldAnalyze = m_pPlugin->initialize(sampleRate);
    }

    if (bShouldAnalyze) {
        qDebug() << "Key calculation started with plugin" << m_pluginId;
    } else {
        qDebug() << "Key calculation will not start.";
        m_pPlugin.reset();
    }

    return bShouldAnalyze;
}

bool AnalyzerKey::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
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
            return true;
        } else if (m_bPreferencesReanalyzeEnabled) {
            return false;
        } else {
            qDebug() << "Track has previous key detection result that is not up"
                     << "to date with latest settings but user preferences"
                     << "indicate we should not re-analyze it.";
            return true;
        }
    } else {
        // If we got here, we want to analyze this track.
        return false;
    }
}

void AnalyzerKey::process(const CSAMPLE *pIn, const int iLen) {
    if (m_pPlugin.isNull()) {
        return;
    }
    bool success = m_pPlugin->process(pIn, iLen);
    if (!success) {
        m_pPlugin.reset();
    }
}

void AnalyzerKey::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    m_pPlugin.reset();
}

void AnalyzerKey::finalize(TrackPointer tio) {
    if (m_pPlugin.isNull()) {
        return;
    }

    bool success = m_pPlugin->finalize();
    qDebug() << "Key Detection" << (success ? "complete" : "failed");

    if (!success) {
        m_pPlugin.reset();
        return;
    }

    KeyChangeList key_changes = m_pPlugin->getKeyChanges();
    m_pPlugin.reset();

    QHash<QString, QString> extraVersionInfo = getExtraVersionInfo(
        m_pluginId, m_bPreferencesFastAnalysisEnabled);
    Keys track_keys = KeyFactory::makePreferredKeys(
        key_changes, extraVersionInfo,
        m_iSampleRate, m_iTotalSamples);
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
