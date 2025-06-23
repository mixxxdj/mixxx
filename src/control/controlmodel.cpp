#include "control/controlmodel.h"

#include <QStringBuilder>

#include "moc_controlmodel.cpp"

ControlModel::ControlModel(QObject* pParent)
        : QAbstractTableModel(pParent) {

    setHeaderData(CONTROL_COLUMN_GROUP, Qt::Horizontal, tr("Group"));
    setHeaderData(CONTROL_COLUMN_ITEM, Qt::Horizontal, tr("Item"));
    setHeaderData(CONTROL_COLUMN_VALUE, Qt::Horizontal, tr("Value"));
    setHeaderData(CONTROL_COLUMN_PARAMETER, Qt::Horizontal, tr("Parameter"));
    setHeaderData(CONTROL_COLUMN_TITLE, Qt::Horizontal, tr("Title"));
    setHeaderData(CONTROL_COLUMN_DESCRIPTION, Qt::Horizontal, tr("Description"));

    // Add all controls to Model
    const QList<QSharedPointer<ControlDoublePrivate>> controlsList =
            ControlDoublePrivate::getAllInstances();

    QSet<ConfigKey> controlKeys;

    for (const QSharedPointer<ControlDoublePrivate>& pControl : controlsList) {
        if (!pControl) {
            continue;
        }

        // Skip duplicates
        // This skips either the alias or original key, whatever comes first
        // in controlsList, but that doesn't make a difference here.
        if (controlKeys.contains(pControl->getKey())) {
            continue;
        }
        controlKeys.insert(pControl->getKey());

        addControl(pControl->getKey(),
                pControl->name(),
                pControl->description());
    }
}

ControlModel::~ControlModel() {
}

void ControlModel::addControl(const ConfigKey& key,
                              const QString& title,
                              const QString& description) {
    ControlInfo info;
    info.key = key;
    info.title = title;
    info.description = description;
    info.pControl = new ControlProxy(info.key, this);

    const int row = m_controls.size();

    beginInsertRows(QModelIndex(), row, row);
    m_controls.append(info);
    endInsertRows();

    info.pControl->connectValueChanged(this, [this, row]() {
        const QModelIndex topLeft = index(row, CONTROL_COLUMN_VALUE);
        const QModelIndex bottomRight = index(row, CONTROL_COLUMN_PARAMETER);
        emit dataChanged(topLeft, bottomRight);
    });
}

int ControlModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return m_controls.size();
}

int ControlModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return NUM_CONTROL_COLUMNS;
}

QVariant ControlModel::data(const QModelIndex& index,
                            int role) const {
    if (!index.isValid() || (role != Qt::DisplayRole &&
                             role != Qt::EditRole)) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_controls.size()) {
        return QVariant();
    }

    const ControlInfo& control = m_controls.at(row);
    switch (column) {
        case CONTROL_COLUMN_GROUP:
            return control.key.group;
        case CONTROL_COLUMN_ITEM:
            return control.key.item;
        case CONTROL_COLUMN_VALUE:
            return control.pControl->get();
        case CONTROL_COLUMN_PARAMETER:
            return control.pControl->getParameter();
        case CONTROL_COLUMN_TITLE:
            return control.title;
        case CONTROL_COLUMN_DESCRIPTION:
            return control.description;
        case CONTROL_COLUMN_FILTER:
            return QVariant(control.key.group % QStringLiteral(",") % control.key.item);
    }
    return QVariant();
}

bool ControlModel::setHeaderData(int section,
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

QVariant ControlModel::headerData(int section,
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

bool ControlModel::setData(const QModelIndex& modelIndex,
        const QVariant& value,
        int role) {
    if (!modelIndex.isValid() || role != Qt::EditRole) {
        return false;
    }

    int row = modelIndex.row();
    int column = modelIndex.column();

    if (row < 0 || row >= m_controls.size()) {
        return false;
    }

    ControlInfo& control = m_controls[row];

    static_assert(CONTROL_COLUMN_VALUE + 1 == CONTROL_COLUMN_PARAMETER);
    switch (column) {
        case CONTROL_COLUMN_VALUE:
            control.pControl->set(value.toDouble());
            emit dataChanged(modelIndex, index(modelIndex.row(), CONTROL_COLUMN_PARAMETER));
            return true;
        case CONTROL_COLUMN_PARAMETER:
            control.pControl->setParameter(value.toDouble());
            emit dataChanged(index(modelIndex.row(), CONTROL_COLUMN_VALUE), modelIndex);
            return true;
    }

    return false;
}

Qt::ItemFlags ControlModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    if (index.column() == CONTROL_COLUMN_VALUE ||
            index.column() == CONTROL_COLUMN_PARAMETER) {
        defaultFlags |= Qt::ItemIsEditable;
    }
    return defaultFlags;
}
