// library.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QItemSelectionModel>

#include "library/library.h"
#include "library/libraryfeature.h"
#include "library/librarytablemodel.h"
#include "library/sidebarmodel.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "library/browse/browsefeature.h"
#include "library/cratefeature.h"
#include "library/rhythmbox/rhythmboxfeature.h"
#include "library/recording/recordingfeature.h"
#include "library/itunes/itunesfeature.h"
#include "library/mixxxlibraryfeature.h"
#include "library/autodjfeature.h"
#include "library/playlistfeature.h"
#include "library/preparefeature.h"
#ifdef __PROMO__
#include "library/promotracksfeature.h"
#endif
#include "library/traktor/traktorfeature.h"
#include "library/librarycontrol.h"
#include "library/setlogfeature.h"

#include "widget/wtracktableview.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

#include "mixxxkeyboard.h"

// This is is the name which we use to register the WTrackTableView with the
// WLibrary
const QString Library::m_sTrackViewName = QString("WTrackTableView");

Library::Library(QObject* parent, ConfigObject<ConfigValue>* pConfig, bool firstRun,
                 RecordingManager* pRecordingManager) :
        m_pConfig(pConfig),
        m_pSidebarModel(new SidebarModel(parent)),
        m_pTrackCollection(new TrackCollection(pConfig)),
        m_pLibraryControl(new LibraryControl),
        m_pRecordingManager(pRecordingManager) {

    // TODO(rryan) -- turn this construction / adding of features into a static
    // method or something -- CreateDefaultLibrary
    m_pMixxxLibraryFeature = new MixxxLibraryFeature(this, m_pTrackCollection,m_pConfig);
    addFeature(m_pMixxxLibraryFeature);

#ifdef __PROMO__
    if (PromoTracksFeature::isSupported(m_pConfig)) {
        m_pPromoTracksFeature = new PromoTracksFeature(this, pConfig,
                                                       m_pTrackCollection,
                                                       firstRun);
        addFeature(m_pPromoTracksFeature);
    } else {
        m_pPromoTracksFeature = NULL;
    }
#endif

    addFeature(new AutoDJFeature(this, pConfig, m_pTrackCollection));
    m_pPlaylistFeature = new PlaylistFeature(this, m_pTrackCollection, pConfig);
    addFeature(m_pPlaylistFeature);
    m_pCrateFeature = new CrateFeature(this, m_pTrackCollection, pConfig);
    addFeature(m_pCrateFeature);
    addFeature(new BrowseFeature(this, pConfig, m_pTrackCollection, m_pRecordingManager));
    addFeature(new RecordingFeature(this, pConfig, m_pTrackCollection, m_pRecordingManager));
    addFeature(new SetlogFeature(this, pConfig, m_pTrackCollection));
    m_pPrepareFeature = new PrepareFeature(this, pConfig, m_pTrackCollection);
    addFeature(m_pPrepareFeature);
    //iTunes and Rhythmbox should be last until we no longer have an obnoxious
    //messagebox popup when you select them. (This forces you to reach for your
    //mouse or keyboard if you're using MIDI control and you scroll through them...)
    if (RhythmboxFeature::isSupported() &&
        pConfig->getValueString(ConfigKey("[Library]","ShowRhythmboxLibrary"),"1").toInt()) {
        addFeature(new RhythmboxFeature(this, m_pTrackCollection));
    }
    if (ITunesFeature::isSupported() &&
        pConfig->getValueString(ConfigKey("[Library]","ShowITunesLibrary"),"1").toInt()) {
        addFeature(new ITunesFeature(this, m_pTrackCollection));
    }
    if (TraktorFeature::isSupported() &&
        pConfig->getValueString(ConfigKey("[Library]","ShowTraktorLibrary"),"1").toInt()) {
        addFeature(new TraktorFeature(this, m_pTrackCollection));
    }

    //Show the promo tracks view on first run, otherwise show the library
    if (firstRun) {
        //qDebug() << "First Run, switching to PROMO view!";
        //This doesn't trigger onShow()... argh
        //m_pSidebarModel->setDefaultSelection(1);
        //slotSwitchToView(tr("Bundled Songs"));
        //Note the promo tracks item has index=1... hardcoded hack. :/
    }
}

Library::~Library() {
    // Delete the sidebar model first since it depends on the LibraryFeatures.
    delete m_pSidebarModel;

    QMutableListIterator<LibraryFeature*> features_it(m_features);
    while(features_it.hasNext()) {
        LibraryFeature* feature = features_it.next();
        features_it.remove();
        delete feature;
    }

    delete m_pLibraryControl;
    //IMPORTANT: m_pTrackCollection gets destroyed via the QObject hierarchy somehow.
    //           Qt does it for us due to the way RJ wrote all this stuff.
    //Update:  - OR NOT! As of Dec 8, 2009, this pointer must be destroyed manually otherwise
    // we never see the TrackCollection's destructor being called... - Albert
    // Has to be deleted at last because the features holds references of it.
    delete m_pTrackCollection;
}

