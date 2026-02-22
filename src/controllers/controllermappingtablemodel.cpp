#include "controllers/controllermappingtablemodel.h"

#include "controllers/midi/legacymidicontrollermapping.h"
#include "library/searchqueryparser.h"
#include "moc_controllermappingtablemodel.cpp"

ControllerMappingTableModel::ControllerMappingTableModel(QObject* pParent,
        ControlPickerMenu* pControlPickerMenu,
        QTableView* pTableView)
        : QAbstractTableModel(pParent),
          m_pControlPickerMenu(pControlPickerMenu),
          m_pTableView(pTableView),
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

ControllerMappingTableProxyModel::ControllerMappingTableProxyModel(
        ControllerMappingTableModel* sourceModel)
        : m_currentSearch("") {
    VERIFY_OR_DEBUG_ASSERT(sourceModel) {
        return;
    }
    setSourceModel(sourceModel);
    m_pModel = sourceModel;
    setSortRole(Qt::UserRole);
}

ControllerMappingTableProxyModel::~ControllerMappingTableProxyModel() {
}

void ControllerMappingTableProxyModel::search(const QString& searchText) {
    m_currentSearch = searchText;
    // This triggers the search, i.e. iterate over all rows with filterAcceptsRow(),
    // no matter if the fxedFilterString was changed.
    setFilterFixedString(searchText);
}

bool ControllerMappingTableProxyModel::filterAcceptsRow(int sourceRow,
        const QModelIndex& sourceParent) const {
    if (!m_pModel) {
        return false;
    }

    const QString currSearch = m_currentSearch.trimmed();
    if (currSearch.isEmpty()) {
        return true;
    }

    QStringList tokens = SearchQueryParser::splitQueryIntoWords(currSearch);
    tokens.removeDuplicates();

    for (const auto& token : std::as_const(tokens)) {
        bool tokenMatch = false;
        for (int column = 0; column < columnCount(); column++) {
            QModelIndex index = m_pModel->index(sourceRow, column, sourceParent);
            QString strData = m_pModel->getDisplayString(index);
            if (!strData.isEmpty()) {
                QString tokNoQuotes = token;
                tokNoQuotes.remove('\"');
                if (strData.contains(tokNoQuotes, Qt::CaseInsensitive)) {
                    tokenMatch = true;
                    tokens.removeOne(token);
                }
            }
            if (tokenMatch) {
                break;
            }
        }
    }

    if (tokens.length() > 0) {
        return false;
    }
    return true;
}
