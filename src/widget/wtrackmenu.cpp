#include "widget/wtrackmenu.h"

#include <QCheckBox>
#include <QInputDialog>
#include <QModelIndex>

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
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/desktophelper.h"
#include "util/parented_ptr.h"
#include "util/qt.h"
#include "widget/wcolorpickeraction.h"
#include "widget/wcoverartlabel.h"
#include "widget/wcoverartmenu.h"
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
        connect(m_pPropertiesAct, &QAction::triggered, this, &WTrackMenu::slotShowDlgTrackInfo);
    }

    if (featureIsEnabled(Feature::FileBrowser)) {
        m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
        connect(m_pFileBrowserAct, &QAction::triggered, this, &WTrackMenu::slotOpenInFileBrowser);
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
                m_pLibrary->trackCollections()->externalCollections().size());
        for (auto* const pExternalTrackCollection :
                m_pLibrary->trackCollections()->externalCollections()) {
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
}

void WTrackMenu::setupActions() {
    if (featureIsEnabled(Feature::SearchRelated)) {
        addMenu(m_pSearchRelatedMenu);
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
        addMenu(m_pMetadataMenu);
    }

    if (featureIsEnabled(Feature::Reset)) {
        m_pClearMetadataMenu->addAction(m_pClearBeatsAction);
        m_pClearMetadataMenu->addAction(m_pClearPlayCountAction);
        m_pClearMetadataMenu->addAction(m_pClearRatingAction);
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
        for (const auto& pTrack : m_trackPointerList) {
            if (pTrack->isBpmLocked()) {
                return true;
            }
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
        commonColor = m_trackPointerList.first()->getColor();
        for (const auto& pTrack : m_trackPointerList) {
            if (commonColor != pTrack->getColor()) {
                // Multiple, different colors
                return std::nullopt;
            }
        }
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
        int iNumDecks = static_cast<int>(m_pNumDecks->get());
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

QList<TrackRef> WTrackMenu::getTrackRefs() const {
    QList<TrackRef> trackRefs;
    if (m_pTrackModel) {
        trackRefs.reserve(m_trackIndexList.size());
        for (const auto& index : m_trackIndexList) {
            auto trackRef = TrackRef::fromFileInfo(
                    m_pTrackModel->getTrackLocation(index),
                    m_pTrackModel->getTrackId(index));
            if (!trackRef.isValid()) {
                // Skip unavailable tracks
                continue;
            }
            trackRefs.push_back(std::move(trackRef));
        }
    } else {
        trackRefs.reserve(m_trackPointerList.size());
        for (const auto& pTrack : m_trackPointerList) {
            DEBUG_ASSERT(pTrack);
            auto trackRef = TrackRef::fromFileInfo(
                    pTrack->getLocation(),
                    pTrack->getId());
            trackRefs.push_back(std::move(trackRef));
        }
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
    } else {
        if (m_trackPointerList.isEmpty()) {
            return TrackPointer();
        }
        DEBUG_ASSERT(m_trackPointerList.first());
        return m_trackPointerList.first();
    }
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
    } else {
        if (m_trackPointerList.isEmpty()) {
            return nullptr;
        }
        return std::make_unique<mixxx::TrackPointerListIterator>(
                m_trackPointerList);
    }
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
            m_pLibrary->trackCollections(),
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

namespace {

class ImportMetadataFromFileTagsTrackPointerOperation : public mixxx::TrackPointerOperation {
  private:
    void doApply(
            const TrackPointer& pTrack) const override {
        // The user has explicitly requested to reload metadata from the file
        // to override the information within Mixxx! Custom cover art must be
        // reloaded separately.
        SoundSourceProxy(pTrack).updateTrackFromSource(
                SoundSourceProxy::ImportTrackMetadataMode::Again);
    }
};

} // anonymous namespace

void WTrackMenu::slotImportMetadataFromFileTags() {
    const auto progressLabelText =
            tr("Importing metadata of %n track(s) from file tags", "", getTrackCount());
    const auto trackOperator =
            ImportMetadataFromFileTagsTrackPointerOperation();
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
    const PlaylistDAO& playlistDao = m_pLibrary->trackCollections()
                                             ->internalCollection()
                                             ->getPlaylistDAO();
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
            auto* pAction = new QAction(
                    mixxx::escapeTextPropertyWithoutShortcuts(it.key()),
                    m_pPlaylistMenu);
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

    PlaylistDAO& playlistDao = m_pLibrary->trackCollections()
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
    m_pLibrary->trackCollections()->unhideTracks(trackIds);
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
            m_pLibrary->trackCollections()
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
            m_pLibrary->trackCollections()
                    ->internalCollection()
                    ->removeCrateTracks(crateId, trackIds);
        }
    } else {
        if (!crateId.isValid()) { // i.e. a new crate is suppose to be created
            crateId = CrateFeatureHelper(
                    m_pLibrary->trackCollections()->internalCollection(), m_pConfig)
                              .createEmptyCrate();
        }
        if (crateId.isValid()) {
            m_pLibrary->trackCollections()->unhideTracks(trackIds);
            m_pLibrary->trackCollections()->internalCollection()->addCrateTracks(crateId, trackIds);
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
            m_pLibrary->trackCollections()->internalCollection(), m_pConfig)
                              .createEmptyCrate();

    if (crateId.isValid()) {
        m_pLibrary->trackCollections()->unhideTracks(trackIds);
        m_pLibrary->trackCollections()->internalCollection()->addCrateTracks(crateId, trackIds);
    }
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
    explicit ScaleBpmTrackPointerOperation(mixxx::Beats::BPMScale bpmScale)
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
        pTrack->setBeats(pBeats->scale(m_bpmScale));
    }

    const mixxx::Beats::BPMScale m_bpmScale;
};

} // anonymous namespace

