#include "widget/wtrackmenu.h"

#include <qlist.h>

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QListWidget>
#include <QModelIndex>
#include <QVBoxLayout>

#include "analyzer/analyzerscheduledtrack.h"
#include "analyzer/analyzersilence.h"
#include "analyzer/analyzertrack.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "library/coverartutils.h"
#include "library/dao/trackdao.h"
#include "library/dao/trackschema.h"
#include "library/dlgtagfetcher.h"
#include "library/dlgtrackinfo.h"
#include "library/dlgtrackmetadataexport.h"
#include "library/externaltrackcollection.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackmodel.h"
#include "library/trackmodeliterator.h"
#include "library/trackprocessing.h"
#include "library/trackset/crate/cratefeaturehelper.h"
#include "mixer/playermanager.h"
#include "moc_wtrackmenu.cpp"
#include "preferences/colorpalettesettings.h"
#include "preferences/configobject.h"
#include "preferences/dialog/dlgprefdeck.h"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/desktophelper.h"
#include "util/parented_ptr.h"
#include "util/qt.h"
#include "util/widgethelper.h"
#include "widget/findonwebmenufactory.h"
#include "widget/wcolorpickeraction.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"
#include "widget/wfindonwebmenu.h"
#include "widget/wsearchrelatedtracksmenu.h"
#include "widget/wskincolor.h"
#include "widget/wstarrating.h"
#include "widget/wwidget.h"

WTrackMenu::WTrackMenu(
        QWidget* parent,
        UserSettingsPointer pConfig,
        Library* pLibrary,
        Features flags,
        TrackModel* trackModel)
        : QMenu(parent),
          m_pTrackModel(trackModel),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_bPlaylistMenuLoaded(false),
          m_bCrateMenuLoaded(false),
          m_eActiveFeatures(flags),
          m_eTrackModelFeatures(Feature::TrackModelFeatures) {
    m_pNumSamplers = new ControlProxy(
            "[Master]", "num_samplers", this);
    m_pNumDecks = new ControlProxy(
            "[Master]", "num_decks", this);
    m_pNumPreviewDecks = new ControlProxy(
            "[Master]", "num_preview_decks", this);

    // Warn if any of the chosen features depend on a TrackModel
    VERIFY_OR_DEBUG_ASSERT(trackModel || (m_eTrackModelFeatures & flags) == 0) {
        // Remove unsupported features
        m_eActiveFeatures &= !m_eTrackModelFeatures;
    }

    createMenus();
    createActions();
    setupActions();
}

WTrackMenu::~WTrackMenu() {
    // ~QPointer() needs the definition of the wrapped type
    // upon deletion! Otherwise the behavior is undefined.
    // The wrapped types of some QPointer members are only
    // forward declared in the header file.
}

int WTrackMenu::getTrackCount() const {
    if (m_pTrackModel) {
        return m_trackIndexList.size();
    } else {
        return m_pTrack ? 1 : 0;
    }
}

void WTrackMenu::closeEvent(QCloseEvent* event) {
    // Actually the event is accepted by default. doing it explicitly doesn't hurt.
    // If it's not accepted the menu remains open and entire GUI will be blocked!
    event->accept();
    emit trackMenuVisible(false);
}

void WTrackMenu::popup(const QPoint& pos, QAction* at) {
    if (isEmpty()) {
        return;
    }
    QMenu::popup(pos, at);
    emit trackMenuVisible(true);
}

void WTrackMenu::createMenus() {
    if (featureIsEnabled(Feature::LoadTo)) {
        m_pLoadToMenu = new QMenu(this);
        m_pLoadToMenu->setTitle(tr("Load to"));
        m_pDeckMenu = new QMenu(m_pLoadToMenu);
        m_pDeckMenu->setTitle(tr("Deck"));
        m_pSamplerMenu = new QMenu(m_pLoadToMenu);
        m_pSamplerMenu->setTitle(tr("Sampler"));
    }

    if (featureIsEnabled(Feature::Playlist)) {
        m_pPlaylistMenu = new QMenu(this);
        m_pPlaylistMenu->setTitle(tr("Add to Playlist"));
        connect(m_pPlaylistMenu, &QMenu::aboutToShow, this, &WTrackMenu::slotPopulatePlaylistMenu);
    }

    if (featureIsEnabled(Feature::Crate)) {
        m_pCrateMenu = new QMenu(this);
        m_pCrateMenu->setTitle(tr("Crates"));
        m_pCrateMenu->setObjectName("CratesMenu");
        connect(m_pCrateMenu, &QMenu::aboutToShow, this, &WTrackMenu::slotPopulateCrateMenu);
    }

    if (featureIsEnabled(Feature::Metadata)) {
        m_pMetadataMenu = new QMenu(this);
        m_pMetadataMenu->setTitle(tr("Metadata"));

        m_pMetadataUpdateExternalCollectionsMenu = new QMenu(m_pMetadataMenu);
        m_pMetadataUpdateExternalCollectionsMenu->setTitle(tr("Update external collections"));

        m_pCoverMenu = new WCoverArtMenu(m_pMetadataMenu);
        m_pCoverMenu->setTitle(tr("Cover Art"));
        connect(m_pCoverMenu, &WCoverArtMenu::coverInfoSelected, this, &WTrackMenu::slotCoverInfoSelected);
        connect(m_pCoverMenu, &WCoverArtMenu::reloadCoverArt, this, &WTrackMenu::slotReloadCoverArt);
    }

    if (featureIsEnabled(Feature::BPM)) {
        m_pBPMMenu = new QMenu(this);
        m_pBPMMenu->setTitle(tr("Adjust BPM"));
    }

    if (featureIsEnabled(Feature::Color)) {
        m_pColorMenu = new QMenu(this);
        m_pColorMenu->setTitle(tr("Select Color"));
    }

    if (featureIsEnabled(Feature::Reset)) {
        m_pClearMetadataMenu = new QMenu(this);
        //: Reset metadata in right click track context menu in library
        m_pClearMetadataMenu->setTitle(tr("Reset"));
    }

    if (featureIsEnabled(Feature::Analyze)) {
        m_pAnalyzeMenu = new QMenu(this);
        m_pAnalyzeMenu->setTitle(tr("Analyze"));
    }

    if (featureIsEnabled(Feature::SearchRelated)) {
        DEBUG_ASSERT(!m_pSearchRelatedMenu);
        m_pSearchRelatedMenu =
                make_parented<WSearchRelatedTracksMenu>(this);
        connect(m_pSearchRelatedMenu,
                &QMenu::aboutToShow,
                this,
                [this] {
                    m_pSearchRelatedMenu->clear();
                    const auto pTrack = getFirstTrackPointer();
                    if (pTrack) {
                        m_pSearchRelatedMenu->addActionsForTrack(*pTrack);
                    }
                    m_pSearchRelatedMenu->setEnabled(
                            !m_pSearchRelatedMenu->isEmpty());
                });
        connect(m_pSearchRelatedMenu,
                &WSearchRelatedTracksMenu::triggerSearch,
                this,
                [this](const QString& searchQuery) {
                    m_pLibrary->searchTracksInCollection(searchQuery);
                });
    }

    if (featureIsEnabled(Feature::FindOnWeb)) {
        DEBUG_ASSERT(!m_pFindOnWebMenu);
        m_pFindOnWebMenu = make_parented<WFindOnWebMenu>(this);
        connect(m_pFindOnWebMenu,
                &QMenu::aboutToShow,
                this,
                [this] {
                    m_pFindOnWebMenu->clear();
                    const auto pTrack = getFirstTrackPointer();
                    if (pTrack) {
                        mixxx::library::createFindOnWebSubmenus(
                                m_pFindOnWebMenu,
                                *pTrack);
                    }
                    m_pFindOnWebMenu->setEnabled(
                            !m_pFindOnWebMenu->isEmpty());
                });
    }

    if (featureIsEnabled(Feature::RemoveFromDisk)) {
        m_pRemoveFromDiskMenu = new QMenu(this);
        m_pRemoveFromDiskMenu->setTitle(tr("Delete Track Files"));
    }
}

