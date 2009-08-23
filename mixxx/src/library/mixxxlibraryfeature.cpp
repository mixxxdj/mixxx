// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/trackcollection.h"
#include "library/librarytablemodel.h"
#include "library/mixxxlibraryfeature.h"

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pLibraryTableModel(new LibraryTableModel(this, pTrackCollection)) {
}

MixxxLibraryFeature::~MixxxLibraryFeature() {

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
    emit(showTrackModel(m_pLibraryTableModel));
}

void MixxxLibraryFeature::activateChild(int n) {

}

void MixxxLibraryFeature::onRightClick(QModelIndex index) {

}

void MixxxLibraryFeature::onClick(QModelIndex index) {

}
