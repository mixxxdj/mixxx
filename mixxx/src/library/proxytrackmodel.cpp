// proxytrackmodel.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include "library/proxytrackmodel.h"

ProxyTrackModel::ProxyTrackModel(TrackModel* pTrackModel)
        : m_pTrackModel(pTrackModel) {
}

ProxyTrackModel::~ProxyTrackModel() {
}

TrackInfoObject* ProxyTrackModel::getTrack(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel->getTrack(indexSource);
}

QString ProxyTrackModel::getTrackLocation(const QModelIndex& index) const {
    QModelIndex indexSource = mapToSource(index);
    return m_pTrackModel->getTrackLocation(indexSource);
}

void ProxyTrackModel::search(const QString& searchText) {
    m_currentSearch = searchText;
    setFilterFixedString(searchText);
}

const QString ProxyTrackModel::currentSearch() {
    return m_currentSearch;
}

void ProxyTrackModel::removeTrack(const QModelIndex& index) {
    QModelIndex indexSource = mapToSource(index);
    m_pTrackModel->removeTrack(indexSource);
}

void ProxyTrackModel::addTrack(const QModelIndex& index, QString location) {
    QModelIndex indexSource = mapToSource(index);
    m_pTrackModel->addTrack(indexSource, location);
}

void ProxyTrackModel::moveTrack(const QModelIndex& sourceIndex,
                                const QModelIndex& destIndex) {
    QModelIndex sourceIndexSource = mapToSource(sourceIndex);
    QModelIndex destIndexSource = mapToSource(destIndex);
    m_pTrackModel->moveTrack(sourceIndexSource, destIndexSource);
}

QItemDelegate* ProxyTrackModel::delegateForColumn(const int i) {
    return m_pTrackModel->delegateForColumn(i);
}

TrackModel::CapabilitiesFlags ProxyTrackModel::getCapabilities() const {
    return m_pTrackModel->getCapabilities();
}

