#include "widget/wtrackmenu.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QDesktopServices>
#include <QInputDialog>
#include <QModelIndex>
#include <QWidgetAction>


#include <utility>
#include <iostream>


#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "library/coverartutils.h"
#include "library/crate/cratefeaturehelper.h"
#include "library/dao/trackdao.h"
#include "library/dao/trackschema.h"
#include "library/dlgtagfetcher.h"
#include "library/dlgtrackinfo.h"
#include "library/dlgtrackmetadataexport.h"
#include "library/externaltrackcollection.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackmodel.h"
#include "mixer/playermanager.h"
#include "preferences/colorpalettesettings.h"
#include "sources/soundsourceproxy.h"
#include "util/desktophelper.h"
#include "widget/wlibrarytableview.h"
#include "track/track.h"
#include "track/trackref.h"
#include "util/desktophelper.h"
#include "util/parented_ptr.h"
#include "waveform/guitick.h"
#include "widget/wcolorpickeraction.h"
#include "widget/wcoverartmenu.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"



WTrackMenu::WTrackMenu(QWidget *parent, UserSettingsPointer pConfig, TrackCollectionManager *pTrackCollectionManager)
        : QMenu(parent),
        m_pConfig(std::move(pConfig)),
        m_pTrackCollectionManager(pTrackCollectionManager),
        m_bPlaylistMenuLoaded(false),
        m_bCrateMenuLoaded(false),
        m_iCoverSourceColumn(-1),
        m_iCoverTypeColumn(-1),
        m_iCoverLocationColumn(-1),
        m_iCoverHashColumn(-1),
        m_iCoverColumn(-1) {
    m_pNumSamplers = new ControlProxy(
            "[Master]", "num_samplers", this);
    m_pNumDecks = new ControlProxy(
            "[Master]", "num_decks", this);
    m_pNumPreviewDecks = new ControlProxy(
            "[Master]", "num_preview_decks", this);

    constructMenus();
}

WTrackMenu::~WTrackMenu() {
    // Explicit destruction of heap allocated objects would cause segfault.
}

void WTrackMenu::constructMenus() {
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
}



