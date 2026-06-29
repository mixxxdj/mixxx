#pragma once

#include "library/librarytablemodel.h"

class BrowseLibraryTableModel : public LibraryTableModel {
    Q_OBJECT
  public:
    BrowseLibraryTableModel(
            QObject* pParent,
            TrackCollectionManager* pTrackCollectionManager,
            const char* settingsNamespace);
    ~BrowseLibraryTableModel() override = default;

    void setPath(QString path);
    int addTracks(const QModelIndex&, const QList<QString>&) final {
        return 0;
    };

    Capabilities getCapabilities() const final;

  private:
    QString m_directoryFilter;
};
