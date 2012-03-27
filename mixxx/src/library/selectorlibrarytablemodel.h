// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#ifndef SELECTORLIBRARYTABLEMODEL_H_
#define SELECTORLIBRARYTABLEMODEL_H_

#include <QModelIndexList>
#include "librarytablemodel.h"
#include "controlobjectthreadmain.h"


class ControlObjectThreadMain;

class SelectorLibraryTableModel : public LibraryTableModel
{
    Q_OBJECT
  public:
    SelectorLibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~SelectorLibraryTableModel();

    virtual void search(const QString& searchText);
    virtual bool isColumnInternal(int column);

  public slots:
    void filterByGenre(bool value);
    void filterByBpm(bool value, int range);
    void filterByYear(bool value);
    void filterByRating(bool value);
    void filterByKey(bool value);
    void filterByHarmonicKey(bool value);
    /*
    void updateFilter(
        bool filterByGenre, 
        bool filterByBpm,
        bool filterByYear,
        bool filterByRating,
        bool filterByKey,
        bool filterByHarmonicKey
        );
        */
  private slots:
    void slotSearch(const QString& searchText);
    void slotPlayingDeckChanged(int deck);
    void slotChannel1BpmChanged(double value);
  signals:
    void doSearch(const QString& searchText);
  private:
    float frequencyRatioToOctaveDifference(
        float currentBpm, float originalBpm);
    QString adjustPitchBy(QString pitch, int change);
    void setBpmFilter(float rate);
    void setKeyFilters(float rate);
    float getRate();
    bool m_bFilterGenre;
    bool m_bFilterBpm;
    int m_iFilterBpmRange;
    bool m_bFilterYear;
    bool m_bFilterRating;
    bool m_bFilterKey;
    bool m_bFilterHarmonicKey;
    QHash<QString, QString> m_pHarmonics;
    QStringList m_pSemitoneList;
    QString m_pFilterGenre;
    QString m_pFilterBpm;
    QString m_pFilterYear;
    QString m_pFilterRating;
    QString m_pFilterKey;
    QString m_pFilterHarmonicKey;
    QString m_pChannel;
    TrackPointer m_pLoadedTrack;
    ControlObjectThreadMain* m_pChannelBpm;

};




#endif



