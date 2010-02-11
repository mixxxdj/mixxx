// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010

#include <QtDebug>

#include "library/basesqltablemodel.h"

BaseSqlTableModel::BaseSqlTableModel(QObject* parent,
                                     QSqlDatabase db) :
        QSqlRelationalTableModel(parent, db) {
}

BaseSqlTableModel::~BaseSqlTableModel() {
}

bool BaseSqlTableModel::select() {
    qDebug() << "select()";
    bool result = QSqlRelationalTableModel::select();

    if (result) {
        // We need to fetch as much data as is available or else the database will
        // be locked.
        while (canFetchMore()) {
            fetchMore();
        }
    }

    return result;
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