void WTrackMenu::createActions() {
    const auto hideRemoveKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);

    if (featureIsEnabled(Feature::AutoDJ)) {
        m_pAutoDJBottomAct = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
        connect(m_pAutoDJBottomAct, &QAction::triggered, this, &WTrackMenu::slotAddToAutoDJBottom);

        m_pAutoDJTopAct = new QAction(tr("Add to Auto DJ Queue (top)"), this);
        connect(m_pAutoDJTopAct, &QAction::triggered, this, &WTrackMenu::slotAddToAutoDJTop);

        m_pAutoDJReplaceAct = new QAction(tr("Add to Auto DJ Queue (replace)"), this);
        connect(m_pAutoDJReplaceAct, &QAction::triggered, this, &WTrackMenu::slotAddToAutoDJReplace);
    }

    if (featureIsEnabled(Feature::LoadTo)) {
        m_pAddToPreviewDeck = new QAction(tr("Preview Deck"), m_pLoadToMenu);
        // currently there is only one preview deck so just map it here.
        QString previewDeckGroup = PlayerManager::groupForPreviewDeck(0);
        connect(m_pAddToPreviewDeck, &QAction::triggered, this, [this, previewDeckGroup] { loadSelectionToGroup(previewDeckGroup); });
    }

    if (featureIsEnabled(Feature::Remove)) {
        // Keyboard shortcuts are set here just to have them displayed in the menu.
        // Actual keypress is handled in WTrackTableView::keyPressEvent().
        m_pRemoveAct = new QAction(tr("Remove"), this);
        m_pRemoveAct->setShortcut(hideRemoveKeySequence);
        connect(m_pRemoveAct, &QAction::triggered, this, &WTrackMenu::slotRemove);

        m_pRemovePlaylistAct = new QAction(tr("Remove from Playlist"), this);
        m_pRemovePlaylistAct->setShortcut(hideRemoveKeySequence);
        connect(m_pRemovePlaylistAct, &QAction::triggered, this, &WTrackMenu::slotRemove);

        m_pRemoveCrateAct = new QAction(tr("Remove from Crate"), this);
        m_pRemoveCrateAct->setShortcut(hideRemoveKeySequence);
        connect(m_pRemoveCrateAct, &QAction::triggered, this, &WTrackMenu::slotRemove);
    }

    if (featureIsEnabled(Feature::HideUnhidePurge)) {
        m_pHideAct = new QAction(tr("Hide from Library"), this);
        // This is just for having the shortcut displayed next to the action in the menu.
        // The actual keypress is handled in WTrackTableView::keyPressEvent().
        m_pHideAct->setShortcut(hideRemoveKeySequence);
        connect(m_pHideAct, &QAction::triggered, this, &WTrackMenu::slotHide);

        m_pUnhideAct = new QAction(tr("Unhide from Library"), this);
        connect(m_pUnhideAct, &QAction::triggered, this, &WTrackMenu::slotUnhide);

        m_pPurgeAct = new QAction(tr("Purge from Library"), this);
        connect(m_pPurgeAct, &QAction::triggered, this, &WTrackMenu::slotPurge);
    }

    if (featureIsEnabled(Feature::RemoveFromDisk)) {
        m_pRemoveFromDiskAct = new QAction(tr("Delete Files from Disk"), m_pRemoveFromDiskMenu);
        connect(m_pRemoveFromDiskAct,
                &QAction::triggered,
                this,
                &WTrackMenu::slotRemoveFromDisk);
    }

    if (featureIsEnabled(Feature::Properties)) {
        m_pPropertiesAct = new QAction(tr("Properties"), this);
        // This is just for having the shortcut displayed next to the action
        // when the menu is invoked from the tracks table.
        // The keypress is caught in WTrackTableView::keyPressEvent
        if (m_pTrackModel) {
            m_pPropertiesAct->setShortcut(
                    // TODO(XXX): Qt6 replace enum | with QKeyCombination
                    QKeySequence(
                            static_cast<int>(kPropertiesShortcutModifier) |
                            kPropertiesShortcutKey));
        }
        connect(m_pPropertiesAct, &QAction::triggered, this, &WTrackMenu::slotShowDlgTrackInfo);
    }

    if (featureIsEnabled(Feature::FileBrowser)) {
        m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
        connect(m_pFileBrowserAct, &QAction::triggered, this, &WTrackMenu::slotOpenInFileBrowser);
    }

    if (featureIsEnabled(Feature::SelectInLibrary)) {
        m_pSelectInLibraryAct = new QAction(tr("Select in Library"), this);
        connect(m_pSelectInLibraryAct, &QAction::triggered, this, &WTrackMenu::slotSelectInLibrary);
    }

    if (featureIsEnabled(Feature::Metadata)) {
        m_pImportMetadataFromFileAct =
                new QAction(tr("Import From File Tags"), m_pMetadataMenu);
        connect(m_pImportMetadataFromFileAct,
                &QAction::triggered,
                this,
                &WTrackMenu::slotImportMetadataFromFileTags);

        m_pImportMetadataFromMusicBrainzAct =
                new QAction(tr("Import From MusicBrainz"), m_pMetadataMenu);
        connect(m_pImportMetadataFromMusicBrainzAct,
                &QAction::triggered,
                this,
                &WTrackMenu::slotShowDlgTagFetcher);

        m_pExportMetadataAct =
                new QAction(tr("Export To File Tags"), m_pMetadataMenu);
        connect(m_pExportMetadataAct,
                &QAction::triggered,
                this,
                &WTrackMenu::slotExportMetadataIntoFileTags);

        m_updateInExternalTrackCollections.reserve(
                m_pLibrary->trackCollectionManager()->externalCollections().size());
        for (auto* const pExternalTrackCollection :
                m_pLibrary->trackCollectionManager()->externalCollections()) {
            UpdateExternalTrackCollection updateInExternalTrackCollection;
            updateInExternalTrackCollection.externalTrackCollection = pExternalTrackCollection;
            updateInExternalTrackCollection.action = new QAction(
                    pExternalTrackCollection->name(), m_pMetadataMenu);
            updateInExternalTrackCollection.action->setToolTip(
                    pExternalTrackCollection->description());
            m_updateInExternalTrackCollections += updateInExternalTrackCollection;
            connect(updateInExternalTrackCollection.action,
                    &QAction::triggered,
                    this,
                    [this, pExternalTrackCollection] {
                        slotUpdateExternalTrackCollection(pExternalTrackCollection);
                    });
        }
    }

    if (featureIsEnabled(Feature::Reset)) {
        // Clear metadata actions
        m_pClearBeatsAction = new QAction(tr("BPM and Beatgrid"), m_pClearMetadataMenu);
        connect(m_pClearBeatsAction, &QAction::triggered, this, &WTrackMenu::slotClearBeats);

        m_pClearPlayCountAction = new QAction(tr("Play Count"), m_pClearMetadataMenu);
        connect(m_pClearPlayCountAction, &QAction::triggered, this, &WTrackMenu::slotClearPlayCount);

        m_pClearRatingAction = new QAction(tr("Rating"), m_pClearMetadataMenu);
        connect(m_pClearRatingAction, &QAction::triggered, this, &WTrackMenu::slotClearRating);

        m_pClearMainCueAction = new QAction(tr("Cue Point"), m_pClearMetadataMenu);
        connect(m_pClearMainCueAction, &QAction::triggered, this, &WTrackMenu::slotResetMainCue);

        m_pClearHotCuesAction = new QAction(tr("Hotcues"), m_pClearMetadataMenu);
        connect(m_pClearHotCuesAction, &QAction::triggered, this, &WTrackMenu::slotClearHotCues);

        m_pClearIntroCueAction = new QAction(tr("Intro"), m_pClearMetadataMenu);
        connect(m_pClearIntroCueAction, &QAction::triggered, this, &WTrackMenu::slotResetIntroCue);

        m_pClearOutroCueAction = new QAction(tr("Outro"), m_pClearMetadataMenu);
        connect(m_pClearOutroCueAction, &QAction::triggered, this, &WTrackMenu::slotResetOutroCue);

        m_pClearLoopsAction = new QAction(tr("Loops"), m_pClearMetadataMenu);
        connect(m_pClearLoopsAction, &QAction::triggered, this, &WTrackMenu::slotClearLoops);

        m_pClearKeyAction = new QAction(tr("Key"), m_pClearMetadataMenu);
        connect(m_pClearKeyAction, &QAction::triggered, this, &WTrackMenu::slotClearKey);

        m_pClearReplayGainAction = new QAction(tr("ReplayGain"), m_pClearMetadataMenu);
        connect(m_pClearReplayGainAction, &QAction::triggered, this, &WTrackMenu::slotClearReplayGain);

        m_pClearWaveformAction = new QAction(tr("Waveform"), m_pClearMetadataMenu);
        connect(m_pClearWaveformAction, &QAction::triggered, this, &WTrackMenu::slotClearWaveform);

        m_pClearCommentAction = new QAction(tr("Comment"), m_pClearMetadataMenu);
        connect(m_pClearCommentAction, &QAction::triggered, this, &WTrackMenu::slotClearComment);

        m_pClearAllMetadataAction = new QAction(tr("All"), m_pClearMetadataMenu);
        connect(m_pClearAllMetadataAction, &QAction::triggered, this, &WTrackMenu::slotClearAllMetadata);
    }

    if (featureIsEnabled(Feature::BPM)) {
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

        connect(m_pBpmDoubleAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::BpmScale::Double);
        });
        connect(m_pBpmHalveAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::BpmScale::Halve);
        });
        connect(m_pBpmTwoThirdsAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::BpmScale::TwoThirds);
        });
        connect(m_pBpmThreeFourthsAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::BpmScale::ThreeFourths);
        });
        connect(m_pBpmFourThirdsAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::BpmScale::FourThirds);
        });
        connect(m_pBpmThreeHalvesAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::BpmScale::ThreeHalves);
        });

        m_pBpmResetAction = new QAction(tr("Reset BPM"), m_pBPMMenu);
        connect(m_pBpmResetAction,
                &QAction::triggered,
                this,
                &WTrackMenu::slotClearBeats);
    }

    if (featureIsEnabled(Feature::Analyze)) {
        m_pAnalyzeAction = new QAction(tr("Analyze"), this);
        connect(m_pAnalyzeAction, &QAction::triggered, this, &WTrackMenu::slotAnalyze);

        m_pReanalyzeAction = new QAction(tr("Reanalyze"), this);
        connect(m_pReanalyzeAction, &QAction::triggered, this, &WTrackMenu::slotReanalyze);

        m_pReanalyzeConstBpmAction = new QAction(tr("Reanalyze (constant BPM)"), this);
        connect(m_pReanalyzeConstBpmAction,
                &QAction::triggered,
                this,
                &WTrackMenu::slotReanalyzeWithFixedTempo);

        m_pReanalyzeVarBpmAction = new QAction(tr("Reanalyze (variable BPM)"), this);
        connect(m_pReanalyzeVarBpmAction,
                &QAction::triggered,
                this,
                &WTrackMenu::slotReanalyzeWithVariableTempo);
    }

    // This action is only usable when m_deckGroup is set. That is true only
    // for WTrackmenu instantiated by WTrackProperty and other deck widgets, thus
    // don't create it if a track model is set.
    if (!m_pTrackModel && featureIsEnabled(Feature::UpdateReplayGainFromPregain)) {
        m_pUpdateReplayGainAct =
                new QAction(tr("Update ReplayGain from Deck Gain"), m_pClearMetadataMenu);
        connect(m_pUpdateReplayGainAct,
                &QAction::triggered,
                this,
                &WTrackMenu::slotUpdateReplayGainFromPregain);
    }

    if (featureIsEnabled(Feature::Color)) {
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
    if (featureIsEnabled(Feature::SearchRelated)) {
        addMenu(m_pSearchRelatedMenu);
    }

    if (featureIsEnabled(Feature::SelectInLibrary)) {
        addAction(m_pSelectInLibraryAct);
    }

    if (featureIsEnabled(Feature::SearchRelated) ||
            featureIsEnabled(Feature::SelectInLibrary)) {
        addSeparator();
    }

    if (featureIsEnabled(Feature::AutoDJ)) {
        addAction(m_pAutoDJBottomAct);
        addAction(m_pAutoDJTopAct);
        addAction(m_pAutoDJReplaceAct);
        addSeparator();
    }

    if (featureIsEnabled(Feature::LoadTo)) {
        m_pLoadToMenu->addMenu(m_pDeckMenu);

        m_pLoadToMenu->addMenu(m_pSamplerMenu);

        if (m_pNumPreviewDecks->get() > 0.0) {
            m_pLoadToMenu->addAction(m_pAddToPreviewDeck);
        }

        addMenu(m_pLoadToMenu);
        addSeparator();
    }

    if (featureIsEnabled(Feature::Playlist)) {
        addMenu(m_pPlaylistMenu);
    }

    if (featureIsEnabled(Feature::Crate)) {
        addMenu(m_pCrateMenu);
    }

    if (featureIsEnabled(Feature::Remove)) {
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Remove)) {
            addAction(m_pRemoveAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::RemovePlaylist)) {
            addAction(m_pRemovePlaylistAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::RemoveCrate)) {
            addAction(m_pRemoveCrateAct);
        }
    }

    addSeparator();

    if (featureIsEnabled(Feature::BPM)) {
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
        m_pBPMMenu->addAction(m_pBpmResetAction);
        m_pBPMMenu->addSeparator();

        addMenu(m_pBPMMenu);
    }

    if (featureIsEnabled(Feature::Color)) {
        m_pColorMenu->addAction(m_pColorPickerAction);
        addMenu(m_pColorMenu);
    }

    if (featureIsEnabled(Feature::Metadata)) {
        m_pMetadataMenu->addAction(m_pImportMetadataFromFileAct);
        m_pMetadataMenu->addAction(m_pImportMetadataFromMusicBrainzAct);
        m_pMetadataMenu->addAction(m_pExportMetadataAct);

        for (const auto& updateInExternalTrackCollection :
                qAsConst(m_updateInExternalTrackCollections)) {
            m_pMetadataUpdateExternalCollectionsMenu->addAction(
                    updateInExternalTrackCollection.action);
        }
        if (!m_pMetadataUpdateExternalCollectionsMenu->isEmpty()) {
            m_pMetadataMenu->addMenu(m_pMetadataUpdateExternalCollectionsMenu);
            // Enable/disable entries depending on the connection status
            // that may change at runtime.
            connect(m_pMetadataUpdateExternalCollectionsMenu,
                    &QMenu::aboutToShow,
                    this,
                    [this] {
                        for (const auto& updateInExternalTrackCollection :
                                qAsConst(m_updateInExternalTrackCollections)) {
                            updateInExternalTrackCollection.action->setEnabled(
                                    updateInExternalTrackCollection
                                            .externalTrackCollection
                                            ->isConnected());
                        }
                    });
        }

        m_pMetadataMenu->addMenu(m_pCoverMenu);
        if (featureIsEnabled(Feature::FindOnWeb)) {
            m_pMetadataMenu->addMenu(m_pFindOnWebMenu);
        }
        addSeparator();
        addMenu(m_pMetadataMenu);
    }

    if (featureIsEnabled(Feature::Reset)) {
        m_pClearMetadataMenu->addAction(m_pClearBeatsAction);
        m_pClearMetadataMenu->addAction(m_pClearPlayCountAction);
        m_pClearMetadataMenu->addAction(m_pClearRatingAction);
        m_pClearMetadataMenu->addAction(m_pClearCommentAction);
        m_pClearMetadataMenu->addAction(m_pClearMainCueAction);
        m_pClearMetadataMenu->addAction(m_pClearHotCuesAction);
        m_pClearMetadataMenu->addAction(m_pClearIntroCueAction);
        m_pClearMetadataMenu->addAction(m_pClearOutroCueAction);
        m_pClearMetadataMenu->addAction(m_pClearLoopsAction);
        m_pClearMetadataMenu->addAction(m_pClearKeyAction);
        m_pClearMetadataMenu->addAction(m_pClearReplayGainAction);
        m_pClearMetadataMenu->addAction(m_pClearWaveformAction);
        m_pClearMetadataMenu->addSeparator();
        m_pClearMetadataMenu->addAction(m_pClearAllMetadataAction);
        addMenu(m_pClearMetadataMenu);
    }

    if (featureIsEnabled(Feature::Analyze)) {
        m_pAnalyzeMenu->addAction(m_pAnalyzeAction);
        m_pAnalyzeMenu->addAction(m_pReanalyzeAction);
        m_pAnalyzeMenu->addAction(m_pReanalyzeConstBpmAction);
        m_pAnalyzeMenu->addAction(m_pReanalyzeVarBpmAction);
        addMenu(m_pAnalyzeMenu);
    }

    // This action is created only for menus instantiated by deck widgets (e.g.
    // WTrackProperty) and if UpdateReplayGainFromPregain is supported.
    if (m_pUpdateReplayGainAct) {
        addAction(m_pUpdateReplayGainAct);
    }

    addSeparator();

    if (featureIsEnabled(Feature::HideUnhidePurge)) {
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Hide)) {
            addAction(m_pHideAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Unhide)) {
            addAction(m_pUnhideAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Purge)) {
            addAction(m_pPurgeAct);
        }
    }

    if (featureIsEnabled(Feature::RemoveFromDisk)) {
        m_pRemoveFromDiskMenu->addAction(m_pRemoveFromDiskAct);
        addMenu(m_pRemoveFromDiskMenu);
    }

    if (featureIsEnabled(Feature::FileBrowser)) {
        addAction(m_pFileBrowserAct);
    }

    if (featureIsEnabled(Feature::Properties)) {
        addSeparator();
        addAction(m_pPropertiesAct);
    }
}

