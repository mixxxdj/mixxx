#include "controllers/controllermappingtablemodel.h"

#include "moc_controllermappingtablemodel.cpp"

ControllerMappingTableModel::ControllerMappingTableModel(QObject* pParent)
        : QAbstractTableModel(pParent),
          m_pMidiPreset(nullptr),
          m_pHidPreset(nullptr) {
}

ControllerMappingTableModel::~ControllerMappingTableModel() {

}

void ControllerMappingTableModel::setPreset(ControllerPresetPointer pPreset) {
    m_pPreset = pPreset;
    if (m_pPreset) {
        // This immediately calls one of the two visit() methods below.
        m_pPreset->accept(this);
    }

    // Notify the child class a preset was loaded.
    onPresetLoaded();
}

void ControllerMappingTableModel::cancel() {
    // Apply mutates the preset so to revert to the time just before the last
    // apply, simply call setPreset again.
    setPreset(m_pPreset);
}

void ControllerMappingTableModel::visit(MidiControllerPreset* pMidiPreset) {
    m_pMidiPreset = pMidiPreset;
}

void ControllerMappingTableModel::visit(HidControllerPreset* pHidPreset) {
    m_pHidPreset = pHidPreset;
}

bool ControllerMappingTableModel::setHeaderData(int section,
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
    emit headerDataChanged(orientation, section, section);
    return true;
}

QVariant ControllerMappingTableModel::headerData(int section,
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

Qt::ItemFlags ControllerMappingTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);
    return defaultFlags | Qt::ItemIsEditable;
}