void WTrackMenu::createActions() {
    DEBUG_ASSERT(this);
    DEBUG_ASSERT(m_pSamplerMenu);

    m_pRemoveAct = new QAction(tr("Remove"), this);
    connect(m_pRemoveAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

    m_pRemovePlaylistAct = new QAction(tr("Remove from Playlist"), this);
    connect(m_pRemovePlaylistAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

    m_pRemoveCrateAct = new QAction(tr("Remove from Crate"), this);
    connect(m_pRemoveCrateAct, SIGNAL(triggered()), this, SLOT(slotRemove()));

    m_pHideAct = new QAction(tr("Hide from Library"), this);
    connect(m_pHideAct, SIGNAL(triggered()), this, SLOT(slotHide()));

    m_pUnhideAct = new QAction(tr("Unhide from Library"), this);
    connect(m_pUnhideAct, SIGNAL(triggered()), this, SLOT(slotUnhide()));

    m_pPurgeAct = new QAction(tr("Purge from Library"), this);
    connect(m_pPurgeAct, SIGNAL(triggered()), this, SLOT(slotPurge()));

    m_pPropertiesAct = new QAction(tr("Properties"), this);
    connect(m_pPropertiesAct, SIGNAL(triggered()),
            this, SLOT(slotShowTrackInfo()));

    m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
    connect(m_pFileBrowserAct, SIGNAL(triggered()),
            this, SLOT(slotOpenInFileBrowser()));

    m_pAutoDJBottomAct = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAutoDJBottomAct, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJBottom()));

    m_pAutoDJTopAct = new QAction(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAutoDJTopAct, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJTop()));

    m_pAutoDJReplaceAct = new QAction(tr("Add to Auto DJ Queue (replace)"), this);
    connect(m_pAutoDJReplaceAct, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJReplace()));

    m_pImportMetadataFromFileAct = new QAction(tr("Import From File Tags"), this);
    connect(m_pImportMetadataFromFileAct, SIGNAL(triggered()),
            this, SLOT(slotImportTrackMetadataFromFileTags()));

    m_pImportMetadataFromMusicBrainzAct = new QAction(tr("Import From MusicBrainz"),this);
    connect(m_pImportMetadataFromMusicBrainzAct, SIGNAL(triggered()),
            this, SLOT(slotShowDlgTagFetcher()));

    m_pExportMetadataAct = new QAction(tr("Export To File Tags"), this);
    connect(m_pExportMetadataAct, SIGNAL(triggered()),
            this, SLOT(slotExportTrackMetadataIntoFileTags()));


    for (const auto& externalTrackCollection : m_pTrackCollectionManager->externalCollections()) {
        UpdateExternalTrackCollection updateInExternalTrackCollection;
        updateInExternalTrackCollection.externalTrackCollection = externalTrackCollection;
        updateInExternalTrackCollection.action = new QAction(externalTrackCollection->name(), this);
        updateInExternalTrackCollection.action->setToolTip(externalTrackCollection->description());
        m_updateInExternalTrackCollections += updateInExternalTrackCollection;
        auto externalTrackCollectionPtr = updateInExternalTrackCollection.externalTrackCollection;
        connect(updateInExternalTrackCollection.action, &QAction::triggered,
                this, [this, externalTrackCollectionPtr] {
                    slotUpdateExternalTrackCollection(externalTrackCollectionPtr);
                });
    }

    m_pAddToPreviewDeck = new QAction(tr("Preview Deck"), this);
    // currently there is only one preview deck so just map it here.
    QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
    connect(m_pAddToPreviewDeck, &QAction::triggered,
            this, [this, previewDeckGroup] { loadSelectionToGroup(previewDeckGroup); });


    // Clear metadata actions
    m_pClearBeatsAction = new QAction(tr("BPM and Beatgrid"), this);
    connect(m_pClearBeatsAction, SIGNAL(triggered()),
            this, SLOT(slotClearBeats()));

    m_pClearPlayCountAction = new QAction(tr("Play Count"), this);
    connect(m_pClearPlayCountAction, SIGNAL(triggered()),
            this, SLOT(slotClearPlayCount()));

    m_pClearMainCueAction = new QAction(tr("Cue Point"), this);
    connect(m_pClearMainCueAction, SIGNAL(triggered()),
            this, SLOT(slotClearMainCue()));

    m_pClearHotCuesAction = new QAction(tr("Hotcues"), this);
    connect(m_pClearHotCuesAction, SIGNAL(triggered()),
            this, SLOT(slotClearHotCues()));

    m_pClearIntroCueAction = new QAction(tr("Intro"), this);
    connect(m_pClearIntroCueAction, SIGNAL(triggered()),
            this, SLOT(slotClearIntroCue()));

    m_pClearOutroCueAction = new QAction(tr("Outro"), this);
    connect(m_pClearOutroCueAction, SIGNAL(triggered()),
            this, SLOT(slotClearOutroCue()));

    m_pClearLoopAction = new QAction(tr("Loop"), this);
    connect(m_pClearLoopAction, SIGNAL(triggered()),
            this, SLOT(slotClearLoop()));

    m_pClearKeyAction = new QAction(tr("Key"), this);
    connect(m_pClearKeyAction, SIGNAL(triggered()),
            this, SLOT(slotClearKey()));

    m_pClearReplayGainAction = new QAction(tr("ReplayGain"), this);
    connect(m_pClearReplayGainAction, SIGNAL(triggered()),
            this, SLOT(slotClearReplayGain()));

    m_pClearWaveformAction = new QAction(tr("Waveform"), this);
    connect(m_pClearWaveformAction, SIGNAL(triggered()),
            this, SLOT(slotClearWaveform()));

    m_pClearAllMetadataAction = new QAction(tr("All"), this);
    connect(m_pClearAllMetadataAction, SIGNAL(triggered()),
            this, SLOT(slotClearAllMetadata()));


    m_pBpmLockAction = new QAction(tr("Lock BPM"), this);
    m_pBpmUnlockAction = new QAction(tr("Unlock BPM"), this);
    connect(m_pBpmLockAction, SIGNAL(triggered()),
            this, SLOT(slotLockBpm()));
    connect(m_pBpmUnlockAction, SIGNAL(triggered()),
            this, SLOT(slotUnlockBpm()));

    //BPM edit actions
    m_pBpmDoubleAction = new QAction(tr("Double BPM"), this);
    m_pBpmHalveAction = new QAction(tr("Halve BPM"), this);
    m_pBpmTwoThirdsAction = new QAction(tr("2/3 BPM"), this);
    m_pBpmThreeFourthsAction = new QAction(tr("3/4 BPM"), this);
    m_pBpmFourThirdsAction = new QAction(tr("4/3 BPM"), this);
    m_pBpmThreeHalvesAction = new QAction(tr("3/2 BPM"), this);

    connect(m_pBpmDoubleAction, &QAction::triggered,
            this, [this] { slotScaleBpm(Beats::DOUBLE); });
    connect(m_pBpmHalveAction, &QAction::triggered,
            this, [this] { slotScaleBpm(Beats::HALVE); });
    connect(m_pBpmTwoThirdsAction, &QAction::triggered,
            this, [this] { slotScaleBpm(Beats::TWOTHIRDS); });
    connect(m_pBpmThreeFourthsAction, &QAction::triggered,
            this, [this] { slotScaleBpm(Beats::THREEFOURTHS); });
    connect(m_pBpmFourThirdsAction, &QAction::triggered,
            this, [this] { slotScaleBpm(Beats::FOURTHIRDS); });
    connect(m_pBpmThreeHalvesAction, &QAction::triggered,
            this, [this] { slotScaleBpm(Beats::THREEHALVES); });

    ColorPaletteSettings colorPaletteSettings(m_pConfig);
    m_pColorPickerAction = new WColorPickerAction(WColorPicker::Option::AllowNoColor, colorPaletteSettings.getTrackColorPalette(), this);
    m_pColorPickerAction->setObjectName("TrackColorPickerAction");
    connect(m_pColorPickerAction,
            &WColorPickerAction::colorPicked,
            this,
            &WTrackMenu::slotColorPicked);
}

void WTrackMenu::teardownActions()
{
    clear();
    m_pLoadToMenu->clear();
    m_pDeckMenu->clear();
    m_pSamplerMenu->clear();
    m_pPlaylistMenu->clear();
    m_pCrateMenu->clear();
    m_pMetadataMenu->clear();
    m_pMetadataUpdateExternalCollectionsMenu->clear();
    m_pClearMetadataMenu->clear();
    m_pBPMMenu->clear();
    m_pColorMenu->clear();
}

