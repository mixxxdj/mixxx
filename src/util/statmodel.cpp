#include "util/statmodel.h"

#include <limits>

#include "moc_statmodel.cpp"
#include "util/math.h"

StatModel::StatModel(QObject* pParent)
        : QAbstractTableModel(pParent) {

    setHeaderData(STAT_COLUMN_NAME, Qt::Horizontal, tr("Name"));
    setHeaderData(STAT_COLUMN_COUNT, Qt::Horizontal, tr("Count"));
    setHeaderData(STAT_COLUMN_TYPE, Qt::Horizontal, tr("Type"));
    setHeaderData(STAT_COLUMN_UNITS, Qt::Horizontal, tr("Units"));
    setHeaderData(STAT_COLUMN_SUM, Qt::Horizontal, tr("Sum"));
    setHeaderData(STAT_COLUMN_MIN, Qt::Horizontal, tr("Min"));
    setHeaderData(STAT_COLUMN_MAX, Qt::Horizontal, tr("Max"));
    setHeaderData(STAT_COLUMN_MEAN, Qt::Horizontal, tr("Mean"));
    setHeaderData(STAT_COLUMN_VARIANCE, Qt::Horizontal, tr("Variance"));
    setHeaderData(STAT_COLUMN_STDDEV, Qt::Horizontal, tr("Standard Deviation"));
}

StatModel::~StatModel() {
}

void StatModel::statUpdated(const Stat& stat) {
    auto it = m_statNameToRow.constFind(stat.m_tag);
    if (it != m_statNameToRow.constEnd()) {
        int row = it.value();
        m_stats[row] = stat;
        QModelIndex left = index(row, 0);
        QModelIndex right = index(row, columnCount() - 1);
        emit dataChanged(left, right);
    } else {
        beginInsertRows(QModelIndex(), m_stats.size(),
                        m_stats.size());
        m_statNameToRow[stat.m_tag] = m_stats.size();
        m_stats.append(stat);
        endInsertRows();
    }
}

int StatModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_stats.size();
}

int StatModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return NUM_STAT_COLUMNS;
}

QVariant StatModel::data(const QModelIndex& index,
                            int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole &&
                             role != Qt::EditRole)) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_stats.size()) {
        return QVariant();
    }

    const Stat& stat = m_stats.at(row);
    switch (column) {
        case STAT_COLUMN_NAME:
            return stat.m_tag;
        case STAT_COLUMN_TYPE:
            return stat.m_type;
        case STAT_COLUMN_COUNT:
            return stat.m_report_count;
        case STAT_COLUMN_SUM:
            return stat.m_sum;
        case STAT_COLUMN_MIN:
            return std::numeric_limits<double>::max() == stat.m_min ?
                    QVariant("XXX") : QVariant(stat.m_min);
        case STAT_COLUMN_MAX:
            return std::numeric_limits<double>::min() == stat.m_max ?
                    QVariant("XXX") : QVariant(stat.m_max);
        case STAT_COLUMN_MEAN:
            return stat.m_report_count > 0 ?
                    QVariant(stat.m_sum / stat.m_report_count) : QVariant("XXX");
        case STAT_COLUMN_VARIANCE:
            return stat.variance();
        case STAT_COLUMN_STDDEV:
            return sqrt(stat.variance());
        case STAT_COLUMN_UNITS:
            return stat.valueUnits();
    }
    return QVariant();
}

bool StatModel::setHeaderData(int section,
                              Qt::Orientation orientation,
                              const QVariant& value,
                              int role) {
    int numColumns = columnCount();
    if (section < 0 || section >= numColumns) {
        return false;
    }

    if (orientation != Qt::Horizontal) {
        // We only care about horizontal headers.
        return false;
    }

    if (m_headerInfo.size() != numColumns) {
        m_headerInfo.resize(numColumns);
    }

    m_headerInfo[section][role] = value;
    emit headerDataChanged(orientation, section, section);
    return true;
}

QVariant StatModel::headerData(int section,
                               Qt::Orientation orientation,
                               int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        QVariant headerValue = m_headerInfo.value(section).value(role);
        if (!headerValue.isValid()) {
            // Try EditRole if DisplayRole wasn't present
            headerValue = m_headerInfo.value(section).value(Qt::EditRole);
        }
        if (!headerValue.isValid()) {
            headerValue = QVariant(section).toString();
        }
        return headerValue;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}
