#include "controllers/controllermappingtablemodel.h"

#include "moc_controllermappingtablemodel.cpp"

ControllerMappingTableModel::ControllerMappingTableModel(QObject* pParent)
        : QAbstractTableModel(pParent),
          m_pMidiMapping(nullptr) {
}

ControllerMappingTableModel::~ControllerMappingTableModel() {

}

void ControllerMappingTableModel::setMapping(std::shared_ptr<LegacyControllerMapping> pMapping) {
    m_pMidiMapping = std::dynamic_pointer_cast<LegacyMidiControllerMapping>(pMapping);
    // Only legacy MIDI mappings are supported
    // TODO: prevent calling this code for unsupported mapping types?
    if (!m_pMidiMapping) {
        return;
    }

    // Notify the child class a mapping was loaded.
    onMappingLoaded();
}

void ControllerMappingTableModel::cancel() {
    // Apply mutates the mapping so to revert to the time just before the last
    // apply, simply reload the mapping.
    onMappingLoaded();
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