void Library::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    m_pLibraryControl->bindSidebarWidget(pSidebarWidget);

    // Setup the sources view
    pSidebarWidget->setModel(m_pSidebarModel);
    connect(m_pSidebarModel, SIGNAL(selectIndex(const QModelIndex&)),
            pSidebarWidget, SLOT(selectIndex(const QModelIndex&)));
    connect(pSidebarWidget, SIGNAL(pressed(const QModelIndex&)),
            m_pSidebarModel, SLOT(clicked(const QModelIndex&)));
    // Lazy model: Let triangle symbol increment the model
    connect(pSidebarWidget, SIGNAL(expanded(const QModelIndex&)),
            m_pSidebarModel, SLOT(doubleClicked(const QModelIndex&)));

    connect(pSidebarWidget, SIGNAL(rightClicked(const QPoint&, const QModelIndex&)),
            m_pSidebarModel, SLOT(rightClicked(const QPoint&, const QModelIndex&)));
}

void Library::bindWidget(WLibrary* pLibraryWidget,
                         MixxxKeyboard* pKeyboard) {
    WTrackTableView* pTrackTableView =
            new WTrackTableView(pLibraryWidget, m_pConfig, m_pTrackCollection);
    pTrackTableView->installEventFilter(pKeyboard);
    connect(this, SIGNAL(showTrackModel(QAbstractItemModel*)),
            pTrackTableView, SLOT(loadTrackModel(QAbstractItemModel*)));
    connect(pTrackTableView, SIGNAL(loadTrack(TrackPointer)),
            this, SLOT(slotLoadTrack(TrackPointer)));
    connect(pTrackTableView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SLOT(slotLoadTrackToPlayer(TrackPointer, QString, bool)));
    pLibraryWidget->registerView(m_sTrackViewName, pTrackTableView);

    connect(this, SIGNAL(switchToView(const QString&)),
            pLibraryWidget, SLOT(switchToView(const QString&)));

    m_pLibraryControl->bindWidget(pLibraryWidget, pKeyboard);

    QListIterator<LibraryFeature*> feature_it(m_features);
    while(feature_it.hasNext()) {
        LibraryFeature* feature = feature_it.next();
        feature->bindWidget(pLibraryWidget, pKeyboard);
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
    connect(feature, SIGNAL(loadTrack(TrackPointer)),
            this, SLOT(slotLoadTrack(TrackPointer)));
    connect(feature, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SLOT(slotLoadTrackToPlayer(TrackPointer, QString, bool)));
    connect(feature, SIGNAL(restoreSearch(const QString&)),
            this, SLOT(slotRestoreSearch(const QString&)));
}

void Library::slotShowTrackModel(QAbstractItemModel* model) {
    //qDebug() << "Library::slotShowTrackModel" << model;
    TrackModel* trackModel = dynamic_cast<TrackModel*>(model);
    Q_ASSERT(trackModel);
    emit(showTrackModel(model));
    emit(switchToView(m_sTrackViewName));
    emit(restoreSearch(trackModel->currentSearch()));
}

void Library::slotSwitchToView(const QString& view) {
    //qDebug() << "Library::slotSwitchToView" << view;
    emit(switchToView(view));
}

void Library::slotLoadTrack(TrackPointer pTrack) {
    emit(loadTrack(pTrack));
}

void Library::slotLoadLocationToPlayer(QString location, QString group) {
    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    int track_id = track_dao.getTrackId(location);
    if (track_id < 0) {
        // Add Track to library
        track_id = track_dao.addTrack(location, true);
    }

    TrackPointer pTrack;
    if (track_id < 0) {
        // Add Track to library failed, create a transient TrackInfoObject
        pTrack = TrackPointer(new TrackInfoObject(location), &QObject::deleteLater);
    } else {
        pTrack = track_dao.getTrack(track_id);
    }
    emit(loadTrackToPlayer(pTrack, group));
}

void Library::slotLoadTrackToPlayer(TrackPointer pTrack, QString group, bool play) {
    emit(loadTrackToPlayer(pTrack, group, play));
}

void Library::slotRestoreSearch(const QString& text) {
    emit(restoreSearch(text));
}

void Library::slotRefreshLibraryModels()
{
   m_pMixxxLibraryFeature->refreshLibraryModels();
   m_pPrepareFeature->refreshLibraryModels();
}

void Library::slotCreatePlaylist()
{
    m_pPlaylistFeature->slotCreatePlaylist();
}

void Library::slotCreateCrate()
{
    m_pCrateFeature->slotCreateCrate();
}

void Library::onSkinLoadFinished() {
    // Enable the default selection when a new skin is loaded.
    m_pSidebarModel->activateDefaultSelection();
}

QList<TrackPointer> Library::getTracksToAutoLoad() {
#ifdef __PROMO__
    if (m_pPromoTracksFeature)
        return m_pPromoTracksFeature->getTracksToAutoLoad();
#endif
    return QList<TrackPointer>();
}