bool WTrackMenu::isAnyTrackBpmLocked() const {
    if (m_pTrackModel) {
        const int column =
                m_pTrackModel->fieldIndex(LIBRARYTABLE_BPM_LOCK);
        for (const auto& trackIndex : m_trackIndexList) {
            QModelIndex bpmLockedIndex =
                    trackIndex.sibling(trackIndex.row(), column);
            if (bpmLockedIndex.data().toBool()) {
                return true;
            }
        }
    } else {
        if (m_pTrack && m_pTrack->isBpmLocked()) {
            return true;
        }
    }
    return false;
}

std::optional<std::optional<mixxx::RgbColor>> WTrackMenu::getCommonTrackColor() const {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return std::nullopt;
    }
    std::optional<mixxx::RgbColor> commonColor;
    if (m_pTrackModel) {
        const int column =
                m_pTrackModel->fieldIndex(LIBRARYTABLE_COLOR);
        commonColor = mixxx::RgbColor::fromQVariant(
                m_trackIndexList.first().sibling(m_trackIndexList.first().row(), column).data());
        for (const auto& trackIndex : m_trackIndexList) {
            const auto otherColor = mixxx::RgbColor::fromQVariant(
                    trackIndex.sibling(trackIndex.row(), column).data());
            if (commonColor != otherColor) {
                // Multiple, different colors
                return std::nullopt;
            }
        }
    } else {
        if (!m_pTrack) {
            return std::nullopt;
        }
        commonColor = m_pTrack->getColor();
    }
    return make_optional(commonColor);
}

