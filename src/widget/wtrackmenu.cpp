#include "widget/wtrackmenu.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QDesktopServices>
#include <QInputDialog>
#include <QModelIndex>
#include <QWidgetAction>

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
#include "track/trackref.h"
#include "util/desktophelper.h"
#include "util/parented_ptr.h"
#include "widget/wcolorpickeraction.h"
#include "widget/wskincolor.h"
#include "widget/wwidget.h"

WTrackMenu::WTrackMenu(QWidget* parent,
        UserSettingsPointer pConfig,
        TrackCollectionManager* pTrackCollectionManager,
        Filters flags,
        TrackModel* trackModel)
        : QMenu(parent),
          m_pTrackModel(trackModel),
          m_pConfig(pConfig),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_bPlaylistMenuLoaded(false),
          m_bCrateMenuLoaded(false),
          m_iCoverSourceColumn(-1),
          m_iCoverTypeColumn(-1),
          m_iCoverLocationColumn(-1),
          m_iCoverHashColumn(-1),
          m_iCoverColumn(-1),
          m_eFilters(flags) {
    m_pNumSamplers = new ControlProxy(
            "[Master]", "num_samplers", this);
    m_pNumDecks = new ControlProxy(
            "[Master]", "num_decks", this);
    m_pNumPreviewDecks = new ControlProxy(
            "[Master]", "num_preview_decks", this);

    if (trackModel) {
        m_iCoverSourceColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_SOURCE);
        m_iCoverTypeColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_TYPE);
        m_iCoverLocationColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_LOCATION);
        m_iCoverHashColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART_HASH);
        m_iCoverColumn = trackModel->fieldIndex(LIBRARYTABLE_COVERART);
        m_iTrackLocationColumn = trackModel->fieldIndex(TRACKLOCATIONSTABLE_LOCATION);
    }

    createMenus();
    createActions();
    setupActions();
}

void WTrackMenu::createMenus() {
    if (optionIsEnabled(Filter::LoadTo)) {
        m_pLoadToMenu = new QMenu(this);
        m_pLoadToMenu->setTitle(tr("Load to"));
        m_pDeckMenu = new QMenu(m_pLoadToMenu);
        m_pDeckMenu->setTitle(tr("Deck"));
        m_pSamplerMenu = new QMenu(m_pLoadToMenu);
        m_pSamplerMenu->setTitle(tr("Sampler"));
    }

    if (optionIsEnabled(Filter::Playlist)) {
        m_pPlaylistMenu = new QMenu(this);
        m_pPlaylistMenu->setTitle(tr("Add to Playlist"));
        connect(m_pPlaylistMenu, &QMenu::aboutToShow, this, &WTrackMenu::slotPopulatePlaylistMenu);
    }

    if (optionIsEnabled(Filter::Crate)) {
        m_pCrateMenu = new QMenu(this);
        m_pCrateMenu->setTitle(tr("Crates"));
        connect(m_pCrateMenu, &QMenu::aboutToShow, this, &WTrackMenu::slotPopulateCrateMenu);
    }

    if (optionIsEnabled(Filter::Metadata)) {
        m_pMetadataMenu = new QMenu(this);
        m_pMetadataMenu->setTitle(tr("Metadata"));

        m_pMetadataUpdateExternalCollectionsMenu = new QMenu(m_pMetadataMenu);
        m_pMetadataUpdateExternalCollectionsMenu->setTitle(tr("Update external collections"));

        m_pCoverMenu = new WCoverArtMenu(m_pMetadataMenu);
        m_pCoverMenu->setTitle(tr("Cover Art"));
        connect(m_pCoverMenu, &WCoverArtMenu::coverInfoSelected, this, &WTrackMenu::slotCoverInfoSelected);
        connect(m_pCoverMenu, &WCoverArtMenu::reloadCoverArt, this, &WTrackMenu::slotReloadCoverArt);
    }

    if (optionIsEnabled(Filter::BPM)) {
        m_pBPMMenu = new QMenu(this);
        m_pBPMMenu->setTitle(tr("Adjust BPM"));
    }

    if (optionIsEnabled(Filter::Color)) {
        m_pColorMenu = new QMenu(this);
        m_pColorMenu->setTitle(tr("Select Color"));
    }

    if (optionIsEnabled(Filter::Reset)) {
        m_pClearMetadataMenu = new QMenu(this);
        //: Reset metadata in right click track context menu in library
        m_pClearMetadataMenu->setTitle(tr("Reset"));
    }
}

