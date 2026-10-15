#include "library/autodj/autodjfeature.h"

#include <QMenu>
#include <QtDebug>

#include "ai/aimixplanner.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/autodj/autodjprocessor.h"
#include "library/autodj/dlgautodj.h"
#include "library/autodj/harmonicmixing.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/parser.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/cratestorage.h"
#include "library/treeitem.h"
#include "moc_autodjfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/keyutils.h"
#include "track/track.h"
#include "util/clipboard.h"
#include "util/defs.h"
#include "util/dnd.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"

namespace {
constexpr int kMaxRetrieveAttempts = 3;

int findOrCreateAutoDjPlaylistId(PlaylistDAO& playlistDAO) {
    int playlistId = playlistDAO.getPlaylistIdFromName(AUTODJ_TABLE);
    // If the AutoDJ playlist does not exist yet then create it.
    if (playlistId < 0) {
        playlistId = playlistDAO.createPlaylist(
                AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
        VERIFY_OR_DEBUG_ASSERT(playlistId >= 0) {
            qWarning() << "Failed to create Auto DJ playlist!";
        }
    }
    return playlistId;
}
} // anonymous namespace

AutoDJFeature::AutoDJFeature(Library* pLibrary,
        UserSettingsPointer pConfig,
        PlayerManagerInterface* pPlayerManager)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("autodj")),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_playlistDao(m_pTrackCollection->getPlaylistDAO()),
          m_iAutoDJPlaylistId(findOrCreateAutoDjPlaylistId(m_playlistDao)),
          m_pAutoDJProcessor(nullptr),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_pAutoDJView(nullptr),
          m_viewName(Library::kAutoDJViewName),
          m_autoDjCratesDao(m_iAutoDJPlaylistId, pLibrary->trackCollectionManager(), m_pConfig) {
    qRegisterMetaType<AutoDJProcessor::AutoDJState>("AutoDJState");
    m_pAutoDJProcessor = new AutoDJProcessor(this,
            m_pConfig,
            pPlayerManager,
            pLibrary->trackCollectionManager(),
            m_iAutoDJPlaylistId);

    // Connect loadTrackToPlayer signal as a queued connection to make sure all callbacks of a
    // previous load attempt have been called #10504.
    connect(m_pAutoDJProcessor,
            &AutoDJProcessor::loadTrackToPlayer,
            this,
            &LibraryFeature::loadTrackToPlayer,
            Qt::QueuedConnection);

    m_playlistDao.setAutoDJProcessor(m_pAutoDJProcessor);

    // Create the "Crates" tree-item under the root item.
    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    m_pCratesTreeItem = pRootItem->appendChild(tr("Crates"));
    m_pCratesTreeItem->setIcon(QIcon(":/images/library/ic_library_crates.svg"));

    // Create tree-items under "Crates".
    constructCrateChildModel();

    m_pSidebarModel->setRootItem(std::move(pRootItem));

    // Be notified when the status of crates changes.
    connect(m_pTrackCollection,
            &TrackCollection::crateInserted,
            this,
            &AutoDJFeature::slotCrateChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateUpdated,
            this,
            &AutoDJFeature::slotCrateChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateDeleted,
            this,
            &AutoDJFeature::slotCrateChanged);

    // Create context-menu items for enabling/disabling the auto-DJ
    m_pEnableAutoDJAction = make_parented<QAction>(tr("Enable Auto DJ"), this);
    connect(m_pEnableAutoDJAction.get(),
            &QAction::triggered,
            this,
            &AutoDJFeature::slotEnableAutoDJ);

    m_pDisableAutoDJAction = make_parented<QAction>(tr("Disable Auto DJ"), this);
    connect(m_pDisableAutoDJAction.get(),
            &QAction::triggered,
            this,
            &AutoDJFeature::slotDisableAutoDJ);

    // Create context-menu item for clearing the auto-DJ queue
    m_pClearQueueAction = make_parented<QAction>(tr("Clear Auto DJ Queue"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pClearQueueAction->setShortcut(removeKeySequence);
    connect(m_pClearQueueAction.get(),
            &QAction::triggered,
            this,
            &AutoDJFeature::slotClearQueue);
    // Create context menu item to allow crates to be removed from AutoDJ sources.
    // onRightClickChild() gets the clicked crate's id form the sidebar model and
    // assigns it to this action's data.
    // In slotRemoveCrateFromAutoDj() we retrieve the CrateId data and finally
    // remove the crate from sources in removeCrateFromAutoDj().
    m_pRemoveCrateFromAutoDjAction =
            make_parented<QAction>(tr("Remove Crate as Track Source"), this);
    m_pRemoveCrateFromAutoDjAction->setShortcut(removeKeySequence);
    connect(m_pRemoveCrateFromAutoDjAction.get(),
            &QAction::triggered,
            this,
            &AutoDJFeature::slotRemoveCrateFromAutoDj);
}

AutoDJFeature::~AutoDJFeature() {
    delete m_pAutoDJProcessor;
}

QVariant AutoDJFeature::title() {
    return tr("Auto DJ");
}

void AutoDJFeature::bindLibraryWidget(
        WLibrary* libraryWidget,
        KeyboardEventFilter* keyboard) {
    m_pAutoDJView = new DlgAutoDJ(
            libraryWidget,
            m_pConfig,
            m_pLibrary,
            m_pAutoDJProcessor,
            keyboard);
    libraryWidget->registerView(m_viewName, m_pAutoDJView);
    connect(m_pAutoDJView,
            &DlgAutoDJ::loadTrack,
            this,
            &AutoDJFeature::loadTrack);
    connect(m_pAutoDJView,
            &DlgAutoDJ::loadTrackToPlayer,
            this,
            &LibraryFeature::loadTrackToPlayer);

    connect(m_pAutoDJView,
            &DlgAutoDJ::trackSelected,
            this,
            &AutoDJFeature::trackSelected);
    connect(m_pAutoDJView,
            &DlgAutoDJ::aiMixRequested,
            this,
            &AutoDJFeature::slotAiMixRequested);

    // Be informed when the user wants to add another random track.
    connect(m_pAutoDJProcessor,
            &AutoDJProcessor::randomTrackRequested,
            this,
            &AutoDJFeature::slotRandomQueue);
    connect(m_pAutoDJView,
            &DlgAutoDJ::addRandomTrackButton,
            this,
            &AutoDJFeature::slotAddRandomTrack);

    // Update shortcuts displayed in the context menu
    QKeySequence toggleAutoDJShortcut = QKeySequence(
            keyboard->getKeyboardConfig()->getValueString(ConfigKey("[AutoDJ]", "enabled")),
            QKeySequence::PortableText);
    m_pEnableAutoDJAction->setShortcut(toggleAutoDJShortcut);
    m_pDisableAutoDJAction->setShortcut(toggleAutoDJShortcut);
}

void AutoDJFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* AutoDJFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void AutoDJFeature::activate() {
    //qDebug() << "AutoDJFeature::activate()";
    emit switchToView(m_viewName);
    emit disableSearch();
    emit enableCoverArtDisplay(true);
}

void AutoDJFeature::clear() {
    QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
            tr("Confirmation Clear"),
            tr("Do you really want to remove all tracks from the Auto DJ queue?") +
                    tr("This can not be undone."),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No);
    if (btn == QMessageBox::Yes) {
            m_playlistDao.clearAutoDJQueue();
    }
}

void AutoDJFeature::paste() {
    emit pasteFromSidebar();
}

// Called by SidebarModel
void AutoDJFeature::deleteItem(const QModelIndex& index) {
    TreeItem* pSelectedItem = static_cast<TreeItem*>(index.internalPointer());
    if (!pSelectedItem || pSelectedItem == m_pCratesTreeItem) {
            return;
    }
    CrateId crateId(pSelectedItem->getData());
    removeCrateFromAutoDj(crateId);
}

// Called by deleteItem and slotRemoveCrateFromAutoDj()
void AutoDJFeature::removeCrateFromAutoDj(CrateId crateId) {
    DEBUG_ASSERT(crateId.isValid());
    // TODO Confirm dialog?
    m_pTrackCollection->updateAutoDjCrate(crateId, false);
}

bool AutoDJFeature::dropAccept(const QList<QUrl>& urls, QObject* pSource) {
    // If a track is dropped onto the Auto DJ tree node, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // Auto DJ playlist.
    // pSource != nullptr it is a drop from inside Mixxx and indicates all
    // tracks already in the DB
    const QList<mixxx::FileInfo> fileInfos =
            // collect all tracks, accept playlist files
            DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    const QList<TrackId> trackIds =
            m_pLibrary->trackCollectionManager()->resolveTrackIds(fileInfos, pSource);
    if (trackIds.isEmpty()) {
        return false;
    }

    // Return whether appendTracksToPlaylist succeeded.
    return m_playlistDao.appendTracksToPlaylist(trackIds, m_iAutoDJPlaylistId);
}

bool AutoDJFeature::dragMoveAccept(const QList<QUrl>& urls) {
    return DragAndDropHelper::urlsContainSupportedTrackFiles(urls, true);
}

void AutoDJFeature::slotEnableAutoDJ() {
    m_pAutoDJProcessor->toggleAutoDJ(true);
}

void AutoDJFeature::slotDisableAutoDJ() {
    m_pAutoDJProcessor->toggleAutoDJ(false);
}

void AutoDJFeature::slotClearQueue() {
    clear();
}

// Add a crate to the AutoDJ sources
void AutoDJFeature::slotAddCrateToAutoDj(CrateId crateId) {
    m_pTrackCollection->updateAutoDjCrate(crateId, true);
}

void AutoDJFeature::slotRemoveCrateFromAutoDj() {
    CrateId crateId(m_pRemoveCrateFromAutoDjAction->data());
    removeCrateFromAutoDj(crateId);
}

void AutoDJFeature::slotCrateChanged(CrateId crateId) {
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(crateId, &crate) && crate.isAutoDjSource()) {
        // Crate exists and is already a source for AutoDJ
        // -> Find and update the corresponding child item
        for (int i = 0; i < m_crateList.length(); ++i) {
            if (m_crateList[i].getId() == crateId) {
                QModelIndex parentIndex = m_pSidebarModel->index(0, 0);
                QModelIndex childIndex = m_pSidebarModel->index(i, 0, parentIndex);
                m_pSidebarModel->setData(childIndex, crate.getName(), Qt::DisplayRole);
                m_crateList[i] = crate;
                return; // early exit
            }
        }
        // No child item for crate found
        // -> Create and append a new child item for this crate
        // TODO() Use here std::span to get around the heap alloctaion of
        // std::vector for a single element.
        std::vector<std::unique_ptr<TreeItem>> rows;
        rows.push_back(std::make_unique<TreeItem>(crate.getName(), crate.getId().toVariant()));
        QModelIndex parentIndex = m_pSidebarModel->index(0, 0);
        m_pSidebarModel->insertTreeItemRows(std::move(rows), m_crateList.length(), parentIndex);
        m_crateList.append(crate);
    } else {
        // Crate does not exist or is not a source for AutoDJ
        // -> Find and remove the corresponding child item
        for (int i = 0; i < m_crateList.length(); ++i) {
            if (m_crateList[i].getId() == crateId) {
                QModelIndex parentIndex = m_pSidebarModel->index(0, 0);
                m_pSidebarModel->removeRows(i, 1, parentIndex);
                m_crateList.removeAt(i);
                return; // early exit
            }
        }
    }
}

