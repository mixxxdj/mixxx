// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtDebug>

#include "trackinfoobject.h"
#include "library/trackcollection.h"
#include "library/basesqltablemodel.h"

BaseSqlTableModel::BaseSqlTableModel(QObject* parent,
                                     TrackCollection* pTrackCollection,
                                     QSqlDatabase db) :
        QSqlRelationalTableModel(parent, db),
        m_pTrackCollection(pTrackCollection),
        m_trackDAO(m_pTrackCollection->getTrackDAO()) {
    connect(&m_trackDAO, SIGNAL(trackChanged(int)),
            this, SLOT(trackChanged(int)));
}

BaseSqlTableModel::~BaseSqlTableModel() {
}

bool BaseSqlTableModel::select() {
    qDebug() << "select()";
    bool result = QSqlRelationalTableModel::select();
    m_rowToTrackId.clear();
    m_trackIdToRow.clear();

    if (result) {
        // We need to fetch as much data as is available or else the database will
        // be locked.
        while (canFetchMore()) {
            fetchMore();
        }

        // TODO(XXX) let child specify this
        int idColumn = record().indexOf("id");
        qDebug() << "idColumn" << idColumn;
        for (int row = 0; row < rowCount(); ++row) {
            QModelIndex ind = index(row, idColumn);
            int trackId = QSqlRelationalTableModel::data(ind).toInt();
            m_rowToTrackId[row] = trackId;
            m_trackIdToRow[trackId] = row;
        }
    }

    return result;
}

void BaseSqlTableModel::trackChanged(int trackId) {
    m_trackOverrides.insert(trackId);
    qDebug() << "trackChanged" << trackId;
    if (m_trackIdToRow.contains(trackId)) {
        int row = m_trackIdToRow[trackId];
        qDebug() << "Row in this result set was updated. Signalling update. track:" << trackId << "row:" << row;
        QModelIndex left = index(row, 0);
        QModelIndex right = index(row, columnCount());
        emit(dataChanged(left, right));
    }
}

void BaseSqlTableModel::setTable(const QString& tableName) {
    m_qTableName = tableName;
    QSqlRelationalTableModel::setTable(tableName);
}

void BaseSqlTableModel::setSort(int column, Qt::SortOrder order) {
    m_iSortColumn = column;
    m_eSortOrder = order;
    QSqlRelationalTableModel::setSort(column, order);
}

QString BaseSqlTableModel::orderByClause() const {
    // This is all stolen from QSqlTableModel::orderByClause(), just rigged to
    // sort case-insensitively.
    QString s;
    int sortColumn = 0;
    QSqlField f = record().field(m_iSortColumn);
    if (!f.isValid())
        return s;

    QString table = m_qTableName;
    QString field = database().driver()->escapeIdentifier(f.name(),
                                                           QSqlDriver::FieldName);
    s.append(QLatin1String("ORDER BY lower("));
    s.append(table);
    s.append(QLatin1Char('.'));
    s.append(field);
    s.append(QLatin1Char(')'));

    s += m_eSortOrder == Qt::AscendingOrder ? QLatin1String(" ASC") : QLatin1String(" DESC");
    return s;
}
