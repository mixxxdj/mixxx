#include "controllers/controllerinputmappingtablemodel.h"

#include <QTableView>

#include "controllers/delegates/controldelegate.h"
#include "controllers/delegates/midibytedelegate.h"
#include "controllers/delegates/midichanneldelegate.h"
#include "controllers/delegates/midiopcodedelegate.h"
#include "controllers/delegates/midioptionsdelegate.h"
#include "controllers/midi/legacymidicontrollermapping.h"
#include "controllers/midi/midimessage.h"
#include "controllers/midi/midiutils.h"
#include "moc_controllerinputmappingtablemodel.cpp"

ControllerInputMappingTableModel::ControllerInputMappingTableModel(QObject* pParent,
        ControlPickerMenu* pControlPickerMenu,
        QTableView* pTableView)
        : ControllerMappingTableModel(pParent, pControlPickerMenu, pTableView) {
}

ControllerInputMappingTableModel::~ControllerInputMappingTableModel() {
}

void ControllerInputMappingTableModel::apply() {
    if (m_pMidiMapping != nullptr) {
        // Clear existing input mappings and insert all the input mappings in
        // the table into the mapping.
        QMultiHash<uint16_t, MidiInputMapping> mappings;
        for (const MidiInputMapping& mapping : std::as_const(m_midiInputMappings)) {
            // There can be multiple input mappings for the same input
            // MidiKey, so we need to use a QMultiHash here.
            mappings.insert(mapping.key.key, mapping);
        }
        m_pMidiMapping->setInputMappings(mappings);
    }
}

void ControllerInputMappingTableModel::onMappingLoaded() {
    clear();

    if (m_pMidiMapping != nullptr) {
        // TODO(rryan): Tooltips
        setHeaderData(MIDI_COLUMN_CHANNEL, Qt::Horizontal, tr("Channel"));
        setHeaderData(MIDI_COLUMN_OPCODE, Qt::Horizontal, tr("Opcode"));
        setHeaderData(MIDI_COLUMN_CONTROL, Qt::Horizontal, tr("Control"));
        setHeaderData(MIDI_COLUMN_OPTIONS, Qt::Horizontal, tr("Options"));
        setHeaderData(MIDI_COLUMN_ACTION, Qt::Horizontal, tr("Action"));
        setHeaderData(MIDI_COLUMN_COMMENT, Qt::Horizontal, tr("Comment"));

        if (!m_pMidiMapping->getInputMappings().isEmpty()) {
            beginInsertRows(QModelIndex(), 0, m_pMidiMapping->getInputMappings().size() - 1);
            m_midiInputMappings = m_pMidiMapping->getInputMappings().values();
            endInsertRows();
        }
        connect(this,
                &QAbstractTableModel::dataChanged,
                this,
                [this]() {
                    m_pMidiMapping->setDirty(true);
                });
    }
}

void ControllerInputMappingTableModel::clear() {
    if (m_pMidiMapping != nullptr) {
        if (!m_midiInputMappings.isEmpty()) {
            beginRemoveRows(QModelIndex(), 0, m_midiInputMappings.size() - 1);
            m_midiInputMappings.clear();
            endRemoveRows();
        }
    }
}

void ControllerInputMappingTableModel::addMappings(const MidiInputMappings& mappings) {
    if (mappings.isEmpty()) {
        return;
    }

    if (m_pMidiMapping != nullptr) {
        // When we add mappings from controller learning, we first remove the
        // duplicates from the table. We allow multiple mappings per MIDI
        // message but MIDI learning over-writes duplicates instead of adding.

        if (!m_midiInputMappings.isEmpty()) {
            QSet<uint16_t> keys;
            foreach (const MidiInputMapping& mapping, mappings) {
                keys.insert(mapping.key.key);
            }

            for (int row = m_midiInputMappings.size() - 1; row >= 0; row--) {
                if (keys.contains(m_midiInputMappings.at(row).key.key)) {
                    beginRemoveRows(QModelIndex(), row, row);
                    m_midiInputMappings.removeAt(row);
                    endRemoveRows();
                }
            }
        }

        beginInsertRows(QModelIndex(), m_midiInputMappings.size(),
                        m_midiInputMappings.size() + mappings.size() - 1);
        m_midiInputMappings.append(mappings);
        endInsertRows();
    }
}

void ControllerInputMappingTableModel::addEmptyMapping() {
    if (m_pMidiMapping != nullptr) {
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
    std::sort(rows.begin(), rows.end());

    int lastRow = -1;
    while (!rows.empty()) {
        int row = rows.takeLast();
        if (row == lastRow) {
            continue;
        }

        lastRow = row;
        beginRemoveRows(QModelIndex(), row, row);
        m_midiInputMappings.removeAt(row);
        endRemoveRows();
    }
}

QAbstractItemDelegate* ControllerInputMappingTableModel::delegateForColumn(
        int column, QWidget* pParent) {
    if (m_pMidiMapping != nullptr) {
        ControlDelegate* pControlDelegate = nullptr;
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
                pControlDelegate = new ControlDelegate(this, m_pControlPickerMenu);
                pControlDelegate->setMidiOptionsColumn(MIDI_COLUMN_OPTIONS);
                return pControlDelegate;
        }
    }
    return nullptr;
}

int ControllerInputMappingTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    if (m_pMidiMapping != nullptr) {
        return m_midiInputMappings.size();
    }
    return 0;
}

int ControllerInputMappingTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    // Control and description.
    constexpr int kBaseColumns = 2;
    if (m_pMidiMapping != nullptr) {
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

    if (m_pMidiMapping != nullptr) {
        if (row < 0 || row >= m_midiInputMappings.size()) {
            return QVariant();
        }

        const MidiInputMapping& mapping = m_midiInputMappings.at(row);

        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                return MidiUtils::channelFromStatus(mapping.key.status);
            case MIDI_COLUMN_OPCODE:
                return MidiUtils::opCodeValue(MidiUtils::opCodeFromStatus(mapping.key.status));
            case MIDI_COLUMN_CONTROL:
                if (MidiUtils::isMessageTwoBytes(mapping.key.status)) {
                return mapping.key.control;
                } else {
                return QVariant();
                }
            case MIDI_COLUMN_OPTIONS:
                // UserRole is used for sorting.
                if (role == Qt::UserRole) {
                    return QVariant(mapping.options);
                }
                return QVariant::fromValue(mapping.options);
            case MIDI_COLUMN_ACTION: {
                const auto* const pControl = std::get_if<ConfigKey>(&mapping.control);
                if (!pControl) {
                    const auto* const ppJSValue =
                            std::get_if<std::shared_ptr<QJSValue>>(
                                    &mapping.control);
                    if (ppJSValue && *ppJSValue) {
                    return QVariant::fromValue((*ppJSValue)->toString());
                    }
                    return QVariant();
                }
                if (role == Qt::UserRole) { // sort by displaystring
                    QStyledItemDelegate* del = getDelegateForIndex(index);
                    VERIFY_OR_DEBUG_ASSERT(del) {
                    return QVariant();
                    }
                    return del->displayText(QVariant::fromValue(*pControl), QLocale());
                }
                return QVariant::fromValue(*pControl);
            }
            case MIDI_COLUMN_COMMENT:
                return mapping.description;
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QString ControllerInputMappingTableModel::getDisplayString(const QModelIndex& index) const {
    if (!m_pMidiMapping || !m_pTableView || !index.isValid()) {
        return QString();
    }

    int row = index.row();
    int column = index.column();
    const MidiInputMapping& mapping = m_midiInputMappings.at(row);

    if (!std::holds_alternative<ConfigKey>(mapping.control)) {
        return QString();
    }

    switch (column) {
    case MIDI_COLUMN_COMMENT: {
        return mapping.description;
    }
    case MIDI_COLUMN_CHANNEL:
    case MIDI_COLUMN_OPCODE:
    case MIDI_COLUMN_CONTROL:
    case MIDI_COLUMN_OPTIONS: {
        QStyledItemDelegate* del = getDelegateForIndex(index);
        VERIFY_OR_DEBUG_ASSERT(del) {
            return QString();
        }
        return del->displayText(data(index, Qt::DisplayRole), QLocale());
    }
    case MIDI_COLUMN_ACTION: {
        QStyledItemDelegate* del = getDelegateForIndex(index);
        VERIFY_OR_DEBUG_ASSERT(del) {
            return QString();
        }
        QString displayText;
        // Return both the raw ConfigKey group + key and the translated display
        // string and the translated description from ControlPickerMenu.
        // Note: this may contain duplicate key strings in case this is an
        // untranslated script control
        const auto* const pControl = std::get_if<ConfigKey>(&mapping.control);
        if (pControl) {
            displayText = pControl->group + QChar(',') + pControl->item + QChar(' ');
        }
        return displayText + data(index, Qt::UserRole).toString();
    }
    default:
        return QString();
    }
}

bool ControllerInputMappingTableModel::setData(const QModelIndex& index,
                                               const QVariant& value,
                                               int role) {
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    int row = index.row();
    int column = index.column();

    if (m_pMidiMapping != nullptr) {
        if (row < 0 || row >= m_midiInputMappings.size()) {
            return false;
        }

        MidiInputMapping& mapping = m_midiInputMappings[row];

        switch (column) {
            case MIDI_COLUMN_CHANNEL:
                mapping.key.status = static_cast<unsigned char>(
                    MidiUtils::opCodeFromStatus(mapping.key.status)) |
                        static_cast<unsigned char>(value.toInt());
                emit dataChanged(index, index);
                return true;
            case MIDI_COLUMN_OPCODE:
                mapping.key.status = static_cast<unsigned char>(
                    MidiUtils::channelFromStatus(mapping.key.status)) |
                        static_cast<unsigned char>(value.toInt());
                emit dataChanged(index, index);
                return true;
            case MIDI_COLUMN_CONTROL:
                mapping.key.control = static_cast<unsigned char>(value.toInt());
                emit dataChanged(index, index);
                return true;
            case MIDI_COLUMN_OPTIONS:
                mapping.options = value.value<MidiOptions>();
                emit dataChanged(index, index);
                return true;
            case MIDI_COLUMN_ACTION:
                mapping.control = value.value<ConfigKey>();
                emit dataChanged(index, index);
                return true;
            case MIDI_COLUMN_COMMENT:
                mapping.description = value.toString();
                emit dataChanged(index, index);
                return true;
            default:
                return false;
        }
    }

    return false;
}
