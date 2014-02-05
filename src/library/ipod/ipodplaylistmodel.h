#ifndef IPODPLAYLISTMODEL_H
#define IPODPLAYLISTMODEL_H

#include <QtCore>
#include <QHash>
#include <QtGui>
#include <QtSql>

#include "library/trackmodel.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"

extern "C"
{
#include <gpod/itdb.h>
}

class IPodPlaylistModel : public QAbstractTableModel , public virtual TrackModel
{
    Q_OBJECT
  public:

    struct playlist_member {
        IPodPlaylistModel* pClass;
        int pos;
        Itdb_Track* pTrack;
    };

    IPodPlaylistModel(QObject* pParent, TrackCollection* pTrackCollection);
    virtual ~IPodPlaylistModel();

    ////////////////////////////////////////////////////////////////////////////
    // Methods implemented from QAbstractItemModel
    ////////////////////////////////////////////////////////////////////////////

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual int getTrackId(const QModelIndex& index) const;
    virtual const QLinkedList<int> getTrackRows(int trackId) const;
    virtual void search(const QString& searchText, const QString& extraFilter = QString());
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    QMimeData* mimeData(const QModelIndexList &indexes) const;

    QAbstractItemDelegate* delegateForColumn(const int i, QObject* pParent);
    TrackModel::CapabilitiesFlags getCapabilities() const;

    virtual void sort(int column, Qt::SortOrder order);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
    virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role=Qt::DisplayRole) const;

    void setPlaylist(Itdb_Playlist* pPlaylist);

    ////////////////////////////////////////////////////////////////////////////
    // Other public methods
    ////////////////////////////////////////////////////////////////////////////

    virtual const QString currentSearch() const;
    virtual int fieldIndex(const QString& fieldName) const;

  protected:
    // Use this if you want a model that is read-only.
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    // Use this if you want a model that can be changed
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;

    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    virtual void setSearchColumns(const QStringList& searchColumns);
    // virtual QString orderByClause() const;
    virtual void initHeaderData();
    virtual void initDefaultSearchColumns();

  private slots:
    void trackChanged(int trackId);

  private:
    static bool columnLessThan(const playlist_member &s1, const playlist_member &s2);
    Itdb_Track* getPTrackFromModelIndex(const QModelIndex& index) const;
    static bool findInUtf8Case(gchar* heystack, gchar* needles);

    QString m_tableName;
    QStringList m_columnNames;
    QString m_columnNamesJoined;
    QHash<QString, int> m_columnIndex;
    QSet<QString> m_tableColumns;
    QString m_tableColumnsJoined;
    QSet<int> m_tableColumnIndices;

    QStringList m_searchColumns;
    QVector<int> m_searchColumnIndices;
    QString m_idColumn;

    int m_iSortColumn;
    Qt::SortOrder m_eSortOrder;

    bool m_bIndexBuilt;
    QSqlRecord m_queryRecord;
    QHash<int, QVector<QVariant> > m_recordCache;
    QVector<QPair<int, QHash<int, QVariant> > > m_rowInfo;
    QHash<int, QLinkedList<int> > m_trackIdToRows;
    QSet<int> m_trackOverrides;

    QString m_currentSearch;
    QString m_currentSearchFilter;

    QList<QPair<QString, size_t> > m_headerList;

    QList<IPodPlaylistModel::playlist_member> m_sortedPlaylist;

    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;

    Itdb_Playlist* m_pPlaylist;
};

#endif // IPODPLAYLISTMODEL_H
