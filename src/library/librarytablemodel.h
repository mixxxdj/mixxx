#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include <QByteArrayData>
#include <QList>
#include <QString>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"

class QModelIndex;
class QObject;
class TrackCollectionManager;

class LibraryTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    LibraryTableModel(QObject* parent, TrackCollectionManager* pTrackCollectionManager,
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
