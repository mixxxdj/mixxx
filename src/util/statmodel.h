#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>

#include "util/stat.h"

class StatModel final : public QAbstractTableModel {
    Q_OBJECT
  public:
    enum StatColumn {
        STAT_COLUMN_NAME = 0,
        STAT_COLUMN_COUNT,
        STAT_COLUMN_TYPE,
        STAT_COLUMN_UNITS,
        STAT_COLUMN_SUM,
        STAT_COLUMN_MIN,
        STAT_COLUMN_MAX,
        STAT_COLUMN_MEAN,
        STAT_COLUMN_VARIANCE,
        STAT_COLUMN_STDDEV,
        NUM_STAT_COLUMNS
    };

    StatModel(QObject* pParent=NULL);
    virtual ~StatModel();

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

  public slots:
    void statUpdated(const Stat& stat);

  private:
    QVector<QHash<int, QVariant> > m_headerInfo;
    QList<Stat> m_stats;
    QHash<QString, int> m_statNameToRow;
};
