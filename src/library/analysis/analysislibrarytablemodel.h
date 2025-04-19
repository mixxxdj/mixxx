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

    void showRecentSongs();
    void showAllSongs();
    void searchCurrentTrackSet(const QString& text, bool useRecentFilter);
};
