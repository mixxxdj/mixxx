#include "widget/wtrackmenu.h"

#include <QCheckBox>
#include <QInputDialog>
#include <QModelIndex>

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
#include "library/trackcollectionmanager.h"
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
        Features flags,
        TrackModel* trackModel)
        : QMenu(parent),
          m_pTrackModel(trackModel),
          m_pConfig(pConfig),
          m_pTrackCollectionManager(pTrackCollectionManager),
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
        return m_trackPointerList.size();
    }
}

void WTrackMenu::popup(const QPoint& pos, QAction* at) {
    if (isEmpty()) {
        return;
    }
    QMenu::popup(pos, at);
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
}

void WTrackMenu::createActions() {
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
        m_pRemoveAct = new QAction(tr("Remove"), this);
        connect(m_pRemoveAct, &QAction::triggered, this, &WTrackMenu::slotRemove);

        m_pRemovePlaylistAct = new QAction(tr("Remove from Playlist"), this);
        connect(m_pRemovePlaylistAct, &QAction::triggered, this, &WTrackMenu::slotRemove);

        m_pRemoveCrateAct = new QAction(tr("Remove from Crate"), this);
        connect(m_pRemoveCrateAct, &QAction::triggered, this, &WTrackMenu::slotRemove);
    }

    if (featureIsEnabled(Feature::HideUnhidePurge)) {
        m_pHideAct = new QAction(tr("Hide from Library"), this);
        connect(m_pHideAct, &QAction::triggered, this, &WTrackMenu::slotHide);

        m_pUnhideAct = new QAction(tr("Unhide from Library"), this);
        connect(m_pUnhideAct, &QAction::triggered, this, &WTrackMenu::slotUnhide);

        m_pPurgeAct = new QAction(tr("Purge from Library"), this);
        connect(m_pPurgeAct, &QAction::triggered, this, &WTrackMenu::slotPurge);
    }

    if (featureIsEnabled(Feature::Properties)) {
        m_pPropertiesAct = new QAction(tr("Properties"), this);
        connect(m_pPropertiesAct, &QAction::triggered, this, &WTrackMenu::slotShowTrackInfo);
    }

    if (featureIsEnabled(Feature::FileBrowser)) {
        m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
        connect(m_pFileBrowserAct, &QAction::triggered, this, &WTrackMenu::slotOpenInFileBrowser);
    }

    if (featureIsEnabled(Feature::Metadata)) {
        m_pImportMetadataFromFileAct = new QAction(tr("Import From File Tags"), m_pMetadataMenu);
        connect(m_pImportMetadataFromFileAct, &QAction::triggered, this, &WTrackMenu::slotImportTrackMetadataFromFileTags);

        m_pImportMetadataFromMusicBrainzAct = new QAction(tr("Import From MusicBrainz"), m_pMetadataMenu);
        connect(m_pImportMetadataFromMusicBrainzAct, &QAction::triggered, this, &WTrackMenu::slotShowDlgTagFetcher);

        // Give a nullptr parent because otherwise it inherits our style which can
        // make it unreadable. Bug #673411
        m_pTagFetcher.reset(new DlgTagFetcher(nullptr, m_pTrackModel));

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

    if (featureIsEnabled(Feature::Reset)) {
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
            slotScaleBpm(mixxx::Beats::DOUBLE);
        });
        connect(m_pBpmHalveAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::HALVE);
        });
        connect(m_pBpmTwoThirdsAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::TWOTHIRDS);
        });
        connect(m_pBpmThreeFourthsAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::THREEFOURTHS);
        });
        connect(m_pBpmFourThirdsAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::FOURTHIRDS);
        });
        connect(m_pBpmThreeHalvesAction, &QAction::triggered, this, [this] {
            slotScaleBpm(mixxx::Beats::THREEHALVES);
        });

        m_pBpmResetAction = new QAction(tr("Reset BPM"), m_pBPMMenu);
        connect(m_pBpmResetAction,
                &QAction::triggered,
                this,
                &WTrackMenu::slotClearBeats);
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

    if (featureIsEnabled(Feature::Properties)) {
        // Give a nullptr parent because otherwise it inherits our style which can
        // make it unreadable. Bug #673411
        m_pTrackInfo.reset(new DlgTrackInfo(nullptr, m_pConfig, m_pTrackModel));
    }
}

