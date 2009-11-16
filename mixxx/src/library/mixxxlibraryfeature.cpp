// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"

#define CHILD_MISSING tr("Missing Songs")

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pLibraryTableModel(new LibraryTableModel(this, pTrackCollection)),
      m_pLibraryTableModelProxy(new ProxyTrackModel(m_pLibraryTableModel, false)),
      m_pMissingTableModel(new MissingTableModel(this, pTrackCollection)),
      m_pMissingTableModelProxy(new ProxyTrackModel(m_pMissingTableModel, false)) {
    m_pLibraryTableModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_pMissingTableModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    QStringList children;
    children << CHILD_MISSING; //Insert michael jackson joke here
    m_childModel.setStringList(children);
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    // TODO(XXX) delete these
    //delete m_pLibraryTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return tr("Library");
}

QIcon MixxxLibraryFeature::getIcon() {
    return QIcon();
}

QAbstractItemModel* MixxxLibraryFeature::getChildModel() {
    return &m_childModel;
}

void MixxxLibraryFeature::activate() {
    qDebug() << "MixxxLibraryFeature::activate()";
    emit(showTrackModel(m_pLibraryTableModel));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data().toString();
    if (itemName == CHILD_MISSING) //lulz!
        emit(showTrackModel(m_pMissingTableModelProxy));
}

void MixxxLibraryFeature::onRightClick(const QPoint& globalPos) {
}

void MixxxLibraryFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool MixxxLibraryFeature::dropAccept(QUrl url) {
    return false;
}

bool MixxxLibraryFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool MixxxLibraryFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}
