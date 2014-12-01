// autodjfeature.cpp
// FORK FORK FORK on 11/1/2009 by Albert Santoni (alberts@mixxx.org)
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>
#include <QMetaObject>
#ifdef __AUTODJCRATES__
#include <QMenu>
#endif // __AUTODJCRATES__

#include "library/autodj/autodjfeature.h"

#include "library/parser.h"
#include "playermanager.h"
#include "library/autodj/autodjprocessor.h"
#include "library/trackcollection.h"
#include "dlgautodj.h"
#include "library/treeitem.h"
#include "widget/wlibrary.h"
#include "mixxxkeyboard.h"
#include "soundsourceproxy.h"
#include "util/dnd.h"

const QString AutoDJFeature::m_sAutoDJViewName = QString("Auto DJ");

AutoDJFeature::AutoDJFeature(QObject* parent,
                             ConfigObject<ConfigValue>* pConfig,
                             PlayerManagerInterface* pPlayerManager,
                             TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pConfig(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_crateDao(pTrackCollection->getCrateDAO()),
          m_playlistDao(pTrackCollection->getPlaylistDAO()),
          m_iAutoDJPlaylistId(-1),
          m_pAutoDJProcessor(NULL),
          m_pAutoDJView(NULL)
#ifdef __AUTODJCRATES__
          , m_autoDjCratesDao(pTrackCollection->getDatabase(),
                              pTrackCollection->getTrackDAO(),
                              pTrackCollection->getCrateDAO(),
                              pTrackCollection->getPlaylistDAO(),
                              pConfig)
#endif // __AUTODJCRATES__
{
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

#ifdef __AUTODJCRATES__

    // Create the "Crates" tree-item under the root item.
    TreeItem* root = m_childModel.getItem(QModelIndex());
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

#endif // __AUTODJCRATES__
}

AutoDJFeature::~AutoDJFeature() {
#ifdef __AUTODJCRATES__
    delete m_pRemoveCrateFromAutoDj;
#endif // __AUTODJCRATES__
    delete m_pAutoDJProcessor;
}

QVariant AutoDJFeature::title() {
    return tr("Auto DJ");
}

QIcon AutoDJFeature::getIcon() {
    return QIcon(":/images/library/ic_library_autodj.png");
}

void AutoDJFeature::bindWidget(WLibrary* libraryWidget,
                               MixxxKeyboard* keyboard) {
    m_pAutoDJView = new DlgAutoDJ(libraryWidget,
                                  m_pConfig,
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

#ifdef __AUTODJCRATES__
    // Be informed when the user wants to add another random track.
    connect(m_pAutoDJView, SIGNAL(addRandomButton(bool)),
            this, SLOT(slotAddRandomTrack(bool)));
    connect(this, SIGNAL(enableAddRandom(bool)),
            m_pAutoDJView, SLOT(enableRandomButton(bool)));

    // Let subscribers know whether it's possible to add a random track.
    emit(enableAddRandom(m_crateList.length() > 0));
#endif // __AUTODJCRATES__
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
    TrackDAO &trackDao = m_pTrackCollection->getTrackDAO();

    // If a track is dropped onto a playlist's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    QList<int> trackIds;
    if (pSource) {
        trackIds = trackDao.getTrackIds(files);
        trackDao.unhideTracks(trackIds);
    } else {
        trackIds = trackDao.addTracks(files, true);
    }

    // remove tracks that could not be added
    for (int trackId = 0; trackId < trackIds.size(); trackId++) {
        if (trackIds.at(trackId) < 0) {
            trackIds.removeAt(trackId--);
        }
    }

    // Return whether the tracks were appended.
    return m_playlistDao.appendTracksToPlaylist(trackIds, m_iAutoDJPlaylistId);
}

bool AutoDJFeature::dragMoveAccept(QUrl url) {
    QFileInfo file(url.toLocalFile());
    return SoundSourceProxy::isFilenameSupported(file.fileName()) ||
            Parser::isPlaylistFilenameSupported(file.fileName());
}

// Add a crate to the auto-DJ queue.
void AutoDJFeature::slotAddCrateToAutoDj(int crateId) {
#ifdef __AUTODJCRATES__
    m_crateDao.setCrateInAutoDj(crateId, true);
#endif // __AUTODJCRATES__
}

void AutoDJFeature::slotRemoveCrateFromAutoDj() {
#ifdef __AUTODJCRATES__
    // Get the crate that was right-clicked on.
    QString crateName = m_lastRightClickedIndex.data().toString();

    // Get the ID of that crate.
    int crateId = m_crateDao.getCrateIdByName(crateName);

    // Clear its auto-DJ status.
    m_crateDao.setCrateInAutoDj(crateId, false);
#endif // __AUTODJCRATES__
}

void AutoDJFeature::slotCrateAdded(int crateId) {
#ifdef __AUTODJCRATES__
    // If this newly-added crate is in the auto-DJ queue, add it to the list.
    if (m_crateDao.isCrateInAutoDj(crateId)) {
        slotCrateAutoDjChanged(crateId, true);
    }
#endif // __AUTODJCRATES__
}

// Signaled by the crate DAO when a crate is renamed.
void AutoDJFeature::slotCrateRenamed(int crateId, QString newName) {
#ifdef __AUTODJCRATES__
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
#endif // __AUTODJCRATES__
}

void AutoDJFeature::slotCrateDeleted(int crateId) {
#ifdef __AUTODJCRATES__
    // The crate can't be queried for its auto-DJ status, because it's been
    // deleted by the time this code is reached.  But we can handle that.
    // Another solution would be to add a "crateDeleting" signal to CrateDAO.
    slotCrateAutoDjChanged(crateId, false);
#endif // __AUTODJCRATES__
}

void AutoDJFeature::slotCrateAutoDjChanged(int crateId, bool added) {
#ifdef __AUTODJCRATES__
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

    // Let subscribers know whether it's possible to add a random track.
    emit(enableAddRandom(m_crateList.length() > 0));
#endif // __AUTODJCRATES__
}

void AutoDJFeature::slotAddRandomTrack(bool) {
#ifdef __AUTODJCRATES__
    // Get access to the auto-DJ playlist.
    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();
    if (m_iAutoDJPlaylistId >= 0) {
        // Get the ID of a randomly-selected track.
        int iTrackId = m_autoDjCratesDao.getRandomTrackId();
        if (iTrackId != -1) {
            // Add this randomly-selected track to the auto-DJ playlist.
            playlistDao.appendTrackToPlaylist(iTrackId, m_iAutoDJPlaylistId);

            // Display the newly-added track.
            m_pAutoDJView->onShow();
        }
    }
#endif // __AUTODJCRATES__
}

#ifdef __AUTODJCRATES__

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
    m_lastRightClickedIndex = index;

    TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
    QString crateName = item->dataPath().toString();
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

#endif // __AUTODJCRATES__
