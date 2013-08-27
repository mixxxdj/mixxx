#ifndef SELECTORSIMILARITY_H
#define SELECTORSIMILARITY_H

#include <QObject>
#include <QList>
#include <QHash>

#include "library/dao/trackdao.h"
#include "library/trackcollection.h"
#include "library/selector/selectorfilters.h"

class SelectorSimilarity : public QObject {
    Q_OBJECT
  public:
    SelectorSimilarity(QObject *parent,
                       TrackCollection* pTrackCollection,
                       ConfigObject<ConfigValue>* pConfig,
                       SelectorFilters& selectorFilters);
    ~SelectorSimilarity();

    QList<QPair<int, double> > calculateSimilarities(int iSeedTrackId,
                                                     QList<int> trackIds);
  public slots:
    QList<int> getFollowupTracks(int iSeedTrackId, int n = -1);
    int getTopFollowupTrack(int iSeedTrackId);
    void setSimilarityContributions(
            const QHash<QString, double>& contributions);

  private:
    typedef double (*SimilarityFunc)(TrackPointer pTrack1,
                                     TrackPointer pTrack2);

    void loadStoredSimilarityContributions();
    QHash<QString, double> normalizeContributions(TrackPointer pSeedTrack);

    static bool similaritySort(const QPair<int, double> s1,
                               const QPair<int, double> s2);

    static double timbreSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);
    static double rhythmSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);
    static double tagSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    QSqlDatabase& m_database;
    TrackDAO& m_trackDAO;

    QHash<QString, double> m_similarityContributions;
    QHash<QString, SimilarityFunc> m_similarityFunctions;

    SelectorFilters& m_selectorFilters;

};

#endif // SELECTORSIMILARITY_H
