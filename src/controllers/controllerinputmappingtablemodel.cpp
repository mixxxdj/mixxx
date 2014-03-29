#include <QtAlgorithms>

#include "controllers/controllerinputmappingtablemodel.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "controllers/delegates/controldelegate.h"
#include "controllers/delegates/midichanneldelegate.h"
#include "controllers/delegates/midiopcodedelegate.h"
#include "controllers/delegates/midibytedelegate.h"
#include "controllers/delegates/midioptionsdelegate.h"

ControllerInputMappingTableModel::ControllerInputMappingTableModel(QObject* pParent)
        : ControllerMappingTableModel(pParent) {
}

ControllerInputMappingTableModel::~ControllerInputMappingTableModel() {
}

void ControllerInputMappingTableModel::apply() {
    if (m_pMidiPreset != NULL) {
        // Clear existing input mappings and insert all the input mappings in
        // the table into the preset.
        m_pMidiPreset->mappings.clear();
        foreach (const MidiInputMapping& mapping, m_midiInputMappings) {
            m_pMidiPreset->mappings.insert(mapping.key.key, qMakePair(
                mapping.control, mapping.options));
        }
    }
}

void ControllerInputMappingTableModel::onPresetLoaded() {
    clear();

    if (m_pMidiPreset != NULL) {
        // TODO(rryan): Tooltips
        setHeaderData(MIDI_COLUMN_CHANNEL, Qt::Horizontal, tr("Channel"));
        setHeaderData(MIDI_COLUMN_OPCODE, Qt::Horizontal, tr("Opcode"));
        setHeaderData(MIDI_COLUMN_CONTROL, Qt::Horizontal, tr("Control"));
        setHeaderData(MIDI_COLUMN_OPTIONS, Qt::Horizontal, tr("Options"));
        setHeaderData(MIDI_COLUMN_ACTION, Qt::Horizontal, tr("Action"));
        setHeaderData(MIDI_COLUMN_COMMENT, Qt::Horizontal, tr("Comment"));

        beginInsertRows(QModelIndex(), 0, m_pMidiPreset->mappings.size() - 1);
        for (QHash<uint16_t, QPair<MixxxControl,
                                   MidiOptions> >::const_iterator it =
                     m_pMidiPreset->mappings.begin();
             it != m_pMidiPreset->mappings.end(); ++it) {
            MidiInputMapping mapping;
            mapping.key.key = it.key();
            mapping.options = it.value().second;
            mapping.control = it.value().first;
            m_midiInputMappings.append(mapping);
        }
        endInsertRows();
    }
}

void ControllerInputMappingTableModel::clear() {
    if (m_pMidiPreset != NULL) {
        beginRemoveRows(QModelIndex(), 0, m_midiInputMappings.size() - 1);
        m_midiInputMappings.clear();
        endRemoveRows();
    }
}

void ControllerInputMappingTableModel::addEmptyMapping() {
    if (m_pMidiPreset != NULL) {
        beginInsertRows(QModelIndex(), m_midiInputMappings.size(),
                        m_midiInputMappings.size());
        m_midiInputMappings.append(MidiInputMapping());
        endInsertRows();
    }
}

void ControllerInputMappingTableModel::removeMappings(QModelIndexList indices) {
    // Values don't matter, it's just to get a consistent ordering.
    QList<int> rows;
    foreach (const QModelIndex& index, indices) {
        rows.append(index.row());
    }
    qSort(rows);

    int lastRow = -1;
    while (!rows.empty()) {
        int row = rows.takeLast();
        if (row == lastRow) {
            continue;
        }

        beginRemoveRows(QModelIndex(), row, row);
        m_midiInputMappings.removeAt(row);
        endRemoveRows();
    }
}

