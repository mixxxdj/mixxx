// autodjfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMetaObject>
#include <QMenu>

#include "library/autodj/autodjfeature.h"

#include "library/library.h"
#include "library/parser.h"
#include "mixer/playermanager.h"
#include "library/autodj/autodjprocessor.h"
#include "library/trackcollection.h"
#include "library/autodj/dlgautodj.h"
#include "library/treeitem.h"
#include "library/crate/cratestorage.h"
#include "widget/wlibrary.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"

const QString AutoDJFeature::m_sAutoDJViewName = QString("Auto DJ");

namespace {
    const int kMaxRetrieveAttempts = 3;

    int findOrCrateAutoDjPlaylistId(PlaylistDAO& playlistDAO) {
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
                             PlayerManagerInterface* pPlayerManager,
                             TrackCollection* pTrackCollection)
        : LibraryFeature(pLibrary),
          m_pConfig(pConfig),
          m_pLibrary(pLibrary),
          m_pTrackCollection(pTrackCollection),
          m_playlistDao(pTrackCollection->getPlaylistDAO()),
          m_iAutoDJPlaylistId(findOrCrateAutoDjPlaylistId(m_playlistDao)),
          m_pAutoDJProcessor(NULL),
          m_pAutoDJView(NULL),
          m_autoDjCratesDao(m_iAutoDJPlaylistId, pTrackCollection, pConfig),
          m_icon(":/images/library/ic_library_autodj.svg") {

    qRegisterMetaType<AutoDJProcessor::AutoDJState>("AutoDJState");
    m_pAutoDJProcessor = new AutoDJProcessor(
            this, m_pConfig, pPlayerManager, m_iAutoDJPlaylistId, m_pTrackCollection);
    connect(m_pAutoDJProcessor, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    m_playlistDao.setAutoDJProcessor(m_pAutoDJProcessor);

    // Create the "Crates" tree-item under the root item.
    auto pRootItem = std::make_unique<TreeItem>(this);
    m_pCratesTreeItem = pRootItem->appendChild(tr("Crates"));
    m_pCratesTreeItem->setIcon(QIcon(":/images/library/ic_library_crates.svg"));

    // Create tree-items under "Crates".
    constructCrateChildModel();

    m_childModel.setRootItem(std::move(pRootItem));

    // Be notified when the status of crates changes.
    connect(m_pTrackCollection, SIGNAL(crateInserted(CrateId)),
            this, SLOT(slotCrateChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateUpdated(CrateId)),
            this, SLOT(slotCrateChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateDeleted(CrateId)),
            this, SLOT(slotCrateChanged(CrateId)));

    // Create context-menu items to allow crates to be added to, and removed
    // from, the auto-DJ queue.
    connect(&m_crateMapper, SIGNAL(mapped(int)),
            this, SLOT(slotAddCrateToAutoDj(int)));
    m_pRemoveCrateFromAutoDj = new QAction(tr("Remove Crate as Track Source"), this);
    connect(m_pRemoveCrateFromAutoDj, SIGNAL(triggered()),
            this, SLOT(slotRemoveCrateFromAutoDj()));
}

AutoDJFeature::~AutoDJFeature() {
    delete m_pRemoveCrateFromAutoDj;
    delete m_pAutoDJProcessor;
}

QVariant AutoDJFeature::title() {
    return tr("Auto DJ");
}

QIcon AutoDJFeature::getIcon() {
    return m_icon;
}

void AutoDJFeature::bindWidget(WLibrary* libraryWidget,
                               KeyboardEventFilter* keyboard) {
    m_pAutoDJView = new DlgAutoDJ(libraryWidget,
                                  m_pConfig,
                                  m_pLibrary,
                                  m_pAutoDJProcessor,
                                  m_pTrackCollection,
                                  keyboard);
    libraryWidget->registerView(m_sAutoDJViewName, m_pAutoDJView);
    connect(m_pAutoDJView, SIGNAL(loadTrack(TrackPointer)),
            this, SIGNAL(loadTrack(TrackPointer)));
    connect(m_pAutoDJView, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));

    connect(m_pAutoDJView, SIGNAL(trackSelected(TrackPointer)),
            this, SIGNAL(trackSelected(TrackPointer)));

    // Be informed when the user wants to add another random track.
    connect(m_pAutoDJProcessor,SIGNAL(randomTrackRequested(int)),
            this,SLOT(slotRandomQueue(int)));
    connect(m_pAutoDJView, SIGNAL(addRandomButton(bool)),
            this, SLOT(slotAddRandomTrack()));
}

TreeItemModel* AutoDJFeature::getChildModel() {
    return &m_childModel;
}

void AutoDJFeature::activate() {
    //qDebug() << "AutoDJFeature::activate()";
    emit(switchToView(m_sAutoDJViewName));
    emit disableSearch();
    emit(enableCoverArtDisplay(true));
}

bool AutoDJFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    // If a track is dropped onto a playlist's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    QList<TrackId> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
        m_pTrackCollection->unhideTracks(trackIds);
    } else {
        trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(files, true);
    }

    // remove tracks that could not be added
    for (int trackIdIndex = 0; trackIdIndex < trackIds.size(); trackIdIndex++) {
        if (!trackIds.at(trackIdIndex).isValid()) {
            trackIds.removeAt(trackIdIndex--);
        }
    }

    // Return whether the tracks were appended.
    return m_playlistDao.appendTracksToPlaylist(trackIds, m_iAutoDJPlaylistId);
}

