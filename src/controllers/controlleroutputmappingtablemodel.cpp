#include <QtAlgorithms>

#include "controllers/controlleroutputmappingtablemodel.h"
#include "controllers/midi/midimessage.h"
#include "controllers/delegates/midichanneldelegate.h"
#include "controllers/delegates/midiopcodedelegate.h"
#include "controllers/delegates/midibytedelegate.h"

ControllerOutputMappingTableModel::ControllerOutputMappingTableModel(QObject* pParent)
        : QAbstractTableModel(pParent),
          m_pMidiPreset(NULL),
          m_controlPickerMenu(NULL) {
}

ControllerOutputMappingTableModel::~ControllerOutputMappingTableModel() {
}

void ControllerOutputMappingTableModel::setPreset(ControllerPresetPointer pPreset) {
    m_pPreset = pPreset;
    if (m_pPreset) {
        // This immediately calls one of the two visit() methods below.
        m_pPreset->accept(this);
    }

    if (m_pMidiPreset != NULL) {
        // TODO(rryan): Tooltips
        setHeaderData(MIDI_COLUMN_CHANNEL, Qt::Horizontal, tr("Channel"));
        setHeaderData(MIDI_COLUMN_OPCODE, Qt::Horizontal, tr("Opcode"));
        setHeaderData(MIDI_COLUMN_CONTROL, Qt::Horizontal, tr("Control"));
        setHeaderData(MIDI_COLUMN_ON, Qt::Horizontal, tr("On Value"));
        setHeaderData(MIDI_COLUMN_OFF, Qt::Horizontal, tr("Off Value"));
        setHeaderData(MIDI_COLUMN_ACTION, Qt::Horizontal, tr("Action"));
        setHeaderData(MIDI_COLUMN_MIN, Qt::Horizontal, tr("On Range Min"));
        setHeaderData(MIDI_COLUMN_MAX, Qt::Horizontal, tr("On Range Max"));
        setHeaderData(MIDI_COLUMN_COMMENT, Qt::Horizontal, tr("Comment"));

        for (QHash<MixxxControl, MidiOutput>::const_iterator it =
                     m_pMidiPreset->outputMappings.begin();
             it != m_pMidiPreset->outputMappings.end(); ++it) {
            MidiOutputMapping mapping;
            mapping.control = it.key();
            mapping.output = it.value();
            m_midiOutputMappings.append(mapping);
        }
    }
}

void ControllerOutputMappingTableModel::visit(MidiControllerPreset* pMidiPreset) {
    m_pMidiPreset = pMidiPreset;
}

void ControllerOutputMappingTableModel::visit(HidControllerPreset* pHidPreset) {
    Q_UNUSED(pHidPreset);
    qDebug() << "WARNING: HID controller presets are not currently mappable.";
}

void ControllerOutputMappingTableModel::clear() {
    if (m_pMidiPreset != NULL) {
        beginRemoveRows(QModelIndex(), 0, m_midiOutputMappings.size() - 1);
        m_midiOutputMappings.clear();
        endRemoveRows();
    }
}

void ControllerOutputMappingTableModel::addEmptyOutputMapping() {
    if (m_pMidiPreset != NULL) {
        beginInsertRows(QModelIndex(), m_midiOutputMappings.size(),
                        m_midiOutputMappings.size());
        m_midiOutputMappings.append(MidiOutputMapping());
        endInsertRows();
    }
}

void ControllerOutputMappingTableModel::removeOutputMappings(QModelIndexList indices) {
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
        m_midiOutputMappings.removeAt(row);
        endRemoveRows();
    }
}

bool ControllerOutputMappingTableModel::validate() {
    // TODO(rryan): validation
    return true;
}

QAbstractItemDelegate* ControllerOutputMappingTableModel::delegateForColumn(int column, QWidget* pParent) const {
    if (m_pMidiPreset != NULL) {
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return new MidiChannelDelegate(pParent);
            case MIDI_COLUMN_OPCODE:
                return new MidiOpCodeDelegate(pParent);
            case MIDI_COLUMN_CONTROL:
            case MIDI_COLUMN_ON:
            case MIDI_COLUMN_OFF:
                return new MidiByteDelegate(pParent);
        }
    }
    return NULL;
}

int ControllerOutputMappingTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    if (m_pMidiPreset != NULL) {
        return m_midiOutputMappings.size();
    }
    return 0;
}

int ControllerOutputMappingTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    // Control and description
    const int kBaseColumns = 2;
    if (m_pMidiPreset != NULL) {
        // Channel, Opcode, Control, On, Off, Min, Max
        return kBaseColumns + 7;
    }
    return 0;
}

QVariant ControllerOutputMappingTableModel::data(const QModelIndex& index,
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
        if (row < 0 || row >= m_midiOutputMappings.size()) {
            return QVariant();
        }

        const MidiOutputMapping& mapping = m_midiOutputMappings.at(row);
        QString value;
        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return channelFromStatus(mapping.output.status);
            case MIDI_COLUMN_OPCODE:
                return opCodeFromStatus(mapping.output.status);
            case MIDI_COLUMN_CONTROL:
                return mapping.output.control;
            case MIDI_COLUMN_ON:
                return mapping.output.on;
            case MIDI_COLUMN_OFF:
                return mapping.output.off;
            case MIDI_COLUMN_MIN:
                return mapping.output.min;
            case MIDI_COLUMN_MAX:
                return mapping.output.max;
            case MIDI_COLUMN_ACTION:
                if (mapping.control.group().isEmpty() &&
                    mapping.control.item().isEmpty()) {
                    return tr("No action chosen.");
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

bool ControllerOutputMappingTableModel::setData(const QModelIndex& index,
                                                const QVariant& value,
                                                int role) {
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    int row = index.row();
    int column = index.column();

    if (m_pMidiPreset != NULL) {
        if (row < 0 || row >= m_midiOutputMappings.size()) {
            return false;
        }

        MidiOutputMapping& mapping = m_midiOutputMappings[row];

        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                mapping.output.status = static_cast<unsigned char>(
                    opCodeFromStatus(mapping.output.status)) |
                        static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_OPCODE:
                mapping.output.status = static_cast<unsigned char>(
                    channelFromStatus(mapping.output.status)) |
                        static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_CONTROL:
                mapping.output.control = static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_ON:
                mapping.output.on = static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_OFF:
                mapping.output.off = static_cast<unsigned char>(value.toInt());
                return true;
            case MIDI_COLUMN_MIN:
                mapping.output.min = value.toDouble();
                return true;
            case MIDI_COLUMN_MAX:
                mapping.output.max = value.toDouble();
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

bool ControllerOutputMappingTableModel::setHeaderData(int section,
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

QVariant ControllerOutputMappingTableModel::headerData(int section,
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

Qt::ItemFlags ControllerOutputMappingTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    return defaultFlags | Qt::ItemIsEditable;
}
