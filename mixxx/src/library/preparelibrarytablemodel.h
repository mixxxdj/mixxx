#ifndef PREPARELIBRARYTABLEMODEL_H_
#define PREPARELIBRARYTABLEMODEL_H_

#include <QModelIndexList>
#include "librarytablemodel.h"

class PrepareLibraryTableModel : public LibraryTableModel
{
    Q_OBJECT
  public:
    PrepareLibraryTableModel(QObject* parent, TrackCollection* pTrackCollection);
    virtual ~PrepareLibraryTableModel();

    virtual void search(const QString& searchText);

  public slots:
    void showRecentSongs();
    void showAllSongs();
  private slots:
    void slotSearch(const QString& searchText);
  signals:
    void doSearch(const QString& searchText);
  private:
    bool m_bShowRecentSongs;
};

#endif
