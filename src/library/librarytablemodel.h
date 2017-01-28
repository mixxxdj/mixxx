#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include "library/basesqltablemodel.h"

class LibraryTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    LibraryTableModel(QObject* parent, TrackCollection* pTrackCollection,
                      const char* settingsNamespace);
    ~LibraryTableModel() override;

    void setTableModel(int id =-1);

    bool isColumnInternal(int column) final;
    // Takes a list of locations and add the tracks to the library. Returns the
    // number of successful additions.
    int addTracks(const QModelIndex& index, const QList<QString>& locations) final;
    TrackModel::CapabilitiesFlags getCapabilities() const final;
};

#endif