void WTrackMenu::createActions() {
    if (optionIsEnabled(Filter::AutoDJ)) {
        m_pAutoDJBottomAct = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
        connect(m_pAutoDJBottomAct, &QAction::triggered, this, &WTrackMenu::slotAddToAutoDJBottom);

        m_pAutoDJTopAct = new QAction(tr("Add to Auto DJ Queue (top)"), this);
        connect(m_pAutoDJBottomAct, &QAction::triggered, this, &WTrackMenu::slotAddToAutoDJTop);

        m_pAutoDJReplaceAct = new QAction(tr("Add to Auto DJ Queue (replace)"), this);
        connect(m_pAutoDJBottomAct, &QAction::triggered, this, &WTrackMenu::slotAddToAutoDJReplace);
    }

    if (optionIsEnabled(Filter::LoadTo)) {
        m_pAddToPreviewDeck = new QAction(tr("Preview Deck"), m_pLoadToMenu);
        // currently there is only one preview deck so just map it here.
        QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
        connect(m_pAddToPreviewDeck, &QAction::triggered, this, [this, previewDeckGroup] { loadSelectionToGroup(previewDeckGroup); });
    }

    if (optionIsEnabled(Filter::Remove)) {
        m_pRemoveAct = new QAction(tr("Remove"), this);
        connect(m_pRemoveAct, &QAction::triggered, this, &WTrackMenu::slotRemove);

        m_pRemovePlaylistAct = new QAction(tr("Remove from Playlist"), this);
        connect(m_pRemovePlaylistAct, &QAction::triggered, this, &WTrackMenu::slotRemove);

        m_pRemoveCrateAct = new QAction(tr("Remove from Crate"), this);
        connect(m_pRemoveCrateAct, &QAction::triggered, this, &WTrackMenu::slotRemove);
    }

    if (optionIsEnabled(Filter::HideUnhidePurge)) {
        m_pHideAct = new QAction(tr("Hide from Library"), this);
        connect(m_pHideAct, &QAction::triggered, this, &WTrackMenu::slotHide);

        m_pUnhideAct = new QAction(tr("Unhide from Library"), this);
        connect(m_pUnhideAct, &QAction::triggered, this, &WTrackMenu::slotUnhide);

        m_pPurgeAct = new QAction(tr("Purge from Library"), this);
        connect(m_pPurgeAct, &QAction::triggered, this, &WTrackMenu::slotPurge);
    }

    if (optionIsEnabled(Filter::Properties)) {
        m_pPropertiesAct = new QAction(tr("Properties"), this);
        connect(m_pPropertiesAct, &QAction::triggered, this, &WTrackMenu::slotShowTrackInfo);
    }

    if (optionIsEnabled(Filter::FileBrowser)) {
        m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
        connect(m_pFileBrowserAct, &QAction::triggered, this, &WTrackMenu::slotOpenInFileBrowser);
    }

    if (optionIsEnabled(Filter::Metadata)) {
        m_pImportMetadataFromFileAct = new QAction(tr("Import From File Tags"), m_pMetadataMenu);
        connect(m_pImportMetadataFromFileAct, &QAction::triggered, this, &WTrackMenu::slotImportTrackMetadataFromFileTags);

        m_pImportMetadataFromMusicBrainzAct = new QAction(tr("Import From MusicBrainz"), m_pMetadataMenu);
        connect(m_pImportMetadataFromMusicBrainzAct, &QAction::triggered, this, &WTrackMenu::slotShowDlgTagFetcher);

        m_pExportMetadataAct = new QAction(tr("Export To File Tags"), m_pMetadataMenu);
        connect(m_pExportMetadataAct, &QAction::triggered, this, &WTrackMenu::slotExportTrackMetadataIntoFileTags);

        for (const auto& externalTrackCollection : m_pTrackCollectionManager->externalCollections()) {
            UpdateExternalTrackCollection updateInExternalTrackCollection;
            updateInExternalTrackCollection.externalTrackCollection = externalTrackCollection;
            updateInExternalTrackCollection.action = new QAction(externalTrackCollection->name(), m_pMetadataMenu);
            updateInExternalTrackCollection.action->setToolTip(externalTrackCollection->description());
            m_updateInExternalTrackCollections += updateInExternalTrackCollection;
            auto externalTrackCollectionPtr = updateInExternalTrackCollection.externalTrackCollection;
            connect(updateInExternalTrackCollection.action, &QAction::triggered, this, [this, externalTrackCollectionPtr] {
                slotUpdateExternalTrackCollection(externalTrackCollectionPtr);
            });
        }
    }

    if (optionIsEnabled(Filter::Reset)) {
        // Clear metadata actions
        m_pClearBeatsAction = new QAction(tr("BPM and Beatgrid"), m_pClearMetadataMenu);
        connect(m_pClearBeatsAction, &QAction::triggered, this, &WTrackMenu::slotClearBeats);

        m_pClearPlayCountAction = new QAction(tr("Play Count"), m_pClearMetadataMenu);
        connect(m_pClearPlayCountAction, &QAction::triggered, this, &WTrackMenu::slotClearPlayCount);

        m_pClearMainCueAction = new QAction(tr("Cue Point"), m_pClearMetadataMenu);
        connect(m_pClearMainCueAction, &QAction::triggered, this, &WTrackMenu::slotClearMainCue);

        m_pClearHotCuesAction = new QAction(tr("Hotcues"), m_pClearMetadataMenu);
        connect(m_pClearHotCuesAction, &QAction::triggered, this, &WTrackMenu::slotClearHotCues);

        m_pClearIntroCueAction = new QAction(tr("Intro"), m_pClearMetadataMenu);
        connect(m_pClearIntroCueAction, &QAction::triggered, this, &WTrackMenu::slotClearIntroCue);

        m_pClearOutroCueAction = new QAction(tr("Outro"), m_pClearMetadataMenu);
        connect(m_pClearOutroCueAction, &QAction::triggered, this, &WTrackMenu::slotClearOutroCue);

        m_pClearLoopAction = new QAction(tr("Loop"), m_pClearMetadataMenu);
        connect(m_pClearLoopAction, &QAction::triggered, this, &WTrackMenu::slotClearLoop);

        m_pClearKeyAction = new QAction(tr("Key"), m_pClearMetadataMenu);
        connect(m_pClearKeyAction, &QAction::triggered, this, &WTrackMenu::slotClearKey);

        m_pClearReplayGainAction = new QAction(tr("ReplayGain"), m_pClearMetadataMenu);
        connect(m_pClearReplayGainAction, &QAction::triggered, this, &WTrackMenu::slotClearReplayGain);

        m_pClearWaveformAction = new QAction(tr("Waveform"), m_pClearMetadataMenu);
        connect(m_pClearWaveformAction, &QAction::triggered, this, &WTrackMenu::slotClearWaveform);

        m_pClearAllMetadataAction = new QAction(tr("All"), m_pClearMetadataMenu);
        connect(m_pClearAllMetadataAction, &QAction::triggered, this, &WTrackMenu::slotClearAllMetadata);
    }

    if (optionIsEnabled(Filter::BPM)) {
        m_pBpmLockAction = new QAction(tr("Lock BPM"), m_pBPMMenu);
        m_pBpmUnlockAction = new QAction(tr("Unlock BPM"), m_pBPMMenu);
        connect(m_pBpmLockAction, &QAction::triggered, this, &WTrackMenu::slotLockBpm);
        connect(m_pBpmUnlockAction, &QAction::triggered, this, &WTrackMenu::slotUnlockBpm);

        //BPM edit actions
        m_pBpmDoubleAction = new QAction(tr("Double BPM"), m_pBPMMenu);
        m_pBpmHalveAction = new QAction(tr("Halve BPM"), m_pBPMMenu);
        m_pBpmTwoThirdsAction = new QAction(tr("2/3 BPM"), m_pBPMMenu);
        m_pBpmThreeFourthsAction = new QAction(tr("3/4 BPM"), m_pBPMMenu);
        m_pBpmFourThirdsAction = new QAction(tr("4/3 BPM"), m_pBPMMenu);
        m_pBpmThreeHalvesAction = new QAction(tr("3/2 BPM"), m_pBPMMenu);

        connect(m_pBpmDoubleAction, &QAction::triggered, this, [this] { slotScaleBpm(Beats::DOUBLE); });
        connect(m_pBpmHalveAction, &QAction::triggered, this, [this] { slotScaleBpm(Beats::HALVE); });
        connect(m_pBpmTwoThirdsAction, &QAction::triggered, this, [this] { slotScaleBpm(Beats::TWOTHIRDS); });
        connect(m_pBpmThreeFourthsAction, &QAction::triggered, this, [this] { slotScaleBpm(Beats::THREEFOURTHS); });
        connect(m_pBpmFourThirdsAction, &QAction::triggered, this, [this] { slotScaleBpm(Beats::FOURTHIRDS); });
        connect(m_pBpmThreeHalvesAction, &QAction::triggered, this, [this] { slotScaleBpm(Beats::THREEHALVES); });
    }

    if (optionIsEnabled(Filter::Color)) {
        ColorPaletteSettings colorPaletteSettings(m_pConfig);
        m_pColorPickerAction = new WColorPickerAction(WColorPicker::Option::AllowNoColor,
                colorPaletteSettings.getTrackColorPalette(),
                m_pColorMenu);
        m_pColorPickerAction->setObjectName("TrackColorPickerAction");
        connect(m_pColorPickerAction,
                &WColorPickerAction::colorPicked,
                this,
                &WTrackMenu::slotColorPicked);
    }
}

