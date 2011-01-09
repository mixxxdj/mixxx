// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"

#include "library/trackcollection.h"
#include "treeitem.h"

#define CHILD_MISSING "Missing Songs"

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
    : LibraryFeature(parent),
      m_pLibraryTableModel(new LibraryTableModel(this, pTrackCollection)),
      m_pMissingTableModel(new MissingTableModel(this, pTrackCollection)) {
    
    TreeItem *rootItem = new TreeItem("$root","$root", this);
    TreeItem *childItem = new TreeItem(CHILD_MISSING,CHILD_MISSING, this,rootItem);
    rootItem->appendChild(childItem);
    m_childModel.setRootItem(rootItem);    
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

TreeItemModel* MixxxLibraryFeature::getChildModel() {
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

    /*if (itemName == m_childModel.stringList().at(0))
        emit(showTrackModel(m_pMissingTableModel));
     */
    if (itemName == CHILD_MISSING) 
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