CoverInfo WTrackMenu::getCoverInfoOfLastTrack() const {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return CoverInfo();
    }
    if (m_pTrackModel) {
        const QModelIndex lastIndex = m_trackIndexList.last();
        CoverInfo coverInfo;
        coverInfo.source = static_cast<CoverInfo::Source>(
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_SOURCE))
                        .data()
                        .toInt());
        coverInfo.type = static_cast<CoverInfo::Type>(
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_TYPE))
                        .data()
                        .toInt());
        coverInfo.color = mixxx::RgbColor::fromQVariant(
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_COLOR))
                        .data());
        const auto imageDigest =
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_DIGEST))
                        .data()
                        .toByteArray();
        const auto legacyHash =
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_HASH))
                        .data()
                        .toUInt();
        coverInfo.setImageDigest(imageDigest, legacyHash);
        coverInfo.coverLocation =
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_LOCATION))
                        .data()
                        .toString();
        coverInfo.trackLocation =
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(TRACKLOCATIONSTABLE_LOCATION))
                        .data()
                        .toString();
        return coverInfo;
    } else {
        return m_pTrack->getCoverInfoWithLocation();
    }
}

void WTrackMenu::updateMenus() {
    if (isEmpty()) {
        return;
    }

    // Gray out some stuff if multiple songs were selected.
    const bool singleTrackSelected = getTrackCount() == 1;

    if (featureIsEnabled(Feature::LoadTo)) {
        int iNumDecks = static_cast<int>(m_pNumDecks->get());
        m_pDeckMenu->clear();
        if (iNumDecks > 0) {
            for (int i = 1; i <= iNumDecks; ++i) {
                // PlayerManager::groupForDeck is 0-indexed.
                QString deckGroup = PlayerManager::groupForDeck(i - 1);
                bool deckPlaying = ControlObject::get(
                                           ConfigKey(deckGroup, "play")) > 0.0;
                bool allowLoadTrackIntoPlayingDeck = false;
                if (m_pConfig->exists(kConfigKeyLoadWhenDeckPlaying)) {
                    int loadWhenDeckPlaying =
                            m_pConfig->getValueString(kConfigKeyLoadWhenDeckPlaying).toInt();
                    switch (static_cast<LoadWhenDeckPlaying>(loadWhenDeckPlaying)) {
                    case LoadWhenDeckPlaying::Allow:
                    case LoadWhenDeckPlaying::AllowButStopDeck:
                        allowLoadTrackIntoPlayingDeck = true;
                        break;
                    case LoadWhenDeckPlaying::Reject:
                        break;
                    }
                } else {
                    // support older version of this flag
                    allowLoadTrackIntoPlayingDeck = m_pConfig->getValue<bool>(
                            ConfigKey("[Controls]", "AllowTrackLoadToPlayingDeck"));
                }
                bool deckEnabled =
                        (!deckPlaying || allowLoadTrackIntoPlayingDeck) &&
                        singleTrackSelected;
                QAction* pAction = new QAction(tr("Deck %1").arg(i), this);
                pAction->setEnabled(deckEnabled);
                m_pDeckMenu->addAction(pAction);
                connect(pAction, &QAction::triggered, this, [this, deckGroup] { loadSelectionToGroup(deckGroup); });
            }
        }

        int iNumSamplers = static_cast<int>(m_pNumSamplers->get());
        if (iNumSamplers > 0) {
            m_pSamplerMenu->clear();
            for (int i = 1; i <= iNumSamplers; ++i) {
                // PlayerManager::groupForSampler is 0-indexed.
                QString samplerGroup = PlayerManager::groupForSampler(i - 1);
                bool samplerPlaying = ControlObject::get(
                                              ConfigKey(samplerGroup, "play")) > 0.0;
                bool samplerEnabled = !samplerPlaying && singleTrackSelected;
                QAction* pAction = new QAction(tr("Sampler %1").arg(i), m_pSamplerMenu);
                pAction->setEnabled(samplerEnabled);
                m_pSamplerMenu->addAction(pAction);
                connect(pAction, &QAction::triggered, this, [this, samplerGroup] { loadSelectionToGroup(samplerGroup); });
            }
        }
    }

    if (featureIsEnabled(Feature::Playlist)) {
        // Playlist menu is lazy loaded on hover by slotPopulatePlaylistMenu
        // to avoid unnecessary database queries
        m_bPlaylistMenuLoaded = false;
    }

    if (featureIsEnabled(Feature::Crate)) {
        // Crate menu is lazy loaded on hover by slotPopulateCrateMenu
        // to avoid unnecessary database queries
        m_bCrateMenuLoaded = false;
    }

    if (featureIsEnabled(Feature::Remove)) {
        bool locked = m_pTrackModel->hasCapabilities(TrackModel::Capability::Locked);
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Remove)) {
            m_pRemoveAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::RemovePlaylist)) {
            m_pRemovePlaylistAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::RemoveCrate)) {
            m_pRemoveCrateAct->setEnabled(!locked);
        }
    }

    if (featureIsEnabled(Feature::Metadata)) {
        m_pImportMetadataFromMusicBrainzAct->setEnabled(singleTrackSelected);

        // We use the last selected track for the cover art context to be
        // consistent with selectionChanged above.
        m_pCoverMenu->setCoverArt(getCoverInfoOfLastTrack());
        m_pMetadataMenu->addMenu(m_pCoverMenu);
    }

    if (featureIsEnabled(Feature::Analyze)) {
        bool useFixedTempo = m_pConfig->getValue<bool>(
                ConfigKey("[BPM]", "BeatDetectionFixedTempoAssumption"));
        // Since we already have a 'Reanalyze' action that uses the configured
        // default, we hide the redundant menu as per suggestion:
        // https://github.com/mixxxdj/mixxx/pull/10931#issuecomment-1262559750
        m_pReanalyzeConstBpmAction->setVisible(!useFixedTempo);
        m_pReanalyzeVarBpmAction->setVisible(useFixedTempo);
    }

    if (featureIsEnabled(Feature::Reset) ||
            featureIsEnabled(Feature::BPM)) {
        const bool anyBpmLocked = isAnyTrackBpmLocked();
        if (featureIsEnabled(Feature::Reset)) {
            m_pClearBeatsAction->setEnabled(!anyBpmLocked);
        }
        if (featureIsEnabled(Feature::BPM)) {
            m_pBpmUnlockAction->setEnabled(anyBpmLocked);
            m_pBpmLockAction->setEnabled(!anyBpmLocked);
            m_pBpmDoubleAction->setEnabled(!anyBpmLocked);
            m_pBpmHalveAction->setEnabled(!anyBpmLocked);
            m_pBpmTwoThirdsAction->setEnabled(!anyBpmLocked);
            m_pBpmThreeFourthsAction->setEnabled(!anyBpmLocked);
            m_pBpmFourThirdsAction->setEnabled(!anyBpmLocked);
            m_pBpmThreeHalvesAction->setEnabled(!anyBpmLocked);
            m_pBpmResetAction->setEnabled(!anyBpmLocked);
        }
    }

    // This action is created only for menus instantiated by deck widgets (e.g.
    // WTrackProperty) and if UpdateReplayGainFromPregain is supported.
    // Disable it if no deck group was set.
    if (m_pUpdateReplayGainAct) {
        m_pUpdateReplayGainAct->setEnabled(!m_deckGroup.isEmpty());
    }

    if (featureIsEnabled(Feature::Color)) {
        m_pColorPickerAction->setColorPalette(
                ColorPaletteSettings(m_pConfig).getTrackColorPalette());

        // Resize Menu to fit changed palette
        QResizeEvent resizeEvent(QSize(), m_pColorMenu->size());
        qApp->sendEvent(m_pColorMenu, &resizeEvent);

        const auto commonColor = getCommonTrackColor();
        if (commonColor) {
            m_pColorPickerAction->setSelectedColor(*commonColor);
        } else {
            m_pColorPickerAction->resetSelectedColor();
        }
    }

    if (featureIsEnabled(Feature::HideUnhidePurge)) {
        bool locked = m_pTrackModel->hasCapabilities(TrackModel::Capability::Locked);
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Hide)) {
            m_pHideAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Unhide)) {
            m_pUnhideAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::Capability::Purge)) {
            m_pPurgeAct->setEnabled(!locked);
        }
    }

    if (featureIsEnabled(Feature::RemoveFromDisk)) {
        if (m_pTrackModel) {
            bool locked = m_pTrackModel->hasCapabilities(TrackModel::Capability::Locked);
            if (m_pTrackModel->hasCapabilities(TrackModel::Capability::RemoveFromDisk)) {
                m_pRemoveFromDiskAct->setEnabled(!locked);
            }
        }
    }

    if (featureIsEnabled(Feature::SelectInLibrary)) {
        bool enabled = false;
        if (m_pTrack) {
            enabled = m_pLibrary->isTrackIdInCurrentLibraryView(m_pTrack->getId());
        }
        m_pSelectInLibraryAct->setEnabled(enabled);
    }

    if (featureIsEnabled(Feature::Properties)) {
        m_pPropertiesAct->setEnabled(singleTrackSelected);
    }

    if (featureIsEnabled(Feature::FindOnWeb)) {
        const auto pTrack = getFirstTrackPointer();
        const bool enableMenu = pTrack ? WFindOnWebMenu::hasEntriesForTrack(*pTrack) : false;
        m_pFindOnWebMenu->setEnabled(enableMenu);
    }
}

