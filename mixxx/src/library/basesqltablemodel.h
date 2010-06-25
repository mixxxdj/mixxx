// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010
#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QtCore>
#include <QHash>
#include <QtSql>

#include "library/dao/trackdao.h"

class TrackCollection;
class TrackInfoObject;

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

  protected:
    virtual QString orderByClause() const;
  private slots:
    void trackChanged(int trackId);
  private:
    QString m_qTableName;
    int m_iSortColumn;
    Qt::SortOrder m_eSortOrder;
    QHash<int, int> m_rowToTrackId;
    QHash<int, int> m_trackIdToRow;
    QSet<int> m_trackOverrides;
    TrackCollection* m_pTrackCollection;
    TrackDAO& m_trackDAO;
};

#endif /* BASESQLTABLEMODEL_H */