void WTrackMenu::setupActions() {
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
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE)) {
            addAction(m_pRemoveAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST)) {
            addAction(m_pRemovePlaylistAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_CRATE)) {
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

    if (featureIsEnabled(Feature::Reset)) {
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

    addSeparator();
    if (featureIsEnabled(Feature::HideUnhidePurge)) {
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_HIDE)) {
            addAction(m_pHideAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE)) {
            addAction(m_pUnhideAct);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_PURGE)) {
            addAction(m_pPurgeAct);
        }
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
        for (const auto trackIndex : m_trackIndexList) {
            QModelIndex bpmLockedIndex =
                    trackIndex.sibling(trackIndex.row(), column);
            if (bpmLockedIndex.data().toBool()) {
                return true;
            }
        }
    } else {
        for (const auto& pTrack : m_trackPointerList) {
            if (pTrack->isBpmLocked()) {
                return true;
            }
        }
    }
    return false;
}

std::optional<mixxx::RgbColor> WTrackMenu::getCommonTrackColor() const {
    VERIFY_OR_DEBUG_ASSERT(!isEmpty()) {
        return std::nullopt;
    }
    std::optional<mixxx::RgbColor> commonColor;
    if (m_pTrackModel) {
        const int column =
                m_pTrackModel->fieldIndex(LIBRARYTABLE_COLOR);
        commonColor = mixxx::RgbColor::fromQVariant(
                m_trackIndexList.first().sibling(m_trackIndexList.first().row(), column).data());
        if (!commonColor) {
            return std::nullopt;
        }
        for (const auto trackIndex : m_trackIndexList) {
            const auto otherColor = mixxx::RgbColor::fromQVariant(
                    trackIndex.sibling(trackIndex.row(), column).data());
            if (commonColor != otherColor) {
                // Multiple, different colors
                return std::nullopt;
            }
        }
    } else {
        auto commonColor = m_trackPointerList.first()->getColor();
        if (!commonColor) {
            return std::nullopt;
        }
        for (const auto& pTrack : m_trackPointerList) {
            if (commonColor != pTrack->getColor()) {
                // Multiple, different colors
                return std::nullopt;
            }
        }
    }
    return commonColor;
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
        coverInfo.hash =
                lastIndex
                        .sibling(
                                lastIndex.row(),
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_COVERART_HASH))
                        .data()
                        .toUInt();
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
                                m_pTrackModel->fieldIndex(LIBRARYTABLE_LOCATION))
                        .data()
                        .toString();
        return coverInfo;
    } else {
        return m_trackPointerList.last()->getCoverInfoWithLocation();
    }
}

void WTrackMenu::updateMenus() {
    if (isEmpty()) {
        return;
    }

    // Gray out some stuff if multiple songs were selected.
    const bool singleTrackSelected = getTrackCount() == 1;

    if (featureIsEnabled(Feature::LoadTo)) {
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
                bool deckEnabled = (!deckPlaying || loadTrackIntoPlayingDeck) && singleTrackSelected;
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
        bool locked = m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_LOCKED);
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE)) {
            m_pRemoveAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST)) {
            m_pRemovePlaylistAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_REMOVE_CRATE)) {
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

    if (featureIsEnabled(Feature::Color)) {
        m_pColorPickerAction->setColorPalette(
                ColorPaletteSettings(m_pConfig).getTrackColorPalette());

        // Resize Menu to fit changed palette
        QResizeEvent resizeEvent(QSize(), m_pColorMenu->size());
        qApp->sendEvent(m_pColorMenu, &resizeEvent);

        const auto commonColor = getCommonTrackColor();
        if (commonColor) {
            m_pColorPickerAction->setSelectedColor(commonColor);
        } else {
            m_pColorPickerAction->resetSelectedColor();
        }
    }

    if (featureIsEnabled(Feature::HideUnhidePurge)) {
        bool locked = m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_LOCKED);
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_HIDE)) {
            m_pHideAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE)) {
            m_pUnhideAct->setEnabled(!locked);
        }
        if (m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_PURGE)) {
            m_pPurgeAct->setEnabled(!locked);
        }
    }

    if (featureIsEnabled(Feature::Properties)) {
        m_pPropertiesAct->setEnabled(singleTrackSelected);
    }
}

void WTrackMenu::loadTrack(
        const TrackPointer& pTrack) {
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
    m_trackPointerList = TrackPointerList{pTrack};
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
        trackIds.reserve(m_trackPointerList.size());
        for (const auto& pTrack : m_trackPointerList) {
            const auto trackId = pTrack->getId();
            DEBUG_ASSERT(trackId.isValid());
            trackIds.push_back(trackId);
        }
    }
    return trackIds;
}

