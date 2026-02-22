#include "library/proxytrackmodel.h"

#include "library/browse/browsetablemodel.h"
#include "library/searchqueryparser.h"
#include "moc_proxytrackmodel.cpp"
#include "util/assert.h"

ProxyTrackModel::ProxyTrackModel(QAbstractItemModel* pTrackModel,
        bool bHandleSearches)
        // ProxyTrackModel proxies settings requests to the composed TrackModel,
        // don't initialize its TrackModel with valid parameters.
        : TrackModel(QSqlDatabase(), ""),
          m_pTrackModel(dynamic_cast<TrackModel*>(pTrackModel)),
          m_currentSearch(""),
          m_bHandleSearches(bHandleSearches) {
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel && pTrackModel) {
        return;
    }
    setSourceModel(pTrackModel);
}

ProxyTrackModel::~ProxyTrackModel() {
}

void ProxyTrackModel::maybeStopModelPopulation() {
    // If the source model is a BrowseTableModel we tell it to stop its
    // BrowseThread. In case that is still running and population the model
    // chunk by chunk, its update signals would still affect the selection in the
    // shared WTrackTableView even if another model is loaded.
    BrowseTableModel* pBrowseModel = static_cast<BrowseTableModel*>(sourceModel());
    if (pBrowseModel) {
        pBrowseModel->stopBrowseThread();
    }
};

TrackModel::SortColumnId ProxyTrackModel::sortColumnIdFromColumnIndex(int index) const {
    return m_pTrackModel ? m_pTrackModel->sortColumnIdFromColumnIndex(index)
                         : TrackModel::SortColumnId::Invalid;
}

int ProxyTrackModel::columnIndexFromSortColumnId(TrackModel::SortColumnId sortColumn) const {
    return m_pTrackModel ? m_pTrackModel->columnIndexFromSortColumnId(sortColumn)
                         : -1;
}

TrackId ProxyTrackModel::getTrackId(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getTrackId(indexSource) : TrackId();
}

QUrl ProxyTrackModel::getTrackUrl(const QModelIndex& index) const {
    if (!m_pTrackModel) {
        return {};
    }
    return m_pTrackModel->getTrackUrl(mapToSource(index));
}

CoverInfo ProxyTrackModel::getCoverInfo(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getCoverInfo(indexSource) : CoverInfo();
}

const QVector<int> ProxyTrackModel::getTrackRows(TrackId trackId) const {
    return m_pTrackModel ? m_pTrackModel->getTrackRows(trackId) : QVector<int>();
}

TrackPointer ProxyTrackModel::getTrack(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getTrack(indexSource) : TrackPointer();
}

TrackPointer ProxyTrackModel::getTrackByRef(const TrackRef& trackRef) const {
    return m_pTrackModel ? m_pTrackModel->getTrackByRef(trackRef) : TrackPointer();
}

QString ProxyTrackModel::getTrackLocation(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getTrackLocation(indexSource) : QString();
}

void ProxyTrackModel::search(const QString& searchText) {
    if (m_bHandleSearches) {
        m_currentSearch = searchText;
        setFilterFixedString(searchText);
    } else if (m_pTrackModel) {
        m_pTrackModel->search(searchText);
    }
}

QString ProxyTrackModel::modelKey(bool noSearch) const {
    if (m_pTrackModel) {
        if (m_bHandleSearches) {
            return m_pTrackModel->modelKey(true) + QStringLiteral("#") + currentSearch();
        } else {
            return m_pTrackModel->modelKey(noSearch);
        }
    }
    return QString();
}

const QString ProxyTrackModel::currentSearch() const {
    if (m_bHandleSearches) {
        return m_currentSearch;
    }
    return m_pTrackModel ? m_pTrackModel->currentSearch() : QString();
}

bool ProxyTrackModel::isColumnInternal(int column) {
    return m_pTrackModel ? m_pTrackModel->isColumnInternal(column) : false;
}

bool ProxyTrackModel::isColumnHiddenByDefault(int column) {
    return m_pTrackModel ? m_pTrackModel->isColumnHiddenByDefault(column) : false;
}

void ProxyTrackModel::removeTracks(const QModelIndexList& indices) {
    QModelIndexList translatedList;
    foreach (QModelIndex index, indices) {
        QModelIndex indexSource = mapToSource(index);
        translatedList.append(indexSource);
    }
    if (m_pTrackModel) {
        m_pTrackModel->removeTracks(translatedList);
    }
}

void ProxyTrackModel::copyTracks(const QModelIndexList& indices) const {
    QModelIndexList translatedList;
    foreach (QModelIndex index, indices) {
        QModelIndex indexSource = mapToSource(index);
        translatedList.append(indexSource);
    }
    if (m_pTrackModel) {
        m_pTrackModel->copyTracks(translatedList);
    }
}

void ProxyTrackModel::moveTrack(const QModelIndex& sourceIndex,
        const QModelIndex& destIndex) {
    QModelIndex sourceIndexSource = mapToSource(sourceIndex);
    QModelIndex destIndexSource = mapToSource(destIndex);
    if (m_pTrackModel) {
        m_pTrackModel->moveTrack(sourceIndexSource, destIndexSource);
    }
}

QAbstractItemDelegate* ProxyTrackModel::delegateForColumn(const int i, QObject* pParent) {
    return m_pTrackModel ? m_pTrackModel->delegateForColumn(i, pParent) : nullptr;
}

TrackModel::Capabilities ProxyTrackModel::getCapabilities() const {
    return m_pTrackModel ? m_pTrackModel->getCapabilities() : Capability::None;
}

bool ProxyTrackModel::updateTrackGenre(
        Track* pTrack,
        const QString& genre) const {
    return m_pTrackModel ? m_pTrackModel->updateTrackGenre(pTrack, genre) : false;
}

#if defined(__EXTRA_METADATA__)
bool ProxyTrackModel::updateTrackMood(
        Track* pTrack,
        const QString& mood) const {
    return m_pTrackModel ? m_pTrackModel->updateTrackMood(pTrack, mood) : false;
}
#endif // __EXTRA_METADATA__

bool ProxyTrackModel::filterAcceptsRow(int sourceRow,
        const QModelIndex& sourceParent) const {
    if (!m_bHandleSearches) {
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
    }

    if (m_pTrackModel == nullptr) {
        return false;
    }

    const QString currSearch = m_currentSearch.trimmed();
    if (currSearch.isEmpty()) {
        return true;
    }

    QStringList tokens = SearchQueryParser::splitQueryIntoWords(currSearch);
    tokens.removeDuplicates();

    const QList<int>& filterColumns = m_pTrackModel->searchColumns();
    QAbstractItemModel* itemModel =
            dynamic_cast<QAbstractItemModel*>(m_pTrackModel);

    for (const auto& token : std::as_const(tokens)) {
        bool tokenMatch = false;
        for (const auto column : std::as_const(filterColumns)) {
            QModelIndex index = itemModel->index(sourceRow, column, sourceParent);
            QString strData = itemModel->data(index).toString();
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

QString ProxyTrackModel::getModelSetting(const QString& name) {
    if (m_pTrackModel == nullptr) {
        return QString();
    }
    return m_pTrackModel->getModelSetting(name);
}

bool ProxyTrackModel::setModelSetting(const QString& name, const QVariant& value) {
    if (m_pTrackModel == nullptr) {
        return false;
    }
    return m_pTrackModel->setModelSetting(name, value);
}

void ProxyTrackModel::sort(int column, Qt::SortOrder order) {
    if (m_pTrackModel->isColumnSortable(column)) {
        QSortFilterProxyModel::sort(column, order);
    }
}