void WTrackMenu::setupActions() {
    if (optionIsEnabled(Filter::AutoDJ)) {
        addAction(m_pAutoDJBottomAct);
        addAction(m_pAutoDJTopAct);
        addAction(m_pAutoDJReplaceAct);
        addSeparator();
    }

    if (optionIsEnabled(Filter::LoadTo)) {
        m_pLoadToMenu->addMenu(m_pDeckMenu);

        m_pLoadToMenu->addMenu(m_pSamplerMenu);

        if (m_pNumPreviewDecks->get() > 0.0) {
            m_pLoadToMenu->addAction(m_pAddToPreviewDeck);
        }

        addMenu(m_pLoadToMenu);
        addSeparator();
    }

    if (optionIsEnabled(Filter::Playlist)) {
        addMenu(m_pPlaylistMenu);
    }

    if (optionIsEnabled(Filter::Crate)) {
        addMenu(m_pCrateMenu);
    }

    if (optionIsEnabled(Filter::Remove)) {
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE)) {
            addAction(m_pRemoveAct);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST)) {
            addAction(m_pRemovePlaylistAct);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_CRATE)) {
            addAction(m_pRemoveCrateAct);
        }
    }

    addSeparator();

    if (optionIsEnabled(Filter::Metadata)) {
        m_pMetadataMenu->addAction(m_pImportMetadataFromFileAct);
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

        m_pMetadataMenu->addMenu(m_pCoverMenu);
        addMenu(m_pMetadataMenu);
    }

    if (optionIsEnabled(Filter::Reset)) {
        m_pClearMetadataMenu->addAction(m_pClearBeatsAction);
        m_pClearMetadataMenu->addAction(m_pClearPlayCountAction);
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
        addMenu(m_pClearMetadataMenu);
    }

    if (optionIsEnabled(Filter::BPM)) {
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

        addMenu(m_pBPMMenu);
    }

    if (optionIsEnabled(Filter::Color)) {
        m_pColorMenu->addAction(m_pColorPickerAction);
        addMenu(m_pColorMenu);
    }

    addSeparator();
    if (optionIsEnabled(Filter::HideUnhidePurge)) {
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_HIDE)) {
            addAction(m_pHideAct);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE)) {
            addAction(m_pUnhideAct);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_PURGE)) {
            addAction(m_pPurgeAct);
        }
    }

    if (optionIsEnabled(Filter::FileBrowser)) {
        addAction(m_pFileBrowserAct);
    }

    if (optionIsEnabled(Filter::Properties)) {
        addSeparator();
        addAction(m_pPropertiesAct);
    }
}