TrackPointerList WTrackMenu::getTrackPointers(
        int maxSize) const {
    if (!m_pTrackModel) {
        return m_trackPointerList;
    }
    TrackPointerList trackPointers;
    trackPointers.reserve(m_trackIndexList.size());
    for (const auto& index : m_trackIndexList) {
        DEBUG_ASSERT(maxSize < 0 ||
                maxSize <= trackPointers.size());
        if (maxSize >= 0 && maxSize <= trackPointers.size()) {
            return trackPointers;
        }
        const auto pTrack = m_pTrackModel->getTrack(index);
        if (!pTrack) {
            // Skip unavailable tracks
            continue;
        }
        trackPointers.push_back(pTrack);
    }
    return trackPointers;
}

QModelIndexList WTrackMenu::getTrackIndices() const {
    // Indices are associated with a TrackModel. Can only be obtained
    // if a TrackModel is available.
    DEBUG_ASSERT(m_pTrackModel);
    return m_trackIndexList;
}

void WTrackMenu::slotOpenInFileBrowser() {
    const auto trackPointers = getTrackPointers();
    QStringList locations;
    locations.reserve(trackPointers.size());
    for (const auto& pTrack : trackPointers) {
        locations << pTrack->getLocation();
    }
    mixxx::DesktopHelper::openInFileBrowser(locations);
}

void WTrackMenu::slotImportTrackMetadataFromFileTags() {
    for (const auto& pTrack : getTrackPointers()) {
        // The user has explicitly requested to reload metadata from the file
        // to override the information within Mixxx! Custom cover art must be
        // reloaded separately.
        SoundSourceProxy(pTrack).updateTrackFromSource(
                SoundSourceProxy::ImportTrackMetadataMode::Again);
    }
}

void WTrackMenu::slotExportTrackMetadataIntoFileTags() {
    mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();

    for (const auto& pTrack : getTrackPointers()) {
        // Export of metadata is deferred until all references to the
        // corresponding track object have been dropped. Otherwise
        // writing to files that are still used for playback might
        // cause crashes or at least audible glitches!
        mixxx::DlgTrackMetadataExport::showMessageBoxOncePerSession();
        pTrack->markForMetadataExport();
    }
}

void WTrackMenu::slotUpdateExternalTrackCollection(
        ExternalTrackCollection* externalTrackCollection) {
    VERIFY_OR_DEBUG_ASSERT(externalTrackCollection) {
        return;
    }
    auto trackPointers = getTrackPointers();

    QList<TrackRef> trackRefs;
    trackRefs.reserve(trackPointers.size());
    for (const auto& pTrack : trackPointers) {
        trackRefs.append(
                TrackRef::fromFileInfo(
                        pTrack->getLocation(),
                        pTrack->getId()));
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
    for (const auto& pTrack : getTrackPointers()) {
        if (pTrack->isBpmLocked()) {
            continue;
        }
        mixxx::BeatsPointer pBeats = pTrack->getBeats();
        if (!pBeats) {
            continue;
        }
        pBeats->scale(static_cast<mixxx::Beats::BPMScale>(scale));
    }
}

void WTrackMenu::lockBpm(bool lock) {
    // TODO: This should be done in a thread for large selections
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->setBpmLocked(lock);
    }
}

void WTrackMenu::slotColorPicked(mixxx::RgbColor::optional_t color) {
    // TODO: This should be done in a thread for large selections
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->setColor(color);
    }

    hide();
}

void WTrackMenu::loadSelectionToGroup(QString group, bool play) {
    const auto trackPointers = getTrackPointers(1);
    if (trackPointers.empty()) {
        return;
    }
    // Only the first track was requested
    DEBUG_ASSERT(trackPointers.size() == 1);
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
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->resetPlayCounter();
    }
}

void WTrackMenu::slotClearBeats() {
    // TODO: This should be done in a thread for large selections
    for (const auto& pTrack : getTrackPointers()) {
        if (pTrack->isBpmLocked()) {
            continue;
        }
        pTrack->setBeats(mixxx::BeatsPointer());
    }
}

void WTrackMenu::slotClearMainCue() {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->removeCuesOfType(mixxx::CueType::MainCue);
    }
}

void WTrackMenu::slotClearOutroCue() {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->removeCuesOfType(mixxx::CueType::Outro);
    }
}

