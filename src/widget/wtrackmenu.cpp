#include "widget/wtrackmenu.h"

#include <utility>
#include <iostream>
#include <library/trackmodel.h>
#include <util/desktophelper.h>
#include "widget/wlibrarytableview.h"
#include "library/dao/trackdao.h"
#include "library/trackcollection.h"


WTrackMenu::WTrackMenu(QWidget *parent, TrackCollectionManager* pTrackCollectionManager)
        : QMenu(parent),
        m_pTrackCollectionManager(pTrackCollectionManager){
    m_pLoadToMenu = new QMenu(this);
    m_pLoadToMenu->setTitle(tr("Load to"));
    m_pDeckMenu = new QMenu(this);
    m_pDeckMenu->setTitle(tr("Deck"));
    m_pSamplerMenu = new QMenu(this);
    m_pSamplerMenu->setTitle(tr("Sampler"));

    m_pPlaylistMenu = new QMenu(this);
    m_pPlaylistMenu->setTitle(tr("Add to Playlist"));
    connect(m_pPlaylistMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotPopulatePlaylistMenu()));
    m_pCrateMenu = new QMenu(this);
    m_pCrateMenu->setTitle(tr("Crates"));
    connect(m_pCrateMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotPopulateCrateMenu()));

    m_pMetadataMenu = new QMenu(this);
    m_pMetadataMenu->setTitle(tr("Metadata"));

    m_pMetadataUpdateExternalCollectionsMenu = new QMenu(this);
    m_pMetadataUpdateExternalCollectionsMenu->setTitle(tr("Update external collections"));

    m_pBPMMenu = new QMenu(this);
    m_pBPMMenu->setTitle(tr("Adjust BPM"));

    m_pColorMenu = new QMenu(this);
    m_pColorMenu->setTitle(tr("Select Color"));

    m_pClearMetadataMenu = new QMenu(this);
    //: Reset metadata in right click track context menu in library
    m_pClearMetadataMenu->setTitle(tr("Reset"));

    m_pCoverMenu = new WCoverArtMenu(this);
    m_pCoverMenu->setTitle(tr("Cover Art"));

    connect(m_pCoverMenu, SIGNAL(coverInfoSelected(const CoverInfoRelative&)),
            this, SLOT(slotCoverInfoSelected(const CoverInfoRelative&)));
    connect(m_pCoverMenu, SIGNAL(reloadCoverArt()),
            this, SLOT(slotReloadCoverArt()));
    createActions();
}

WTrackMenu::~WTrackMenu() {
    delete m_pImportMetadataFromFileAct;
    delete m_pImportMetadataFromMusicBrainzAct;
    delete m_pExportMetadataAct;
    delete m_pAddToPreviewDeck;
    delete m_pAutoDJBottomAct;
    delete m_pAutoDJTopAct;
    delete m_pAutoDJReplaceAct;
    delete m_pRemoveAct;
    delete m_pRemovePlaylistAct;
    delete m_pRemoveCrateAct;
    delete m_pHideAct;
    delete m_pUnhideAct;
    delete m_pPropertiesAct;
    delete m_pLoadToMenu;
    delete m_pDeckMenu;
    delete m_pSamplerMenu;
    delete m_pPlaylistMenu;
    delete m_pCrateMenu;
    delete m_pMetadataMenu;
    delete m_pClearMetadataMenu;
    delete m_pCoverMenu;
    delete m_pBpmLockAction;
    delete m_pBpmUnlockAction;
    delete m_pBpmDoubleAction;
    delete m_pBpmHalveAction;
    delete m_pBpmTwoThirdsAction;
    delete m_pBpmThreeFourthsAction;
    delete m_pBpmFourThirdsAction;
    delete m_pBpmThreeHalvesAction;
    delete m_pBPMMenu;
    delete m_pColorMenu;
    delete m_pClearBeatsAction;
    delete m_pClearPlayCountAction;
    delete m_pClearMainCueAction;
    delete m_pClearHotCuesAction;
    delete m_pClearIntroCueAction;
    delete m_pClearOutroCueAction;
    delete m_pClearLoopAction;
    delete m_pClearReplayGainAction;
    delete m_pClearWaveformAction;
    delete m_pClearKeyAction;
    delete m_pClearAllMetadataAction;
    delete m_pPurgeAct;
    delete m_pFileBrowserAct;
}