void AutoDJFeature::slotAddRandomTrack() {
    if (m_iAutoDJPlaylistId >= 0) {
        TrackPointer pRandomTrack;
        for (int failedRetrieveAttempts = 0;
                !pRandomTrack && (failedRetrieveAttempts < 2 * kMaxRetrieveAttempts); // 2 rounds
                ++failedRetrieveAttempts) {
            const bool fromCrates = !m_crateList.isEmpty();

            // If harmonic mixing is enabled, first try to continue with a track
            // that mixes harmonically with the track currently at the end of
            // the queue, then fall back to the plain random selection if there
            // is no compatible candidate.
            TrackId randomTrackId;
            QList<int> compatibleKeyIds;
            QList<mixxx::HarmonicMixing::BpmRange> bpmRanges;
            if (getHarmonicSelectionConstraints(&compatibleKeyIds, &bpmRanges)) {
                // Try the configured tempo window first and widen it before
                // dropping the tempo constraint, so that the key compatibility
                // is kept even when nothing matches the tempo.
                for (const auto& bpmRange : bpmRanges) {
                    randomTrackId = m_autoDjCratesDao.getRandomTrackIdHarmonic(
                            fromCrates,
                            m_iAutoDJPlaylistId,
                            compatibleKeyIds,
                            bpmRange.isValid() ? bpmRange.min : 0.0,
                            bpmRange.isValid() ? bpmRange.max : 0.0);
                    if (randomTrackId.isValid()) {
                        break;
                    }
                }
            }
            if (!randomTrackId.isValid()) {
                if (fromCrates) {
                    // Fetch track from crates.
                    // We do not fall back to Library if this fails because this
                    // may add banned tracks
                    randomTrackId = m_autoDjCratesDao.getRandomTrackId();
                } else {
                    // Fetch Track from Library since we have no assigned crates
                    randomTrackId = m_autoDjCratesDao.getRandomTrackIdFromLibrary(
                            m_iAutoDJPlaylistId);
                }
            }

            if (randomTrackId.isValid()) {
                pRandomTrack = m_pLibrary->trackCollectionManager()->getTrackById(randomTrackId);
                VERIFY_OR_DEBUG_ASSERT(pRandomTrack) {
                    qWarning() << "Track does not exist:"
                            << randomTrackId;
                    continue;
                }
                if (!pRandomTrack->getFileInfo().checkFileExists()) {
                    qWarning() << "Track does not exist:"
                               << pRandomTrack->getInfo()
                               << pRandomTrack->getLocation();
                    pRandomTrack.reset();
                }
            }
        }
        if (pRandomTrack) {
            m_pTrackCollection->getPlaylistDAO().appendTrackToPlaylist(
                    pRandomTrack->getId(), m_iAutoDJPlaylistId);
            m_pAutoDJView->onShow();
            return; // success
        }
    }
    qWarning() << "Could not load random track.";
}