bool AutoDJFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

// Add a crate to the auto-DJ queue.
void AutoDJFeature::slotAddCrateToAutoDj(int iCrateId) {
    m_pTrackCollection->updateAutoDjCrate(CrateId(iCrateId), true);
}

void AutoDJFeature::slotRemoveCrateFromAutoDj() {
    CrateId crateId(m_pRemoveCrateFromAutoDj->data());
    DEBUG_ASSERT(crateId.isValid());
    m_pTrackCollection->updateAutoDjCrate(crateId, false);
}

void AutoDJFeature::slotCrateChanged(CrateId crateId) {
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(crateId, &crate) && crate.isAutoDjSource()) {
        // Crate exists and is already a source for AutoDJ
        // -> Find and update the corresponding child item
        for (int i = 0; i < m_crateList.length(); ++i) {
            if (m_crateList[i].getId() == crateId) {
                QModelIndex parentIndex = m_childModel.index(0, 0);
                QModelIndex childIndex = parentIndex.child(i, 0);
                m_childModel.setData(childIndex, crate.getName(), Qt::DisplayRole);
                m_crateList[i] = crate;
                return; // early exit
            }
        }
        // No child item for crate found
        // -> Create and append a new child item for this crate
        QList<TreeItem*> rows;
        rows.append(new TreeItem(this, crate.getName(), crate.getId().toVariant()));
        QModelIndex parentIndex = m_childModel.index(0, 0);
        m_childModel.insertTreeItemRows(rows, m_crateList.length(), parentIndex);
        DEBUG_ASSERT(rows.isEmpty()); // ownership passed to m_childModel
        m_crateList.append(crate);
    } else {
        // Crate does not exist or is not a source for AutoDJ
        // -> Find and remove the corresponding child item
        for (int i = 0; i < m_crateList.length(); ++i) {
            if (m_crateList[i].getId() == crateId) {
                QModelIndex parentIndex = m_childModel.index(0, 0);
                m_childModel.removeRows(i, 1, parentIndex);
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
            TrackId randomTrackId;
            if (m_crateList.isEmpty()) {
                // Fetch Track from Library since we have no assigned crates
                randomTrackId = m_autoDjCratesDao.getRandomTrackIdFromLibrary(
                        m_iAutoDJPlaylistId);
            } else {
                // Fetch track from crates.
                // We do not fall back to Library if this fails because this
                // may add banned tracks
                randomTrackId = m_autoDjCratesDao.getRandomTrackId();
            }

            if (randomTrackId.isValid()) {
                pRandomTrack = m_pTrackCollection->getTrackDAO().getTrack(randomTrackId);
                VERIFY_OR_DEBUG_ASSERT(pRandomTrack) {
                    qWarning() << "Track does not exist:"
                            << randomTrackId;
                    continue;
                }
                if (!pRandomTrack->checkFileExists()) {
                    qWarning() << "Track does not exist:"
                            << pRandomTrack->getInfo()
                            << pRandomTrack->getFileInfo();
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

void AutoDJFeature::onRightClickChild(const QPoint& globalPos,
                                      QModelIndex index) {
    TreeItem* pClickedItem = static_cast<TreeItem*>(index.internalPointer());
    if (m_pCratesTreeItem == pClickedItem) {
        // The "Crates" parent item was right-clicked.
        // Bring up the context menu.
        QMenu crateMenu;
        crateMenu.setTitle(tr("Add Crate as Track Source"));
        CrateSelectResult nonAutoDjCrates(m_pTrackCollection->crates().selectAutoDjCrates(false));
        Crate crate;
        while (nonAutoDjCrates.populateNext(&crate)) {
            auto pAction = std::make_unique<QAction>(crate.getName(), &crateMenu);
            m_crateMapper.setMapping(pAction.get(), crate.getId().value());
            connect(pAction.get(), SIGNAL(triggered()), &m_crateMapper, SLOT(map()));
            crateMenu.addAction(pAction.get());
            pAction.release();
        }
        QMenu contextMenu;
        contextMenu.addMenu(&crateMenu);
        contextMenu.exec(globalPos);
    } else {
        // A crate child item was right-clicked.
        // Bring up the context menu.
        m_pRemoveCrateFromAutoDj->setData(pClickedItem->getData()); // the selected CrateId
        QMenu contextMenu;
        contextMenu.addAction(m_pRemoveCrateFromAutoDj);
        contextMenu.exec(globalPos);
    }
}

void AutoDJFeature::slotRandomQueue(int numTracksToAdd) {
    for (int addCount = 0; addCount < numTracksToAdd; ++addCount) {
        slotAddRandomTrack();
    }
}
