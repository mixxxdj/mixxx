#pragma once

#include <QAbstractItemModel>
#include <QAbstractTableModel>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QtQml>

#include "qml/qmltablefromlistmodelcolumn.h"

namespace mixxx {
namespace qml {

class QmlTableFromListModel : public QAbstractTableModel, public QQmlParserStatus {
    Q_OBJECT
    Q_PROPERTY(int columnCount READ columnCount NOTIFY columnCountChanged FINAL)
    Q_PROPERTY(QAbstractItemModel* sourceModel READ sourceModel WRITE
                    setSourceModel NOTIFY sourceModelChanged REQUIRED FINAL)
    Q_PROPERTY(QQmlListProperty<mixxx::qml::QmlTableFromListModelColumn> columns
                    READ columns CONSTANT FINAL)
    Q_INTERFACES(QQmlParserStatus)
    Q_CLASSINFO("DefaultProperty", "columns")
    QML_NAMED_ELEMENT(TableFromListModel)
  public:
    QmlTableFromListModel(QObject* parent = nullptr);
    ~QmlTableFromListModel() override;

    QAbstractItemModel* sourceModel() const;
    void setSourceModel(QAbstractItemModel* pSourceModel);

    QQmlListProperty<QmlTableFromListModelColumn> columns();
    static void columns_append(
            QQmlListProperty<QmlTableFromListModelColumn>* property,
            QmlTableFromListModelColumn* value);
    static qsizetype columns_count(QQmlListProperty<QmlTableFromListModelColumn>* property);
    static QmlTableFromListModelColumn* columns_at(
            QQmlListProperty<QmlTableFromListModelColumn>* property, qsizetype index);
    static void columns_clear(QQmlListProperty<QmlTableFromListModelColumn>* property);

    QModelIndex index(int row,
            int column,
            const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Q_INVOKABLE QVariant data(const QModelIndex& index, const QString& role) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE bool setData(const QModelIndex& index, const QString& role, const QVariant& value);
    bool setData(const QModelIndex& index,
            const QVariant& value,
            int role = Qt::DisplayRole) override;

  signals:
    void columnCountChanged();
    void sourceModelChanged();

  private:
    class ColumnRoleMetadata {
      public:
        ColumnRoleMetadata();
        ColumnRoleMetadata(const QString& name, int roleId);
        bool isValid() const;
        // If this is false, it's a function role.
        QString name;
        int roleId = -1;
    };
    struct ColumnMetadata {
        // Key = role name that will be made visible to the delegate
        // Value = metadata about that role, including actual name in the model data, type, etc.
        QHash<QString, ColumnRoleMetadata> roles;
    };
    enum NewRowOperationFlag {
        OtherOperation, // insert(), set(), etc.
        SetRowsOperation,
        AppendOperation
    };

    void classBegin() override;
    void componentComplete() override;
    bool componentCompleted = false;

    QmlTableFromListModel::ColumnRoleMetadata fetchColumnRoleData(
            const QString& roleNameKey,
            QmlTableFromListModelColumn* pColumn,
            int columnIndex) const;
    void fetchColumnMetadata();

    QList<QmlTableFromListModelColumn*> m_columns;
    int m_columnCount = 0;
    // Each entry contains information about the properties of the column at that index.
    QVector<ColumnMetadata> m_columnMetadata;
    // key = property index (0 to number of properties across all columns)
    // value = role name
    QAbstractItemModel* m_pSourceModel;
    QHash<QString, int> m_sourceRoles;
};

} // namespace qml
} // namespace mixxx