void WTrackMenu::loadTrack(
        const TrackPointer& pTrack, const QString& deckGroup) {
    // This asserts that this function is only accessible when a track model is not set,
    // thus maintaining only the TrackPointerList in state and avoiding storing
    // duplicate state with TrackIdList and QModelIndexList.
    VERIFY_OR_DEBUG_ASSERT(!m_pTrackModel) {
        return;
    }

    // Clean all forms of track store
    clearTrackSelection();

    if (!pTrack) {
        return;
    }
    m_pTrack = pTrack;
    m_deckGroup = deckGroup;
    updateMenus();
}

void WTrackMenu::loadTrackModelIndices(
        const QModelIndexList& trackIndexList) {
    // This asserts that this function is only accessible when a track model is set,
    // thus maintaining only the QModelIndexList in state and avoiding storing
    // duplicate state with TrackIdList and TrackPointerList.
    VERIFY_OR_DEBUG_ASSERT(m_pTrackModel) {
        return;
    }

    // Clean all forms of track store
    clearTrackSelection();

    m_trackIndexList = trackIndexList;
    updateMenus();
}

TrackIdList WTrackMenu::getTrackIds() const {
    TrackIdList trackIds;
    if (m_pTrackModel) {
        trackIds.reserve(m_trackIndexList.size());
        for (const auto& index : m_trackIndexList) {
            const auto trackId = m_pTrackModel->getTrackId(index);
            if (!trackId.isValid()) {
                // Skip unavailable tracks
                continue;
            }
            trackIds.push_back(trackId);
        }
    } else {
        if (m_pTrack) {
            const auto trackId = m_pTrack->getId();
            DEBUG_ASSERT(trackId.isValid());
            trackIds.push_back(trackId);
        }
    }
    return trackIds;
}

QList<TrackRef> WTrackMenu::getTrackRefs() const {
    QList<TrackRef> trackRefs;
    if (m_pTrackModel) {
        trackRefs.reserve(m_trackIndexList.size());
        for (const auto& index : m_trackIndexList) {
            auto trackRef = TrackRef::fromFilePath(
                    m_pTrackModel->getTrackLocation(index),
                    m_pTrackModel->getTrackId(index));
            if (!trackRef.isValid()) {
                // Skip unavailable tracks
                continue;
            }
            trackRefs.push_back(std::move(trackRef));
        }
    } else if (m_pTrack) {
        auto trackRef = TrackRef::fromFileInfo(
                m_pTrack->getFileInfo(),
                m_pTrack->getId());
        trackRefs.push_back(std::move(trackRef));
    }
    return trackRefs;
}

TrackPointer WTrackMenu::getFirstTrackPointer() const {
    if (m_pTrackModel) {
        for (const auto& index : m_trackIndexList) {
            const auto pTrack = m_pTrackModel->getTrack(index);
            if (pTrack) {
                return pTrack;
            }
            // Skip unavailable tracks
        }
        return TrackPointer();
    }
    return m_pTrack;
}

std::unique_ptr<mixxx::TrackPointerIterator> WTrackMenu::newTrackPointerIterator() const {
    if (m_pTrackModel) {
        if (m_trackIndexList.isEmpty()) {
            return nullptr;
        }
        // m_pTrackModel must not be modified during the iteration,
        // neither directly nor indirectly through signals!!!
        return std::make_unique<mixxx::TrackPointerModelIterator>(
                m_pTrackModel,
                m_trackIndexList);
    } else if (m_pTrack) {
        return std::make_unique<mixxx::TrackPointerListIterator>(
                TrackPointerList{m_pTrack});
    }
    return nullptr;
}

int WTrackMenu::applyTrackPointerOperation(
        const QString& progressLabelText,
        const mixxx::TrackPointerOperation* pTrackPointerOperation,
        mixxx::ModalTrackBatchOperationProcessor::Mode operationMode) const {
    const auto pTrackPointerIter = newTrackPointerIterator();
    if (!pTrackPointerIter) {
        // Empty, i.e. nothing to do
        return 0;
    }
    mixxx::ModalTrackBatchOperationProcessor modalOperation(
            pTrackPointerOperation,
            operationMode);
    return modalOperation.processTracks(
            progressLabelText,
            m_pLibrary->trackCollectionManager(),
            pTrackPointerIter.get());
}

const QModelIndexList& WTrackMenu::getTrackIndices() const {
    // Indices are associated with a TrackModel. Can only be obtained
    // if a TrackModel is available.
    DEBUG_ASSERT(m_pTrackModel);
    return m_trackIndexList;
}

void WTrackMenu::slotOpenInFileBrowser() {
    const auto trackRefs = getTrackRefs();
    QStringList locations;
    locations.reserve(trackRefs.size());
    for (const auto& trackRef : trackRefs) {
        locations << trackRef.getLocation();
    }
    mixxx::DesktopHelper::openInFileBrowser(locations);
}

void WTrackMenu::slotSelectInLibrary() {
    if (m_pTrack) {
        emit m_pLibrary->selectTrack(m_pTrack->getId());
    }
}

namespace {

class ImportMetadataFromFileTagsTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ImportMetadataFromFileTagsTrackPointerOperation(
            const UserSettings& userSettings)
            : m_params(SyncTrackMetadataParams::readFromUserSettings(userSettings)) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        // The user has explicitly requested to reload metadata from the file
        // to override the information within Mixxx! Custom cover art must be
        // reloaded separately.
        SoundSourceProxy(pTrack).updateTrackFromSource(
                SoundSourceProxy::UpdateTrackFromSourceMode::Always,
                m_params);
    }

    const SyncTrackMetadataParams m_params;
};

} // anonymous namespace

void WTrackMenu::slotUpdateReplayGainFromPregain() {
    VERIFY_OR_DEBUG_ASSERT(m_pTrack) {
        return;
    }
    VERIFY_OR_DEBUG_ASSERT(!m_deckGroup.isEmpty()) {
        return;
    }

    const double gain = ControlObject::get(ConfigKey(m_deckGroup, "pregain"));
    // Gain is at unity already, ignore and return.
    if (gain == 1.0) {
        return;
    }
    m_pTrack->adjustReplayGainFromPregain(gain);
}

void WTrackMenu::slotImportMetadataFromFileTags() {
    const auto progressLabelText =
            tr("Importing metadata of %n track(s) from file tags", "", getTrackCount());
    const auto trackOperator =
            ImportMetadataFromFileTagsTrackPointerOperation(*m_pConfig);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator,
            // Update the database to reflect the recent changes. This is
            // crucial for additional metadata like custom tags that are
            // directly fetched from the database for certain use cases!
            mixxx::ModalTrackBatchOperationProcessor::Mode::ApplyAndSave);
}

namespace {

class ExportMetadataIntoFileTagsTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->markForMetadataExport();
    }
};

} // anonymous namespace

