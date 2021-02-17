#pragma once

#include <QAbstractItemDelegate>
#include <QAbstractTableModel>
#include <QHash>
#include <QModelIndex>
#include <QVariant>
#include <QVector>

#include "controllers/hid/legacyhidcontrollermapping.h"
#include "controllers/legacycontrollermapping.h"
#include "controllers/midi/legacymidicontrollermapping.h"

class ControllerMappingTableModel : public QAbstractTableModel {
    Q_OBJECT
  public:
    ControllerMappingTableModel(QObject* pParent);
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

  protected:
    // Called after a mapping is loaded. If the mapping is a MIDI mapping,
    // m_pMidiMapping points to the MIDI mapping. If the mapping is an HID mapping,
    // m_pHidMapping points to the HID mapping.
    virtual void onMappingLoaded() = 0;

    QVector<QHash<int, QVariant> > m_headerInfo;
    std::shared_ptr<LegacyMidiControllerMapping> m_pMidiMapping;
};
