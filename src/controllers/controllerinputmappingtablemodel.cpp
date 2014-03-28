#include <QtAlgorithms>

#include "controllers/controllerinputmappingtablemodel.h"
#include "controllers/midi/midimessage.h"
#include "controllers/delegates/midichanneldelegate.h"
#include "controllers/delegates/midiopcodedelegate.h"
#include "controllers/delegates/midicontroldelegate.h"
#include "controllers/delegates/midioptionsdelegate.h"

ControllerInputMappingTableModel::ControllerInputMappingTableModel(QObject* pParent)
        : QAbstractTableModel(pParent),
          m_pMidiPreset(NULL),
          m_controlPickerMenu(NULL) {
}

ControllerInputMappingTableModel::~ControllerInputMappingTableModel() {
}

void ControllerInputMappingTableModel::setPreset(ControllerPresetPointer pPreset) {
    m_pPreset = pPreset;
    if (m_pPreset) {
        // This immediately calls one of the two visit() methods below.
        m_pPreset->accept(this);
    }
}

void ControllerInputMappingTableModel::visit(MidiControllerPreset* pMidiPreset) {
    m_pMidiPreset = pMidiPreset;

    if (m_pMidiPreset != NULL) {
        // TODO(rryan): Tooltips
        setHeaderData(MIDI_COLUMN_CHANNEL, Qt::Horizontal, tr("Channel"));
        setHeaderData(MIDI_COLUMN_OPCODE, Qt::Horizontal, tr("Opcode"));
        setHeaderData(MIDI_COLUMN_CONTROL, Qt::Horizontal, tr("Control"));
        setHeaderData(MIDI_COLUMN_OPTIONS, Qt::Horizontal, tr("Options"));
        setHeaderData(MIDI_COLUMN_ACTION, Qt::Horizontal, tr("Action"));
        setHeaderData(MIDI_COLUMN_COMMENT, Qt::Horizontal, tr("Comment"));

        for (QHash<uint16_t, QPair<MixxxControl,
                                   MidiOptions> >::const_iterator it =
                     m_pMidiPreset->mappings.begin();
             it != m_pMidiPreset->mappings.end(); ++it) {
            MidiKey key;
            key.key = it.key();

            MidiInputMapping mapping;
            mapping.key.key = it.key();
            mapping.options = it.value().second;
            mapping.control = it.value().first;
            m_midiInputMappings.append(mapping);
        }
    }
}

void ControllerInputMappingTableModel::visit(HidControllerPreset* pHidPreset) {
    Q_UNUSED(pHidPreset);
    qDebug() << "WARNING: HID controller presets are not currently mappable.";
}

void ControllerInputMappingTableModel::clear() {
    if (m_pMidiPreset != NULL) {
        beginRemoveRows(QModelIndex(), 0, m_midiInputMappings.size() - 1);
        m_midiInputMappings.clear();
        endRemoveRows();
    }
}

void ControllerInputMappingTableModel::addEmptyInputMapping() {
    if (m_pMidiPreset != NULL) {
        beginInsertRows(QModelIndex(), m_midiInputMappings.size(),
                        m_midiInputMappings.size());
        m_midiInputMappings.append(MidiInputMapping());
        endInsertRows();
    }
}

void ControllerInputMappingTableModel::removeInputMappings(QModelIndexList indices) {
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

bool ControllerInputMappingTableModel::validate() {
    // TODO(rryan): validation
    return true;
}

QAbstractItemDelegate* ControllerInputMappingTableModel::delegateForColumn(int column, QWidget* pParent) {
    if (m_pMidiPreset != NULL) {
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return new MidiChannelDelegate(pParent);
            case MIDI_COLUMN_OPCODE:
                return new MidiOpCodeDelegate(pParent);
            case MIDI_COLUMN_CONTROL:
                return new MidiControlDelegate(pParent);
            case MIDI_COLUMN_OPTIONS:
                return new MidiOptionsDelegate(pParent);
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
    if (!index.isValid() || (role != Qt::DisplayRole &&
                             role != Qt::EditRole)) {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();

    if (m_pMidiPreset != NULL) {
        if (row >= m_midiInputMappings.size()) {
            return QVariant();
        }

        const MidiInputMapping& mapping = m_midiInputMappings.at(row);
        QString value;
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return channelFromStatus(mapping.key.status);
            case MIDI_COLUMN_OPCODE:
                return opCodeFromStatus(mapping.key.status);
            case MIDI_COLUMN_CONTROL:
                return mapping.key.control;
            case MIDI_COLUMN_OPTIONS:
                return qVariantFromValue(mapping.options);
            case MIDI_COLUMN_ACTION:
                if (mapping.control.group().isEmpty() &&
                    mapping.control.item().isEmpty()) {
                    return tr("No action chosen.");
                }

                if (mapping.options.script) {
                    return tr("Script: %1(%2)").arg(
                        mapping.control.item(),
                        mapping.control.group());
                }

                value = m_controlPickerMenu.descriptionForConfigKey(
                    ConfigKey(mapping.control.group(), mapping.control.item()));
                if (!value.isNull()) {
                    return value;
                }

                return mapping.control.group() + "," + mapping.control.item();
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

        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                mapping.key.status = static_cast<unsigned char>(
                    opCodeFromStatus(mapping.key.status)) |
                        static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_OPCODE:
                mapping.key.status = static_cast<unsigned char>(
                    channelFromStatus(mapping.key.status)) |
                        static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_CONTROL:
                mapping.key.control = static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_OPTIONS:
                mapping.options = qVariantValue<MidiOptions>(value);
                return true;
            case MIDI_COLUMN_ACTION:
                // TODO(rryan): How do we represent config keys?
                return true;
            case MIDI_COLUMN_COMMENT:
                mapping.control.setDescription(value.toString());
                return true;
            default:
                return false;
        }
    }

    return false;
}

bool ControllerInputMappingTableModel::setHeaderData(int section,
                                                     Qt::Orientation orientation,
                                                     const QVariant& value,
                                                     int role) {
    int numColumns = columnCount();
    if (section < 0 || section >= numColumns) {
        return false;
    }

    if (orientation != Qt::Horizontal) {
        // We only care about horizontal headers.
        return false;
    }

    if (m_headerInfo.size() != numColumns) {
        m_headerInfo.resize(numColumns);
    }

    m_headerInfo[section][role] = value;
    emit(headerDataChanged(orientation, section, section));
    return true;
}

QVariant ControllerInputMappingTableModel::headerData(int section,
                                                      Qt::Orientation orientation,
                                                      int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        QVariant headerValue = m_headerInfo.value(section).value(role);
        if (!headerValue.isValid()) {
            // Try EditRole if DisplayRole wasn't present
            headerValue = m_headerInfo.value(section).value(Qt::EditRole);
        }
        if (!headerValue.isValid()) {
            headerValue = QVariant(section).toString();
        }
        return headerValue;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ControllerInputMappingTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    return defaultFlags | Qt::ItemIsEditable;
}