void WTrackMenu::setupActions() {
    teardownActions();
    createActions();

    QModelIndexList indices = m_pSelectedTrackIndices;

    // Gray out some stuff if multiple songs were selected.
    bool oneSongSelected = indices.size() == 1;
    TrackModel* trackModel = m_pTrackModel;


    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ)) {
        addAction(m_pAutoDJBottomAct);
        addAction(m_pAutoDJTopAct);
        addAction(m_pAutoDJReplaceAct);
        addSeparator();
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTODECK)) {
        int iNumDecks = m_pNumDecks->get();
        if (iNumDecks > 0) {
            for (int i = 1; i <= iNumDecks; ++i) {
                // PlayerManager::groupForDeck is 0-indexed.
                QString deckGroup = PlayerManager::groupForDeck(i - 1);
                bool deckPlaying = ControlObject::get(
                        ConfigKey(deckGroup, "play")) > 0.0;
                bool loadTrackIntoPlayingDeck = m_pConfig->getValue<bool>(
                        ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"));
                bool deckEnabled = (!deckPlaying  || loadTrackIntoPlayingDeck)  && oneSongSelected;
                QAction* pAction = new QAction(tr("Deck %1").arg(i), this);
                pAction->setEnabled(deckEnabled);
                m_pDeckMenu->addAction(pAction);
                connect(pAction, &QAction::triggered,
                        this, [this, deckGroup] { loadSelectionToGroup(deckGroup); });
            }
        }
        m_pLoadToMenu->addMenu(m_pDeckMenu);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTOSAMPLER)) {
        int iNumSamplers = m_pNumSamplers->get();
        if (iNumSamplers > 0) {
            for (int i = 1; i <= iNumSamplers; ++i) {
                // PlayerManager::groupForSampler is 0-indexed.
                QString samplerGroup = PlayerManager::groupForSampler(i - 1);
                bool samplerPlaying = ControlObject::get(
                        ConfigKey(samplerGroup, "play")) > 0.0;
                bool samplerEnabled = !samplerPlaying && oneSongSelected;
                QAction* pAction = new QAction(tr("Sampler %1").arg(i), m_pSamplerMenu);
                pAction->setEnabled(samplerEnabled);
                m_pSamplerMenu->addAction(pAction);
                connect(pAction, &QAction::triggered,
                        this, [this, samplerGroup] {loadSelectionToGroup(samplerGroup); } );

            }
            m_pLoadToMenu->addMenu(m_pSamplerMenu);
        }
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOADTOPREVIEWDECK) &&
        m_pNumPreviewDecks->get() > 0.0) {
        m_pLoadToMenu->addAction(m_pAddToPreviewDeck);
    }

    addMenu(m_pLoadToMenu);
    addSeparator();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOPLAYLIST)) {
        // Playlist menu is lazy loaded on hover by slotPopulatePlaylistMenu
        // to avoid unnecessary database queries
        m_bPlaylistMenuLoaded = false;
        addMenu(m_pPlaylistMenu);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOCRATE)) {
        // Crate menu is lazy loaded on hover by slotPopulateCrateMenu
        // to avoid unnecessary database queries
        m_bCrateMenuLoaded = false;
        addMenu(m_pCrateMenu);
    }

    // REMOVE and HIDE should not be at the first menu position to avoid accidental clicks
    bool locked = modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOCKED);
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE)) {
        m_pRemoveAct->setEnabled(!locked);
        addAction(m_pRemoveAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST)) {
        m_pRemovePlaylistAct->setEnabled(!locked);
        addAction(m_pRemovePlaylistAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_CRATE)) {
        m_pRemoveCrateAct->setEnabled(!locked);
        addAction(m_pRemoveCrateAct);
    }

    addSeparator();

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        m_pMetadataMenu->addAction(m_pImportMetadataFromFileAct);
        m_pImportMetadataFromMusicBrainzAct->setEnabled(oneSongSelected);
        m_pMetadataMenu->addAction(m_pImportMetadataFromMusicBrainzAct);
        m_pMetadataMenu->addAction(m_pExportMetadataAct);

        for (const auto& updateInExternalTrackCollection : m_updateInExternalTrackCollections) {
            ExternalTrackCollection* externalTrackCollection =
                    updateInExternalTrackCollection.externalTrackCollection;
            if (externalTrackCollection) {
                updateInExternalTrackCollection.action->setEnabled(
                        externalTrackCollection->isConnected());
                m_pMetadataUpdateExternalCollectionsMenu->addAction(
                        updateInExternalTrackCollection.action);
            }
        }
        if (!m_pMetadataUpdateExternalCollectionsMenu->isEmpty()) {
            m_pMetadataMenu->addMenu(m_pMetadataUpdateExternalCollectionsMenu);
        }

        for (const auto& updateInExternalTrackCollection : m_updateInExternalTrackCollections) {
            ExternalTrackCollection* externalTrackCollection =
                    updateInExternalTrackCollection.externalTrackCollection;
            if (externalTrackCollection) {
                updateInExternalTrackCollection.action->setEnabled(
                        externalTrackCollection->isConnected());
                m_pMetadataUpdateExternalCollectionsMenu->addAction(
                        updateInExternalTrackCollection.action);
            }
        }
        if (!m_pMetadataUpdateExternalCollectionsMenu->isEmpty()) {
            m_pMetadataMenu->addMenu(m_pMetadataUpdateExternalCollectionsMenu);
        }

        if (trackModel == nullptr) {
            return;
        }
        bool allowClear = true;
        int column = trackModel->fieldIndex(LIBRARYTABLE_BPM_LOCK);
        for (int i = 0; i < indices.size() && allowClear; ++i) {
            int row = indices.at(i).row();
            QModelIndex index = indices.at(i).sibling(row,column);
            if (index.data().toBool()) {
                allowClear = false;
            }
        }
        m_pClearBeatsAction->setEnabled(allowClear);
        m_pClearMetadataMenu->addAction(m_pClearBeatsAction);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_RESETPLAYED)) {
        m_pClearMetadataMenu->addAction(m_pClearPlayCountAction);
    }

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        // FIXME: Why is clearing the loop not working?
        m_pClearMetadataMenu->addAction(m_pClearMainCueAction);
        m_pClearMetadataMenu->addAction(m_pClearHotCuesAction);
        m_pClearMetadataMenu->addAction(m_pClearIntroCueAction);
        m_pClearMetadataMenu->addAction(m_pClearOutroCueAction);
        //m_pClearMetadataMenu->addAction(m_pClearLoopAction);
        m_pClearMetadataMenu->addAction(m_pClearKeyAction);
        m_pClearMetadataMenu->addAction(m_pClearReplayGainAction);
        m_pClearMetadataMenu->addAction(m_pClearWaveformAction);
        m_pClearMetadataMenu->addSeparator();
        m_pClearMetadataMenu->addAction(m_pClearAllMetadataAction);

        // Cover art menu only applies if at least one track is selected.
        if (indices.size()) {
            // We load a single track to get the necessary context for the cover (we use
            // last to be consistent with selectionChanged above).
            QModelIndex last = indices.last();
            CoverInfo info;
            info.source = static_cast<CoverInfo::Source>(
                    last.sibling(last.row(), m_iCoverSourceColumn).data().toInt());
            info.type = static_cast<CoverInfo::Type>(
                    last.sibling(last.row(), m_iCoverTypeColumn).data().toInt());
            info.hash = last.sibling(last.row(), m_iCoverHashColumn).data().toUInt();
            info.trackLocation = last.sibling(
                    last.row(), m_iTrackLocationColumn).data().toString();
            info.coverLocation = last.sibling(
                    last.row(), m_iCoverLocationColumn).data().toString();
            m_pCoverMenu->setCoverArt(info);
            m_pMetadataMenu->addMenu(m_pCoverMenu);
        }

        addMenu(m_pMetadataMenu);
        addMenu(m_pClearMetadataMenu);

        m_pBPMMenu->addAction(m_pBpmDoubleAction);
        m_pBPMMenu->addAction(m_pBpmHalveAction);
        m_pBPMMenu->addAction(m_pBpmTwoThirdsAction);
        m_pBPMMenu->addAction(m_pBpmThreeFourthsAction);
        m_pBPMMenu->addAction(m_pBpmFourThirdsAction);
        m_pBPMMenu->addAction(m_pBpmThreeHalvesAction);
        m_pBPMMenu->addSeparator();
        m_pBPMMenu->addAction(m_pBpmLockAction);
        m_pBPMMenu->addAction(m_pBpmUnlockAction);
        m_pBPMMenu->addSeparator();
        if (oneSongSelected) {
            if (trackModel == nullptr) {
                return;
            }
            int column = trackModel->fieldIndex(LIBRARYTABLE_BPM_LOCK);
            QModelIndex index = indices.at(0).sibling(indices.at(0).row(),column);
            if (index.data().toBool()) { //BPM is locked
                m_pBpmUnlockAction->setEnabled(true);
                m_pBpmLockAction->setEnabled(false);
                m_pBpmDoubleAction->setEnabled(false);
                m_pBpmHalveAction->setEnabled(false);
                m_pBpmTwoThirdsAction->setEnabled(false);
                m_pBpmThreeFourthsAction->setEnabled(false);
                m_pBpmFourThirdsAction->setEnabled(false);
                m_pBpmThreeHalvesAction->setEnabled(false);
            } else { //BPM is not locked
                m_pBpmUnlockAction->setEnabled(false);
                m_pBpmLockAction->setEnabled(true);
                m_pBpmDoubleAction->setEnabled(true);
                m_pBpmHalveAction->setEnabled(true);
                m_pBpmTwoThirdsAction->setEnabled(true);
                m_pBpmThreeFourthsAction->setEnabled(true);
                m_pBpmFourThirdsAction->setEnabled(true);
                m_pBpmThreeHalvesAction->setEnabled(true);
            }
        } else {
            bool anyLocked = false; //true if any of the selected items are locked
            int column = trackModel->fieldIndex(LIBRARYTABLE_BPM_LOCK);
            for (int i = 0; i < indices.size() && !anyLocked; ++i) {
                int row = indices.at(i).row();
                QModelIndex index = indices.at(i).sibling(row,column);
                if (index.data().toBool()) {
                    anyLocked = true;
                }
            }
            if (anyLocked) {
                m_pBpmLockAction->setEnabled(false);
                m_pBpmUnlockAction->setEnabled(true);
                m_pBpmDoubleAction->setEnabled(false);
                m_pBpmHalveAction->setEnabled(false);
                m_pBpmTwoThirdsAction->setEnabled(false);
                m_pBpmThreeFourthsAction->setEnabled(false);
                m_pBpmFourThirdsAction->setEnabled(false);
                m_pBpmThreeHalvesAction->setEnabled(false);
            } else {
                m_pBpmLockAction->setEnabled(true);
                m_pBpmUnlockAction->setEnabled(false);
                m_pBpmDoubleAction->setEnabled(true);
                m_pBpmHalveAction->setEnabled(true);
                m_pBpmTwoThirdsAction->setEnabled(true);
                m_pBpmThreeFourthsAction->setEnabled(true);
                m_pBpmFourThirdsAction->setEnabled(true);
                m_pBpmThreeHalvesAction->setEnabled(true);
            }
        }
        addMenu(m_pBPMMenu);

        // Track color menu only appears if at least one track is selected
        if (!indices.empty()) {
            m_pColorPickerAction->setColorPalette(
                    ColorPaletteSettings(m_pConfig).getTrackColorPalette());

            // Get color of first selected track
            int column = trackModel->fieldIndex(LIBRARYTABLE_COLOR);
            QModelIndex index = indices.at(0).sibling(indices.at(0).row(), column);
            auto trackColor = mixxx::RgbColor::fromQVariant(index.data());

            // Check if all other selected tracks have the same color
            bool multipleTrackColors = false;
            for (int i = 1; i < indices.size(); ++i) {
                int row = indices.at(i).row();
                QModelIndex index = indices.at(i).sibling(row, column);

                if (trackColor != mixxx::RgbColor::fromQVariant(index.data())) {
                    trackColor = mixxx::RgbColor::nullopt();
                    multipleTrackColors = true;
                    break;
                }
            }

            if (multipleTrackColors) {
                m_pColorPickerAction->resetSelectedColor();
            } else {
                m_pColorPickerAction->setSelectedColor(trackColor);
            }
            m_pColorMenu->addAction(m_pColorPickerAction);
            addMenu(m_pColorMenu);
        }
    }

    addSeparator();
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_HIDE)) {
        m_pHideAct->setEnabled(!locked);
        addAction(m_pHideAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE)) {
        m_pUnhideAct->setEnabled(!locked);
        addAction(m_pUnhideAct);
    }
    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_PURGE)) {
        m_pPurgeAct->setEnabled(!locked);
        addAction(m_pPurgeAct);
    }

    addAction(m_pFileBrowserAct);

    if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        addSeparator();
        m_pPropertiesAct->setEnabled(oneSongSelected);
        addAction(m_pPropertiesAct);
    }
}

