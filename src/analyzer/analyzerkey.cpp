#include "analyzer/analyzerkey.h"

#include <QtDebug>

#include "analyzer/analyzertrack.h"
#include "analyzer/constants.h"
#if defined __KEYFINDER__
#include "analyzer/plugins/analyzerkeyfinder.h"
#endif
#include "analyzer/plugins/analyzerqueenmarykey.h"
#include "proto/keys.pb.h"
#include "track/keyfactory.h"
#include "track/track.h"

// static
QList<mixxx::AnalyzerPluginInfo> AnalyzerKey::availablePlugins() {
    QList<mixxx::AnalyzerPluginInfo> analyzers;
    // First one below is the default
    analyzers.push_back(mixxx::AnalyzerQueenMaryKey::pluginInfo());
#if defined __KEYFINDER__
    analyzers.push_back(mixxx::AnalyzerKeyFinder::pluginInfo());
#endif
    return analyzers;
}

// static
mixxx::AnalyzerPluginInfo AnalyzerKey::defaultPlugin() {
    const auto plugins = availablePlugins();
    DEBUG_ASSERT(!plugins.isEmpty());
    return plugins.at(0);
}

AnalyzerKey::AnalyzerKey(const KeyDetectionSettings& keySettings)
        : m_keySettings(keySettings),
          m_sampleRate(0),
          m_totalFrames(0),
          m_maxFramesToProcess(0),
          m_currentFrame(0),
          m_bPreferencesKeyDetectionEnabled(true),
          m_bPreferencesFastAnalysisEnabled(false),
          m_bPreferencesReanalyzeEnabled(false) {
}

bool AnalyzerKey::initialize(const AnalyzerTrack& track,
        mixxx::audio::SampleRate sampleRate,
        SINT frameLength) {
    if (frameLength <= 0) {
        return false;
    }

    m_bPreferencesKeyDetectionEnabled = m_keySettings.getKeyDetectionEnabled();
    if (!m_bPreferencesKeyDetectionEnabled) {
        qDebug() << "Key detection is deactivated";
        return false;
    }

    m_bPreferencesFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    m_bPreferencesReanalyzeEnabled = m_keySettings.getReanalyzeWhenSettingsChange();

    const auto plugins = availablePlugins();
    if (!plugins.isEmpty()) {
        m_pluginId = defaultPlugin().id();
        QString pluginId = m_keySettings.getKeyPluginId();
        for (const auto& info : plugins) {
            if (info.id() == pluginId) {
                m_pluginId = pluginId; // configured Plug-In available
                break;
            }
        }
    }

    qDebug() << "AnalyzerKey preference settings:"
             << "\nPlugin:" << m_pluginId
             << "\nRe-analyze when settings change:" << m_bPreferencesReanalyzeEnabled
             << "\nFast analysis:" << m_bPreferencesFastAnalysisEnabled;

    m_sampleRate = sampleRate;
    m_totalFrames = frameLength;
    // In fast analysis mode, skip processing after
    // kFastAnalysisSecondsToAnalyze seconds are analyzed.
    if (m_bPreferencesFastAnalysisEnabled) {
        m_maxFramesToProcess = mixxx::kFastAnalysisSecondsToAnalyze * m_sampleRate;
    } else {
        m_maxFramesToProcess = frameLength;
    }
    m_currentFrame = 0;

    // if we can't load a stored track reanalyze it
    bool bShouldAnalyze = shouldAnalyze(track.getTrack());

    DEBUG_ASSERT(!m_pPlugin);
    if (bShouldAnalyze) {
        if (m_pluginId == mixxx::AnalyzerQueenMaryKey::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerQueenMaryKey>();
#if defined __KEYFINDER__
        } else if (m_pluginId == mixxx::AnalyzerKeyFinder::pluginInfo().id()) {
            m_pPlugin = std::make_unique<mixxx::AnalyzerKeyFinder>();
#endif
        } else {
            // This must not happen, because we have already verified above
            // that the PlugInId is valid
            DEBUG_ASSERT(false);
        }

        if (m_pPlugin) {
            if (m_pPlugin->initialize(mixxx::audio::SampleRate(m_sampleRate))) {
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

bool AnalyzerKey::shouldAnalyze(TrackPointer pTrack) const {
    bool bPreferencesFastAnalysisEnabled = m_keySettings.getFastAnalysis();
    QString pluginID = m_keySettings.getKeyPluginId();
    if (pluginID.isEmpty()) {
        pluginID = defaultPlugin().id();
    }

    const Keys keys = pTrack->getKeys();
    if (keys.getGlobalKey() != mixxx::track::io::key::INVALID) {
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

bool AnalyzerKey::processSamples(const CSAMPLE* pIn, SINT count) {
    VERIFY_OR_DEBUG_ASSERT(m_pPlugin) {
        return false;
    }

    m_currentFrame += count / mixxx::kAnalysisChannels;
    if (m_currentFrame > m_maxFramesToProcess) {
        return true; // silently ignore remaining samples
    }

    return m_pPlugin->processSamples(pIn, count);
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
            key_changes, extraVersionInfo, m_sampleRate, m_totalFrames);
    tio->setKeys(track_keys);
}

// static
QHash<QString, QString> AnalyzerKey::getExtraVersionInfo(
        const QString& pluginId, bool bPreferencesFastAnalysis) {
    QHash<QString, QString> extraVersionInfo;
    extraVersionInfo["vamp_plugin_id"] = pluginId;
    if (bPreferencesFastAnalysis) {
        extraVersionInfo["fast_analysis"] = "1";
    }
    return extraVersionInfo;
}