bool AutoDJFeature::getHarmonicSelectionConstraints(
        QList<int>* pCompatibleKeyIds,
        QList<mixxx::HarmonicMixing::BpmRange>* pBpmRanges) const {
    VERIFY_OR_DEBUG_ASSERT(pCompatibleKeyIds && pBpmRanges) {
        return false;
    }

    if (!m_pConfig->getValue<bool>(ConfigKey("[Auto DJ]", "HarmonicMixing"))) {
        return false;
    }

    // The new track is mixed with the track that is currently at the end of the
    // Auto DJ queue, i.e. the one queued most recently.
    const QList<TrackId> queuedTrackIds =
            m_playlistDao.getTrackIdsInPlaylistOrder(m_iAutoDJPlaylistId);
    if (queuedTrackIds.isEmpty()) {
        return false;
    }
    const TrackPointer pReferenceTrack =
            m_pLibrary->trackCollectionManager()->getTrackById(queuedTrackIds.constLast());
    if (!pReferenceTrack) {
        return false;
    }

    const auto bpmRanges = mixxx::HarmonicMixing::bpmFallbackRanges(
            pReferenceTrack->getBpm(),
            m_pConfig->getValue(
                    ConfigKey("[Auto DJ]", "HarmonicBpmTolerance"),
                    static_cast<int>(mixxx::HarmonicMixing::kDefaultBpmTolerancePercent)));
    if (bpmRanges.isEmpty()) {
        return false;
    }
    const auto compatibleKeys =
            mixxx::HarmonicMixing::compatibleKeys(pReferenceTrack->getKey());
    if (compatibleKeys.isEmpty()) {
        return false;
    }

    pCompatibleKeyIds->clear();
    pCompatibleKeyIds->reserve(compatibleKeys.size());
    for (const auto key : compatibleKeys) {
        pCompatibleKeyIds->append(static_cast<int>(key));
    }
    *pBpmRanges = bpmRanges;
    return true;
}