void WTrackMenu::setTrackIds(TrackIdList trackIdList) {
    // Clean all forms of track store
    clearTrackSelection();

    m_pTrackIdList = std::move(trackIdList);
    // Store the track pointers at each initialization of track ids.
    for (const auto trackId : m_pTrackIdList) {
        TrackPointer trackPointer = m_pTrackCollectionManager->internalCollection()->getTrackById(trackId);
        m_pTrackPointerList.push_back(trackPointer);
    }
    // Add actions to menu
    setupActions();
}

void WTrackMenu::setTrackId(TrackId trackId) {
    // Create a QList of single track to maintain common functions
    // for single and multi track selection.
    TrackIdList singleItemTrackIdList;
    singleItemTrackIdList.push_back(trackId);
    // Use setTrackIds to set a list of single element.
    setTrackIds(singleItemTrackIdList);
}

void WTrackMenu::setTrackIndexList(QModelIndexList indexList) {
    clearTrackSelection();

    m_pSelectedTrackIndices = std::move(indexList);
    for (auto index : m_pSelectedTrackIndices) {
        TrackPointer trackPointer = m_pTrackModel->getTrack(index);
        m_pTrackPointerList.push_back(trackPointer);
        TrackId trackId = trackPointer->getId();
        m_pTrackIdList.push_back(trackId);
    }

    setupActions();
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

bool WTrackMenu::modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) const {
    if (!m_pTrackModel) {
        return false;
    }
    TrackModel* trackModel = m_pTrackModel;
    return trackModel &&
           (trackModel->getCapabilities() & capabilities) == capabilities;
}



