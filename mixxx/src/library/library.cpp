// library.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QItemSelectionModel>

#include "trackinfoobject.h"
#include "durationdelegate.h"
#include "library/library.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/sidebarmodel.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "library/browsefeature.h"
#include "library/rhythmboxfeature.h"
#include "library/itunesfeature.h"
#include "library/mixxxlibraryfeature.h"
#include "library/playlistfeature.h"

#include "wtracktableview.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

// This is is the name which we use to register the WTrackTableView with the
// WLibrary
const QString Library::m_sTrackViewName = QString("WTrackTableView");

Library::Library(QObject* parent, ConfigObject<ConfigValue>* pConfig)
    : m_pConfig(pConfig) {
    m_pTrackCollection = new TrackCollection();
    m_pSidebarModel = new SidebarModel(parent);
    // TODO(rryan) -- turn this construction / adding of features into a static
    // method or something -- CreateDefaultLibrary
    addFeature(new MixxxLibraryFeature(this, m_pTrackCollection));
    addFeature(new PlaylistFeature(this, m_pTrackCollection));
    addFeature(new RhythmboxFeature(this));
    addFeature(new ITunesFeature(this));
    addFeature(new BrowseFeature(this, pConfig, m_pTrackCollection));
}

Library::~Library() {
    delete m_pSidebarModel;
    //IMPORTANT: m_pTrackCollection gets destroyed via the QObject hierarchy somehow.
    //           Qt does it for us due to the way RJ wrote all this stuff.
    //delete m_pTrackCollection;
    QMutableListIterator<LibraryFeature*> features_it(m_features);;
    while(features_it.hasNext()) {
        LibraryFeature* feature = features_it.next();
        features_it.remove();
        delete feature;
    }
}

void Library::bindWidget(WLibrarySidebar* pSidebarWidget,
                         WLibrary* pLibraryWidget) {

    WTrackTableView* pTrackTableView =
        new WTrackTableView(pLibraryWidget, m_pConfig);
    connect(this, SIGNAL(searchCleared()),
            pTrackTableView, SLOT(restoreVScrollBarPos()));
	connect(this, SIGNAL(searchStarting()),
            pTrackTableView, SLOT(saveVScrollBarPos()));
    connect(this, SIGNAL(showTrackModel(QAbstractItemModel*)),
            pTrackTableView, SLOT(loadTrackModel(QAbstractItemModel*)));
    connect(pTrackTableView, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SLOT(slotLoadTrack(TrackInfoObject*)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SLOT(slotLoadTrackToPlayer(TrackInfoObject*, int)));
    pLibraryWidget->registerView(m_sTrackViewName, pTrackTableView);

    connect(this, SIGNAL(switchToView(const QString&)),
            pLibraryWidget, SLOT(switchToView(const QString&)));
    emit(switchToView(m_sTrackViewName));

    // Setup the sources view
    pSidebarWidget->setModel(m_pSidebarModel);
    connect(pSidebarWidget, SIGNAL(clicked(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));
    connect(pSidebarWidget, SIGNAL(activated(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));
    connect(pSidebarWidget, SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            m_pSidebarModel, SLOT(rightClicked(const QPoint&, const QModelIndex&)));

    // Enable the default selection
    pSidebarWidget->selectionModel()
        ->select(m_pSidebarModel->getDefaultSelection(),
                 QItemSelectionModel::SelectCurrent);
    m_pSidebarModel->activateDefaultSelection();

    QListIterator<LibraryFeature*> feature_it(m_features);
    while(feature_it.hasNext()) {
        LibraryFeature* feature = feature_it.next();
        feature->bindWidget(pSidebarWidget, pLibraryWidget);
    }
}

void Library::addFeature(LibraryFeature* feature) {
    Q_ASSERT(feature);
    m_features.push_back(feature);
    m_pSidebarModel->addLibraryFeature(feature);
    connect(feature, SIGNAL(showTrackModel(QAbstractItemModel*)),
            this, SLOT(slotShowTrackModel(QAbstractItemModel*)));
    connect(feature, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotSwitchToView(const QString&)));
    connect(feature, SIGNAL(loadTrack(TrackInfoObject*)),
            this, SLOT(slotLoadTrack(TrackInfoObject*)));
    connect(feature, SIGNAL(loadTrackToPlayer(TrackInfoObject*, int)),
            this, SLOT(slotLoadTrackToPlayer(TrackInfoObject*, int)));
}

void Library::slotShowTrackModel(QAbstractItemModel* model) {
    qDebug() << "Library::slotShowTrackModel" << model;
    emit(switchToView(m_sTrackViewName));
    emit(showTrackModel(model));
}

void Library::slotSwitchToView(const QString& view) {
    qDebug() << "Library::slotSwitchToView" << view;
    emit(switchToView(view));
}

void Library::slotSearch(const QString& text) {
    qDebug() << "Library::search" << text;
    emit(search(text));
}

void Library::slotSearchCleared() {
    qDebug() << "Library::searchCleared";
    emit(searchCleared());
}

void Library::slotSearchStarting() {
    qDebug() << "Library::searchStarting";
    emit(searchStarting());
}

void Library::slotLoadTrack(TrackInfoObject* pTrack) {
    emit(loadTrack(pTrack));
}

void Library::slotLoadTrackToPlayer(TrackInfoObject* pTrack, int player) {
    emit(loadTrackToPlayer(pTrack, player));
}