void WTrackMenu::updateMenus() {
    // Changes menu options depending on track(s)
    auto trackModel = getTrackModel();
    const auto indices = getTrackIndices();
    const auto trackIds = getTrackIds();
    const auto trackPointers = getTrackPointers();

    // Gray out some stuff if multiple songs were selected.
    bool oneSongSelected = trackPointers.size() == 1;

    if (optionIsEnabled(Filter::LoadTo)) {
        int iNumDecks = m_pNumDecks->get();
        m_pDeckMenu->clear();
        if (iNumDecks > 0) {
            for (int i = 1; i <= iNumDecks; ++i) {
                // PlayerManager::groupForDeck is 0-indexed.
                QString deckGroup = PlayerManager::groupForDeck(i - 1);
                bool deckPlaying = ControlObject::get(
                                           ConfigKey(deckGroup, "play")) > 0.0;
                bool loadTrackIntoPlayingDeck = m_pConfig->getValue<bool>(
                        ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"));
                bool deckEnabled = (!deckPlaying || loadTrackIntoPlayingDeck) && oneSongSelected;
                QAction* pAction = new QAction(tr("Deck %1").arg(i), this);
                pAction->setEnabled(deckEnabled);
                m_pDeckMenu->addAction(pAction);
                connect(pAction, &QAction::triggered, this, [this, deckGroup] { loadSelectionToGroup(deckGroup); });
            }
        }

        int iNumSamplers = m_pNumSamplers->get();
        if (iNumSamplers > 0) {
            m_pSamplerMenu->clear();
            for (int i = 1; i <= iNumSamplers; ++i) {
                // PlayerManager::groupForSampler is 0-indexed.
                QString samplerGroup = PlayerManager::groupForSampler(i - 1);
                bool samplerPlaying = ControlObject::get(
                                              ConfigKey(samplerGroup, "play")) > 0.0;
                bool samplerEnabled = !samplerPlaying && oneSongSelected;
                QAction* pAction = new QAction(tr("Sampler %1").arg(i), m_pSamplerMenu);
                pAction->setEnabled(samplerEnabled);
                m_pSamplerMenu->addAction(pAction);
                connect(pAction, &QAction::triggered, this, [this, samplerGroup] { loadSelectionToGroup(samplerGroup); });
            }
        }
    }

    if (optionIsEnabled(Filter::Playlist)) {
        // Playlist menu is lazy loaded on hover by slotPopulatePlaylistMenu
        // to avoid unnecessary database queries
        m_bPlaylistMenuLoaded = false;
    }

    if (optionIsEnabled(Filter::Crate)) {
        // Crate menu is lazy loaded on hover by slotPopulateCrateMenu
        // to avoid unnecessary database queries
        m_bCrateMenuLoaded = false;
    }

    if (optionIsEnabled(Filter::Remove)) {
        bool locked = modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOCKED);
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE)) {
            m_pRemoveAct->setEnabled(!locked);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST)) {
            m_pRemovePlaylistAct->setEnabled(!locked);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_CRATE)) {
            m_pRemoveCrateAct->setEnabled(!locked);
        }
    }

    if (optionIsEnabled(Filter::Metadata)) {
        m_pImportMetadataFromMusicBrainzAct->setEnabled(oneSongSelected);

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
                                             last.row(), m_iTrackLocationColumn)
                                         .data()
                                         .toString();
            info.coverLocation = last.sibling(
                                             last.row(), m_iCoverLocationColumn)
                                         .data()
                                         .toString();
            m_pCoverMenu->setCoverArt(info);
        }
    }

    if (optionIsEnabled(Filter::Reset)) {
        VERIFY_OR_DEBUG_ASSERT(trackModel) {
            return;
        }
        bool allowClear = true;
        int column = trackModel->fieldIndex(LIBRARYTABLE_BPM_LOCK);
        for (int i = 0; i < indices.size() && allowClear; ++i) {
            int row = indices.at(i).row();
            QModelIndex index = indices.at(i).sibling(row, column);
            if (index.data().toBool()) {
                allowClear = false;
            }
        }
        m_pClearBeatsAction->setEnabled(allowClear);
    }

    if (optionIsEnabled(Filter::BPM)) {
        if (oneSongSelected) {
            if (!trackModel) {
                return;
            }
            int column = trackModel->fieldIndex(LIBRARYTABLE_BPM_LOCK);
            QModelIndex index = indices.at(0).sibling(indices.at(0).row(), column);
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
                QModelIndex index = indices.at(i).sibling(row, column);
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
    }

    // Track color menu only appears if at least one track is selected
    if (optionIsEnabled(Filter::Color) && !indices.empty()) {
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
    }

    if (optionIsEnabled(Filter::HideUnhidePurge)) {
        bool locked = modelHasCapabilities(TrackModel::TRACKMODELCAPS_LOCKED);
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_HIDE)) {
            m_pHideAct->setEnabled(!locked);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE)) {
            m_pUnhideAct->setEnabled(!locked);
        }
        if (modelHasCapabilities(TrackModel::TRACKMODELCAPS_PURGE)) {
            m_pPurgeAct->setEnabled(!locked);
        }
    }

    if (optionIsEnabled(Filter::Properties)) {
        m_pPropertiesAct->setEnabled(oneSongSelected);
    }
}