QAbstractItemDelegate* ControllerInputMappingTableModel::delegateForColumn(
        int column, QWidget* pParent) {
    if (m_pMidiPreset != NULL) {
        ControlDelegate* pControlDelegate = NULL;
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return new MidiChannelDelegate(pParent);
            case MIDI_COLUMN_OPCODE:
                return new MidiOpCodeDelegate(pParent);
            case MIDI_COLUMN_CONTROL:
                return new MidiByteDelegate(pParent);
            case MIDI_COLUMN_OPTIONS:
                return new MidiOptionsDelegate(pParent);
            case MIDI_COLUMN_ACTION:
                pControlDelegate = new ControlDelegate(this);
                pControlDelegate->setMidiOptionsColumn(MIDI_COLUMN_OPTIONS);
                return pControlDelegate;
        }
    }
    return NULL;
}

int ControllerInputMappingTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    if (m_pMidiPreset != NULL) {
        return m_midiInputMappings.size();
    }
    return 0;
}

int ControllerInputMappingTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    // Control and description.
    const int kBaseColumns = 2;
    if (m_pMidiPreset != NULL) {
        // Channel, Opcode, Control, Options
        return kBaseColumns + 4;
    }
    return 0;
}

QVariant ControllerInputMappingTableModel::data(const QModelIndex& index,
                                                int role) const {
    // We use UserRole as the "sort" role with QSortFilterProxyModel.
    if (!index.isValid() || (role != Qt::DisplayRole &&
                             role != Qt::EditRole &&
                             role != Qt::UserRole)) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (m_pMidiPreset != NULL) {
        if (row < 0 || row >= m_midiInputMappings.size()) {
            return QVariant();
        }

        const MidiInputMapping& mapping = m_midiInputMappings.at(row);
        QString value;
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return MidiUtils::channelFromStatus(mapping.key.status);
            case MIDI_COLUMN_OPCODE:
                return MidiUtils::opCodeFromStatus(mapping.key.status);
            case MIDI_COLUMN_CONTROL:
                return mapping.key.control;
            case MIDI_COLUMN_OPTIONS:
                // UserRole is used for sorting.
                if (role == Qt::UserRole) {
                    return mapping.options.all;
                }
                return qVariantFromValue(mapping.options);
            case MIDI_COLUMN_ACTION:
                if (role == Qt::UserRole) {
                    // TODO(rryan): somehow get the delegate display text?
                    return mapping.control.group() + "," + mapping.control.item();
                }
                return qVariantFromValue(ConfigKey(mapping.control.group(),
                                                   mapping.control.item()));
            case MIDI_COLUMN_COMMENT:
                return mapping.control.description();
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool ControllerInputMappingTableModel::setData(const QModelIndex& index,
                                               const QVariant& value,
                                               int role) {
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    int row = index.row();
    int column = index.column();

    if (m_pMidiPreset != NULL) {
        if (row < 0 || row >= m_midiInputMappings.size()) {
            return false;
        }

        MidiInputMapping& mapping = m_midiInputMappings[row];

        ConfigKey key;
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                mapping.key.status = static_cast<unsigned char>(
                    MidiUtils::opCodeFromStatus(mapping.key.status)) |
                        static_cast<unsigned char>(value.toInt());
                emit(dataChanged(index, index));
                return true;
            case MIDI_COLUMN_OPCODE:
                mapping.key.status = static_cast<unsigned char>(
                    MidiUtils::channelFromStatus(mapping.key.status)) |
                        static_cast<unsigned char>(value.toInt());
                emit(dataChanged(index, index));
                return true;
            case MIDI_COLUMN_CONTROL:
                mapping.key.control = static_cast<unsigned char>(value.toInt());
                emit(dataChanged(index, index));
                return true;
            case MIDI_COLUMN_OPTIONS:
                mapping.options = qVariantValue<MidiOptions>(value);
                emit(dataChanged(index, index));
                return true;
            case MIDI_COLUMN_ACTION:
                key = qVariantValue<ConfigKey>(value);
                // TODO(rryan): nuke MixxxControl from orbit.
                mapping.control.setGroup(key.group);
                mapping.control.setItem(key.item);
                emit(dataChanged(index, index));
                return true;
            case MIDI_COLUMN_COMMENT:
                mapping.control.setDescription(value.toString());
                emit(dataChanged(index, index));
                return true;
            default:
                return false;
        }
    }

    return false;
}
