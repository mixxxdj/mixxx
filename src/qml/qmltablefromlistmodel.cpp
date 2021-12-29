#include "qml/qmltablefromlistmodel.h"

#include <QAbstractListModel>
#include <QVariant>
#include <QtDebug>

#include "util/assert.h"

namespace mixxx {
namespace qml {
QmlTableFromListModel::QmlTableFromListModel(QObject* parent)
        : QAbstractTableModel(parent),
          m_columnCount(0),
          m_pSourceModel(nullptr) {
}

QmlTableFromListModel::~QmlTableFromListModel() {
}

QAbstractItemModel* QmlTableFromListModel::sourceModel() const {
    return m_pSourceModel;
}
void QmlTableFromListModel::setSourceModel(QAbstractItemModel* pSourceModel) {
    if (m_pSourceModel == pSourceModel) {
        return;
    }

    beginResetModel();
    if (m_pSourceModel != nullptr) {
        disconnect(m_pSourceModel, nullptr, this, nullptr);
    }

    m_sourceRoles.clear();
    m_pSourceModel = pSourceModel;
    if (m_pSourceModel != nullptr) {
        const QHash<int, QByteArray>& sourceRoleNames = m_pSourceModel->roleNames();
        for (auto it = sourceRoleNames.cbegin(); it != sourceRoleNames.cend(); it++) {
            int roleId = it.key();
            m_sourceRoles.insert(QString::fromUtf8(it.value()), roleId);
        }
        connect(pSourceModel,
                &QAbstractListModel::dataChanged,
                this,
                [this](const QModelIndex& sourceTopLeft,
                        const QModelIndex& sourceBottomRight,
                        const QVector<int>& roles) {
                    Q_UNUSED(roles);

                    const QModelIndex topLeft = index(sourceTopLeft.row(), 0);
                    const QModelIndex bottomRight =
                            index(sourceBottomRight.row(), columnCount());
                    emit dataChanged(topLeft, bottomRight);
                });
    }
    emit sourceModelChanged();
    endResetModel();

    if (componentCompleted) {
        fetchColumnMetadata();
    }
}

void QmlTableFromListModel::classBegin() {
}
void QmlTableFromListModel::componentComplete() {
    componentCompleted = true;

    m_columnCount = m_columns.size();
    if (m_columnCount > 0)
        emit columnCountChanged();

    fetchColumnMetadata();
}
QQmlListProperty<QmlTableFromListModelColumn> QmlTableFromListModel::columns() {
    return QQmlListProperty<QmlTableFromListModelColumn>(this,
            nullptr,
            &QmlTableFromListModel::columns_append,
            &QmlTableFromListModel::columns_count,
            &QmlTableFromListModel::columns_at,
            &QmlTableFromListModel::columns_clear);
}
void QmlTableFromListModel::columns_append(
        QQmlListProperty<QmlTableFromListModelColumn>* pProperty,
        QmlTableFromListModelColumn* value) {
    QmlTableFromListModel* pModel = static_cast<QmlTableFromListModel*>(pProperty->object);
    QmlTableFromListModelColumn* pColumn = qobject_cast<QmlTableFromListModelColumn*>(value);
    if (pColumn) {
        pModel->m_columns.append(pColumn);
    }
}
qsizetype QmlTableFromListModel::columns_count(
        QQmlListProperty<QmlTableFromListModelColumn>* pProperty) {
    const QmlTableFromListModel* pModel = static_cast<QmlTableFromListModel*>(pProperty->object);
    return pModel->m_columns.count();
}
QmlTableFromListModelColumn* QmlTableFromListModel::columns_at(
        QQmlListProperty<QmlTableFromListModelColumn>* pProperty, qsizetype index) {
    const QmlTableFromListModel* pModel = static_cast<QmlTableFromListModel*>(pProperty->object);
    return pModel->m_columns.at(index);
}
void QmlTableFromListModel::columns_clear(
        QQmlListProperty<QmlTableFromListModelColumn>* pProperty) {
    QmlTableFromListModel* pModel = static_cast<QmlTableFromListModel*>(pProperty->object);
    return pModel->m_columns.clear();
}
QModelIndex QmlTableFromListModel::index(int row, int column, const QModelIndex& parent) const {
    return row >= 0 && row < rowCount() && column >= 0 &&
                    column < columnCount() && !parent.isValid()
            ? createIndex(row, column)
            : QModelIndex();
}
int QmlTableFromListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || m_pSourceModel == nullptr) {
        return 0;
    }
    return m_pSourceModel->rowCount();
}
int QmlTableFromListModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return m_columnCount;
}
QVariant QmlTableFromListModel::data(const QModelIndex& index, const QString& role) const {
    const int roleId = roleNames().key(role.toUtf8(), -1);
    if (roleId >= 0) {
        return data(index, roleId);
    }
    return QVariant();
}
QVariant QmlTableFromListModel::data(const QModelIndex& index, int role) const {
    if (m_pSourceModel == nullptr) {
        return QVariant();
    }

    const int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return QVariant();
    }
    const int column = index.column();
    if (column < 0 || column >= columnCount()) {
        return QVariant();
    }
    const ColumnMetadata columnMetadata = m_columnMetadata.at(index.column());
    const QString roleName = QString::fromUtf8(roleNames().value(role));
    if (!columnMetadata.roles.contains(roleName)) {
        qWarning() << "data(): no role named " << roleName
                   << " at column index " << column << ". The available roles for that column are: "
                   << columnMetadata.roles.keys();
        return QVariant();
    }
    const ColumnRoleMetadata roleData = columnMetadata.roles.value(roleName);
    const int sourceRole = m_sourceRoles.value(roleData.name);
    const QModelIndex modelIndex = m_pSourceModel->index(row, 0);
    return m_pSourceModel->data(modelIndex, sourceRole);
}
bool QmlTableFromListModel::setData(
        const QModelIndex& index, const QString& role, const QVariant& value) {
    const int intRole = roleNames().key(role.toUtf8(), -1);
    if (intRole >= 0) {
        return setData(index, value, intRole);
    }
    return false;
}
bool QmlTableFromListModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (m_pSourceModel == nullptr) {
        return false;
    }

    const int row = index.row();
    if (row < 0 || row >= rowCount())
        return false;
    const int column = index.column();
    if (column < 0 || column >= columnCount())
        return false;
    const QString roleName = QString::fromUtf8(roleNames().value(role));
    qDebug().nospace() << "setData() called with index "
                       << index << ", value " << value << " and role " << roleName;
    // Verify that the role exists for this column.
    const ColumnMetadata columnMetadata = m_columnMetadata.at(index.column());
    if (!columnMetadata.roles.contains(roleName)) {
        qWarning() << "setData(): no role named \"" << roleName
                   << "\" at column index " << column
                   << ". The available roles for that column are: "
                   << columnMetadata.roles.keys();
        return false;
    }
    // Verify that the type of the value is what we expect.
    // If the value set is not of the expected type, we can try to convert it automatically.
    const ColumnRoleMetadata roleData = columnMetadata.roles.value(roleName);
    const int sourceRole = m_sourceRoles.value(roleData.name);
    const QModelIndex modelIndex = m_pSourceModel->index(row, 0);
    return m_pSourceModel->setData(modelIndex, value, sourceRole);
}

