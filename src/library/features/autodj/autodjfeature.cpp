// autodjfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMetaObject>
#include <QMenu>
#include <QScrollArea>
#include <QSplitter>

#include "library/features/autodj/autodjfeature.h"

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/autodj/autodjprocessor.h"
#include "library/features/autodj/dlgautodj.h"
#include "library/parser.h"
#include "library/trackcollection.h"
#include "mixer/playermanager.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "widget/wlibrarysidebar.h"

static const int kMaxRetrieveAttempts = 3;

AutoDJFeature::AutoDJFeature(UserSettingsPointer pConfig,
                             Library* pLibrary,
                             QObject* parent,
                             PlayerManagerInterface* pPlayerManager,
                             TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_pTrackCollection(pTrackCollection),
          m_crateDao(pTrackCollection->getCrateDAO()),
          m_playlistDao(pTrackCollection->getPlaylistDAO()),
          m_iAutoDJPlaylistId(-1),
          m_pAutoDJProcessor(nullptr),
          m_pAutoDJView(nullptr),
          m_autoDjCratesDao(pTrackCollection->getDatabase(),
                            pTrackCollection->getTrackDAO(),
                            pTrackCollection->getCrateDAO(),
                            pTrackCollection->getPlaylistDAO(),
                            pConfig) {
    m_iAutoDJPlaylistId = m_playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);
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
    TreeItem* root = m_childModel.getItem(QModelIndex());
    root->setLibraryFeature(this);
    
    m_pCratesTreeItem = new TreeItem(tr("Crates"), "", this, root);
    m_pCratesTreeItem->setIcon(QIcon(":/images/library/ic_library_crates.png"));
    root->appendChild(m_pCratesTreeItem);

    // Create tree-items under "Crates".
    constructCrateChildModel();

    // Be notified when the status of crates changes.
    connect(&m_crateDao, SIGNAL(added(int)),
            this, SLOT(slotCrateAdded(int)));
    connect(&m_crateDao, SIGNAL(renamed(int,QString)),
            this, SLOT(slotCrateRenamed(int,QString)));
    connect(&m_crateDao, SIGNAL(deleted(int)),
            this, SLOT(slotCrateDeleted(int)));
    connect(&m_crateDao, SIGNAL(autoDjChanged(int,bool)),
            this, SLOT(slotCrateAutoDjChanged(int,bool)));

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

QString AutoDJFeature::getIconPath() {
    return ":/images/library/ic_library_autodj.png";
}

QString AutoDJFeature::getSettingsName() const {
    return "AutoDJFeature";
}

QWidget* AutoDJFeature::createPaneWidget(KeyboardEventFilter*, int paneId) {
    WTrackTableView* pTrackTableView = createTableWidget(paneId);
    pTrackTableView->loadTrackModel(m_pAutoDJProcessor->getTableModel());
    
    connect(pTrackTableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, 
            SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
    
    return pTrackTableView;
}

QWidget* AutoDJFeature::createInnerSidebarWidget(KeyboardEventFilter* pKeyboard) {
    QTabWidget* pContainer = new QTabWidget(nullptr);
    
    // Add controls
    m_pAutoDJView = new DlgAutoDJ(pContainer, m_pAutoDJProcessor);
    m_pAutoDJView->installEventFilter(pKeyboard);
    QScrollArea* pScroll = new QScrollArea(pContainer);
    pScroll->setWidget(m_pAutoDJView);
    pScroll->setWidgetResizable(true);
    pContainer->addTab(pScroll, tr("Controls"));
    
    // Add drop target
    WLibrarySidebar* pSidebar = createLibrarySidebarWidget(pKeyboard);
    pSidebar->setParent(pContainer);
    
    pContainer->addTab(pSidebar, tr("Track source"));

    // Be informed when the user wants to add another random track.
    connect(m_pAutoDJProcessor,SIGNAL(randomTrackRequested(int)),
            this,SLOT(slotRandomQueue(int)));
    connect(m_pAutoDJView, SIGNAL(addRandomButton(bool)),
            this, SLOT(slotAddRandomTrack(bool)));
    
    return pContainer;
}

TreeItemModel* AutoDJFeature::getChildModel() {
    return &m_childModel;
}

void AutoDJFeature::activate() {
    //qDebug() << "AutoDJFeature::activate()";
    DEBUG_ASSERT_AND_HANDLE(!m_pAutoDJView.isNull()) {
        return;
    }
    
    m_pAutoDJView->onShow();
    
    switchToFeature();
    showBreadCrumb();
    restoreSearch(QString()); //Null String disables search box
    
}

bool AutoDJFeature::dropAccept(QList<QUrl> urls, QObject* pSource) {
    TrackDAO &trackDao = m_pTrackCollection->getTrackDAO();

    // If a track is dropped onto a playlist's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    QList<TrackId> trackIds;
    if (pSource) {
        trackIds = trackDao.getTrackIds(files);
        trackDao.unhideTracks(trackIds);
    } else {
        trackIds = trackDao.addMultipleTracks(files, true);
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
void AutoDJFeature::slotAddCrateToAutoDj(int crateId) {
    m_crateDao.setCrateInAutoDj(crateId, true);
}

void AutoDJFeature::slotRemoveCrateFromAutoDj() {
    // Get the crate that was right-clicked on.
    QString crateName = m_lastRightClickedIndex.data().toString();

    // Get the ID of that crate.
    int crateId = m_crateDao.getCrateIdByName(crateName);

    // Clear its auto-DJ status.
    m_crateDao.setCrateInAutoDj(crateId, false);
}

void AutoDJFeature::slotCrateAdded(int crateId) {
    // If this newly-added crate is in the auto-DJ queue, add it to the list.
    if (m_crateDao.isCrateInAutoDj(crateId)) {
        slotCrateAutoDjChanged(crateId, true);
    }
}

// Signaled by the crate DAO when a crate is renamed.
void AutoDJFeature::slotCrateRenamed(int crateId, QString newName) {
    // Look for this crate ID in our list.  It's OK if it's not found.
    for (int i = 0; i < m_crateList.length(); ++i) {
        if (m_crateList[i].first == crateId) {
            // Change the name of this crate.
            m_crateList[i].second = newName;

            // Update the display of this crate's name.
            QModelIndex oCratesIndex = m_childModel.index(0, 0);
            QModelIndex oCrateIndex = oCratesIndex.child(i, 0);
            m_childModel.setData(oCrateIndex, QVariant(newName),
                                 Qt::DisplayRole);
            break;
        }
    }
}

void AutoDJFeature::slotCrateDeleted(int crateId) {
    // The crate can't be queried for its auto-DJ status, because it's been
    // deleted by the time this code is reached.  But we can handle that.
    // Another solution would be to add a "crateDeleting" signal to CrateDAO.
    slotCrateAutoDjChanged(crateId, false);
}

void AutoDJFeature::slotCrateAutoDjChanged(int crateId, bool added) {
    if (added) {
        // Get the name of the crate being added to the auto-DJ list.
        QString strName = m_crateDao.crateName(crateId);

        // Get the index of the row where this crate will be inserted into the
        // tree.
        int iRowIndex = m_crateList.length();

        // Add our record of this crate-ID and name.
        m_crateList.append(qMakePair(crateId, strName));

        // Create a tree-item for this crate.
        TreeItem* item = new TreeItem(strName, strName, this,
                                      m_pCratesTreeItem);

        // Prepare to add it to the "Crates" tree-item.
        QList<TreeItem*> lstItems;
        lstItems.append(item);

        // Add it to the "Crates" tree-item.
        QModelIndex oCratesIndex = m_childModel.index(0, 0);
        m_childModel.insertRows(lstItems, iRowIndex, 1, oCratesIndex);
    } else {
        // Look for this crate ID in our list.  It's OK if it's not found.
        for (int i = 0; i < m_crateList.length(); ++i) {
            if (m_crateList[i].first == crateId) {
                // Remove its corresponding tree-item.
                QModelIndex oCratesIndex = m_childModel.index(0, 0);
                m_childModel.removeRows(i, 1, oCratesIndex);

                // Remove our record of this crate-ID and name.
                m_crateList.removeAt(i);
                break;
            }
        }
    }
}
// Adds a random track : this will be faster when there are sufficiently large
// tracks in the crates

void AutoDJFeature::slotAddRandomTrack(bool) {
    int failedRetrieveAttempts = 0;
    // Get access to the auto-DJ playlist
    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    if (m_iAutoDJPlaylistId >= 0) {
        while (failedRetrieveAttempts < kMaxRetrieveAttempts) {
            // Get the ID of a randomly-selected track.
            TrackId trackId(m_autoDjCratesDao.getRandomTrackId());
            if (trackId.isValid()) {
                // Get Track Information
                TrackPointer addedTrack = (m_pTrackCollection->getTrackDAO()).getTrack(trackId);
                if(addedTrack->exists()) {
                    playlistDao.appendTrackToPlaylist(trackId, m_iAutoDJPlaylistId);
                    DEBUG_ASSERT_AND_HANDLE(!m_pAutoDJView.isNull()) {
                        return;
                    }
                    
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
            TrackId trackId(m_autoDjCratesDao.getRandomTrackIdFromLibrary(m_iAutoDJPlaylistId));
            if (trackId.isValid()) {
                TrackPointer addedTrack = m_pTrackCollection->getTrackDAO().getTrack(trackId);
                if(addedTrack->exists()) {
                    if(!addedTrack->getPlayCounter().isPlayed()) {
                        playlistDao.appendTrackToPlaylist(trackId, m_iAutoDJPlaylistId);
                        DEBUG_ASSERT_AND_HANDLE(!m_pAutoDJView.isNull()) {
                            return;
                        }
                        
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
    // Create a crate table-model with a list of crates that have been added
    // to the auto-DJ queue (and are visible).
    QSqlTableModel crateListTableModel(this, m_pTrackCollection->getDatabase());
    crateListTableModel.setTable(CRATE_TABLE);
    crateListTableModel.setSort(crateListTableModel.fieldIndex(CRATETABLE_NAME),
                                Qt::AscendingOrder);
    crateListTableModel.setFilter(CRATETABLE_AUTODJ_SOURCE + " = 1 AND " + CRATETABLE_SHOW + " = 1");
    crateListTableModel.select();
    while (crateListTableModel.canFetchMore()) {
        crateListTableModel.fetchMore();
    }

    QSqlRecord tableModelRecord = crateListTableModel.record();
    int nameColumn = tableModelRecord.indexOf(CRATETABLE_NAME);
    int idColumn = tableModelRecord.indexOf(CRATETABLE_ID);

    // Create a tree-item for each auto-DJ crate.
    for (int row = 0; row < crateListTableModel.rowCount(); ++row) {
        int id = crateListTableModel.data(
            crateListTableModel.index(row, idColumn)).toInt();
        QString name = crateListTableModel.data(
            crateListTableModel.index(row, nameColumn)).toString();
        m_crateList.append(qMakePair(id, name));

        // Create the TreeItem for this crate.
        TreeItem* item = new TreeItem(name, name, this, m_pCratesTreeItem);
        m_pCratesTreeItem->appendChild(item);
    }
}

void AutoDJFeature::onRightClickChild(const QPoint& globalPos,
                                      QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    QString crateName;
    if (index.isValid()) {
        m_lastRightClickedIndex = index;
    
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        DEBUG_ASSERT_AND_HANDLE(item) {
            return;
        }
    
        crateName = item->dataPath().toString();
    }
    
    if (!crateName.isEmpty()) {
        // A crate was right-clicked.
        // Bring up the context menu.
        QMenu menu(nullptr);
        menu.addAction(m_pRemoveCrateFromAutoDj);
        menu.exec(globalPos);
        return;
    } 
    else {
        // The "Crates" tree-item was right-clicked.
        // Bring up the context menu.
        QMenu menu(nullptr);
        QMenu crateMenu(nullptr);
        crateMenu.setTitle(tr("Add Crate as Track Source"));
        QMap<QString,int> crateMap;
        m_crateDao.getAutoDjCrates(false, &crateMap);
        QMapIterator<QString,int> it(crateMap);
        while (it.hasNext()) {
            it.next();
            // No leak because making the menu the parent means they will be
            // auto-deleted
            QAction* pAction = new QAction(it.key(), &crateMenu);
            crateMenu.addAction(pAction);
            m_crateMapper.setMapping(pAction, it.value());
            connect(pAction, SIGNAL(triggered()), &m_crateMapper, SLOT(map()));
        }

        menu.addMenu(&crateMenu);
        menu.exec(globalPos);
    }
}

void AutoDJFeature::slotRandomQueue(int tracksToAdd) {
    while (tracksToAdd > 0) {
        //Will attempt to add tracks
        slotAddRandomTrack(true);
        tracksToAdd -= 1;
    }
}

void AutoDJFeature::selectionChanged(const QItemSelection&, const QItemSelection&) {
    QPointer<WTrackTableView> pTable = getFocusedTable();
    DEBUG_ASSERT_AND_HANDLE(!m_pAutoDJView.isNull() && !pTable.isNull()) {
        return;
    }
    
    m_pAutoDJView->setSelectedRows(pTable->selectionModel()->selectedRows());
}
