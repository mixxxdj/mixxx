// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#ifndef SELECTORLIBRARYTABLEMODEL_H_
#define SELECTORLIBRARYTABLEMODEL_H_

#include <QModelIndexList>
#include "library/librarytablemodel.h"
#include "controlobjectthreadmain.h"


class ControlObjectThreadMain;

class SelectorLibraryTableModel : public LibraryTableModel
{
    Q_OBJECT
  public:
    SelectorLibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    ~SelectorLibraryTableModel();

    void search(const QString& searchText);
    bool isColumnInternal(int column);
    int rowCount();
	void active(bool value);

    void setSeedTrack(TrackPointer pSeedTrack);
    QString getSeedTrackInfo();
    bool seedTrackGenreExists();
    bool seedTrackBpmExists();
    bool seedTrackYearExists();
    bool seedTrackRatingExists();
    bool seedTrackKeyExists();

  public slots:
    void filterByGenre(bool value);
    void filterByBpm(bool value, int range);
    void filterByYear(bool value);
    void filterByRating(bool value);
	void filterByKey(bool value);
	void filterByKey4th(bool value);
	void filterByKey5th(bool value);
	void filterByKeyRelative(bool value);

  private slots:
    void slotPlayingDeckChanged(int deck);
    void slotChannelBpmChanged(double value);
    void slotChannelKeyChanged(double value);
    void slotFiltersChanged();
  signals:
    void filtersChanged();
    void seedTrackInfoChanged();
  private:
    bool m_bActive;
    void updateFilterText();
    QList<mixxx::track::io::key::ChromaticKey> getHarmonicKeys(mixxx::track::io::key::ChromaticKey key);
    bool m_bFilterGenre;
    bool m_bFilterBpm;
    int m_iFilterBpmRange;
    bool m_bFilterYear;
    bool m_bFilterRating;
    bool m_bFilterKey;
    bool m_bFilterKey4th;
    bool m_bFilterKey5th;
    bool m_bFilterKeyRelative;

    // Current Track Properties
    QString m_sSeedTrackInfo;
    QString m_sSeedTrackGenre;
    float m_fSeedTrackBpm;
    QString m_sSeedTrackYear;
    int m_iSeedTrackRating;
    mixxx::track::io::key::ChromaticKey m_seedTrackKey;

    QString m_pChannel;
    QString m_filterString;
    TrackPointer m_pSeedTrack;
    TrackPointer m_pLoadedTrack;
    ControlObjectThreadMain* m_channelBpm;
    ControlObjectThreadMain* m_channelKey;
};




#endif