void WTrackMenu::loadTracks(TrackIdList trackIdList) {
    // Clean all forms of track store
    clearTrackSelection();

    TrackPointerList trackPointers;
    trackPointers.reserve(trackIdList.size());
    for (const auto& trackId : trackIdList) {
        TrackPointer pTrack = m_pTrackCollectionManager->internalCollection()->getTrackById(trackId);
        if (!pTrack) {
            return;
        }
        trackPointers.push_back(pTrack);
    }
    if (trackPointers.empty()) {
        return;
    }

    m_pTrackPointerList = trackPointers;

    updateMenus();
}

void WTrackMenu::loadTracks(QModelIndexList indexList) {
    // Clean all forms of track store
    clearTrackSelection();

    auto trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    QModelIndexList indices;
    indices.reserve(indexList.size());
    for (const auto& index : indexList) {
        TrackPointer pTrack = trackModel->getTrack(index);
        // Checking if passed indexList is valid
        if (!pTrack) {
            return;
        }
        indices.push_back(index);
    }
    if (indices.empty()) {
        return;
    }

    m_pTrackIndexList = indices;

    updateMenus();
}

void WTrackMenu::loadTrack(TrackId trackId) {
    // Create a QList of single track to maintain common functions
    // for single and multi track selection.
    TrackIdList singleItemTrackIdList;
    singleItemTrackIdList.push_back(trackId);
    // Use setTrackIds to set a list of single element.
    loadTracks(singleItemTrackIdList);
}

void WTrackMenu::loadTrack(QModelIndex index) {
    QModelIndexList singleItemTrackIndexList;
    singleItemTrackIndexList.push_back(index);
    loadTracks(singleItemTrackIndexList);
}

TrackIdList WTrackMenu::getTrackIds() const {
    TrackIdList trackIds;
    auto trackModel = getTrackModel();
    if (trackModel) {
        const QModelIndexList indices = getTrackIndices();
        trackIds.reserve(indices.size());
        for (const auto& index : indices) {
            trackIds.push_back(trackModel->getTrackId(index));
        }
    } else {
        const TrackPointerList trackPointers = getTrackPointers();
        for (const auto& pTrack : trackPointers) {
            trackIds.push_back(pTrack->getId());
        }
    }
    return trackIds;
}

TrackPointerList WTrackMenu::getTrackPointers() const {
    auto trackModel = getTrackModel();
    if (trackModel) {
        const QModelIndexList indices = getTrackIndices();
        TrackPointerList trackPointers;
        trackPointers.reserve(indices.size());
        for (const auto& index : indices) {
            trackPointers.push_back(trackModel->getTrack(index));
        }
        return trackPointers;
    }
    return m_pTrackPointerList;
}

QModelIndexList WTrackMenu::getTrackIndices() const {
    return m_pTrackIndexList;
}

TrackModel* WTrackMenu::getTrackModel() const {
    return m_pTrackModel;
}

void WTrackMenu::slotOpenInFileBrowser() {
    TrackPointerList trackPointerList = getTrackPointers();
    QStringList locations;
    for (const TrackPointer& trackPointer : trackPointerList) {
        locations << trackPointer->getLocation();
    }
    mixxx::DesktopHelper::openInFileBrowser(locations);
}

