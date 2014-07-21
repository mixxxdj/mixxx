#ifndef CONTROLLERMAPPINGTABLEMODEL_H
#define CONTROLLERMAPPINGTABLEMODEL_H

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
    virtual ~ControllerMappingTableModel();

    void setPreset(ControllerPresetPointer pPreset);
    void visit(HidControllerPreset* pHidPreset);
    void visit(MidiControllerPreset* pMidiPreset);

    // Apply the changes to the loaded preset.
    virtual void apply() = 0;

    // Revert changes made since the last apply.
    virtual void cancel();

    // Clears all input mappings in the preset.
    virtual void clear() = 0;

    // Adds an empty mapping.
    virtual void addEmptyMapping() = 0;

    // Removes the provided mappings.
    virtual void removeMappings(QModelIndexList indices) = 0;

    // Returns a delegate for the provided column or NULL if the column does not
    // need a delegate.
    virtual QAbstractItemDelegate* delegateForColumn(int column,
                                                     QWidget* pParent) = 0;

    // Validates the mappings.
    // TODO(rryan): do something with this
    virtual bool validate() {
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

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

#endif /* CONTROLLERMAPPINGTABLEMODEL_H */