void AutoDJFeature::constructCrateChildModel() {
    m_crateList.clear();
    CrateSelectResult autoDjCrates(m_pTrackCollection->crates().selectAutoDjCrates(true));
    Crate crate;
    while (autoDjCrates.populateNext(&crate)) {
        // Create the TreeItem for this crate.
        m_pCratesTreeItem->appendChild(crate.getName(), crate.getId().toVariant());
        m_crateList.append(crate);
    }
}

void AutoDJFeature::onRightClick(const QPoint& globalPos) {
    QMenu menu(m_pSidebarWidget);
    if (m_pAutoDJProcessor->getState() == AutoDJProcessor::ADJ_DISABLED) {
        menu.addAction(m_pEnableAutoDJAction.get());
    } else {
        menu.addAction(m_pDisableAutoDJAction.get());
    }
    menu.addAction(m_pClearQueueAction.get());
    menu.exec(globalPos);
}

void AutoDJFeature::onRightClickChild(const QPoint& globalPos,
        const QModelIndex& index) {
    TreeItem* pClickedItem = static_cast<TreeItem*>(index.internalPointer());
    QMenu menu(m_pSidebarWidget);
    if (m_pCratesTreeItem == pClickedItem) {
        // The "Crates" parent item was right-clicked.
        // Bring up the context menu.
        QMenu crateMenu(m_pSidebarWidget);
        crateMenu.setTitle(tr("Add Crate as Track Source"));
        CrateSelectResult nonAutoDjCrates(m_pTrackCollection->crates().selectAutoDjCrates(false));
        Crate crate;
        while (nonAutoDjCrates.populateNext(&crate)) {
            auto pAction = std::make_unique<QAction>(crate.getName(), &crateMenu);
            auto crateId = crate.getId();
            connect(pAction.get(), &QAction::triggered, this, [this, crateId] {
                slotAddCrateToAutoDj(crateId);
            });
            crateMenu.addAction(pAction.get());
            pAction.release();
        }
        menu.addMenu(&crateMenu);
        menu.exec(globalPos);
    } else {
        // A crate child item was right-clicked.
        // Bring up the context menu.
        m_pRemoveCrateFromAutoDjAction->setData(pClickedItem->getData()); // the selected CrateId
        menu.addAction(m_pRemoveCrateFromAutoDjAction);
        menu.exec(globalPos);
    }
}