void WTrackMenu::slotImportTrackMetadataFromFileTags() {
    if (!modelHasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA)) {
        return;
    }

    const QModelIndexList indices = getTrackIndices();

    auto trackModel = getTrackModel();

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

    auto trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    const QModelIndexList indices = getTrackIndices();
    if (indices.isEmpty()) {
        return;
    }

    mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();

    for (const QModelIndex& index : indices) {
        TrackPointer pTrack = trackModel->getTrack(index);
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

    auto trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }

    const QModelIndexList indices = getTrackIndices();
    if (indices.isEmpty()) {
        return;
    }

    QList<TrackRef> trackRefs;
    trackRefs.reserve(indices.size());
    for (const QModelIndex& index : indices) {
        trackRefs.append(
                TrackRef::fromFileInfo(
                        trackModel->getTrackLocation(index),
                        trackModel->getTrackId(index)));
    }

    externalTrackCollection->updateTracks(std::move(trackRefs));
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
    QMap<QString, int> playlists;
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
            connect(pAction, &QAction::triggered, this, [this, iPlaylistId] { addSelectionToPlaylist(iPlaylistId); });
        }
    }
    m_pPlaylistMenu->addSeparator();
    QAction* newPlaylistAction = new QAction(tr("Create New Playlist"), m_pPlaylistMenu);
    m_pPlaylistMenu->addAction(newPlaylistAction);
    connect(newPlaylistAction, &QAction::triggered, this, [this] { addSelectionToPlaylist(-1); });
    m_bPlaylistMenuLoaded = true;
}

void WTrackMenu::addSelectionToPlaylist(int iPlaylistId) {
    const TrackIdList trackIds = getTrackIds();
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
                    &ok)
                           .trimmed();
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
        iPlaylistId = playlistDao.createPlaylist(name); //-1 is changed to the new playlist ID return from the DAO
        if (iPlaylistId == -1) {
            QMessageBox::warning(nullptr,
                    tr("Playlist Creation Failed"),
                    tr("An unknown error occurred while creating playlist: ") + name);
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
    const TrackIdList trackIds = getTrackIds();

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
        connect(pAction.get(), &QAction::triggered, this, [this, pCheckBox{pCheckBox.get()}] { updateSelectionCrates(pCheckBox); });
        connect(pCheckBox.get(), &QCheckBox::stateChanged, this, [this, pCheckBox{pCheckBox.get()}] { updateSelectionCrates(pCheckBox); });
    }
    m_pCrateMenu->addSeparator();
    QAction* newCrateAction = new QAction(tr("Add to New Crate"), m_pCrateMenu);
    m_pCrateMenu->addAction(newCrateAction);
    connect(newCrateAction, &QAction::triggered, this, &WTrackMenu::addSelectionToNewCrate);
    m_bCrateMenuLoaded = true;
}

void WTrackMenu::updateSelectionCrates(QWidget* pWidget) {
    auto pCheckBox = qobject_cast<QCheckBox*>(pWidget);
    VERIFY_OR_DEBUG_ASSERT(pCheckBox) {
        qWarning() << "crateId is not of CrateId type";
        return;
    }
    CrateId crateId = pCheckBox->property("crateId").value<CrateId>();

    const TrackIdList trackIds = getTrackIds();

    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for crate";
        return;
    }

    // we need to disable tristate again as the mixed state will now be gone and can't be brought back
    pCheckBox->setTristate(false);
    if (!pCheckBox->isChecked()) {
        if (crateId.isValid()) {
            m_pTrackCollectionManager->internalCollection()->removeCrateTracks(crateId, trackIds);
        }
    } else {
        if (!crateId.isValid()) { // i.e. a new crate is suppose to be created
            crateId = CrateFeatureHelper(
                    m_pTrackCollectionManager->internalCollection(), m_pConfig)
                              .createEmptyCrate();
        }
        if (crateId.isValid()) {
            m_pTrackCollectionManager->unhideTracks(trackIds);
            m_pTrackCollectionManager->internalCollection()->addCrateTracks(crateId, trackIds);
        }
    }
}

void WTrackMenu::addSelectionToNewCrate() {
    const TrackIdList trackIds = getTrackIds();

    if (trackIds.isEmpty()) {
        qWarning() << "No tracks selected for crate";
        return;
    }

    CrateId crateId = CrateFeatureHelper(
            m_pTrackCollectionManager->internalCollection(), m_pConfig)
                              .createEmptyCrate();

    if (crateId.isValid()) {
        m_pTrackCollectionManager->unhideTracks(trackIds);
        m_pTrackCollectionManager->internalCollection()->addCrateTracks(crateId, trackIds);
    }
}

void WTrackMenu::slotLockBpm() {
    lockBpm(true);
}

void WTrackMenu::slotUnlockBpm() {
    lockBpm(false);
}

void WTrackMenu::slotScaleBpm(int scale) {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        if (pTrack && !pTrack->isBpmLocked()) {
            BeatsPointer pBeats = pTrack->getBeats();
            if (pBeats) {
                pBeats->scale(static_cast<Beats::BPMScale>(scale));
            }
        }
    }
}

void WTrackMenu::lockBpm(bool lock) {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    // TODO: This should be done in a thread for large selections
    for (const auto& pTrack : trackPointers) {
        if (!pTrack->isBpmLocked()) {
            pTrack->setBpmLocked(lock);
        }
    }
}

