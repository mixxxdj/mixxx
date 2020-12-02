#pragma once

#include <QByteArrayData>
#include <QString>

#include "library/librarytablemodel.h"

class QObject;
class TrackCollectionManager;

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