void WTrackMenu::slotImportTrackMetadataFromFileTags() {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        return;
    }

    const QModelIndexList indices = m_pSelectedTrackIndices;

    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            // The user has explicitly requested to reload metadata from the file
            // to override the information within Mixxx! Custom cover art must be
            // reloaded separately.
            SoundSourceProxy(pTrack).updateTrackFromSource(
                    SoundSourceProxy::ImportTrackMetadataMode::Again);
        }
    }
}

void WTrackMenu::slotExportTrackMetadataIntoFileTags() {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        return;
    }

    TrackModel* pTrackModel = m_pTrackModel;;
    if (!pTrackModel) {
        return;
    }

    const QModelIndexList indices = m_pSelectedTrackIndices;
    if (indices.isEmpty()) {
        return;
    }

    mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = pTrackModel->getTrack(index);
        if (pTrack) {
            // Export of metadata is deferred until all references to the
            // corresponding track object have been dropped. Otherwise
            // writing to files that are still used for playback might
            // cause crashes or at least audible glitches!
            mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();
            pTrack->markForMetadataExport();
        }
    }
}

void WTrackMenu::slotUpdateExternalTrackCollection(
        ExternalTrackCollection* externalTrackCollection) {
    VERIFY_OR_DEBUG_ASSERT(externalTrackCollection) {
        return;
    }

    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        return;
    }

    TrackModel* pTrackModel = m_pTrackModel;;
    if (!pTrackModel) {
        return;
    }

    const QModelIndexList indices = m_pSelectedTrackIndices;
    if (indices.isEmpty()) {
        return;
    }

    QList<TrackRef> trackRefs;
    trackRefs.reserve(indices.size());
    for (const QModelIndex& index : indices) {
        trackRefs.append(
                TrackRef::fromFileInfo(
                        pTrackModel->getTrackLocation(index),
                        pTrackModel->getTrackId(index)));
    }

    externalTrackCollection->updateTracks(std::move(trackRefs));
}

//slot for reset played count, sets count to 0 of one or more tracks
void WTrackMenu::slotClearPlayCount() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->resetPlayCounter();
        }
    }
}

void WTrackMenu::slotPopulatePlaylistMenu() {
    // The user may open the Playlist submenu, move their cursor away, then
    // return to the Playlist submenu before exiting the track context menu.
    // Avoid querying the database multiple times in that case.
    if (m_bPlaylistMenuLoaded) {
        return;
    }
    m_pPlaylistMenu->clear();
    PlaylistDAO& playlistDao = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO();
    QMap<QString,int> playlists;
    int numPlaylists = playlistDao.playlistCount();
    for (int i = 0; i < numPlaylists; ++i) {
        int iPlaylistId = playlistDao.getPlaylistId(i);
        playlists.insert(playlistDao.getPlaylistName(iPlaylistId), iPlaylistId);
    }
    QMapIterator<QString, int> it(playlists);
    while (it.hasNext()) {
        it.next();
        if (!playlistDao.isHidden(it.value())) {
            // No leak because making the menu the parent means they will be
            // auto-deleted
            auto pAction = new QAction(it.key(), m_pPlaylistMenu);
            bool locked = playlistDao.isPlaylistLocked(it.value());
            pAction->setEnabled(!locked);
            m_pPlaylistMenu->addAction(pAction);
            int iPlaylistId = it.value();
            connect(pAction, &QAction::triggered,
                    this, [this, iPlaylistId] { addSelectionToPlaylist(iPlaylistId); });

        }
    }
    m_pPlaylistMenu->addSeparator();
    QAction* newPlaylistAction = new QAction(tr("Create New Playlist"), m_pPlaylistMenu);
    m_pPlaylistMenu->addAction(newPlaylistAction);
    connect(newPlaylistAction, &QAction::triggered,
            this, [this] { addSelectionToPlaylist(-1); });
    m_bPlaylistMenuLoaded = true;
}

void WTrackMenu::addSelectionToPlaylist(int iPlaylistId) {
    const TrackIdList trackIds = m_pTrackIdList;
    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for playlist";
        return;
    }

    PlaylistDAO& playlistDao = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO();

    if (iPlaylistId == -1) { // i.e. a new playlist is suppose to be created
        QString name;
        bool validNameGiven = false;

        do {
            bool ok = false;
            name = QInputDialog::getText(nullptr,
                                         tr("Create New Playlist"),
                                         tr("Enter name for new playlist:"),
                                         QLineEdit::Normal,
                                         tr("New Playlist"),
                                         &ok).trimmed();
            if (!ok) {
                return;
            }
            if (playlistDao.getPlaylistIdFromName(name) != -1) {
                QMessageBox::warning(nullptr,
                                     tr("Playlist Creation Failed"),
                                     tr("A playlist by that name already exists."));
            } else if (name.isEmpty()) {
                QMessageBox::warning(nullptr,
                                     tr("Playlist Creation Failed"),
                                     tr("A playlist cannot have a blank name."));
            } else {
                validNameGiven = true;
            }
        } while (!validNameGiven);
        iPlaylistId = playlistDao.createPlaylist(name);//-1 is changed to the new playlist ID return from the DAO
        if (iPlaylistId == -1) {
            QMessageBox::warning(nullptr,
                                 tr("Playlist Creation Failed"),
                                 tr("An unknown error occurred while creating playlist: ")
                                 +name);
            return;
        }
    }

    // TODO(XXX): Care whether the append succeeded.
    m_pTrackCollectionManager->unhideTracks(trackIds);
    playlistDao.appendTracksToPlaylist(trackIds, iPlaylistId);
}