void WTrackMenu::slotClearIntroCue() {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->removeCuesOfType(mixxx::CueType::Intro);
    }
}

void WTrackMenu::slotClearKey() {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->resetKeys();
    }
}

void WTrackMenu::slotClearReplayGain() {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->setReplayGain(mixxx::ReplayGain());
    }
}

void WTrackMenu::slotClearWaveform() {
    AnalysisDao& analysisDao = m_pTrackCollectionManager->internalCollection()->getAnalysisDAO();
    for (const auto& pTrack : getTrackPointers()) {
        analysisDao.deleteAnalysesForTrack(pTrack->getId());
        pTrack->setWaveform(WaveformPointer());
        pTrack->setWaveformSummary(WaveformPointer());
    }
}

void WTrackMenu::slotClearLoop() {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->removeCuesOfType(mixxx::CueType::Loop);
    }
}

void WTrackMenu::slotClearHotCues() {
    for (const auto& pTrack : getTrackPointers()) {
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

void WTrackMenu::slotShowTrackInfo() {
    if (isEmpty()) {
        return;
    }
    if (m_pTrackModel) {
        m_pTrackInfo->loadTrack(m_trackIndexList.at(0));
    } else {
        m_pTrackInfo->loadTrack(m_trackPointerList.at(0));
    }
    m_pTrackInfo->show();
}

void WTrackMenu::slotShowDlgTagFetcher() {
    if (isEmpty()) {
        return;
    }
    if (m_pTrackModel) {
        m_pTagFetcher->loadTrack(m_trackIndexList.at(0));
    } else {
        m_pTagFetcher->loadTrack(m_trackPointerList.at(0));
    }
    m_pTagFetcher->show();
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

    PlaylistDAO& playlistDao = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO();

    // TODO(XXX): Care whether the append succeeded.
    m_pTrackCollectionManager->unhideTracks(trackIds);
    playlistDao.addTracksToAutoDJQueue(trackIds, loc);
}

void WTrackMenu::slotCoverInfoSelected(const CoverInfoRelative& coverInfo) {
    for (const auto& pTrack : getTrackPointers()) {
        pTrack->setCoverInfo(coverInfo);
    }
}

void WTrackMenu::slotReloadCoverArt() {
    guessTrackCoverInfoConcurrently(getTrackPointers());
}

void WTrackMenu::slotRemove() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->removeTracks(getTrackIndices());
}

void WTrackMenu::slotHide() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->hideTracks(getTrackIndices());
}

void WTrackMenu::slotUnhide() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->unhideTracks(getTrackIndices());
}

void WTrackMenu::slotPurge() {
    if (!m_pTrackModel) {
        return;
    }
    m_pTrackModel->purgeTracks(getTrackIndices());
}

void WTrackMenu::clearTrackSelection() {
    m_trackPointerList.clear();
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
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_ADDTOAUTODJ);
    case Feature::LoadTo:
        return m_pTrackModel->hasCapabilities(
                       TrackModel::TRACKMODELCAPS_LOADTODECK) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::TRACKMODELCAPS_LOADTOSAMPLER) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::TRACKMODELCAPS_LOADTOPREVIEWDECK);
    case Feature::Playlist:
        return m_pTrackModel->hasCapabilities(
                TrackModel::TRACKMODELCAPS_ADDTOPLAYLIST);
    case Feature::Crate:
        return m_pTrackModel->hasCapabilities(
                TrackModel::TRACKMODELCAPS_ADDTOCRATE);
    case Feature::Remove:
        return m_pTrackModel->hasCapabilities(
                       TrackModel::TRACKMODELCAPS_REMOVE) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::TRACKMODELCAPS_REMOVE_PLAYLIST) ||
                m_pTrackModel->hasCapabilities(
                        TrackModel::TRACKMODELCAPS_REMOVE_CRATE);
    case Feature::Metadata:
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA);
    case Feature::Reset:
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA) &&
                m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_RESETPLAYED);
    case Feature::BPM:
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA);
    case Feature::Color:
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA);
    case Feature::HideUnhidePurge:
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_HIDE) ||
                m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_UNHIDE) ||
                m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_PURGE);
    case Feature::FileBrowser:
        return true;
    case Feature::Properties:
        return m_pTrackModel->hasCapabilities(TrackModel::TRACKMODELCAPS_EDITMETADATA);
    default:
        DEBUG_ASSERT(!"unreachable");
        return true;
    }
}
