#include <QStringBuilder>

#include "library/queryutil.h"
#include "library/selector/selector_preferences.h"
#include "library/selector/selectorfilters.h"
#include "library/selector/selectorsimilarity.h"
#include "trackinfoobject.h"
#include "track/timbreutils.h"
#include "util/timer.h"

SelectorSimilarity::SelectorSimilarity(QObject* parent,
                                       TrackCollection* pTrackCollection,
                                       ConfigObject<ConfigValue>* pConfig,
                                       SelectorFilters& selectorFilters)
        : QObject(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_database(m_pTrackCollection->getDatabase()),
          m_trackDAO(m_pTrackCollection->getTrackDAO()),
          m_selectorFilters(selectorFilters) {
    m_similarityFunctions.insert("timbre", &timbreSimilarity);
    m_similarityFunctions.insert("rhythm", &rhythmSimilarity);
}

SelectorSimilarity::~SelectorSimilarity() {
}

QList<SelectorSimilarity::ScorePair> SelectorSimilarity::calculateSimilarities(int iSeedTrackId,
        QList<int> trackIds) {
    QTime timer;
    timer.start();

    loadStoredSimilarityContributions();
    QList<SelectorSimilarity::ScorePair> scores;
    TrackPointer pSeedTrack = m_trackDAO.getTrack(iSeedTrackId);
    QHash<QString, double> contributions = normalizeContributions(pSeedTrack);

    foreach (int trackId, trackIds) {
        double score = 0.0;
        TrackPointer pTrack = m_trackDAO.getTrack(trackId);
        foreach (QString key, contributions.keys()) {
            double contribution = contributions.value(key);
            if (contribution != 0.0) {
                SimilarityFunc simFunc = m_similarityFunctions[key];
                score += simFunc(pSeedTrack, pTrack) * contribution;
            }
        }
        scores << SelectorSimilarity::ScorePair(trackId, score);
    }

    qDebug() << "calculateSimilarities:" << timer.elapsed() << "ms";
    return scores;
}

QHash<QString, double> SelectorSimilarity::compareTracks(TrackPointer pTrack1,
        TrackPointer pTrack2) {
    QHash<QString, double> scores;
    // TODO (kain88) also add contributions scheck here. maybe call this method
    // in the one above
    foreach (QString key, m_similarityFunctions.keys()) {
        SimilarityFunc simFunc = m_similarityFunctions[key];
        scores.insert(key, simFunc(pTrack1, pTrack2));
    }
    return scores;
}

QStringList SelectorSimilarity::getSimilarityTypes() {
    QStringList similarityTypes;
    foreach (QString similarityType, m_similarityFunctions.keys()) {
        similarityTypes << similarityType;
    }
    return similarityTypes;
}

// Return up to n followup tracks for a given seed track, filtered and ranked
// according to current settings in DlgPrefSelector.
// (n defaults to -1, which returns all results.)
QList<int> SelectorSimilarity::getFollowupTracks(int iSeedTrackId, int n) {
    QList<int> results;
    TrackPointer pSeedTrack = m_trackDAO.getTrack(iSeedTrackId);
    QString filterString = m_selectorFilters.getFilterString(pSeedTrack);
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM library WHERE "
                  "mixxx_deleted=0 AND " % filterString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
        return QList<int>();
    }

    QList<int> unrankedResults;
    while (query.next()) {
        int trackId = query.value(0).toInt();
        if (trackId != iSeedTrackId) {
            unrankedResults << trackId;
        }
    }

    QList<SelectorSimilarity::ScorePair> ranks =
            calculateSimilarities(iSeedTrackId, unrankedResults);

    int resultsSize = ranks.size();
    if (n == -1 || n > resultsSize) {
        n = resultsSize;
    }

    qSort(ranks.begin(), ranks.end(), similaritySort);

    for (int i = 0; i < n; i++) {
        results << ranks.at(i).trackId;
    }
    return results;
}

int SelectorSimilarity::getTopFollowupTrack(int iSeedTrackId) {
    QList<int> results = getFollowupTracks(iSeedTrackId, 1);
    if (results.isEmpty()) {
        return -1; // invalid track Id
    }
    // TODO (XXX): ensure that follow-up has not been played recently
    return results.first();
}

void SelectorSimilarity::setSimilarityContributions(
        const QHash<QString, double>& contributions) {
    m_similarityContributions = contributions;
}

void SelectorSimilarity::loadStoredSimilarityContributions() {
    int iTimbreCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, TIMBRE_COEFFICIENT)).toInt();
    int iRhythmCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, RHYTHM_COEFFICIENT)).toInt();

    QHash<QString, double> contributions;
    contributions.insert("timbre", iTimbreCoefficient/100.0);
    contributions.insert("rhythm", iRhythmCoefficient/100.0);
    setSimilarityContributions(contributions);
}

// ensure non-zero items sum to 1
QHash<QString, double> SelectorSimilarity::normalizeContributions(
        TrackPointer pSeedTrack) {
    QHash<QString, double> contributions(m_similarityContributions);

    if (pSeedTrack->getTimbre().isNull()) {
        contributions["timbre"] = 0.0;
        contributions["beat"] = 0.0;
    }
    double total = 0.0;
    foreach (double value, contributions.values()) {
        total += value;
    }
    if (total > 0.0) {
        foreach (QString key, contributions.keys()) {
            contributions[key] /= total;
        }
    }
    return contributions;
}

bool SelectorSimilarity::similaritySort(const SelectorSimilarity::ScorePair s1,
                                        const SelectorSimilarity::ScorePair s2) {
    return s1.score >= s2.score;
}

double SelectorSimilarity::timbreSimilarity(TrackPointer pTrack1,
                                            TrackPointer pTrack2) {
    TimbrePointer pTimbre1 = pTrack1->getTimbre();
    TimbrePointer pTimbre2 = pTrack2->getTimbre();
    if (!pTimbre1.isNull() && !pTimbre2.isNull()) {
        return 1.0 - TimbreUtils::hellingerDistance(pTimbre1, pTimbre2);
    }
    return 0.0;
}

double SelectorSimilarity::rhythmSimilarity(TrackPointer pTrack1,
                                            TrackPointer pTrack2) {
    TimbrePointer pTimbre1 = pTrack1->getTimbre();
    TimbrePointer pTimbre2 = pTrack2->getTimbre();
    if (!pTimbre1.isNull() && !pTimbre2.isNull()) {
        return 1.0 - TimbreUtils::modelDistanceBeats(pTimbre1, pTimbre2);
    }
    return 0.0;
}