void WTrackMenu::slotExportMetadataIntoFileTags() {
    // Export of metadata is deferred until all references to the
    // corresponding track object have been dropped. Otherwise
    // writing to files that are still used for playback might
    // cause crashes or at least audible glitches!
    mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();

    const auto progressLabelText =
            tr("Marking metadata of %n track(s) to be exported into file tags",
                    "",
                    getTrackCount());
    const auto trackOperator =
            ExportMetadataIntoFileTagsTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotUpdateExternalTrackCollection(
        ExternalTrackCollection* externalTrackCollection) {
    VERIFY_OR_DEBUG_ASSERT(externalTrackCollection) {
        return;
    }

    externalTrackCollection->updateTracks(getTrackRefs());
}

void WTrackMenu::slotPopulatePlaylistMenu() {
    // The user may open the Playlist submenu, move their cursor away, then
    // return to the Playlist submenu before exiting the track context menu.
    // Avoid querying the database multiple times in that case.
    if (m_bPlaylistMenuLoaded) {
        return;
    }
    m_pPlaylistMenu->clear();
    const PlaylistDAO& playlistDao = m_pLibrary->trackCollectionManager()
                                             ->internalCollection()
                                             ->getPlaylistDAO();
    QList<QPair<int, QString>> playlists =
            playlistDao.getPlaylists(PlaylistDAO::PLHT_NOT_HIDDEN);

    for (const auto& [id, name] : playlists) {
        // No leak because making the menu the parent means they will be
        // auto-deleted
        int plId = id;
        auto* pAction = new QAction(
                mixxx::escapeTextPropertyWithoutShortcuts(name),
                m_pPlaylistMenu);
        bool locked = playlistDao.isPlaylistLocked(plId);
        pAction->setEnabled(!locked);
        m_pPlaylistMenu->addAction(pAction);
        connect(pAction,
                &QAction::triggered,
                this,
                [this, plId] {
                    addSelectionToPlaylist(plId);
                });
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

    PlaylistDAO& playlistDao = m_pLibrary->trackCollectionManager()
                                       ->internalCollection()
                                       ->getPlaylistDAO();

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
    m_pLibrary->trackCollectionManager()->unhideTracks(trackIds);
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

    CrateSummarySelectResult allCrates(
            m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->crates()
                    .selectCratesWithTrackCount(trackIds));

    CrateSummary crate;
    while (allCrates.populateNext(&crate)) {
        auto pAction = make_parented<QWidgetAction>(
                m_pCrateMenu);
        auto pCheckBox = make_parented<QCheckBox>(
                mixxx::escapeTextPropertyWithoutShortcuts(crate.getName()),
                m_pCrateMenu);
        pCheckBox->setProperty("crateId", QVariant::fromValue(crate.getId()));
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
    auto* pCheckBox = qobject_cast<QCheckBox*>(pWidget);
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
            m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->removeCrateTracks(crateId, trackIds);
        }
    } else {
        if (!crateId.isValid()) { // i.e. a new crate is suppose to be created
            crateId = CrateFeatureHelper(
                    m_pLibrary->trackCollectionManager()->internalCollection(), m_pConfig)
                              .createEmptyCrate();
        }
        if (crateId.isValid()) {
            m_pLibrary->trackCollectionManager()->unhideTracks(trackIds);
            m_pLibrary->trackCollectionManager()
                    ->internalCollection()
                    ->addCrateTracks(crateId, trackIds);
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
            m_pLibrary->trackCollectionManager()->internalCollection(), m_pConfig)
                              .createEmptyCrate();

    if (crateId.isValid()) {
        m_pLibrary->trackCollectionManager()->unhideTracks(trackIds);
        m_pLibrary->trackCollectionManager()
                ->internalCollection()
                ->addCrateTracks(crateId, trackIds);
    }
}

void WTrackMenu::addToAnalysis(AnalyzerTrack::Options options) {
    const TrackIdList trackIds = getTrackIds();
    if (trackIds.empty()) {
        qWarning() << "No tracks selected for analysis";
        return;
    }

    QList<AnalyzerScheduledTrack> tracks;
    for (auto trackId : trackIds) {
        AnalyzerScheduledTrack track(trackId, options);
        tracks.append(track);
    }

    emit m_pLibrary->analyzeTracks(tracks);
}

void WTrackMenu::slotAnalyze() {
    addToAnalysis();
}

void WTrackMenu::slotReanalyze() {
    clearBeats();
    addToAnalysis();
}

void WTrackMenu::slotReanalyzeWithFixedTempo() {
    clearBeats();
    AnalyzerTrack::Options options;
    options.useFixedTempo = true;
    addToAnalysis(options);
}

void WTrackMenu::slotReanalyzeWithVariableTempo() {
    clearBeats();
    AnalyzerTrack::Options options;
    options.useFixedTempo = false;
    addToAnalysis(options);
}

void WTrackMenu::slotLockBpm() {
    lockBpm(true);
}

void WTrackMenu::slotUnlockBpm() {
    lockBpm(false);
}

namespace {

class ScaleBpmTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ScaleBpmTrackPointerOperation(mixxx::Beats::BpmScale bpmScale)
            : m_bpmScale(bpmScale) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        if (pTrack->isBpmLocked()) {
            return;
        }
        const mixxx::BeatsPointer pBeats = pTrack->getBeats();
        if (!pBeats) {
            return;
        }
        const auto scaledBeats = pBeats->tryScale(m_bpmScale);
        if (!scaledBeats) {
            return;
        }
        pTrack->trySetBeats(*scaledBeats);
    }

    const mixxx::Beats::BpmScale m_bpmScale;
};

} // anonymous namespace

void WTrackMenu::slotScaleBpm(mixxx::Beats::BpmScale scale) {
    const auto progressLabelText =
            tr("Scaling BPM of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ScaleBpmTrackPointerOperation(scale);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class LockBpmTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit LockBpmTrackPointerOperation(bool lock)
            : m_lock(lock) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->setBpmLocked(m_lock);
    }

    const bool m_lock;
};

} // anonymous namespace

void WTrackMenu::lockBpm(bool lock) {
    const auto progressLabelText = lock
            ? tr("Locking BPM of %n track(s)", "", getTrackCount())
            : tr("Unlocking BPM of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            LockBpmTrackPointerOperation(lock);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class SetColorTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit SetColorTrackPointerOperation(const mixxx::RgbColor::optional_t& color)
            : m_color(color) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->setColor(m_color);
    }

    const mixxx::RgbColor::optional_t m_color;
};

} // anonymous namespace

void WTrackMenu::slotColorPicked(const mixxx::RgbColor::optional_t& color) {
    const auto progressLabelText =
            tr("Setting color of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            SetColorTrackPointerOperation(color);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);

    hide();
}

void WTrackMenu::loadSelectionToGroup(const QString& group, bool play) {
    TrackPointer pTrack = getFirstTrackPointer();
    if (!pTrack) {
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

    // TODO: load track from this class without depending on
    // external slot to load track
    emit loadTrackToPlayer(pTrack, group, play);
}

namespace {

class ResetPlayCounterTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->resetPlayCounter();
    }
};

} // anonymous namespace

//slot for reset played count, sets count to 0 of one or more tracks
void WTrackMenu::slotClearPlayCount() {
    const auto progressLabelText =
            tr("Resetting play count of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetPlayCounterTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ResetBeatsTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->trySetBeats(mixxx::BeatsPointer());
    }
};

} // anonymous namespace

void WTrackMenu::clearBeats() {
    const auto progressLabelText =
            tr("Resetting beats of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetBeatsTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotClearBeats() {
    clearBeats();
}

namespace {

class ResetRatingTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->resetRating();
    }
};

} // anonymous namespace

//slot for reset played count, sets count to 0 of one or more tracks
void WTrackMenu::slotClearRating() {
    const auto progressLabelText =
            tr("Clearing rating of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetRatingTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ClearCommentTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->clearComment();
    }
};

} // anonymous namespace

//slot for clearing the comment field of one or more tracks
void WTrackMenu::slotClearComment() {
    const auto progressLabelText =
            tr("Clearing comment of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ClearCommentTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class RemoveCuesOfTypeTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit RemoveCuesOfTypeTrackPointerOperation(mixxx::CueType cueType)
            : m_cueType(cueType) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->removeCuesOfType(m_cueType);
    }

    const mixxx::CueType m_cueType;
};

class ResetMainCueTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ResetMainCueTrackPointerOperation(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->removeCuesOfType(mixxx::CueType::MainCue);
        CuePointer pN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);
        if (pN60dBSound) {
            mixxx::audio::FramePos firstSound = pN60dBSound->getPosition();
            if (firstSound.isValid()) {
                AnalyzerSilence::setupMainAndIntroCue(pTrack.get(), firstSound, m_pConfig.data());
            }
        }
    }

    UserSettingsPointer m_pConfig;
};

class ResetIntroTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ResetIntroTrackPointerOperation(UserSettingsPointer pConfig)
            : m_pConfig(pConfig) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->removeCuesOfType(mixxx::CueType::Intro);
        CuePointer pN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);
        if (pN60dBSound) {
            mixxx::audio::FramePos firstSound = pN60dBSound->getPosition();
            if (firstSound.isValid()) {
                AnalyzerSilence::setupMainAndIntroCue(pTrack.get(), firstSound, m_pConfig.data());
            }
        }
    }

    UserSettingsPointer m_pConfig;
};

class ResetOutroTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ResetOutroTrackPointerOperation() {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->removeCuesOfType(mixxx::CueType::Outro);
        CuePointer pN60dBSound = pTrack->findCueByType(mixxx::CueType::N60dBSound);
        if (pN60dBSound) {
            mixxx::audio::FramePos lastSound = pN60dBSound->getEndPosition();
            if (lastSound.isValid()) {
                AnalyzerSilence::setupOutroCue(pTrack.get(), lastSound);
            }
        }
    }
};

} // anonymous namespace

