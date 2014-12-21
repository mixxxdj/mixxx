// proxytrackmodel.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QVariant>

#include "library/proxytrackmodel.h"
#include "util/assert.h"

ProxyTrackModel::ProxyTrackModel(QAbstractItemModel* pTrackModel,
                                 bool bHandleSearches)
        // ProxyTrackModel proxies settings requests to the composed TrackModel,
        // don't initialize its TrackModel with valid parameters.
        : TrackModel(QSqlDatabase(), ""),
          m_pTrackModel(dynamic_cast<TrackModel*>(pTrackModel)),
          m_currentSearch(""),
          m_bHandleSearches(bHandleSearches) {
    DEBUG_ASSERT_AND_HANDLE(m_pTrackModel && pTrackModel) {
        return;
    }
    setSourceModel(pTrackModel);
}

ProxyTrackModel::~ProxyTrackModel() {
}

int ProxyTrackModel::getTrackId(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getTrackId(indexSource) : -1;
}

const QLinkedList<int> ProxyTrackModel::getTrackRows(int trackId) const {
    return m_pTrackModel ? m_pTrackModel->getTrackRows(trackId) : QLinkedList<int>();
}

TrackPointer ProxyTrackModel::getTrack(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getTrack(indexSource) : TrackPointer();
}

QString ProxyTrackModel::getTrackLocation(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel ? m_pTrackModel->getTrackLocation(indexSource) : QString();
}

void ProxyTrackModel::search(const QString& searchText, const QString& extraFilter) {
    Q_UNUSED(extraFilter);
    if (m_bHandleSearches) {
        m_currentSearch = searchText;
        setFilterFixedString(searchText);
    } else if (m_pTrackModel) {
        m_pTrackModel->search(searchText);
    }
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

void ProxyTrackModel::moveTrack(const QModelIndex& sourceIndex,
                                const QModelIndex& destIndex) {
    QModelIndex sourceIndexSource = mapToSource(sourceIndex);
    QModelIndex destIndexSource = mapToSource(destIndex);
    if (m_pTrackModel) {
        m_pTrackModel->moveTrack(sourceIndexSource, destIndexSource);
    }
}

QAbstractItemDelegate* ProxyTrackModel::delegateForColumn(const int i, QObject* pParent) {
    return m_pTrackModel ? m_pTrackModel->delegateForColumn(i, pParent) : NULL;
}

TrackModel::CapabilitiesFlags ProxyTrackModel::getCapabilities() const {
    return m_pTrackModel ? m_pTrackModel->getCapabilities() : TrackModel::TRACKMODELCAPS_NONE;
}

bool ProxyTrackModel::filterAcceptsRow(int sourceRow,
                                       const QModelIndex& sourceParent) const {
    if (!m_bHandleSearches)
        return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);

    if (m_pTrackModel == NULL) {
        return false;
    }

    const QList<int>& filterColumns = m_pTrackModel->searchColumns();
    QAbstractItemModel* itemModel =
            dynamic_cast<QAbstractItemModel*>(m_pTrackModel);
    bool rowMatches = false;

    QRegExp filter = filterRegExp();
    QListIterator<int> iter(filterColumns);

    while (!rowMatches && iter.hasNext()) {
        int i = iter.next();
        QModelIndex index = itemModel->index(sourceRow, i, sourceParent);
        QVariant data = itemModel->data(index);
        if (qVariantCanConvert<QString>(data)) {
            QString strData = qVariantValue<QString>(data);
            if (strData.contains(filter))
                rowMatches = true;
        }
    }
    return rowMatches;
}

QString ProxyTrackModel::getModelSetting(QString name) {
    if (m_pTrackModel == NULL) {
        return QString();
    }
    return m_pTrackModel->getModelSetting(name);
}

bool ProxyTrackModel::setModelSetting(QString name, QVariant value) {
    if (m_pTrackModel == NULL) {
        return false;
    }
    return m_pTrackModel->setModelSetting(name, value);
}
