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
    //m_pLibraryTableModel = new LibraryTableModel(NULL, m_pTrackCollection);
    m_pSidebarModel = new SidebarModel(parent);
    addFeature(new MixxxLibraryFeature(this, m_pTrackCollection));
    addFeature(new PlaylistFeature(this));
    addFeature(new RhythmboxFeature(this));
}

Library::~Library() {
    delete m_pSidebarModel;
    //delete m_pLibraryTableModel;
    delete m_pTrackCollection;
}

void Library::bindWidget(WTrackSourcesView* pSourcesView,
                         WTrackTableView* pTableView) {

    //pTableView->setModel(m_pLibraryTableModel);
    
    //Set up custom view delegates to display things in the track table in
    //special ways.
    
    // const int durationColumnIndex =
    //     m_pLibraryTableModel->fieldIndex(LIBRARYTABLE_DURATION);

    //Set up the duration delegate to handle displaying the track durations in
    //hh:mm:ss rather than what's pulled from the database (which is just the
    //duration in seconds).
    // DurationDelegate *delegate = new DurationDelegate();
    // pTableView->setItemDelegateForColumn(durationColumnIndex, delegate);

    //Needs to be done after the data model is set. Also note that calling
    //restoreVScrollBarPos() here seems to slow down Mixxx's startup
    //somewhat. Might be causing some massive SQL query to run at startup.
    pTableView->restoreVScrollBarPos();

    connect(this, SIGNAL(showTrackModel(QAbstractItemModel*)),
            pTableView, SLOT(loadTrackModel(QAbstractItemModel*)));

    // Setup the sources view
    pSourcesView->setModel(m_pSidebarModel);
    connect(pSourcesView, SIGNAL(clicked(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));
    connect(pSourcesView, SIGNAL(activated(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));

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
