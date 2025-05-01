#pragma once

#include "library/basesqltablemodel.h"

class BrowseLibraryTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    BrowseLibraryTableModel(
            QObject* pParent,
            TrackCollectionManager* pTrackCollectionManager);
    ~BrowseLibraryTableModel() override = default;

    void setTableModel(int id = -1); // param required to avoid override?
    void setPath(QString path);

    bool isColumnInternal(int column) final;
    Capabilities getCapabilities() const final;

  private:
    QString m_directoryFilter;
};
