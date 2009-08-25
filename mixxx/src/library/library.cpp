// library.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QItemSelectionModel>

#include "durationdelegate.h"
#include "library/library.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/sidebarmodel.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "library/rhythmboxfeature.h"
#include "library/mixxxlibraryfeature.h"
#include "library/playlistfeature.h"

#include "wtracktableview.h"
#include "wtracksourcesview.h"

Library::Library(QObject* parent) {

    m_pTrackCollection = new TrackCollection();
    m_pSidebarModel = new SidebarModel(parent);
    // TODO -- turn this construction / adding of features into a static method
    // or something -- CreateDefaultLibrary
    addFeature(new MixxxLibraryFeature(this, m_pTrackCollection));
    addFeature(new PlaylistFeature(this, m_pTrackCollection));
    addFeature(new RhythmboxFeature(this));
}

Library::~Library() {
    delete m_pSidebarModel;
    delete m_pTrackCollection;
}

void Library::bindWidget(WTrackSourcesView* pSourcesView,
                         WTrackTableView* pTableView) {
    connect(this, SIGNAL(showTrackModel(QAbstractItemModel*)),
            pTableView, SLOT(loadTrackModel(QAbstractItemModel*)));

    // Setup the sources view
    pSourcesView->setModel(m_pSidebarModel);
    connect(pSourcesView, SIGNAL(clicked(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));
    connect(pSourcesView, SIGNAL(activated(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));

    // Enable the default selection
    pSourcesView->selectionModel()->select(m_pSidebarModel->getDefaultSelection(),
                                           QItemSelectionModel::SelectCurrent);
    m_pSidebarModel->activateDefaultSelection();
}

void Library::addFeature(LibraryFeature* feature) {
    m_sFeatures.push_back(feature);
    m_pSidebarModel->addLibraryFeature(feature);
    connect(feature, SIGNAL(showTrackModel(QAbstractItemModel*)),
            this, SLOT(slotShowTrackModel(QAbstractItemModel*)));
}

void Library::slotShowTrackModel(QAbstractItemModel* model) {
    qDebug() << "Library::slotShowTrackModel" << model;
    emit(showTrackModel(model));
}