void WTrackMenu::slotPopulateCrateMenu() {
    // The user may open the Crate submenu, move their cursor away, then
    // return to the Crate submenu before exiting the track context menu.
    // Avoid querying the database multiple times in that case.
    if (m_bCrateMenuLoaded) {
        return;
    }
    m_pCrateMenu->clear();
    const TrackIdList trackIds = m_pTrackIdList;

    CrateSummarySelectResult allCrates(m_pTrackCollectionManager->internalCollection()->crates().selectCratesWithTrackCount(trackIds));

    CrateSummary crate;
    while (allCrates.populateNext(&crate)) {
        auto pAction = make_parented<QWidgetAction>(m_pCrateMenu);
        auto pCheckBox = make_parented<QCheckBox>(m_pCrateMenu);

        pCheckBox->setText(crate.getName());
        pCheckBox->setProperty("crateId",
                                QVariant::fromValue(crate.getId()));
        pCheckBox->setEnabled(!crate.isLocked());
        // Strangely, the normal styling of QActions does not automatically
        // apply to QWidgetActions. The :selected pseudo-state unfortunately
        // does not work with QWidgetAction. :hover works for selecting items
        // with the mouse, but not with the keyboard. :focus works for the
        // keyboard but with the mouse, the last clicked item keeps the style
        // after the mouse cursor is moved to hover over another item.

        // ronso0 Disabling this stylesheet allows to override the OS style
        // of the :hover and :focus state.
//        pCheckBox->setStyleSheet(
//            QString("QCheckBox {color: %1;}").arg(
//                    pCheckBox->palette().text().color().name()) + "\n" +
//            QString("QCheckBox:hover {background-color: %1;}").arg(
//                    pCheckBox->palette().highlight().color().name()));
        pAction->setEnabled(!crate.isLocked());
        pAction->setDefaultWidget(pCheckBox.get());

        if (crate.getTrackCount() == 0) {
            pCheckBox->setChecked(false);
        } else if (crate.getTrackCount() == (uint)trackIds.length()) {
            pCheckBox->setChecked(true);
        } else {
            pCheckBox->setTristate(true);
            pCheckBox->setCheckState(Qt::PartiallyChecked);
        }

        m_pCrateMenu->addAction(pAction.get());
        connect(pAction.get(), &QAction::triggered,
                this, [this, pCheckBox{pCheckBox.get()}] { updateSelectionCrates(pCheckBox); });
        connect(pCheckBox.get(), &QCheckBox::stateChanged,
                this, [this, pCheckBox{pCheckBox.get()}] { updateSelectionCrates(pCheckBox); });

    }
    m_pCrateMenu->addSeparator();
    QAction* newCrateAction = new QAction(tr("Create New Crate"), m_pCrateMenu);
    m_pCrateMenu->addAction(newCrateAction);
    connect(newCrateAction, SIGNAL(triggered()), this, SLOT(addSelectionToNewCrate()));
    m_bCrateMenuLoaded = true;
}


void WTrackMenu::updateSelectionCrates(QWidget* pWidget) {
    auto pCheckBox = qobject_cast<QCheckBox*>(pWidget);
    VERIFY_OR_DEBUG_ASSERT(pCheckBox) {
        qWarning() << "crateId is not of CrateId type";
        return;
    }
    CrateId crateId = pCheckBox->property("crateId").value<CrateId>();

    const TrackIdList trackIds = m_pTrackIdList;

    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for crate";
        return;
    }

    // we need to disable tristate again as the mixed state will now be gone and can't be brought back
    pCheckBox->setTristate(false);
    if(!pCheckBox->isChecked()) {
        if (crateId.isValid()) {
            m_pTrackCollectionManager->internalCollection()->removeCrateTracks(crateId, trackIds);
        }
    } else {
        if (!crateId.isValid()) { // i.e. a new crate is suppose to be created
            crateId = CrateFeatureHelper(
                    m_pTrackCollectionManager->internalCollection(), m_pConfig).createEmptyCrate();
        }
        if (crateId.isValid()) {
            m_pTrackCollectionManager->unhideTracks(trackIds);
            m_pTrackCollectionManager->internalCollection()->addCrateTracks(crateId, trackIds);
        }
    }
}

void WTrackMenu::addSelectionToNewCrate() {
    const TrackIdList trackIds = m_pTrackIdList;

    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for crate";
        return;
    }

    CrateId crateId = CrateFeatureHelper(
            m_pTrackCollectionManager->internalCollection(), m_pConfig).createEmptyCrate();

    if (crateId.isValid()) {
        m_pTrackCollectionManager->unhideTracks(trackIds);
        m_pTrackCollectionManager->internalCollection()->addCrateTracks(crateId, trackIds);
    }

}

void WTrackMenu::setTrackModel(TrackModel* trackModel) {
    m_pTrackModel = trackModel;

    // Move this to another function
    m_iCoverSourceColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_SOURCE);
    m_iCoverTypeColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_TYPE);
    m_iCoverLocationColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_LOCATION);
    m_iCoverHashColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_HASH);
    m_iCoverColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART);
    m_iTrackLocationColumn = trackModel->fieldIndex(TRACKLOCATIONSTABLE_LOCATION);
}


void WTrackMenu::slotLockBpm() {
    lockBpm(true);
}

void WTrackMenu::slotUnlockBpm() {
    lockBpm(false);
}

void WTrackMenu::slotScaleBpm(int scale) {
    TrackModel* trackModel = m_pTrackModel;
    if (trackModel == nullptr) {
        return;
    }

    const QModelIndexList selectedTrackIndices = m_pSelectedTrackIndices;
    for (const auto& index : selectedTrackIndices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack && !pTrack->isBpmLocked()) {
            BeatsPointer pBeats = pTrack->getBeats();
            if (pBeats) {
                pBeats->scale(static_cast<Beats::BPMScale>(scale));
            }
        }
    }
}


