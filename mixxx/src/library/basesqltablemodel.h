// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010
#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QtCore>
#include <QHash>
#include <QtSql>

#include "library/dao/trackdao.h"

class TrackCollection;

class BaseSqlTableModel : public QSqlTableModel {
    Q_OBJECT
  public:
    BaseSqlTableModel(QObject* parent,
                      TrackCollection* pTrackCollection,
                      QSqlDatabase db = QSqlDatabase());
    virtual ~BaseSqlTableModel();

    virtual void setTable(const QString& tableName);
    virtual void setSort(int column, Qt::SortOrder order);
    virtual bool select();
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    /** Use this if you want a model that is read-only. */
    virtual Qt::ItemFlags readOnlyFlags(const QModelIndex &index) const;
    /** Use this if you want a model that can be changed  */
    virtual Qt::ItemFlags readWriteFlags(const QModelIndex &index) const;
    /** calls readWriteFlags() by default */
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual void setLibraryPrefix(QString sPrefix);
  protected:
    virtual QString orderByClause() const;
    virtual void initHeaderData();
  private slots:
    void trackChanged(int trackId);
  private:
    QVariant getBaseValue(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QString m_qTableName;
    int m_iSortColumn;
    Qt::SortOrder m_eSortOrder;
    QHash<int, int> m_rowToTrackId;
    QHash<int, int> m_trackIdToRow;
    QSet<int> m_trackOverrides;
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;
    QString m_sPrefix;
};

#endif /* BASESQLTABLEMODEL_H */
