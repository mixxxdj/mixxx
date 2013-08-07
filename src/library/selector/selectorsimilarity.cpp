
#include "trackinfoobject.h"

#include "track/timbreutils.h"
#include "track/tagutils.h"

#include "library/selector/selector_preferences.h"
#include "library/selector/selectorsimilarity.h"


const double maxBpmDiff = 10.0;

SelectorSimilarity::SelectorSimilarity(QObject* parent,
                                       TrackCollection* pTrackCollection,
                                       ConfigObject<ConfigValue>* pConfig)
    : QObject(parent),
      m_pConfig(pConfig),
      m_pTrackCollection(pTrackCollection),
      m_trackDAO(m_pTrackCollection->getTrackDAO()) {
}

SelectorSimilarity::~SelectorSimilarity() {
}

QList<QPair<int, double> > SelectorSimilarity::calculateSimilarities(
        int seedTrackId, QList<int> trackIds) {

    loadStoredSimilarityContributions();
    QList<QPair<int, double> > scores;
    TrackPointer pSeedTrack = m_trackDAO.getTrack(seedTrackId);
    QHash<QString, double> contributions = normalizeContributions(pSeedTrack);

    qDebug() << contributions;

    double seedTrackBpm = pSeedTrack->getBpm();
    TimbrePointer pSeedTrackTimbre = pSeedTrack->getTimbre();
    TagCounts seedTrackTags = pSeedTrack->getTags();

    foreach (int trackId, trackIds) {
        double score = 0.0;

        TrackPointer pTrack = m_trackDAO.getTrack(trackId);

        double bpmContribution = contributions.value("bpm");
        if (bpmContribution > 0.0) {
            double bpm = pTrack->getBpm();
            double bpmDiff = abs(bpm - seedTrackBpm);
            double bpmScore = ((maxBpmDiff - bpmDiff) / maxBpmDiff);

            if (bpmScore > 1.0) bpmScore = 1.0;
            else if (bpmScore < 0.0) bpmScore = 0.0;
            bpmScore *= bpmContribution;

            score += bpmScore;
        }

        double timbreContribution = contributions.value("timbre");
        double rhythmContribution = contributions.value("rhythm");

        if (timbreContribution > 0.0 || rhythmContribution > 0.0) {
            TimbrePointer pTimbre = pTrack->getTimbre();
            if (!pSeedTrackTimbre.isNull() && !pTimbre.isNull()) {
                double timbreScore =
                        1 - TimbreUtils::hellingerDistance(pSeedTrackTimbre,
                                                           pTimbre);
                double rhythmScore =
                        1 - TimbreUtils::modelDistanceBeats(pSeedTrackTimbre,
                                                            pTimbre);

    //            qDebug() << m_sSeedTrackInfo << "x"
    //                     << otherTrack->getInfo() << timbreScore;
                timbreScore *= timbreContribution;
                rhythmScore *= rhythmContribution;
                score += timbreScore + rhythmScore;
            }
        }

        double lastFmContribution = m_similarityContributions.value("lastfm");
        if (lastFmContribution > 0.0) {
            TagCounts trackTags = pTrack->getTags();
            if (!seedTrackTags.isEmpty() && !trackTags.isEmpty()) {
                double tagsScore =
                    TagUtils::overlapSimilarity(seedTrackTags, trackTags);
                tagsScore *= lastFmContribution;
                score += tagsScore;
            }
        }
        scores << QPair<int, double>(trackId, score);
    }
    return scores;
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
