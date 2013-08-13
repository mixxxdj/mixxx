// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#ifndef SELECTORLIBRARYTABLEMODEL_H_
#define SELECTORLIBRARYTABLEMODEL_H_

#include <QHash>
#include <QModelIndexList>

#include "controlobjectthreadmain.h"
#include "library/librarytablemodel.h"
#include "track/timbre.h"
#include "track/tagutils.h"
#include "library/selector/selectorsimilarity.h"

#define SELECTOR_TABLE "selector_table"

class ControlObjectThreadMain;

class SelectorLibraryTableModel : public LibraryTableModel {
    Q_OBJECT
  public:
    SelectorLibraryTableModel(QObject* parent,
                              ConfigObject<ConfigValue>* pConfig,
                              TrackCollection* pTrackCollection);
    ~SelectorLibraryTableModel();

    void setTableModel(int id = -1);

    void search(const QString& searchText);
	void active(bool value);

    void setSeedTrack(TrackPointer pSeedTrack);
    QString getSeedTrackInfo();
    bool seedTrackGenreExists();
    bool seedTrackBpmExists();
    bool seedTrackKeyExists();

  public slots:
    void filterByGenre(bool value);
    void filterByBpm(bool value, int range);
	void filterByKey(bool value);
	void filterByKey4th(bool value);
	void filterByKey5th(bool value);
	void filterByKeyRelative(bool value);
    void calculateSimilarity();
    void calculateAllSimilarities(const QString& filename);

  private slots:
    void slotPlayingDeckChanged(int deck);
    void slotChannelBpmChanged(double value);
    void slotChannelKeyChanged(double value);
    void slotFiltersChanged();
    void slotResetFilters();

  signals:
    void filtersChanged();
    void resetFilters();
    void seedTrackInfoChanged();

  private:
    bool m_bActive;
    void clearSeedTrackInfo();
    void updateFilterText();
    void initHeaderData();
    QVariant scoreTrack(const QModelIndex& index);
    QList<mixxx::track::io::key::ChromaticKey> getHarmonicKeys(mixxx::track::io::key::ChromaticKey key);
    bool m_bFilterGenre;
    bool m_bFilterBpm;
    int m_iFilterBpmRange;
    bool m_bFilterKey;
    bool m_bFilterKey4th;
    bool m_bFilterKey5th;
    bool m_bFilterKeyRelative;

    // Current Track Properties
    QString m_sSeedTrackInfo;
    QString m_sSeedTrackGenre;
    float m_fSeedTrackBpm;
    mixxx::track::io::key::ChromaticKey m_seedTrackKey;
    TimbrePointer m_pSeedTrackTimbre;
    TagCounts m_seedTrackTags;

    QString m_pChannel;
    QString m_filterString;
    TrackPointer m_pSeedTrack;
    TrackPointer m_pLoadedTrack;
    ControlObjectThreadMain* m_channelBpm;
    ControlObjectThreadMain* m_channelKey;

    ConfigObject<ConfigValue>* m_pConfig;

    SelectorSimilarity m_selectorSimilarity;
};




#endif



