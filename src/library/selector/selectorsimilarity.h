#ifndef SELECTORSIMILARITY_H
#define SELECTORSIMILARITY_H

#include <QObject>
#include <QList>
#include <QHash>

#include "library/dao/trackdao.h"
#include "library/trackcollection.h"
#include "library/selector/scorepair.h"
#include "library/selector/selectorfilters.h"

class SelectorSimilarity : public QObject {
    Q_OBJECT
  public:
    SelectorSimilarity(QObject *parent,
                       TrackCollection* pTrackCollection,
                       ConfigObject<ConfigValue>* pConfig,
                       SelectorFilters& selectorFilters);
    ~SelectorSimilarity();

    QList<ScorePair> calculateSimilarities(int iSeedTrackId,
                                           QList<int> trackIds);
    // compare two tracks using every similarity function; used
    // for similarity diagnostics
    QHash<QString, double> compareTracks(TrackPointer pTrack1,
                                         TrackPointer pTrack2);
    QStringList getSimilarityTypes();

  public slots:
    // Return up to n followup tracks for a given seed track, filtered and ranked
    // according to current settings. (n defaults to -1, which returns all results.)
    QList<int> getFollowupTracks(int iSeedTrackId, int n = -1);
    int getTopFollowupTrack(int iSeedTrackId);
    void setSimilarityContributions(const QHash<QString, double>& contributions);

  private:
    void loadStoredSimilarityContributions();
    QHash<QString, double> normalizeContributions(TrackPointer pSeedTrack);
    QHash<QString, double> m_similarityContributions;
    static bool similaritySort(const ScorePair s1,
                               const ScorePair s2);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    QSqlDatabase& m_database;
    TrackDAO& m_trackDAO;
    // typedef to store similarity functions in a QHash
    typedef double (*SimilarityFunc)(TrackPointer pTrack1,
                                     TrackPointer pTrack2);
    QHash<QString, SimilarityFunc> m_similarityFunctions;
    // This nice QHash only works if we ensure that these are callable without
    // having to initiate a class instance
    static double timbreSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);
    static double rhythmSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);
    SelectorFilters& m_selectorFilters;
};

#endif // SELECTORSIMILARITY_H
