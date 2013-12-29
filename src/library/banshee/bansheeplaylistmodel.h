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
#include "library/stardelegate.h"
#include "library/basesqltablemodel.h"

class BansheePlaylistModel : public BaseSqlTableModel {
    Q_OBJECT
  public:
    enum Columns {
        VIEW_ORDER,
        ARTIST,
        TITLE,
        DURATION,
        URI,
        ALBUM,
        ALBUM_ARTIST,
        YEAR,
        RATING,
        GENRE,
        GROUPING,
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

    void setTableModel(int playlistId);

    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual bool isColumnInternal(int column);
    virtual bool isColumnHiddenByDefault(int column);

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    TrackModel::CapabilitiesFlags getCapabilities() const;

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role=Qt::EditRole);


  protected:
    // Use this if you want a model that is read-only.
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    // Use this if you want a model that can be changed
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;

    // Set the columns used for searching. Names must correspond to the column
    // names in the table provided to setTable. Must be called after setTable is
    // called.
    virtual void initDefaultSearchColumns();

  private slots:
    void trackChanged(int trackId);

  private:
    QString getFieldString(const QModelIndex& index, const QString& fieldName) const;

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
    QList<struct ColumnsInfo> m_headerList;

    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;

    BansheeDbConnection* m_pConnection;
    int m_playlistId;
};

#endif // BANSHEEPLAYLISTMODEL_H
