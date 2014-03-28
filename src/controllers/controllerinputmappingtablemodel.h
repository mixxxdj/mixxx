#ifndef CONTROLLERINPUTMAPPINGTABLEMODEL_H
#define CONTROLLERINPUTMAPPINGTABLEMODEL_H

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
#include "controllers/controlpickermenu.h"

class ControllerInputMappingTableModel : public QAbstractTableModel,
                                         public ControllerPresetVisitor {
    Q_OBJECT
  public:
    ControllerInputMappingTableModel(QObject* pParent);
    virtual ~ControllerInputMappingTableModel();

    void setPreset(ControllerPresetPointer pPreset);
    void visit(HidControllerPreset* pHidPreset);
    void visit(MidiControllerPreset* pMidiPreset);

    // Clears all input mappings in the preset.
    void clear();

    // Adds an empty input mapping.
    void addEmptyInputMapping();

    // Removes the provided input mappings.
    void removeInputMappings(QModelIndexList indices);

    // Validates the output mappings.
    // TODO(rryan): rough
    bool validate();

    QAbstractItemDelegate* delegateForColumn(int column, QWidget* pParent);

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole);
    bool setHeaderData(int section, Qt::Orientation orientation,
                       const QVariant& value, int role = Qt::EditRole);
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex& index) const;

  private:
    enum MidiColumn {
        MIDI_COLUMN_CHANNEL = 0,
        MIDI_COLUMN_OPCODE,
        MIDI_COLUMN_CONTROL,
        MIDI_COLUMN_OPTIONS,
        MIDI_COLUMN_ACTION,
        MIDI_COLUMN_COMMENT
    };

    ControllerPresetPointer m_pPreset;
    MidiControllerPreset* m_pMidiPreset;
    QVector<QHash<int, QVariant> > m_headerInfo;

    struct MidiInputMapping {
        MidiKey key;
        MidiOptions options;
        MixxxControl control;
    };
    QList<MidiInputMapping> m_midiInputMappings;

    // TODO(XXX): Separate the control information from the menu aspect.
    ControlPickerMenu m_controlPickerMenu;
};

#endif /* CONTROLLERINPUTMAPPINGTABLEMODEL_H */