void WTrackMenu::createActions() {
//    DEBUG_ASSERT(this);
//    DEBUG_ASSERT(m_pSamplerMenu);
//
//    m_pRemoveAct = new QAction(tr("Remove"), this);
//    connect(m_pRemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));
//
//    m_pRemovePlaylistAct = new QAction(tr("Remove from Playlist"), this);
//    connect(m_pRemovePlaylistAct, SIGNAL(triggered()), this, SLOT(slotRemove()));
//
//    m_pRemoveCrateAct = new QAction(tr("Remove from Crate"), this);
//    connect(m_pRemoveCrateAct, SIGNAL(triggered()), this, SLOT(slotRemove()));
//
//    m_pHideAct = new QAction(tr("Hide from Library"), this);
//    connect(m_pHideAct, SIGNAL(triggered()), this, SLOT(slotHide()));
//
//    m_pUnhideAct = new QAction(tr("Unhide from Library"), this);
//    connect(m_pUnhideAct, SIGNAL(triggered()), this, SLOT(slotUnhide()));
//
//    m_pPurgeAct = new QAction(tr("Purge from Library"), this);
//    connect(m_pPurgeAct, SIGNAL(triggered()), this, SLOT(slotPurge()));
//
//    m_pPropertiesAct = new QAction(tr("Properties"), this);
//    connect(m_pPropertiesAct, SIGNAL(triggered()),
//            this, SLOT(slotShowTrackInfo()));
//
    m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
    connect(m_pFileBrowserAct, SIGNAL(triggered()),
            this, SLOT(slotOpenInFileBrowser()));
//
//    m_pAutoDJBottomAct = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
//    connect(m_pAutoDJBottomAct, SIGNAL(triggered()),
//            this, SLOT(slotAddToAutoDJBottom()));
//
//    m_pAutoDJTopAct = new QAction(tr("Add to Auto DJ Queue (top)"), this);
//    connect(m_pAutoDJTopAct, SIGNAL(triggered()),
//            this, SLOT(slotAddToAutoDJTop()));
//
//    m_pAutoDJReplaceAct = new QAction(tr("Add to Auto DJ Queue (replace)"), this);
//    connect(m_pAutoDJReplaceAct, SIGNAL(triggered()),
//            this, SLOT(slotAddToAutoDJReplace()));
//
//    m_pImportMetadataFromFileAct = new QAction(tr("Import From File Tags"), this);
//    connect(m_pImportMetadataFromFileAct, SIGNAL(triggered()),
//            this, SLOT(slotImportTrackMetadataFromFileTags()));
//
//    m_pImportMetadataFromMusicBrainzAct = new QAction(tr("Import From MusicBrainz"),this);
//    connect(m_pImportMetadataFromMusicBrainzAct, SIGNAL(triggered()),
//            this, SLOT(slotShowDlgTagFetcher()));
//
//    m_pExportMetadataAct = new QAction(tr("Export To File Tags"), this);
//    connect(m_pExportMetadataAct, SIGNAL(triggered()),
//            this, SLOT(slotExportTrackMetadataIntoFileTags()));
}

void WTrackMenu::setTracks(TrackIdList trackIdList) {
    m_pTrackIdList = std::move(trackIdList);
    // Store the track pointers at each initialization of track ids.
    trackIdsToTrackPointers();
    // Add actions to menu
    setupActions();
}

void WTrackMenu::setTrack(TrackId trackId) {
    // Create a QList of single track to maintain common functions
    // for single and multi track selection.
    TrackIdList singleItemTrackIdList;
    singleItemTrackIdList.push_back(trackId);
    setTracks(singleItemTrackIdList);
}

void WTrackMenu::setupActions() {
    addAction(m_pFileBrowserAct);
}

void WTrackMenu::slotOpenInFileBrowser() {
    TrackPointerList trackPointerList = getTrackPointerList();
    QStringList locations;
    for (const TrackPointer& trackPointer : trackPointerList) {
        locations << trackPointer->getLocation();
    }
    mixxx::DesktopHelper::openInFileBrowser(locations);
}

TrackPointerList WTrackMenu::getTrackPointerList() {
    return m_pTrackPointerList;
}

void WTrackMenu::trackIdsToTrackPointers() {
    m_pTrackPointerList.clear();
    for (const auto trackId : m_pTrackIdList) {
        TrackPointer trackPointer = m_pTrackCollectionManager->internalCollection()->getTrackById(trackId);
        m_pTrackPointerList.push_back(trackPointer);
    }
}
