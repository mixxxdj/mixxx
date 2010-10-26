// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/proxytrackmodel.h"
#include "library/trackcollection.h"

#define CHILD_MISSING "Missing Songs"

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pLibraryTableModel(new LibraryTableModel(this, pTrackCollection)),
      m_pMissingTableModel(new MissingTableModel(this, pTrackCollection)) {
    QStringList children;
    children << tr(CHILD_MISSING); //Insert michael jackson joke here
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
    return QIcon(":/images/library/ic_library_library.png");
}

QAbstractItemModel* MixxxLibraryFeature::getChildModel() {
    return &m_childModel;
}

void MixxxLibraryFeature::refreshLibraryModels()
{
    m_pLibraryTableModel->select();
    m_pMissingTableModel->select();
}

void MixxxLibraryFeature::activate() {
    qDebug() << "MixxxLibraryFeature::activate()";
    emit(showTrackModel(m_pLibraryTableModel));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data().toString();
    if (itemName == tr(CHILD_MISSING)) //lulz!
        emit(showTrackModel(m_pMissingTableModel));
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

void MixxxLibraryFeature::setLibraryPrefix(QString sPrefix)
{
	m_pLibraryTableModel->setLibraryPrefix(sPrefix);
}
