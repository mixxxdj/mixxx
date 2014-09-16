// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#ifndef SELECTORLIBRARYTABLEMODEL_H_
#define SELECTORLIBRARYTABLEMODEL_H_

#include <QHash>
#include <QModelIndexList>

// #include "controlobjectthreadmain.h"
#include "library/librarytablemodel.h"
#include "library/selector/scorepair.h"
#include "library/selector/selectorfilters.h"
#include "library/selector/selectorsimilarity.h"
#include "track/timbre.h"

#define SELECTOR_TABLE "selector_table"

class ControlObjectThread;

class SelectorLibraryTableModel : public LibraryTableModel {
    Q_OBJECT
  public:
    SelectorLibraryTableModel(QObject* parent,
                              ConfigObject<ConfigValue>* pConfig,
                              TrackCollection* pTrackCollection);
    ~SelectorLibraryTableModel();

    void setTableModel(int id = -1);
    void active(bool value);
    void setSeedTrack(TrackPointer pSeedTrack);
    // The following functions are called by DlgSelector to update the UI
    // and determine which filter checkboxes should be enabled.
    QString getSeedTrackInfo();
    bool seedTrackGenreExists();
    bool seedTrackBpmExists();
    bool seedTrackKeyExists();

  public slots:
    SelectorFilters& getFilters();
    void applyFilters();
    void calculateSimilarity();
    void calculateAllSimilarities(const QString& filename);

  private slots:
    void slotPlayingDeckChanged(int deck);
    void slotChannelBpmChanged(double value);
    void slotChannelKeyChanged(double value);
    void slotFiltersChanged();

  signals:
    void filtersChanged();
    void loadStoredFilterSettings();
    void seedTrackInfoChanged();

  private:
    // Override BaseSqlTableModel method to add "Score" column.
    void initHeaderData();
    void clearSeedTrackInfo();
    void updateFilterText();

    bool m_bActive;
    // Current Track Properties
    QString m_sSeedTrackInfo;
    QString m_sSeedTrackGenre;
    float m_fSeedTrackBpm;
    mixxx::track::io::key::ChromaticKey m_seedTrackKey;
    TimbrePointer m_pSeedTrackTimbre;

    QString m_pChannel;
    QString m_filterString;
    TrackPointer m_pSeedTrack;
    TrackPointer m_pLoadedTrack;
    ControlObjectThread* m_channelBpm;
    ControlObjectThread* m_channelKey;
    ConfigObject<ConfigValue>* m_pConfig;
    SelectorFilters m_selectorFilters;
    SelectorSimilarity m_selectorSimilarity;
};
#endif
