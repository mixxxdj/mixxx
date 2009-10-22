// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/librarytablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pLibraryTableModel(new LibraryTableModel(this, pTrackCollection)),
      m_pLibraryTableModelProxy(new ProxyTrackModel(m_pLibraryTableModel, false)) {
    m_pLibraryTableModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    //delete m_pLibraryTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return tr("Library");
}

QIcon MixxxLibraryFeature::getIcon() {
    return QIcon();
}

int MixxxLibraryFeature::numChildren() {
    return 0;
}

QVariant MixxxLibraryFeature::child(int n) {
    return QVariant();
}

void MixxxLibraryFeature::activate() {
    qDebug() << "MixxxLibraryFeature::activate()";
    emit(showTrackModel(m_pLibraryTableModelProxy));
}

void MixxxLibraryFeature::activateChild(int n) {

}

void MixxxLibraryFeature::onRightClick(const QPoint& globalPos, QModelIndex index) {

}

void MixxxLibraryFeature::onClick(QModelIndex index) {

}

bool MixxxLibraryFeature::dropAccept(const QModelIndex& index, QUrl url)
{
    return false;
}

bool MixxxLibraryFeature::dragMoveAccept(const QModelIndex& index, QUrl url)
{
    return false;
}
