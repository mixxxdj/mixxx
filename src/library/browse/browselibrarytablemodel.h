#pragma once

#include "library/librarytablemodel.h"

class RecordingManager;
class TrackCollection;

class BrowseLibraryTableModel : public LibraryTableModel {
    Q_OBJECT
  public:
    BrowseLibraryTableModel(
            QObject* parent,
            TrackCollectionManager* pTrackCollectionManager,
            RecordingManager* pRecordingManager,
            const char* settingsNamespace);
    ~BrowseLibraryTableModel() override = default;

    void setPath(QString path);
    void search(const QString& searchText, const QString& /* extraFilter */) override;
    TrackPointer getTrackByRef(const TrackRef& trackRef) const override;

  private:
    QString m_directoryFilter;
    RecordingManager* m_pRecordingManager;
};