QmlTableFromListModel::ColumnRoleMetadata
QmlTableFromListModel::fetchColumnRoleData(const QString& roleNameKey,
        QmlTableFromListModelColumn* pColumn,
        int columnIndex) const {
    DEBUG_ASSERT(componentCompleted);

    ColumnRoleMetadata roleData;
    VERIFY_OR_DEBUG_ASSERT(m_pSourceModel != nullptr) {
        return roleData;
    }

    QJSValue columnRoleGetter = pColumn->getterAtRole(roleNameKey);
    if (columnRoleGetter.isUndefined()) {
        // This role is not defined, which is fine; just skip it.
        return roleData;
    }
    if (columnRoleGetter.isString()) {
        const QString rolePropertyName = columnRoleGetter.toString();
        roleData.name = rolePropertyName;
        roleData.roleId = m_sourceRoles.value(rolePropertyName);
    } else {
        // Invalid role.
        qWarning() << "TableFromListModelColumn role for column at index "
                   << columnIndex << " must be either a string or a function; actual type is: "
                   << columnRoleGetter.toString();
    }
    return roleData;
}

void QmlTableFromListModel::fetchColumnMetadata() {
    qDebug() << "gathering metadata for" << m_columnCount << "columns";
    m_columnMetadata.clear();
    static const auto supportedRoleNames = QmlTableFromListModelColumn::supportedRoleNames();
    // Since we support different data structures at the row level, we require that there
    // is a TableFromListModelColumn for each column.
    // Collect and cache metadata for each column. This makes data lookup faster.
    for (int columnIndex = 0; columnIndex < m_columns.size(); ++columnIndex) {
        QmlTableFromListModelColumn* column = m_columns.at(columnIndex);
        qDebug().nospace() << "- column " << columnIndex << ":";
        ColumnMetadata metaData;
        const auto builtInRoleKeys = supportedRoleNames.keys();
        for (const int builtInRoleKey : builtInRoleKeys) {
            const QString builtInRoleName = supportedRoleNames.value(builtInRoleKey);
            ColumnRoleMetadata roleData = fetchColumnRoleData(builtInRoleName, column, columnIndex);
            if (!roleData.isValid()) {
                // This built-in role was not specified in this column.
                continue;
            }
            qDebug().nospace() << "  - added metadata for built-in role "
                               << builtInRoleName << " at column index " << columnIndex << ": name="
                               << roleData.name << "roleId=" << roleData.roleId;
            // This column now supports this specific built-in role.
            metaData.roles.insert(builtInRoleName, roleData);
        }
        m_columnMetadata.insert(columnIndex, metaData);
    }
}

QmlTableFromListModel::ColumnRoleMetadata::ColumnRoleMetadata() {
}

QmlTableFromListModel::ColumnRoleMetadata::ColumnRoleMetadata(const QString& name, int roleId)
        : name(name),
          roleId(roleId) {
}

bool QmlTableFromListModel::ColumnRoleMetadata::isValid() const {
    return !name.isEmpty() && roleId >= 0;
}

} // namespace qml
} // namespace mixxx
