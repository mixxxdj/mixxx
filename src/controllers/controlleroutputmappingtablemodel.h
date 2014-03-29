#ifndef CONTROLLEROUTPUTMAPPINGTABLEMODEL_H
#define CONTROLLEROUTPUTMAPPINGTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVariant>
#include <QModelIndex>
#include <QAbstractItemDelegate>

#include "controllers/controllermappingtablemodel.h"

class ControllerOutputMappingTableModel : public ControllerMappingTableModel {
    Q_OBJECT
  public:
    ControllerOutputMappingTableModel(QObject* pParent);
    virtual ~ControllerOutputMappingTableModel();

    // Apply the changes to the loaded preset.
    void apply();

    // Clears all output mappings in the preset.
    void clear();

    // Adds an empty output mapping.
    void addEmptyMapping();

    // Removes the provided output mappings.
    void removeMappings(QModelIndexList mappings);

    // Returns a delegate for the provided column or NULL if the column does not
    // need a delegate.
    QAbstractItemDelegate* delegateForColumn(int column, QWidget* pParent);

    ////////////////////////////////////////////////////////////////////////////
    // QAbstractItemModel methods
    ////////////////////////////////////////////////////////////////////////////
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole);

  protected:
    void onPresetLoaded();

  private:
    enum MidiColumn {
        MIDI_COLUMN_CHANNEL = 0,
        MIDI_COLUMN_OPCODE,
        MIDI_COLUMN_CONTROL,
        MIDI_COLUMN_ON,
        MIDI_COLUMN_OFF,
        MIDI_COLUMN_ACTION,
        MIDI_COLUMN_MIN,
        MIDI_COLUMN_MAX,
        MIDI_COLUMN_COMMENT
    };

    struct MidiOutputMapping {
        MidiOutput output;
        MixxxControl control;
    };
    QList<MidiOutputMapping> m_midiOutputMappings;
};

#endif /* CONTROLLEROUTPUTMAPPINGTABLEMODEL_H */