void WTrackMenu::slotColorPicked(mixxx::RgbColor::optional_t color) {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    // TODO: This should be done in a thread for large selections
    for (const auto& pTrack : trackPointers) {
        if (!pTrack->isBpmLocked()) {
            pTrack->setColor(color);
        }
    }

    hide();
}

void WTrackMenu::loadSelectionToGroup(QString group, bool play) {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }
    // If the track load override is disabled, check to see if a track is
    // playing before trying to load it
    if (!(m_pConfig->getValueString(
                           ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"))
                        .toInt())) {
        // TODO(XXX): Check for other than just the first preview deck.
        if (group != "[PreviewDeck1]" &&
                ControlObject::get(ConfigKey(group, "play")) > 0.0) {
            return;
        }
    }

    TrackPointer pTrack = trackPointers.at(0);
    if (pTrack) {
        // TODO: load track from this class without depending on
        // external slot to load track
        emit loadTrackToPlayer(pTrack, group, play);
    }
}

//slot for reset played count, sets count to 0 of one or more tracks
void WTrackMenu::slotClearPlayCount() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->resetPlayCounter();
    }
}

void WTrackMenu::slotClearBeats() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    // TODO: This should be done in a thread for large selections
    for (const auto& pTrack : trackPointers) {
        if (!pTrack->isBpmLocked()) {
            pTrack->setBeats(BeatsPointer());
        }
    }
}

void WTrackMenu::slotClearMainCue() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->removeCuesOfType(mixxx::CueType::MainCue);
    }
}

void WTrackMenu::slotClearOutroCue() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->removeCuesOfType(mixxx::CueType::Outro);
    }
}

void WTrackMenu::slotClearIntroCue() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->removeCuesOfType(mixxx::CueType::Intro);
    }
}

void WTrackMenu::slotClearKey() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->resetKeys();
    }
}

void WTrackMenu::slotClearReplayGain() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->setReplayGain(mixxx::ReplayGain());
    }
}

void WTrackMenu::slotClearWaveform() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    AnalysisDao& analysisDao = m_pTrackCollectionManager->internalCollection()->getAnalysisDAO();
    for (const auto& pTrack : trackPointers) {
        analysisDao.deleteAnalysesForTrack(pTrack->getId());
        pTrack->setWaveform(WaveformPointer());
        pTrack->setWaveformSummary(WaveformPointer());
    }
}

void WTrackMenu::slotClearLoop() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->removeCuesOfType(mixxx::CueType::Loop);
    }
}

void WTrackMenu::slotClearHotCues() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }

    for (const auto& pTrack : trackPointers) {
        pTrack->removeCuesOfType(mixxx::CueType::HotCue);
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
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    const QModelIndexList indices = getTrackIndices();

    if (indices.size() > 0) {
        showTrackInfo(indices[0]);
    }
}

void WTrackMenu::slotNextTrackInfo() {
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    QModelIndex nextRow = currentTrackInfoIndex.sibling(
            currentTrackInfoIndex.row() + 1, currentTrackInfoIndex.column());
    if (nextRow.isValid()) {
        showTrackInfo(nextRow);
        if (!m_pTagFetcher.isNull()) {
            showDlgTagFetcher(nextRow);
        }
    }
}

void WTrackMenu::slotPrevTrackInfo() {
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    QModelIndex prevRow = currentTrackInfoIndex.sibling(
            currentTrackInfoIndex.row() - 1, currentTrackInfoIndex.column());
    if (prevRow.isValid()) {
        showTrackInfo(prevRow);
        if (!m_pTagFetcher.isNull()) {
            showDlgTagFetcher(prevRow);
        }
    }
}

void WTrackMenu::showTrackInfo(QModelIndex index) {
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }

    if (m_pTrackInfo.isNull()) {
        // Give a NULL parent because otherwise it inherits our style which can
        // make it unreadable. Bug #673411
        m_pTrackInfo.reset(new DlgTrackInfo(m_pConfig, nullptr));

        connect(m_pTrackInfo.data(), &DlgTrackInfo::next, this, &WTrackMenu::slotNextTrackInfo);
        connect(m_pTrackInfo.data(), &DlgTrackInfo::previous, this, &WTrackMenu::slotPrevTrackInfo);
        connect(m_pTrackInfo.data(), &DlgTrackInfo::showTagFetcher, this, &WTrackMenu::slotShowTrackInTagFetcher);
        connect(m_pTrackInfo.data(), &DlgTrackInfo::finished, this, &WTrackMenu::slotTrackInfoClosed);
    }
    TrackPointer pTrack = trackModel->getTrack(index);
    m_pTrackInfo->loadTrack(pTrack); // NULL is fine.
    currentTrackInfoIndex = index;
    m_pTrackInfo->show();
}

void WTrackMenu::slotNextDlgTagFetcher() {
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    QModelIndex nextRow = currentTrackInfoIndex.sibling(
            currentTrackInfoIndex.row() + 1, currentTrackInfoIndex.column());
    if (nextRow.isValid()) {
        showDlgTagFetcher(nextRow);
        if (!m_pTrackInfo.isNull()) {
            showTrackInfo(nextRow);
        }
    }
}

