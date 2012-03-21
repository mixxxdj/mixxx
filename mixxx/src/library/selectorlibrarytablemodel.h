// Created 3/17/2012 by Keith Salisbury (keithsalisbury@gmail.com)

#ifndef SELECTORLIBRARYTABLEMODEL_H_
#define SELECTORLIBRARYTABLEMODEL_H_

#include <QModelIndexList>
#include "librarytablemodel.h"


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
    void filterByBpm(bool value);
  private slots:
    void slotSearch(const QString& searchText);
    void slotPlayingDeckChanged(int deck);
  signals:
    void doSearch(const QString& searchText);
  private:
    bool m_bFilterGenre;
    bool m_bFilterBpm;
    QString m_pFilterGenre;
    QString m_pFilterBpm;
};




#endif



