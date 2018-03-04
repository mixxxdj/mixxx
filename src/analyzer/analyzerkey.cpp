#include "analyzerkey.h"

#include <QtDebug>
#include <QVector>

#include "proto/keys.pb.h"
#include "track/key_preferences.h"
#include "track/keyfactory.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;

AnalyzerKey::AnalyzerKey(UserSettingsPointer pConfig)
        : m_pConfig(pConfig),
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

    m_bPreferencesKeyDetectionEnabled = m_pConfig->getValue<bool>(
            ConfigKey(KEY_CONFIG_KEY, KEY_DETECTION_ENABLED));
    if (!m_bPreferencesKeyDetectionEnabled) {
        qDebug() << "Key detection is deactivated";
        return false;
    }
    m_bPreferencesFastAnalysisEnabled = m_pConfig->getValue<bool>(
            ConfigKey(KEY_CONFIG_KEY, KEY_FAST_ANALYSIS));
    m_bPreferencesReanalyzeEnabled = m_pConfig->getValue<bool>(
            ConfigKey(KEY_CONFIG_KEY, KEY_REANALYZE_WHEN_SETTINGS_CHANGE));

    QString library = m_pConfig->getValue(
            ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYZER_KEY_LIBRARY),
            // TODO(rryan) this default really doesn't belong here.
            "libmixxxminimal");
    QString pluginID = m_pConfig->getValue(
            ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYZER_KEY_PLUGIN_ID),
            // TODO(rryan) this default really doesn't belong here.
            VAMP_ANALYZER_KEY_DEFAULT_PLUGIN_ID);

    m_pluginId = pluginID;
    m_iSampleRate = sampleRate;
    m_iTotalSamples = totalSamples;

    // if we can't load a stored track reanalyze it
    bool bShouldAnalyze = !isDisabledOrLoadStoredSuccess(tio);

    if (bShouldAnalyze) {
        m_pVamp = std::make_unique<VampAnalyzer>();
        bShouldAnalyze = m_pVamp->Init(
            library, m_pluginId, sampleRate, totalSamples,
            m_bPreferencesFastAnalysisEnabled);
        if (!bShouldAnalyze) {
            m_pVamp.reset();
        }
    }

    if (bShouldAnalyze) {
        qDebug() << "Key calculation started with plugin" << m_pluginId;
    } else {
        qDebug() << "Key calculation will not start.";
    }

    return bShouldAnalyze;
}

bool AnalyzerKey::isDisabledOrLoadStoredSuccess(TrackPointer tio) const {
    bool bPreferencesFastAnalysisEnabled = m_pConfig->getValue<bool>(
            ConfigKey(KEY_CONFIG_KEY, KEY_FAST_ANALYSIS));

    QString library = m_pConfig->getValueString(
            ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYZER_KEY_LIBRARY));
    QString pluginID = m_pConfig->getValueString(
            ConfigKey(VAMP_CONFIG_KEY, VAMP_ANALYZER_KEY_PLUGIN_ID));

    // TODO(rryan): This belongs elsewhere.
    if (library.isEmpty() || library.isNull()) {
        library = "libmixxxminimal";
    }

    // TODO(rryan): This belongs elsewhere.
    if (pluginID.isEmpty() || pluginID.isNull()) {
        pluginID = VAMP_ANALYZER_KEY_DEFAULT_PLUGIN_ID;
    }

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
    if (!m_pVamp) {
        return;
    }
    bool success = m_pVamp->Process(pIn, iLen);
    if (!success) {
        m_pVamp.reset();
    }
}

void AnalyzerKey::cleanup(TrackPointer tio) {
    Q_UNUSED(tio);
    m_pVamp.reset();
}

void AnalyzerKey::finalize(TrackPointer tio) {
    if (!m_pVamp) {
        return;
    }

    bool success = m_pVamp->End();
    qDebug() << "Key Detection" << (success ? "complete" : "failed");

    QVector<double> frames = m_pVamp->GetInitFramesVector();
    QVector<double> keys = m_pVamp->GetLastValuesVector();
    m_pVamp.reset();

    if (frames.size() == 0 || frames.size() != keys.size()) {
        qWarning() << "AnalyzerKey: Key sequence and list of times do not match.";
        return;
    }

    KeyChangeList key_changes;
    for (int i = 0; i < keys.size(); ++i) {
        if (ChromaticKey_IsValid(keys[i])) {
            key_changes.push_back(qMakePair(
                // int() intermediate cast required by MSVC.
                static_cast<ChromaticKey>(int(keys[i])), frames[i]));
        }
    }

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