void WTrackMenu::lockBpm(bool lock) {
    TrackModel* trackModel = m_pTrackModel;;
    if (trackModel == nullptr) {
        return;
    }

    const QModelIndexList selectedTrackIndices = m_pSelectedTrackIndices;
    // TODO: This should be done in a thread for large selections
    for (const auto& index : selectedTrackIndices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->setBpmLocked(lock);
        }
    }
}

void WTrackMenu::slotColorPicked(mixxx::RgbColor::optional_t color) {
    TrackModel* trackModel = m_pTrackModel;
    if (trackModel == nullptr) {
        return;
    }

    const QModelIndexList selectedTrackIndices = m_pSelectedTrackIndices;
    // TODO: This should be done in a thread for large selections
    for (const auto& index : selectedTrackIndices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->setColor(color);
        }
    }

    hide();
}

void WTrackMenu::loadSelectionToGroup(QString group, bool play) {
    QModelIndexList indices = m_pSelectedTrackIndices;
    if (indices.size() > 0) {
        // If the track load override is disabled, check to see if a track is
        // playing before trying to load it
        if (!(m_pConfig->getValueString(
                ConfigKey("[Controls]","AllowTrackLoadToPlayingDeck")).toInt())) {
            // TODO(XXX): Check for other than just the first preview deck.
            if (group != "[PreviewDeck1]" &&
                ControlObject::get(ConfigKey(group, "play")) > 0.0) {
                return;
            }
        }
        QModelIndex index = indices.at(0);
        TrackModel* trackModel = m_pTrackModel;
        TrackPointer pTrack;
        if (trackModel &&
            (pTrack = trackModel->getTrack(index))) {
            emit loadTrackToPlayer(pTrack, group, play);
        }
    }
}



void WTrackMenu::slotClearBeats() {
    TrackModel* trackModel = m_pTrackModel;
    if (trackModel == nullptr) {
        return;
    }

    const QModelIndexList selectedTrackIndices = m_pSelectedTrackIndices;
    // TODO: This should be done in a thread for large selections
    for (const auto& index : selectedTrackIndices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack && !pTrack->isBpmLocked()) {
            pTrack->setBeats(BeatsPointer());
        }
    }
}

void WTrackMenu::slotClearMainCue() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->removeCuesOfType(mixxx::CueType::MainCue);
        }
    }
}

void WTrackMenu::slotClearOutroCue() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->removeCuesOfType(mixxx::CueType::HotCue);
        }
    }
}

void WTrackMenu::slotClearIntroCue() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->removeCuesOfType(mixxx::CueType::Intro);
        }
    }
}

void WTrackMenu::slotClearKey() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->resetKeys();
        }
    }
}

void WTrackMenu::slotClearReplayGain() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->setReplayGain(mixxx::ReplayGain());
        }
    }
}

void WTrackMenu::slotClearWaveform() {
    TrackModel* trackModel = m_pTrackModel;;
    if (trackModel == nullptr) {
        return;
    }

    AnalysisDao& analysisDao = m_pTrackCollectionManager->internalCollection()->getAnalysisDAO();
    const QModelIndexList indices = m_pSelectedTrackIndices;
    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (!pTrack) {
            continue;
        }
        analysisDao.deleteAnalysesForTrack(pTrack->getId());
        pTrack->setWaveform(WaveformPointer());
        pTrack->setWaveformSummary(WaveformPointer());
    }
}

void WTrackMenu::slotClearLoop() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->removeCuesOfType(mixxx::CueType::Loop);
        }
    }
}

void WTrackMenu::slotClearHotCues() {
    const QModelIndexList indices = m_pSelectedTrackIndices;
    TrackModel* trackModel = m_pTrackModel;

    if (trackModel == nullptr) {
        return;
    }

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->removeCuesOfType(mixxx::CueType::HotCue);
        }
    }
}


void WTrackMenu::slotClearAllMetadata() {
    slotClearBeats();
    slotClearPlayCount();
    slotClearMainCue();
    slotClearHotCues();
    slotClearIntroCue();
    slotClearOutroCue();
    slotClearLoop();
    slotClearKey();
    slotClearReplayGain();
    slotClearWaveform();
}



void WTrackMenu::slotRemove() {
    QModelIndexList indices = m_pSelectedTrackIndices;
    if (!indices.empty()) {
        TrackModel* trackModel = m_pTrackModel;
        if (trackModel) {
            trackModel->removeTracks(indices);
        }
    }
}



void WTrackMenu::slotHide() {
    QModelIndexList indices = m_pSelectedTrackIndices;
    if (indices.size() > 0) {
        TrackModel* trackModel = m_pTrackModel;
        if (trackModel) {
            trackModel->hideTracks(indices);
        }
    }
}




void WTrackMenu::slotTrackInfoClosed() {
    DlgTrackInfo* pTrackInfo = m_pTrackInfo.take();
    // We are in a slot directly invoked from DlgTrackInfo. Delete it
    // later.
    if (pTrackInfo != nullptr) {
        pTrackInfo->deleteLater();
    }
}

void WTrackMenu::slotTagFetcherClosed() {
    DlgTagFetcher* pTagFetcher = m_pTagFetcher.take();
    // We are in a slot directly invoked from DlgTagFetcher. Delete it
    // later.
    if (pTagFetcher != nullptr) {
        pTagFetcher->deleteLater();
    }
}

void WTrackMenu::slotShowTrackInfo() {
    QModelIndexList indices = m_pSelectedTrackIndices;

    if (indices.size() > 0) {
        showTrackInfo(indices[0]);
    }
}

void WTrackMenu::slotNextTrackInfo() {
    QModelIndex nextRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()+1, currentTrackInfoIndex.column());
    if (nextRow.isValid()) {
        showTrackInfo(nextRow);
        if (!m_pTagFetcher.isNull()) {
            showDlgTagFetcher(nextRow);
        }
    }
}

