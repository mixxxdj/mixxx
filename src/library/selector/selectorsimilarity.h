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
    void loadStoredSimilarityContributions();
    QHash<QString, double> normalizeContributions(TrackPointer pSeedTrack);

    ConfigObject<ConfigValue>* m_pConfig;
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;

    QHash<QString, double> m_similarityContributions;

};

#endif // SELECTORSIMILARITY_H
