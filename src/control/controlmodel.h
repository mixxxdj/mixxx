#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QString>

#include "preferences/usersettings.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"

class ControlModel final : public QAbstractTableModel {
    Q_OBJECT
  public:
    enum ControlColumn {
        CONTROL_COLUMN_GROUP = 0,
        CONTROL_COLUMN_ITEM,
        CONTROL_COLUMN_VALUE,
        CONTROL_COLUMN_PARAMETER,
        CONTROL_COLUMN_TITLE,
        CONTROL_COLUMN_DESCRIPTION,
        CONTROL_COLUMN_FILTER,
        NUM_CONTROL_COLUMNS
    };

    ControlModel(QObject* pParent=NULL);
    virtual ~ControlModel();

    void addControl(const ConfigKey& control, const QString& title,
                    const QString& description);

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
    Qt::ItemFlags flags(const QModelIndex& index) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole);

  private:
    struct ControlInfo {
        ConfigKey key;
        QString title;
        QString description;
        ControlProxy* pControl;
    };

    QVector<QHash<int, QVariant> > m_headerInfo;
    QList<ControlInfo> m_controls;
};
