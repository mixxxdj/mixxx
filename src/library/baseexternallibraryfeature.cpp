#include "library/baseexternallibraryfeature.h"

#include <QMenu>

#include "library/basesqltablemodel.h"

BaseExternalLibraryFeature::BaseExternalLibraryFeature(QObject* pParent,
                                                       TrackCollection* pTrackCollection)
        : LibraryFeature(pParent),
          m_pTrackCollection(pTrackCollection) {
    m_pAddToAutoDJAction = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAddToAutoDJAction, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJ()));

    m_pAddToAutoDJTopAction = new QAction(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAddToAutoDJTopAction, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJTop()));

    m_pImportAsMixxxPlaylistAction = new QAction(tr("Import Playlist"), this);
    connect(m_pImportAsMixxxPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotImportAsMixxxPlaylist()));
}

BaseExternalLibraryFeature::~BaseExternalLibraryFeature() {
    delete m_pAddToAutoDJAction;
    delete m_pAddToAutoDJTopAction;
    delete m_pImportAsMixxxPlaylistAction;
}

void BaseExternalLibraryFeature::onRightClick(const QPoint& globalPos) {
    Q_UNUSED(globalPos);
    m_lastRightClickedIndex = QModelIndex();
}

void BaseExternalLibraryFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    //Create the right-click menu
    QMenu menu;
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pAddToAutoDJTopAction);
    menu.addSeparator();
    menu.addAction(m_pImportAsMixxxPlaylistAction);
    menu.exec(globalPos);
}

void BaseExternalLibraryFeature::slotAddToAutoDJ() {
    qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(false);
}

void BaseExternalLibraryFeature::slotAddToAutoDJTop() {
    qDebug() << "slotAddToAutoDJTop() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(true);
}

// Must be called from Main thread
void BaseExternalLibraryFeature::addToAutoDJ(bool bTop) {
     qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    QList<int> trackIds;
    QString playlist;
    appendTrackIdsFromRightClickIndex(&trackIds, &playlist);
    if (trackIds.isEmpty()) {
        return;
    }

    m_pTrackCollection->callAsync(
                [&bTop, &trackIds] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        PlaylistDAO &playlistDao = pTrackCollectionPrivate->getPlaylistDAO();
        playlistDao.addTracksToAutoDJQueue(trackIds, bTop);
    });
}

void BaseExternalLibraryFeature::slotImportAsMixxxPlaylist() {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    QList<int> trackIds;
    QString playlist;
    appendTrackIdsFromRightClickIndex(&trackIds, &playlist);
    if (trackIds.isEmpty()) {
        return;
    }

    int playlistId = -1;
    m_pTrackCollection->callAsync(
                [&playlist, & trackIds, &playlistId] (TrackCollectionPrivate* pTrackCollectionPrivate) {
        PlaylistDAO &playlistDao = pTrackCollectionPrivate->getPlaylistDAO();
        playlistId = playlistDao.createUniquePlaylist(&playlist);
        if (playlistId != -1) {
            playlistDao.appendTracksToPlaylist(trackIds, playlistId);
        }
    });

    if (playlistId == -1) {
        // Do not change strings here without also changing strings in
        // src/library/baseplaylistfeature.cpp
        QMessageBox::warning(NULL,
                             tr("Playlist Creation Failed"),
                             tr("An unknown error occurred while creating playlist: ")
                             + playlist);
    }
}

// This is a common function for all external Libraries copied to Mixxx DB
void BaseExternalLibraryFeature::appendTrackIdsFromRightClickIndex(
        QList<int>* trackIds, QString* pPlaylist) {
    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    // Qt::UserRole asks TreeItemModel for the TreeItem's dataPath. We need to
    // use the dataPath because models with nested playlists need to use the
    // full path/name of the playlist.
    *pPlaylist = m_lastRightClickedIndex.data(Qt::UserRole).toString();
    QScopedPointer<BaseSqlTableModel> pPlaylistModelToAdd(
            createPlaylistModelForPlaylist(*pPlaylist));

    if (!pPlaylistModelToAdd || !pPlaylistModelToAdd->initialized()) {
        qDebug() << "BaseExternalLibraryFeature::appendTrackIdsFromRightClickIndex "
                "could not initialize a playlist model for playlist:" << *pPlaylist;
        return;
    }

    // tro's lambda idea. This code calls synchronously!
    m_pTrackCollection->callSync(
             [this, &pPlaylistModelToAdd]
                     (TrackCollectionPrivate* pTrackCollectionPrivate) {
        pPlaylistModelToAdd->select(pTrackCollectionPrivate);
    }, __PRETTY_FUNCTION__);

    // Copy Tracks
    int rows = pPlaylistModelToAdd->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistModelToAdd->index(i,0);
        if (index.isValid()) {
            qDebug() << pPlaylistModelToAdd->getTrackLocation(index);
            TrackPointer track = pPlaylistModelToAdd->getTrack(index);
            if (!track) {
                continue;
            }

            int trackId = track->getId();
            if (trackId == -1) {
                continue;
            }

            trackIds->append(trackId);
        }
    }
}

