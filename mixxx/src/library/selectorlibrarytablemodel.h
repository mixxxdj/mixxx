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
    void filterByGenre();
    void filterByBpm();
  private slots:
    void slotSearch(const QString& searchText);
    void slotFilterByGenre(const QString& genre);
    void slotFilterByBpm(const float& bpm);
  signals:
    void doSearch(const QString& searchText);
    void doFilterByGenre(const QString& genre);
    void doFilterByBpm(const float& bpm);
  private:
    QString currentGenre() const;
    //void filterByGenre();
    float currentBpm() const;
    //void filterByBpm();
};

#endif



