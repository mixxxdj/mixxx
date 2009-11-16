// preparefeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)
// Forked 11/11/2009 by Albert Santoni (alberts@mixxx.org)

#include <QtDebug>

#include "library/preparefeature.h"
#include "library/librarytablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"


PrepareFeature::PrepareFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent) {
//      m_pLibraryTableModel(new LibraryTableModel(this, pTrackCollection)),
//      m_pLibraryTableModelProxy(new ProxyTrackModel(m_pLibraryTableModel, false)),
//      m_pLibraryTableModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

}

PrepareFeature::~PrepareFeature() {
    // TODO(XXX) delete these
    //delete m_pLibraryTableModel;
}

QVariant PrepareFeature::title() {
    return tr("Prepare");
}

QIcon PrepareFeature::getIcon() {
    return QIcon();
}

QAbstractItemModel* PrepareFeature::getChildModel() {
    return &m_childModel;
}

void PrepareFeature::activate() {
    qDebug() << "PrepareFeature::activate()";
    emit(switchToView("Prepare"));
}

void PrepareFeature::activateChild(const QModelIndex& index) {
}

void PrepareFeature::onRightClick(const QPoint& globalPos) {
}

void PrepareFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool PrepareFeature::dropAccept(QUrl url) {
    return false;
}

bool PrepareFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool PrepareFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool PrepareFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}