void AutoDJFeature::slotRandomQueue(int numTracksToAdd) {
    for (int addCount = 0; addCount < numTracksToAdd; ++addCount) {
        slotAddRandomTrack();
    }
}

QJsonArray AutoDJFeature::buildAiCandidateTracks() {
    m_aiCandidateTrackIds.clear();

    LibraryTableModel* pTrackTableModel = m_pLibrary->trackTableModel();
    if (pTrackTableModel == nullptr) {
        return QJsonArray();
    }

    const mixxx::ai::AiMixSettings settings =
            mixxx::ai::AiMixSettings::fromUserSettings(*m_pConfig);
    const int rowCount = qMin(pTrackTableModel->rowCount(), settings.maxCandidateTracks);

    QJsonArray candidates;
    m_aiCandidateTrackIds.reserve(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        const TrackPointer pTrack = pTrackTableModel->getTrack(
                pTrackTableModel->index(row, 0));
        if (pTrack == nullptr) {
            continue;
        }

        mixxx::ai::AiCandidateTrack candidate;
        candidate.index = candidates.size();
        candidate.title = pTrack->getTitle();
        candidate.artist = pTrack->getArtist();
        candidate.genre = pTrack->getGenre();
        candidate.bpm = pTrack->getBpm();
        const mixxx::track::io::key::ChromaticKey key = pTrack->getKey();
        if (key != mixxx::track::io::key::INVALID) {
            // Camelot notation, which is what the system prompt refers to.
            candidate.key = KeyUtils::keyToString(key, KeyUtils::KeyNotation::Lancelot);
        }
        candidate.durationSeconds = static_cast<int>(pTrack->getDuration());

        candidates.append(candidate.toJson());
        m_aiCandidateTrackIds.append(pTrack->getId());
    }
    return candidates;
}

void AutoDJFeature::slotAiMixRequested(const QString& instruction) {
    if (m_pAutoDJView == nullptr) {
        return;
    }

    // Created on demand: setting up the planner during start-up is not only
    // unnecessary, it also runs before Auto DJ itself is ready.
    if (m_pAiMixPlanner == nullptr) {
        m_pAiMixPlanner = new mixxx::ai::AiMixPlanner(this, m_pConfig);
        connect(m_pAiMixPlanner,
                &mixxx::ai::AiMixPlanner::succeeded,
                this,
                &AutoDJFeature::slotAiMixSucceeded);
        connect(m_pAiMixPlanner,
                &mixxx::ai::AiMixPlanner::failed,
                this,
                &AutoDJFeature::slotAiMixFailed);
    }

    const QJsonArray candidates = buildAiCandidateTracks();
    QString error;
    if (!m_pAiMixPlanner->requestSetList(instruction, candidates, &error)) {
        m_pAutoDJView->setAiStatusMessage(error, true);
        return;
    }
    m_pAutoDJView->setAiMixBusy(true);
}

void AutoDJFeature::slotAiMixSucceeded(const QList<int>& trackOrder) {
    if (m_pAutoDJView == nullptr) {
        return;
    }
    m_pAutoDJView->setAiMixBusy(false);

    QList<TrackId> trackIds;
    trackIds.reserve(trackOrder.size());
    for (const int index : trackOrder) {
        if (index >= 0 && index < m_aiCandidateTrackIds.size()) {
            trackIds.append(m_aiCandidateTrackIds.at(index));
        }
    }
    if (trackIds.isEmpty()) {
        m_pAutoDJView->setAiStatusMessage(
                tr("The AI assistant did not pick any track from the library."), true);
        return;
    }

    m_playlistDao.addTracksToAutoDJQueue(trackIds, PlaylistDAO::AutoDJSendLoc::REPLACE);
    m_pAutoDJView->setAiStatusMessage(
            tr("The AI assistant built a set of %n track(s).", "", trackIds.size()),
            false);
}

void AutoDJFeature::slotAiMixFailed(const QString& errorMessage) {
    if (m_pAutoDJView == nullptr) {
        return;
    }
    m_pAutoDJView->setAiMixBusy(false);
    m_pAutoDJView->setAiStatusMessage(errorMessage, true);
}
