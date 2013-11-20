#ifndef BANSHEEPLAYLISTMODEL_H
#define BANSHEEPLAYLISTMODEL_H

#include <QtCore>
#include <QHash>
#include <QtGui>
#include <QtSql>

#include "library/trackmodel.h"
#include "library/trackcollection.h"
#include "library/dao/trackdao.h"
#include "library/banshee/bansheedbconnection.h"

// BaseSqlTableModel is a custom-written SQL-backed table which aggressively
// caches the contents of the table and supports lightweight updates.
class BansheePlaylistModel : public QAbstractTableModel , public virtual TrackModel
{
    Q_OBJECT
  public:
    enum Columns {
        VIEW_ORDER,
        ARTIST,
        TITLE,
        DURATION,
        URI,
        ALBUM,
        YEAR,
        RATING,
        GENRE,
        TRACKNUMBER,
        DATEADDED,
        BPM,
        BITRATE,
        COMMENT,
        PLAYCOUNT,
        COMPOSER
    };

    struct ColumnsInfo {
        enum Columns id;
        QString lable;
        bool (*lessThen)(struct BansheeDbConnection::PlaylistEntry &s1, struct BansheeDbConnection::PlaylistEntry &s2);
        bool (*greaterThen)(struct BansheeDbConnection::PlaylistEntry &s1, struct BansheeDbConnection::PlaylistEntry &s2);
    };

    BansheePlaylistModel(QObject* pParent, TrackCollection* pTrackCollection, BansheeDbConnection* pConnection);
    virtual ~BansheePlaylistModel();

    ////////////////////////////////////////////////////////////////////////////
    // Methods implemented from QAbstractItemModel
    ////////////////////////////////////////////////////////////////////////////

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual int getTrackId(const QModelIndex& index) const;
    virtual const QLinkedList<int> getTrackRows(int trackId) const;
    virtual void search(const QString& searchText);
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    QItemDelegate* delegateForColumn(const int i);
    TrackModel::CapabilitiesFlags getCapabilities() const;

    virtual void sort(int column, Qt::SortOrder order);
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);
    virtual int columnCount(const QModelIndex& parent=QModelIndex()) const;
    virtual int rowCount(const QModelIndex& parent=QModelIndex()) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role=Qt::DisplayRole) const;

    void setPlaylist(int playlistId);

    ////////////////////////////////////////////////////////////////////////////
    // Other public methods
    ////////////////////////////////////////////////////////////////////////////

    virtual const QString currentSearch() const;
    virtual void setSort(int column, Qt::SortOrder order);

  protected:
    /** Use this if you want a model that is read-only. */
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    /** Use this if you want a model that can be changed  */
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;

    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    virtual void initHeaderData();
    virtual void initDefaultSearchColumns();

  private slots:
    void trackChanged(int trackId);

  private:
    inline TrackPointer lookupCachedTrack(int trackId) const;
    inline QVariant getTrackValueForColumn(TrackPointer pTrack, int column) const;

    void appendColumnsInfo(
            enum Columns id,
            QString lable,
            bool (*lessThen)(struct BansheeDbConnection::PlaylistEntry &s1, struct BansheeDbConnection::PlaylistEntry &s2),
            bool (*greaterThen)(struct BansheeDbConnection::PlaylistEntry &s1, struct BansheeDbConnection::PlaylistEntry &s2));


//    Itdb_Track* getPTrackFromModelIndex(const QModelIndex& index) const;

//    static bool findInUtf8Case(gchar* heystack, gchar* needles);

    QString m_tableName;
    QStringList m_columnNames;
    QString m_columnNamesJoined;
    QSet<QString> m_tableColumns;
    QString m_tableColumnsJoined;
    QSet<int> m_tableColumnIndices;

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

    QList<struct ColumnsInfo> m_headerList;

    QList<struct BansheeDbConnection::PlaylistEntry> m_sortedPlaylist;

    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;

    BansheeDbConnection* m_pConnection;
    int m_playlistId;
};

#endif /* BANSHEEPLAYLISTMODEL_H */
