#ifndef SELECTORSIMILARITY_H
#define SELECTORSIMILARITY_H

#include <QObject>
#include <QList>
#include <QHash>

#include "library/dao/trackdao.h"
#include "library/trackcollection.h"

class SelectorSimilarity : public QObject {
    Q_OBJECT
  public:
    SelectorSimilarity(QObject *parent,
                       TrackCollection* pTrackCollection,
                       ConfigObject<ConfigValue>* pConfig);
    ~SelectorSimilarity();

    QList<QPair<int, double> > calculateSimilarities(int seedTrackId,
                                                     QList<int> trackIds);
  public slots:
    void setSimilarityContributions(
            const QHash<QString, double>& contributions);

  private:
    typedef double (*SimilarityFunc)(TrackPointer pTrack1,
                                     TrackPointer pTrack2);

    void loadStoredSimilarityContributions();
    QHash<QString, double> normalizeContributions(TrackPointer pSeedTrack);

    static double timbreSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);
    static double rhythmSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);
    static double tagSimilarity(TrackPointer pTrack1, TrackPointer pTrack2);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;

    QHash<QString, double> m_similarityContributions;
    QHash<QString, SimilarityFunc> m_similarityFunctions;

};

#endif // SELECTORSIMILARITY_H
