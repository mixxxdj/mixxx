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
          m_iAutoDJPlaylistId(-1),
          m_pAutoDJProcessor(NULL),
          m_pAutoDJView(NULL),
          m_autoDjCratesDao(pTrackCollection, pConfig) {
    m_autoDjCratesDao.initialize();

    m_iAutoDJPlaylistId = m_autoDjCratesDao.getPlaylistId();
    // If the AutoDJ playlist does not exist yet then create it.
    if (m_iAutoDJPlaylistId < 0) {
        m_iAutoDJPlaylistId = m_playlistDao.createPlaylist(
                AUTODJ_TABLE, PlaylistDAO::PLHT_AUTO_DJ);
    }
    qRegisterMetaType<AutoDJProcessor::AutoDJState>("AutoDJState");
    m_pAutoDJProcessor = new AutoDJProcessor(
            this, m_pConfig, pPlayerManager, m_iAutoDJPlaylistId, m_pTrackCollection);
    connect(m_pAutoDJProcessor, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)),
            this, SIGNAL(loadTrackToPlayer(TrackPointer, QString, bool)));
    m_playlistDao.setAutoDJProcessor(m_pAutoDJProcessor);


    // Create the "Crates" tree-item under the root item.
    auto pRootItem = std::make_unique<TreeItem>(this);
    m_pCratesTreeItem = pRootItem->appendChild(tr("Crates"));
    m_pCratesTreeItem->setIcon(QIcon(":/images/library/ic_library_crates.png"));

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
    return QIcon(":/images/library/ic_library_autodj.png");
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
    emit(restoreSearch(QString())); //Null String disables search box
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
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(CrateId(iCrateId), &crate) && !crate.isAutoDjSource()) {
        crate.setAutoDjSource(true);
        m_pTrackCollection->updateCrate(crate);
    }
}

void AutoDJFeature::slotRemoveCrateFromAutoDj() {
    QString crateName = m_lastRightClickedIndex.data().toString();

    Crate crate;
    if (m_pTrackCollection->crates().readCrateByName(crateName, &crate) && crate.isAutoDjSource()) {
        crate.setAutoDjSource(false);
        m_pTrackCollection->updateCrate(crate);
    }
}

void AutoDJFeature::slotCrateChanged(CrateId crateId) {
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(CrateId(crateId), &crate) && crate.isAutoDjSource()) {
        for (int i = 0; i < m_crateList.length(); ++i) {
            if (m_crateList[i].getId() == crateId) {
                QModelIndex parentIndex = m_childModel.index(0, 0);
                QModelIndex childIndex = parentIndex.child(i, 0);
                m_childModel.setData(childIndex, crate.getName(), Qt::DisplayRole);
                m_crateList[i] = std::move(crate);
                return; // early exit
            }
        }
        auto pItem = std::make_unique<TreeItem>(
                this, crate.getName(), crate.getId().toVariant());
        QList<TreeItem*> rows;
        rows.append(pItem.get());
        QModelIndex parentIndex = m_childModel.index(0, 0);
        m_childModel.insertRows(rows, m_crateList.length(), rows.length(), parentIndex);
        pItem.release();
        m_crateList.append(std::move(crate));
    } else {
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

// Adds a random track : this will be faster when there are sufficiently large
// tracks in the crates
void AutoDJFeature::slotAddRandomTrack() {
    int failedRetrieveAttempts = 0;
    // Get access to the auto-DJ playlist
    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    if (m_iAutoDJPlaylistId >= 0) {
        while (failedRetrieveAttempts < kMaxRetrieveAttempts) {
            // Get the ID of a randomly-selected track.
            TrackId trackId = m_autoDjCratesDao.getRandomTrackId();
            if (trackId.isValid()) {
                // Get Track Information
                TrackPointer addedTrack = (m_pTrackCollection->getTrackDAO()).getTrack(trackId);
                if(addedTrack->exists()) {
                    playlistDao.appendTrackToPlaylist(trackId, m_iAutoDJPlaylistId);
                    m_pAutoDJView->onShow();
                    return;
                } else {
                    qDebug() << "Track does not exist:"
                            << addedTrack->getInfo()
                            << addedTrack->getLocation();
                }
            }
            failedRetrieveAttempts += 1;
        }
        // If we couldn't get a track from the crates , get one from the library
        qDebug () << "Could not load tracks from crates, attempting to load from library.";
        failedRetrieveAttempts = 0;
        while ( failedRetrieveAttempts < kMaxRetrieveAttempts ) {
            TrackId trackId = m_autoDjCratesDao.getRandomTrackIdFromLibrary(m_iAutoDJPlaylistId);
            if (trackId.isValid()) {
                TrackPointer addedTrack = m_pTrackCollection->getTrackDAO().getTrack(trackId);
                if(addedTrack->exists()) {
                    if(!addedTrack->getPlayCounter().isPlayed()) {
                        playlistDao.appendTrackToPlaylist(trackId, m_iAutoDJPlaylistId);
                        m_pAutoDJView->onShow();
                        return;
                    }
                } else {
                    qDebug() << "Track does not exist:"
                            << addedTrack->getInfo()
                            << addedTrack->getLocation();
                }
            }
            failedRetrieveAttempts += 1;
        }
    }
    // If control reaches here it implies that we couldn't load track
    qDebug() << "Could not load random track.";
}

void AutoDJFeature::constructCrateChildModel() {
    m_crateList.clear();
    CrateSelectIterator autoDjCrates(m_pTrackCollection->crates().selectAutoDjCrates(true));
    Crate crate;
    while (autoDjCrates.readNext(&crate)) {
        // Create the TreeItem for this crate.
        m_pCratesTreeItem->appendChild(crate.getName(), crate.getId().toVariant());
        m_crateList.append(std::move(crate));
    }
}

void AutoDJFeature::onRightClickChild(const QPoint& globalPos,
                                      QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    QString crateName = item->getLabel();
    if (crateName.length() > 0) {
        // A crate was right-clicked.
        // Bring up the context menu.
        QMenu menu(NULL);
        menu.addAction(m_pRemoveCrateFromAutoDj);
        menu.exec(globalPos);
    } else {
        // The "Crates" tree-item was right-clicked.
        // Bring up the context menu.
        QMenu menu(NULL);
        QMenu crateMenu(NULL);
        crateMenu.setTitle(tr("Add Crate as Track Source"));
        CrateSelectIterator nonAutoDjCrates(m_pTrackCollection->crates().selectAutoDjCrates(false));
        Crate crate;
        while (nonAutoDjCrates.readNext(&crate)) {
            auto pAction = std::make_unique<QAction>(crate.getName(), &crateMenu);
            m_crateMapper.setMapping(pAction.get(), crate.getId().toInt());
            connect(pAction.get(), SIGNAL(triggered()), &m_crateMapper, SLOT(map()));
            crateMenu.addAction(pAction.get());
            pAction.release();
        }

        menu.addMenu(&crateMenu);
        menu.exec(globalPos);
    }
}

void AutoDJFeature::slotRandomQueue(int numTracksToAdd) {
    while (numTracksToAdd > 0) {
        //Will attempt to add tracks
        slotAddRandomTrack();
        numTracksToAdd -= 1;
    }
}
