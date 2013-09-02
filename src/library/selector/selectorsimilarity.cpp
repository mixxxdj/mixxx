#include <QStringBuilder>

#include "trackinfoobject.h"

#include "track/timbreutils.h"
#include "track/tagutils.h"

#include "util/timer.h"
#include "library/queryutil.h"

#include "library/selector/selector_preferences.h"
#include "library/selector/selectorfilters.h"
#include "library/selector/selectorsimilarity.h"

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
    m_similarityFunctions.insert("tags", &tagSimilarity);
}

SelectorSimilarity::~SelectorSimilarity() {
}

QList<QPair<int, double> > SelectorSimilarity::calculateSimilarities(
        int iSeedTrackId, QList<int> trackIds) {

    QTime timer;
    timer.start();

    loadStoredSimilarityContributions();
    QList<QPair<int, double> > scores;
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

        scores << QPair<int, double>(trackId, score);
    }

    qDebug() << "calculateSimilarities:" << timer.elapsed() << "ms";
    return scores;
}

// Return up to n followup tracks for a given seed track, filtered and ranked
// according to current settings in dlgprefselctor.
// (n defaults to -1, which returns all results.)
QList<int> SelectorSimilarity::getFollowupTracks(int iSeedTrackId, int n) {
    QList<int> results;
    TrackPointer pSeedTrack = m_trackDAO.getTrack(iSeedTrackId);
    QString filterString = m_selectorFilters.getFilterString(pSeedTrack);
    QSqlQuery query(m_database);
    query.prepare("SELECT id FROM library WHERE "
                  "mixxx_deleted=0 AND " %
                  filterString);
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

    QList<QPair<int, double> > ranks =
            calculateSimilarities(iSeedTrackId, unrankedResults);

    int resultsSize = ranks.size();
    if (n == -1 || n > resultsSize) {
        n = resultsSize;
    }

    qSort(ranks.begin(), ranks.end(), similaritySort);

    for (int i = 0; i < n; i++) {
        results << ranks.at(i).first; // track id
    }
    return results;
}

int SelectorSimilarity::getTopFollowupTrack(int iSeedTrackId) {
    QList<int> results = getFollowupTracks(iSeedTrackId, 1);
    if (results.isEmpty()) {
        return -1; // invalid track Id
    }
    // TODO(chrisjr): ensure that follow-up has not been played recently
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
    int iLastFmCoefficient = m_pConfig->getValueString(
        ConfigKey(SELECTOR_CONFIG_KEY, LASTFM_COEFFICIENT)).toInt();

    QHash<QString, double> contributions;
    contributions.insert("timbre", iTimbreCoefficient/100.0);
    contributions.insert("rhythm", iRhythmCoefficient/100.0);
    contributions.insert("lastfm", iLastFmCoefficient/100.0);
    setSimilarityContributions(contributions);
}

QHash<QString, double> SelectorSimilarity::normalizeContributions(
        TrackPointer pSeedTrack) {
    QHash<QString, double> contributions(m_similarityContributions);

    if (pSeedTrack->getTags().isEmpty()) {
        contributions["lastfm"] = 0.0;
    }
    if (pSeedTrack->getTimbre().isNull()) {
        contributions["timbre"] = 0.0;
        contributions["beat"] = 0.0;
    }
    // ensure non-zero items sum to 1
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

bool SelectorSimilarity::similaritySort(const QPair<int, double> s1,
                                        const QPair<int, double> s2) {
    return s1.second >= s2.second;
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

double SelectorSimilarity::tagSimilarity(TrackPointer pTrack1,
                                         TrackPointer pTrack2) {
    TagCounts trackTags1 = pTrack1->getTags();
    TagCounts trackTags2 = pTrack2->getTags();
    if (!trackTags1.isEmpty() && !trackTags2.isEmpty()) {
        return TagUtils::overlapSimilarity(trackTags1, trackTags2);
    }
    return 0.0;
}
