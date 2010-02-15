// basesqltablemodel.h
// Created by RJ Ryan (rryan@mit.edu) 1/29/2010
#ifndef BASESQLTABLEMODEL_H
#define BASESQLTABLEMODEL_H

#include <QtCore>
#include <QtSql>

class BaseSqlTableModel : public QSqlRelationalTableModel {
    Q_OBJECT
  public:
    BaseSqlTableModel(QObject* parent = NULL,
                      QSqlDatabase db = QSqlDatabase());
    virtual ~BaseSqlTableModel();

    virtual void setTable(const QString& tableName);
    virtual void setSort(int column, Qt::SortOrder order);
    virtual bool select();

  protected:
    virtual QString orderByClause() const;
  private:
    QString m_qTableName;
    int m_iSortColumn;
    Qt::SortOrder m_eSortOrder;
};

#endif /* BASESQLTABLEMODEL_H */
