#include "control/controlsortfiltermodel.h"

#include "moc_controlsortfiltermodel.cpp"

ControlSortFilterModel::ControlSortFilterModel(QObject* pParent)
        : QSortFilterProxyModel(pParent), m_pModel(new ControlModel(this)) {
    setSourceModel(m_pModel);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(ControlModel::CONTROL_COLUMN_FILTER);
}

ControlSortFilterModel::~ControlSortFilterModel() {
}

bool ControlSortFilterModel::sortDescending() const {
    return sortOrder() == Qt::DescendingOrder;
}

void ControlSortFilterModel::sortByColumn(int column, bool descending) {
    const int oldColumn = sortColumn();
    const Qt::SortOrder oldSortOrder = sortOrder();
    const Qt::SortOrder sortOrder = descending ? Qt::DescendingOrder : Qt::AscendingOrder;

    sort(column, sortOrder);

    if (oldColumn != column) {
        emit sortColumnChanged(column);
    }
    if (oldSortOrder != sortOrder) {
        emit sortDescendingChanged(descending);
    }
}
