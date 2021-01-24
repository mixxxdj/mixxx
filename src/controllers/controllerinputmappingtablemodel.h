#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemDelegate>

#include "controllers/controllermappingtablemodel.h"
#include "controllers/midi/midimessage.h"

/// Table Model for the "Inputs" table view in the preferences dialog.
///
/// This allows editing the input mappings for a MIDI mapping.
class ControllerInputMappingTableModel : public ControllerMappingTableModel {
    Q_OBJECT
  public:
    ControllerInputMappingTableModel(QObject* pParent);
    ~ControllerInputMappingTableModel() override;

    // Apply the changes to the loaded mapping.
    void apply();

    // Clears all input mappings in the mapping.
    void clear();

    // Adds an empty input mapping.
    void addEmptyMapping();

    // Removes the provided input mappings.
    void removeMappings(QModelIndexList indices);

    // Add the specified MIDI mappings to the model. If this is not a MIDI
    // mapping model, ignore.

    // HACK(rryan): This method only exists to communicate new mappings from
    // MIDI learn because doing a round-trip through the controller via
    // onMappingLoaded takes too long. In the future we should replace this with
    // a polymorphic mapping structure.
    void addMappings(const MidiInputMappings& mappings);

    // Returns a delegate for the provided column or NULL if the column does not
    // need a delegate.
    QAbstractItemDelegate* delegateForColumn(int column, QWidget* pParent);

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;

  protected:
    void onMappingLoaded() override;

  private:
    enum MidiColumn {
        MIDI_COLUMN_CHANNEL = 0,
        MIDI_COLUMN_OPCODE,
        MIDI_COLUMN_CONTROL,
        MIDI_COLUMN_OPTIONS,
        MIDI_COLUMN_ACTION,
        MIDI_COLUMN_COMMENT
    };

    QList<MidiInputMapping> m_midiInputMappings;
};
