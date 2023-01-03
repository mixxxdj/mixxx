#pragma once

#include <QAbstractItemDelegate>
#include <QAbstractTableModel>
#include <QHash>
#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QVariant>
#include <QVector>

#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/midi/legacymidicontrollermapping.h"

class ControllerMappingTableModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    ControllerMappingTableModel(QObject* pParent, QTableView* pTableView);
    ~ControllerMappingTableModel() override;

    void setMapping(std::shared_ptr<LegacyControllerMapping> pMapping);

    // Revert changes made since the last apply.
    virtual void cancel();

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    virtual QString displayString(const QModelIndex& index) const = 0;

  protected:
    // Called after a mapping is loaded. If the mapping is a MIDI mapping,
    // m_pMidiMapping points to the MIDI mapping. If the mapping is an HID mapping,
    // m_pHidMapping points to the HID mapping.
    virtual void onMappingLoaded() = 0;

    QVector<QHash<int, QVariant>> m_headerInfo;
    QTableView* m_pTableView;
    std::shared_ptr<LegacyMidiControllerMapping> m_pMidiMapping;
};

class ControllerMappingTableProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
  public:
    explicit ControllerMappingTableProxyModel(ControllerMappingTableModel* sourceModel);
    ~ControllerMappingTableProxyModel();

    void search(const QString& searchText);

    // Inherited from QSortFilterProxyModel
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const final;

  private:
    QString m_currentSearch;
    ControllerMappingTableModel* m_pModel;
};
