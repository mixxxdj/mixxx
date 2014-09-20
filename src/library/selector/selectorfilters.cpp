#include <QStringList>
#include <QStringBuilder>

#include "trackinfoobject.h"
#include "track/timbre.h"
#include "track/keyutils.h"

#include "library/selector/selector_preferences.h"

#include "library/selector/selectorfilters.h"

using mixxx::track::io::key::ChromaticKey;
using mixxx::track::io::key::ChromaticKey_IsValid;


SelectorFilters::SelectorFilters(QObject* pParent,
                                 ConfigObject<ConfigValue>* pConfig)
        : QObject(pParent),
        m_pConfig(pConfig) {
    loadStoredFilterSettings();
}

SelectorFilters::~SelectorFilters() {
}

void SelectorFilters::loadStoredFilterSettings() {
    m_bFilterGenre = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_GENRE)).toInt());
    m_bFilterBpm = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM)).toInt());
    m_iFilterBpmRange = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_BPM_RANGE)).toInt();
    m_bFilterKey = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY)).toInt());
    m_bFilterKey4th = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_4TH)).toInt());
    m_bFilterKey5th = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_5TH)).toInt());
    m_bFilterKeyRelative = static_cast<bool>(m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, FILTER_KEY_RELATIVE)).toInt());
}

void SelectorFilters::clear() {
    m_bFilterGenre = false;
    m_bFilterBpm = false;
    m_iFilterBpmRange = 0;
    m_bFilterKey = false;
    m_bFilterKey4th = false;
    m_bFilterKey5th = false;
    m_bFilterKeyRelative = false;
}

void SelectorFilters::setGenreFilter(bool value) {
    m_bFilterGenre = value;
}

void SelectorFilters::setBpmFilter(bool value, int range) {
    m_bFilterBpm = value;
    m_iFilterBpmRange = range;
}

void SelectorFilters::setKeyFilter(bool value) {
    m_bFilterKey = value;
}

void SelectorFilters::setKey4thFilter(bool value) {
    m_bFilterKey4th = value;
}

void SelectorFilters::setKey5thFilter(bool value) {
    m_bFilterKey5th = value;
}

void SelectorFilters::setKeyRelativeFilter(bool value) {
    m_bFilterKeyRelative = value;
}

QString SelectorFilters::getFilterString(TrackPointer pSeedTrack) {
    if (!pSeedTrack) {
        return QString();
    }

    QString filterString;
    QString sSeedTrackGenre = pSeedTrack->getGenre();
    float fSeedTrackBpm = pSeedTrack->getBpm();
    ChromaticKey seedTrackKey = pSeedTrack->getKey();
    TimbrePointer pSeedTrackTimbre = pSeedTrack->getTimbre();
    QStringList filters;

    // Genre
    if (m_bFilterGenre) {
        if (!sSeedTrackGenre.isEmpty()) {
            filters << "Genre == '" % sSeedTrackGenre % "'";
        }
    }
    // BPM
    if (m_bFilterBpm) {
        if (fSeedTrackBpm > 0) {
            filters <<
                "(Bpm > " %
                QString::number(floor(fSeedTrackBpm - m_iFilterBpmRange)) %
                " AND Bpm < " %
                QString::number(ceil(fSeedTrackBpm + m_iFilterBpmRange)) %
                ")";
        }
    }
    // Keys
    if (m_bFilterKey || m_bFilterKey4th ||
            m_bFilterKey5th || m_bFilterKeyRelative) {
        QList<ChromaticKey> hKeys = getHarmonicKeys(seedTrackKey);
        QStringList keyNames;
        foreach(ChromaticKey key, hKeys) {
            QString keyName = "'" % KeyUtils::keyToString(key) % "'";
            keyNames << keyName;
        }
        QString keyString = keyNames.join(",");
        if (!keyString.isEmpty()) {
                qDebug() << "Match keys " << keyString;
                filters << "Key in (" % keyString % ")";
        }
    }

    filterString = filters.join(" AND ");
    return filterString;
}

QList<ChromaticKey> SelectorFilters::getHarmonicKeys(ChromaticKey key) {
    QList<ChromaticKey> keys;

    if (m_bFilterKey) {
        keys.append(key);
    }
    if (m_bFilterKey4th) {
        keys.append(KeyUtils::scaleKeySteps(key, 5));
    }
    if (m_bFilterKey5th) {
        keys.append(KeyUtils::scaleKeySteps(key, 7));
    }
    // if (m_bFilterKeyRelative) {
    //     keys.append(KeyUtils::keyToRelativeMajorOrMinor(key));
    // }

    return keys;
}
