#ifndef LIBRARYTABLEMODEL_H
#define LIBRARYTABLEMODEL_H

#include <QtSql>
#include <QtCore>

#include "library/basesqltablemodel.h"
#include "library/trackmodel.h"
#include "library/dao/trackdao.h"

class TrackCollection;

class LibraryTableModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    LibraryTableModel(QObject* parent, TrackCollection* pTrackCollection,
                      QString settingsNamespace="mixxx.db.model.library");
    virtual ~LibraryTableModel();

    TrackPointer getTrack(const QModelIndex& index) const;
    void search(const QString& searchText);
    bool isColumnInternal(int column);
    bool isColumnHiddenByDefault(int column);
    bool addTrack(const QModelIndex& index, QString location);
    // Takes a list of locations and add the tracks to the library. Returns the
    // number of successful additions.
    int addTracks(const QModelIndex& index, QList<QString> locations);
    void moveTrack(const QModelIndex& sourceIndex,
                           const QModelIndex& destIndex);
    TrackModel::CapabilitiesFlags getCapabilities() const;
    static const QString DEFAULT_LIBRARYFILTER;

  private:
    TrackDAO& m_trackDao;

  private slots:
    void slotSearch(const QString& searchText);

  signals:
    void doSearch(const QString& searchText);
};

#endif