void WTrackMenu::slotPrevTrackInfo() {
    QModelIndex prevRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()-1, currentTrackInfoIndex.column());
    if (prevRow.isValid()) {
        showTrackInfo(prevRow);
        if (!m_pTagFetcher.isNull()) {
            showDlgTagFetcher(prevRow);
        }
    }
}

void WTrackMenu::showTrackInfo(QModelIndex index) {
    TrackModel* trackModel = m_pTrackModel;

    if (!trackModel) {
        return;
    }

    if (m_pTrackInfo.isNull()) {
        // Give a NULL parent because otherwise it inherits our style which can
        // make it unreadable. Bug #673411
        m_pTrackInfo.reset(new DlgTrackInfo(m_pConfig, nullptr));

        connect(m_pTrackInfo.data(), SIGNAL(next()),
                this, SLOT(slotNextTrackInfo()));
        connect(m_pTrackInfo.data(), SIGNAL(previous()),
                this, SLOT(slotPrevTrackInfo()));
        connect(m_pTrackInfo.data(), SIGNAL(showTagFetcher(TrackPointer)),
                this, SLOT(slotShowTrackInTagFetcher(TrackPointer)));
        connect(m_pTrackInfo.data(), SIGNAL(finished(int)),
                this, SLOT(slotTrackInfoClosed()));
    }
    TrackPointer pTrack = trackModel->getTrack(index);
    m_pTrackInfo->loadTrack(pTrack); // NULL is fine.
    currentTrackInfoIndex = index;
    m_pTrackInfo->show();
}

void WTrackMenu::slotNextDlgTagFetcher() {
    QModelIndex nextRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()+1, currentTrackInfoIndex.column());
    if (nextRow.isValid()) {
        showDlgTagFetcher(nextRow);
        if (!m_pTrackInfo.isNull()) {
            showTrackInfo(nextRow);
        }
    }
}

void WTrackMenu::slotPrevDlgTagFetcher() {
    QModelIndex prevRow = currentTrackInfoIndex.sibling(
        currentTrackInfoIndex.row()-1, currentTrackInfoIndex.column());
    if (prevRow.isValid()) {
        showDlgTagFetcher(prevRow);
        if (!m_pTrackInfo.isNull()) {
            showTrackInfo(prevRow);
        }
    }
}

void WTrackMenu::showDlgTagFetcher(QModelIndex index) {
    TrackModel* trackModel = m_pTrackModel;

    if (!trackModel) {
        return;
    }

    TrackPointer pTrack = trackModel->getTrack(index);
    currentTrackInfoIndex = index;
    slotShowTrackInTagFetcher(pTrack);
}

void WTrackMenu::slotShowTrackInTagFetcher(TrackPointer pTrack) {
    if (m_pTagFetcher.isNull()) {
        m_pTagFetcher.reset(new DlgTagFetcher(nullptr));
        connect(m_pTagFetcher.data(), SIGNAL(next()),
                this, SLOT(slotNextDlgTagFetcher()));
        connect(m_pTagFetcher.data(), SIGNAL(previous()),
                this, SLOT(slotPrevDlgTagFetcher()));
        connect(m_pTagFetcher.data(), SIGNAL(finished(int)),
                this, SLOT(slotTagFetcherClosed()));
    }

    // NULL is fine
    m_pTagFetcher->loadTrack(pTrack);
    m_pTagFetcher->show();
}

void WTrackMenu::slotShowDlgTagFetcher() {
    QModelIndexList indices = m_pSelectedTrackIndices;

    if (indices.size() > 0) {
        showDlgTagFetcher(indices[0]);
    }
}


void WTrackMenu::slotAddToAutoDJBottom() {
    // append to auto DJ
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::BOTTOM);
}

void WTrackMenu::slotAddToAutoDJTop() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::TOP);
}

void WTrackMenu::slotAddToAutoDJReplace() {
    addToAutoDJ(PlaylistDAO::AutoDJSendLoc::REPLACE);
}

void WTrackMenu::addToAutoDJ(PlaylistDAO::AutoDJSendLoc loc) {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ)) {
        return;
    }

    const TrackIdList trackIds = m_pTrackIdList;
    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for AutoDJ";
        return;
    }

    PlaylistDAO& playlistDao = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO();

    // TODO(XXX): Care whether the append succeeded.
    m_pTrackCollectionManager->unhideTracks(trackIds);
    playlistDao.addTracksToAutoDJQueue(trackIds, loc);
}


void WTrackMenu::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    TrackModel* trackModel = m_pTrackModel;
    if (trackModel == nullptr) {
        return;
    }
    const QModelIndexList selection = m_pSelectedTrackIndices;
    for (const QModelIndex& index : selection) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            pTrack->setCoverInfo(coverInfo);
        }
    }
}

void WTrackMenu::slotReloadCoverArt() {
    TrackModel* trackModel = m_pTrackModel;
    if (!trackModel) {
        return;
    }
    const QModelIndexList selection = m_pSelectedTrackIndices;
    if (selection.isEmpty()) {
        return;
    }
    QList<TrackPointer> selectedTracks;
    selectedTracks.reserve(selection.size());
    for (const QModelIndex& index : selection) {
        TrackPointer pTrack = trackModel->getTrack(index);
        if (pTrack) {
            selectedTracks.append(pTrack);
        }
    }
    guessTrackCoverInfoConcurrently(selectedTracks);
}

void WTrackMenu::slotPurge() {
    QModelIndexList indices = m_pSelectedTrackIndices;
    if (indices.size() > 0) {
        TrackModel* trackModel = m_pTrackModel;
        if (trackModel) {
            trackModel->purgeTracks(indices);
        }
    }
}

void WTrackMenu::slotUnhide() {
    QModelIndexList indices = m_pSelectedTrackIndices;

    if (indices.size() > 0) {
        TrackModel* trackModel = m_pTrackModel;
        if (trackModel) {
            trackModel->unhideTracks(indices);
        }
    }
}

void WTrackMenu::clearTrackSelection() {
    m_pTrackIdList.clear();
    m_pTrackPointerList.clear();
    m_pSelectedTrackIndices.clear();
}