void WTrackMenu::slotResetMainCue() {
    const auto progressLabelText =
            tr("Removing main cue from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetMainCueTrackPointerOperation(m_pConfig);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotResetOutroCue() {
    const auto progressLabelText =
            tr("Removing outro cue from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetOutroTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotResetIntroCue() {
    const auto progressLabelText =
            tr("Removing intro cue from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetIntroTrackPointerOperation(m_pConfig);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotClearLoops() {
    const auto progressLabelText =
            tr("Removing loop cues from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            RemoveCuesOfTypeTrackPointerOperation(mixxx::CueType::Loop);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotClearHotCues() {
    const auto progressLabelText =
            tr("Removing hot cues from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            RemoveCuesOfTypeTrackPointerOperation(mixxx::CueType::HotCue);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ResetKeysTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->resetKeys();
    }
};

} // anonymous namespace

void WTrackMenu::slotClearKey() {
    const auto progressLabelText =
            tr("Resetting keys of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetKeysTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ResetReplayGainTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->setReplayGain(mixxx::ReplayGain());
    }
};

} // anonymous namespace

void WTrackMenu::slotClearReplayGain() {
    const auto progressLabelText =
            tr("Resetting replay gain of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetReplayGainTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ResetWaveformTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ResetWaveformTrackPointerOperation(AnalysisDao& analysisDao)
            : m_analysisDao(analysisDao) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        m_analysisDao.deleteAnalysesForTrack(pTrack->getId());
        pTrack->setWaveform(WaveformPointer());
        pTrack->setWaveformSummary(WaveformPointer());
        // We Remove the invisible AudibleSound cue here as well, because the
        // same reasons that apply for reanalyze of the waveforms applies also
        // for the AudibleSound cue.
        pTrack->removeCuesOfType(mixxx::CueType::N60dBSound);
    }

    AnalysisDao& m_analysisDao;
};

} // anonymous namespace

void WTrackMenu::slotClearWaveform() {
    const auto progressLabelText =
            tr("Resetting waveform of %n track(s)", "", getTrackCount());
    AnalysisDao& analysisDao =
            m_pLibrary->trackCollectionManager()->internalCollection()->getAnalysisDAO();
    const auto trackOperator =
            ResetWaveformTrackPointerOperation(analysisDao);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ClearAllPerformanceMetadataTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit ClearAllPerformanceMetadataTrackPointerOperation(AnalysisDao& analysisDao)
            : m_removeMainCue(mixxx::CueType::MainCue),
              m_removeIntroCue(mixxx::CueType::Intro),
              m_removeOutroCue(mixxx::CueType::Outro),
              m_removeHotCues(mixxx::CueType::HotCue),
              m_removeLoopCues(mixxx::CueType::Loop),
              m_resetWaveform(analysisDao) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        m_resetBeats.apply(pTrack);
        m_resetPlayCounter.apply(pTrack);
        m_removeMainCue.apply(pTrack);
        m_removeHotCues.apply(pTrack);
        m_removeLoopCues.apply(pTrack);
        m_resetKeys.apply(pTrack);
        m_resetReplayGain.apply(pTrack);
        m_resetWaveform.apply(pTrack);
        m_resetRating.apply(pTrack);
        m_removeIntroCue.apply(pTrack);
        m_removeOutroCue.apply(pTrack);
    }

    const ResetBeatsTrackPointerOperation m_resetBeats;
    const ResetPlayCounterTrackPointerOperation m_resetPlayCounter;
    const RemoveCuesOfTypeTrackPointerOperation m_removeMainCue;
    const RemoveCuesOfTypeTrackPointerOperation m_removeIntroCue;
    const RemoveCuesOfTypeTrackPointerOperation m_removeOutroCue;
    const RemoveCuesOfTypeTrackPointerOperation m_removeHotCues;
    const RemoveCuesOfTypeTrackPointerOperation m_removeLoopCues;
    const ResetKeysTrackPointerOperation m_resetKeys;
    const ResetReplayGainTrackPointerOperation m_resetReplayGain;
    const ResetWaveformTrackPointerOperation m_resetWaveform;
    const ResetRatingTrackPointerOperation m_resetRating;
};

} // anonymous namespace

void WTrackMenu::slotClearAllMetadata() {
    const auto progressLabelText =
            tr("Resetting all performance metadata of %n track(s)", "", getTrackCount());
    AnalysisDao& analysisDao =
            m_pLibrary->trackCollectionManager()->internalCollection()->getAnalysisDAO();
    const auto trackOperator =
            ClearAllPerformanceMetadataTrackPointerOperation(analysisDao);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class RemoveTrackFilesFromDiskTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    const QList<TrackRef>& getTracksToPurge() const {
        return mTracksToPurge;
    }
    const QList<QString>& getTracksToKeep() const {
        return mTracksToKeep;
    }

  private:
    mutable QList<TrackRef> mTracksToPurge;
    mutable QList<QString> mTracksToKeep;

    void doApply(
            const TrackPointer& pTrack) const override {
        auto trackRef = TrackRef::fromFileInfo(
                pTrack->getFileInfo(),
                pTrack->getId());
        VERIFY_OR_DEBUG_ASSERT(trackRef.isValid()) {
            return;
        }
        QString location = pTrack->getLocation();
        QFile file(location);
        if (file.exists() && !file.remove()) {
            // Deletion failed, log warning and queue location for the
            // Failed Deletions warning.
            qWarning()
                    << "Queued file"
                    << location
                    << "could not be deleted. Track is not purged";
            mTracksToKeep.append(location);
        } else {
            // File doesn't exist or was deleted.
            // Note: we must NOT purge every single track here since
            // TrackDAO::afterPurgingTracks would enforce a track model update (select())
            // So we add it to the purge queue and purge all tracks at once
            // in slotRemoveFromDisk() afterwards.
            mTracksToPurge.append(trackRef);
        }
    }
};

} // anonymous namespace

void WTrackMenu::slotRemoveFromDisk() {
    QStringList locations;
    if (m_pTrackModel) {
        const auto trackRefs = getTrackRefs();
        locations.reserve(trackRefs.size());
        for (const auto& trackRef : trackRefs) {
            QString location = trackRef.getLocation();
            locations.append(location);
        }
        locations.removeDuplicates();
    } else if (m_pTrack) {
        QString location = m_pTrack->getLocation();
        locations.append(location);
    } else {
        return;
    }

    {
        // Prepare the delete confirmation dialog
        // List view for the files to be deleted
        // NOTE(ronso0) We could also make this a table to allow showing
        // artist and title if file names don't suffice to identify tracks.
        QListWidget* delListWidget = new QListWidget();
        delListWidget->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                QSizePolicy::MinimumExpanding));
        delListWidget->setFocusPolicy(Qt::ClickFocus);
        delListWidget->addItems(locations);
        mixxx::widgethelper::growListWidget(*delListWidget, *this);

        QString delWarningText;
        if (m_pTrackModel) {
            delWarningText = tr("Permanently delete these files from disk?") +
                    QStringLiteral("<br><br><b>") +
                    tr("This can not be undone!") + QStringLiteral("</b>");
        } else {
            delWarningText =
                    tr("Stop the deck and permanently delete this track file from disk?") +
                    QStringLiteral("<br><br><b>") +
                    tr("This can not be undone!") + QStringLiteral("</b>");
        }
        QLabel* delWarning = new QLabel();
        delWarning->setText(delWarningText);
        delWarning->setTextFormat(Qt::RichText);
        delWarning->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
                QSizePolicy::Minimum));

        QDialogButtonBox* delButtons = new QDialogButtonBox();
        QPushButton* cancelBtn = delButtons->addButton(
                tr("Cancel"),
                QDialogButtonBox::RejectRole);
        QPushButton* deleteBtn = delButtons->addButton(
                tr("Delete Files"),
                QDialogButtonBox::AcceptRole);
        cancelBtn->setDefault(true);

        // Populate the main layout
        QVBoxLayout* delLayout = new QVBoxLayout();
        delLayout->addWidget(delListWidget);
        delLayout->addWidget(delWarning);
        delLayout->addWidget(delButtons);

        QDialog dlgDelConfirm;
        dlgDelConfirm.setModal(true); // just to be sure
        dlgDelConfirm.setWindowTitle(tr("Delete Track Files"));
        // This is required after customizing the buttons, otherwise neither button
        // would close the dialog.
        connect(cancelBtn, &QPushButton::clicked, &dlgDelConfirm, &QDialog::reject);
        connect(deleteBtn, &QPushButton::clicked, &dlgDelConfirm, &QDialog::accept);
        dlgDelConfirm.setLayout(delLayout);

        if (dlgDelConfirm.exec() == QDialog::Rejected) {
            return;
        }
    }

    // If the operation was initiated from a deck's track menu
    // we'll first stop the deck and eject the track.
    if (m_pTrack) {
        ControlObject::set(ConfigKey(m_deckGroup, "stop"), 1.0);
        ControlObject::set(ConfigKey(m_deckGroup, "eject"), 1.0);
    }

    // Set up and initiate the track batch operation
    const auto progressLabelText =
            tr("Removing %n track file(s) from disk...",
                    "",
                    getTrackCount());
    const auto trackOperator =
            RemoveTrackFilesFromDiskTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);

    // Purge deleted tracks and show deletion summary message.
    const QList<TrackRef> tracksToPurge(trackOperator.getTracksToPurge());
    if (!tracksToPurge.isEmpty()) {
        // Purge only those tracks whose files have actually been deleted.
        m_pLibrary->trackCollectionManager()->purgeTracks(tracksToPurge);

        // Show purge summary message
        QMessageBox msgBoxPurgeTracks;
        msgBoxPurgeTracks.setIcon(QMessageBox::Information);
        QString msgTitle;
        QString msgText;
        if (m_pTrackModel) {
            msgTitle = tr("Track Files Deleted");
            msgText =
                    tr("%1 track files were deleted from disk and purged "
                       "from the Mixxx database.")
                            .arg(QString::number(tracksToPurge.length())) +
                    QStringLiteral("<br><br>") +
                    tr("Note: if you are in Browse or Recording you need to "
                       "click the current view again to see changes.");
        } else {
            msgTitle = tr("Track File Deleted");
            msgText = tr(
                    "Track file was deleted from disk and purged "
                    "from the Mixxx database.");
        }
        msgBoxPurgeTracks.setWindowTitle(msgTitle);
        msgBoxPurgeTracks.setText(msgText);
        msgBoxPurgeTracks.setTextFormat(Qt::RichText);
        msgBoxPurgeTracks.setStandardButtons(QMessageBox::Ok);
        msgBoxPurgeTracks.exec();
    }

    const QList<QString> tracksToKeep(trackOperator.getTracksToKeep());
    if (tracksToKeep.isEmpty()) {
        // All selected tracks could be processed. Finish!
        emit restoreCurrentIndex();
        return;
    }
    // Else show a message with a list of tracks that could not be deleted.
    QLabel* notDeletedLabel = new QLabel;
    QString msgText;
    if (m_pTrackModel) {
        msgText =
                tr("The following %1 file(s) could not be deleted from disk")
                        .arg(QString::number(
                                tracksToKeep.length()));
    } else {
        msgText = tr("This track file could not be deleted from disk");
    }
    notDeletedLabel->setText(msgText);
    notDeletedLabel->setTextFormat(Qt::RichText);

    QListWidget* notDeletedListWidget = new QListWidget;
    notDeletedListWidget->setFocusPolicy(Qt::ClickFocus);
    notDeletedListWidget->addItems(tracksToKeep);
    mixxx::widgethelper::growListWidget(*notDeletedListWidget, *this);

    QDialogButtonBox* notDeletedButtons = new QDialogButtonBox();
    QPushButton* closeBtn = notDeletedButtons->addButton(
            tr("Close"),
            QDialogButtonBox::AcceptRole);

    QVBoxLayout* notDeletedLayout = new QVBoxLayout;
    notDeletedLayout->addWidget(notDeletedLabel);
    notDeletedLayout->addWidget(notDeletedListWidget);
    notDeletedLayout->addWidget(notDeletedButtons);

    QDialog dlgNotDeleted;
    dlgNotDeleted.setModal(true);
    dlgNotDeleted.setWindowTitle(tr("Remaining Track File(s)"));
    dlgNotDeleted.setLayout(notDeletedLayout);
    // Required for being able to close the dialog
    connect(closeBtn, &QPushButton::clicked, &dlgNotDeleted, &QDialog::close);
    dlgNotDeleted.exec();
    emit restoreCurrentIndex();
}