void WTrackMenu::slotScaleBpm(int scale) {
    const auto progressLabelText =
            tr("Scaling BPM of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ScaleBpmTrackPointerOperation(
                    static_cast<mixxx::Beats::BPMScale>(scale));
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
        if (pTrack->isBpmLocked()) {
            return;
        }
        pTrack->setBeats(mixxx::BeatsPointer());
    }
};

} // anonymous namespace

void WTrackMenu::slotClearBeats() {
    const auto progressLabelText =
            tr("Resetting beats of %n track(s)", "", getTrackCount());
    const auto trackOperator =
            ResetBeatsTrackPointerOperation();
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
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

} // anonymous namespace

void WTrackMenu::slotClearMainCue() {
    const auto progressLabelText =
            tr("Removing main cue from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            RemoveCuesOfTypeTrackPointerOperation(mixxx::CueType::MainCue);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotClearOutroCue() {
    const auto progressLabelText =
            tr("Removing outro cue from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            RemoveCuesOfTypeTrackPointerOperation(mixxx::CueType::Outro);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotClearIntroCue() {
    const auto progressLabelText =
            tr("Removing intro cue from %n track(s)", "", getTrackCount());
    const auto trackOperator =
            RemoveCuesOfTypeTrackPointerOperation(mixxx::CueType::Intro);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotClearLoop() {
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
    }

    AnalysisDao& m_analysisDao;
};

} // anonymous namespace

void WTrackMenu::slotClearWaveform() {
    const auto progressLabelText =
            tr("Resetting waveform of %n track(s)", "", getTrackCount());
    AnalysisDao& analysisDao =
            m_pLibrary->trackCollections()->internalCollection()->getAnalysisDAO();
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
        m_removeIntroCue.apply(pTrack);
        m_removeOutroCue.apply(pTrack);
        m_removeHotCues.apply(pTrack);
        m_removeLoopCues.apply(pTrack);
        m_resetKeys.apply(pTrack);
        m_resetReplayGain.apply(pTrack);
        m_resetWaveform.apply(pTrack);
        m_resetRating.apply(pTrack);
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
            m_pLibrary->trackCollections()->internalCollection()->getAnalysisDAO();
    const auto trackOperator =
            ClearAllPerformanceMetadataTrackPointerOperation(analysisDao);
    applyTrackPointerOperation(
            progressLabelText,
            &trackOperator);
}

void WTrackMenu::slotShowDlgTrackInfo() {
    if (isEmpty()) {
        return;
    }
    // Create a fresh dialog on invocation
    m_pDlgTrackInfo = std::make_unique<DlgTrackInfo>(
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
        m_pDlgTrackInfo->loadTrack(m_trackPointerList.at(0));
    }
    m_pDlgTrackInfo->show();
}

void WTrackMenu::slotShowDlgTagFetcher() {
    if (isEmpty()) {
        return;
    }
    // Create a fresh dialog on invocation
    m_pDlgTagFetcher = std::make_unique<DlgTagFetcher>(
            m_pTrackModel);
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
        m_pDlgTagFetcher->loadTrack(m_trackPointerList.at(0));
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

    PlaylistDAO& playlistDao = m_pLibrary->trackCollections()
                                       ->internalCollection()
                                       ->getPlaylistDAO();

    // TODO(XXX): Care whether the append succeeded.
    m_pLibrary->trackCollections()->unhideTracks(trackIds);
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
    case Feature::FileBrowser:
        return true;
    case Feature::Properties:
        return m_pTrackModel->hasCapabilities(TrackModel::Capability::EditMetadata);
    case Feature::SearchRelated:
        return m_pLibrary != nullptr;
    default:
        DEBUG_ASSERT(!"unreachable");
        return false;
    }
}
