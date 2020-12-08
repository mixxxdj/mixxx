#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QVector>
#include <QHash>
#include <QModelIndex>
#include <QAbstractItemDelegate>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetvisitor.h"
#include "controllers/midi/midicontrollerpreset.h"
#include "controllers/hid/hidcontrollerpreset.h"

class ControllerMappingTableModel : public QAbstractTableModel,
                                    public ControllerPresetVisitor {
    Q_OBJECT
  public:
    ControllerMappingTableModel(QObject* pParent);
    ~ControllerMappingTableModel() override;

    void setPreset(ControllerPresetPointer pPreset);
    void visit(HidControllerPreset* pHidPreset) override;
    void visit(MidiControllerPreset* pMidiPreset) override;

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
    // Called after a preset is loaded. If the preset is a MIDI preset,
    // m_pMidiPreset points to the MIDI preset. If the preset is an HID preset,
    // m_pHidPreset points to the HID preset.
    virtual void onPresetLoaded() = 0;

    QVector<QHash<int, QVariant> > m_headerInfo;
    ControllerPresetPointer m_pPreset;
    MidiControllerPreset* m_pMidiPreset;
    HidControllerPreset* m_pHidPreset;
};