void WTrackMenu::slotShowDlgTrackInfo() {
    if (isEmpty()) {
        return;
    }
    // Create a fresh dialog on invocation
    m_pDlgTrackInfo = std::make_unique<DlgTrackInfo>(
            m_pConfig,
            m_pTrackModel);
    connect(m_pDlgTrackInfo.get(),
            &QDialog::finished,
            this,
            [this]() {
                if (m_pDlgTrackInfo.get() == sender()) {
                    m_pDlgTrackInfo.release()->deleteLater();
                }
            });
    // Method getFirstTrackPointer() is not applicable here!
    if (m_pTrackModel) {
        m_pDlgTrackInfo->loadTrack(m_trackIndexList.at(0));
    } else {
        m_pDlgTrackInfo->loadTrack(m_pTrack);
    }
    m_pDlgTrackInfo->show();
}

void WTrackMenu::slotShowDlgTagFetcher() {
    if (isEmpty()) {
        return;
    }
    // Create a fresh dialog on invocation
    m_pDlgTagFetcher = std::make_unique<DlgTagFetcher>(
            m_pConfig, m_pTrackModel);
    connect(m_pDlgTagFetcher.get(),
            &QDialog::finished,
            this,
            [this]() {
                if (m_pDlgTagFetcher.get() == sender()) {
                    m_pDlgTagFetcher.release()->deleteLater();
                }
            });
    // Method getFirstTrackPointer() is not applicable here!
    if (m_pTrackModel) {
        m_pDlgTagFetcher->loadTrack(m_trackIndexList.at(0));
    } else {
        m_pDlgTagFetcher->loadTrack(m_pTrack);
    }
    m_pDlgTagFetcher->show();
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
        qWarning() << "No tracks selected for AutoDJ";
        return;
    }

    PlaylistDAO& playlistDao = m_pLibrary->trackCollectionManager()
                                       ->internalCollection()
                                       ->getPlaylistDAO();

    // TODO(XXX): Care whether the append succeeded.
    m_pLibrary->trackCollectionManager()->unhideTracks(trackIds);
    playlistDao.addTracksToAutoDJQueue(trackIds, loc);
}

namespace {

class SetCoverInfoTrackPointerOperation : public mixxx::TrackPointerOperation {
  public:
    explicit SetCoverInfoTrackPointerOperation(CoverInfoRelative&& coverInfo)
            : m_coverInfo(std::move(coverInfo)) {
    }

  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        pTrack->setCoverInfo(m_coverInfo);
    }

    const CoverInfoRelative m_coverInfo;
};

} // anonymous namespace

void WTrackMenu::slotCoverInfoSelected(CoverInfoRelative coverInfo) {
    const auto progressLabelText =
            tr("Setting cover art of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            SetCoverInfoTrackPointerOperation(std::move(coverInfo));
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

namespace {

class ReloadCoverInfoTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        m_coverInfoGuesser.guessAndSetCoverInfoForTrack(*pTrack);
    }

    mutable CoverInfoGuesser m_coverInfoGuesser;
};

} // anonymous namespace

void WTrackMenu::slotReloadCoverArt() {
    const auto progressLabelText =
            tr("Reloading cover art of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ReloadCoverInfoTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotRemove() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->removeTracks(getTrackIndices());
    emit restoreCurrentIndex();
}

void WTrackMenu::slotHide() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->hideTracks(getTrackIndices());
    emit restoreCurrentIndex();
}

void WTrackMenu::slotUnhide() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->unhideTracks(getTrackIndices());
    emit restoreCurrentIndex();
}

void WTrackMenu::slotPurge() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->purgeTracks(getTrackIndices());
    emit restoreCurrentIndex();
}

void WTrackMenu::clearTrackSelection() {
    m_pTrack = nullptr;
    m_deckGroup = QString();
    m_trackIndexList.clear();
}

bool WTrackMenu::featureIsEnabled(Feature flag) const {
    bool optionIsSelected = m_eActiveFeatures.testFlag(flag);
    if (!optionIsSelected) {
        return false;
    }

    if (!m_pTrackModel) {
        return !m_eTrackModelFeatures.testFlag(flag);
    }

    switch (flag) {
    case Feature::AutoDJ:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::AddToAutoDJ);
    case Feature::LoadTo:
        return m_pTrackModel->hasCapabilities(
                       TrackModel::Capability::LoadToDeck) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::Capability::LoadToSampler) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::Capability::LoadToPreviewDeck);
    case Feature::Playlist:
    case Feature::Crate:
        return m_pTrackModel->hasCapabilities(
                TrackModel::Capability::AddToTrackSet);
    case Feature::Remove:
        return m_pTrackModel->hasCapabilities(
                       TrackModel::Capability::Remove) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::Capability::RemovePlaylist) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::Capability::RemoveCrate);
    case Feature::Metadata:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::EditMetadata);
    case Feature::Analyze:
        return m_pTrackModel->hasCapabilities(
                TrackModel::Capability::EditMetadata |
                TrackModel::Capability::Analyze);
    case Feature::Reset:
        return m_pTrackModel->hasCapabilities(
                TrackModel::Capability::EditMetadata |
                TrackModel::Capability::ResetPlayed);
    case Feature::BPM:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::EditMetadata);
    case Feature::Color:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::EditMetadata);
    case Feature::HideUnhidePurge:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::Hide) ||
                m_pTrackModel->hasCapabilities(TrackModel::Capability::Unhide) ||
                m_pTrackModel->hasCapabilities(TrackModel::Capability::Purge);
    case Feature::RemoveFromDisk:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::RemoveFromDisk);
    case Feature::FileBrowser:
        return true;
    case Feature::FindOnWeb:
        return true;
    case Feature::Properties:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::EditMetadata);
    case Feature::SearchRelated:
        return m_pLibrary != nullptr;
    case Feature::SelectInLibrary:
        return m_pTrack != nullptr;
    default:
        DEBUG_ASSERT(!"unreachable");
        return false;
    }
}
