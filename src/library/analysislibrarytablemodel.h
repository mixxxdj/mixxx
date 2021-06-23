#pragma once

#include "library/librarytablemodel.h"

class AnalysisLibraryTableModel : public LibraryTableModel
{
    Q_OBJECT
  public:
    AnalysisLibraryTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager);
    ~AnalysisLibraryTableModel() override = default;

  public slots:
    void showRecentSongs();
    void showAllSongs();
};
