#ifndef ANALYSISLIBRARYTABLEMODEL_H_
#define ANALYSISLIBRARYTABLEMODEL_H_

#include <QModelIndexList>
#include "librarytablemodel.h"

class AnalysisLibraryTableModel : public LibraryTableModel
{
    Q_OBJECT
  public:
    AnalysisLibraryTableModel(QObject* parent,
                             TrackCollection* pTrackCollection);
    virtual ~AnalysisLibraryTableModel();
    void init();

  public slots:
    void showRecentSongs();
    void showAllSongs();
  private:
    bool m_bShowRecentSongs;
};

#endif