void WTrackMenu::slotPrevDlgTagFetcher() {
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }
    QModelIndex prevRow = currentTrackInfoIndex.sibling(
            currentTrackInfoIndex.row() - 1, currentTrackInfoIndex.column());
    if (prevRow.isValid()) {
        showDlgTagFetcher(prevRow);
        if (!m_pTrackInfo.isNull()) {
            showTrackInfo(prevRow);
        }
    }
}

void WTrackMenu::showDlgTagFetcher(QModelIndex index) {
    auto trackModel = getTrackModel();
    VERIFY_OR_DEBUG_ASSERT(trackModel) {
        return;
    }

    TrackPointer pTrack = trackModel->getTrack(index);
    currentTrackInfoIndex = index;
    slotShowTrackInTagFetcher(pTrack);
}

void WTrackMenu::slotShowTrackInTagFetcher(TrackPointer pTrack) {
    if (m_pTagFetcher.isNull()) {
        m_pTagFetcher.reset(new DlgTagFetcher(nullptr));
        connect(m_pTagFetcher.data(), &DlgTagFetcher::next, this, &WTrackMenu::slotNextDlgTagFetcher);
        connect(m_pTagFetcher.data(), &DlgTagFetcher::previous, this, &WTrackMenu::slotPrevDlgTagFetcher);
        connect(m_pTagFetcher.data(), &DlgTagFetcher::finished, this, &WTrackMenu::slotTagFetcherClosed);
    }

    // NULL is fine
    m_pTagFetcher->loadTrack(pTrack);
    m_pTagFetcher->show();
}

void WTrackMenu::slotShowDlgTagFetcher() {
    const QModelIndexList indices = getTrackIndices();
    if (indices.empty()) {
        return;
    }

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
    const TrackIdList trackIds = getTrackIds();
    if (trackIds.empty()) {
        return;
    }
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
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }
    for (const auto& pTrack : trackPointers) {
        pTrack->setCoverInfo(coverInfo);
    }
}

void WTrackMenu::slotReloadCoverArt() {
    const TrackPointerList trackPointers = getTrackPointers();
    if (trackPointers.empty()) {
        return;
    }
    guessTrackCoverInfoConcurrently(trackPointers);
}

void WTrackMenu::slotRemove() {
    auto trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }
    const QModelIndexList indices = getTrackIndices();
    if (!indices.empty()) {
        trackModel->removeTracks(indices);
    }
}

void WTrackMenu::slotHide() {
    auto trackModel = getTrackModel();
    if (!trackModel) {
        return;
    }
    const QModelIndexList indices = getTrackIndices();
    if (indices.size() > 0) {
        trackModel->hideTracks(indices);
    }
}

void WTrackMenu::slotUnhide() {
    auto trackModel = getTrackModel();
    if (trackModel) {
        const QModelIndexList indices = getTrackIndices();

        if (indices.size() > 0) {
            if (trackModel) {
                trackModel->unhideTracks(indices);
            }
        }
    }
}

void WTrackMenu::slotPurge() {
    auto trackModel = getTrackModel();
    if (trackModel) {
        const QModelIndexList indices = getTrackIndices();
        if (indices.size() > 0) {
            auto trackModel = getTrackModel();
            if (trackModel) {
                trackModel->purgeTracks(indices);
            }
        }
    }
}

void WTrackMenu::clearTrackSelection() {
    m_pTrackPointerList.clear();
    m_pTrackIndexList.clear();
}

bool WTrackMenu::modelHasCapabilities(TrackModel::CapabilitiesFlags capabilities) const {
    auto trackModel = getTrackModel();
    return trackModel &&
            (trackModel->getCapabilities() & capabilities) == capabilities;
}

bool WTrackMenu::optionIsEnabled(Filter flag) const {
    auto trackModel = getTrackModel();
    bool optionIsAvailable = m_eFilters.testFlag(Filter::None) || m_eFilters.testFlag(flag);

    if (!optionIsAvailable) {
        return false;
    }

    Filters independentOptions =
            Filter::Playlist |
            Filter::Crate |
            Filter::FileBrowser;

    // Some of these can be made independent of track table.
    Filters trackTableDependentOptions =
            Filter::AutoDJ |
            Filter::LoadTo |
            Filter::Remove |
            Filter::Metadata |
            Filter::Reset |
            Filter::BPM |
            Filter::Color |
            Filter::HideUnhidePurge |
            Filter::Properties;

    if (independentOptions.testFlag(flag)) {
        return true;
    }

    if (trackTableDependentOptions.testFlag(flag)) {
        if (!trackModel) {
            // Add to AutoDJ should be allowed from non-WTrackTableViews
            if (flag == Filter::AutoDJ) {
                return true;
            }
            return false;
        }
        if (flag == Filter::AutoDJ) {
            return modelHasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ);
        } else if (flag == Filter::Remove) {
            return modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE) ||
                    modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST) ||
                    modelHasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_CRATE);
        } else if (flag == Filter::HideUnhidePurge) {
            return modelHasCapabilities(TrackModel::TRACKMODELCAPS_HIDE) ||
                    modelHasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE) ||
                    modelHasCapabilities(TrackModel::TRACKMODELCAPS_PURGE);
        }
    }

    return true;
}
