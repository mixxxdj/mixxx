#include "library/baseexternallibraryfeature.h"

#include "library/basesqltablemodel.h"

BaseExternalLibraryFeature::BaseExternalLibraryFeature(QObject* pParent,
                                                       TrackCollection* pCollection)
        : LibraryFeature(pParent),
          m_pTrackCollection(pCollection) {
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
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(false);
}

void BaseExternalLibraryFeature::slotAddToAutoDJTop() {
    //qDebug() << "slotAddToAutoDJTop() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(true);
}

void BaseExternalLibraryFeature::addToAutoDJ(bool bTop) {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    QString playlist = m_lastRightClickedIndex.data().toString();
    QScopedPointer<BaseSqlTableModel> pPlaylistModelToAdd(
        getPlaylistModelForPlaylist(playlist));
    pPlaylistModelToAdd->select();
    PlaylistDAO &playlistDao = m_pTrackCollection->getPlaylistDAO();
    int autoDJId = playlistDao.getPlaylistIdFromName(AUTODJ_TABLE);

    int rows = pPlaylistModelToAdd->rowCount();
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistModelToAdd->index(i, 0);
        if (!index.isValid()) {
            continue;
        }
        TrackPointer track = pPlaylistModelToAdd->getTrack(index);

        if (!track || track->getId() == -1) {
            continue;
        }

        if (bTop) {
            // Start at position 2 because position 1 was already loaded to the deck
            playlistDao.insertTrackIntoPlaylist(track->getId(), autoDJId, i+2);
        } else {
            playlistDao.appendTrackToPlaylist(track->getId(), autoDJId);
        }
    }
}

void BaseExternalLibraryFeature::slotImportAsMixxxPlaylist() {
    // qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    if (!m_lastRightClickedIndex.isValid()) {
        return;
    }

    QString playlist = m_lastRightClickedIndex.data().toString();
    QScopedPointer<BaseSqlTableModel> pPlaylistModelToAdd(
        getPlaylistModelForPlaylist(playlist));
    pPlaylistModelToAdd->select();
    PlaylistDAO& playlistDao = m_pTrackCollection->getPlaylistDAO();

    int playlistId = playlistDao.getPlaylistIdFromName(playlist);
    int i = 1;

    if (playlistId != -1) {
        // Calculate a unique name
        playlist += "(%1)";
        while (playlistId != -1) {
            i++;
            playlistId = playlistDao.getPlaylistIdFromName(playlist.arg(i));
        }
        playlist = playlist.arg(i);
    }
    playlistId = playlistDao.createPlaylist(playlist);

    if (playlistId != -1) {
        // Copy Tracks
        int rows = pPlaylistModelToAdd->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pPlaylistModelToAdd->index(i,0);
            if (index.isValid()) {
                qDebug() << pPlaylistModelToAdd->getTrackLocation(index);
                TrackPointer track = pPlaylistModelToAdd->getTrack(index);
                playlistDao.appendTrackToPlaylist(track->getId(), playlistId);
            }
        }
    } else {
        // Do not change strings here without also changing strings in
        // src/library/baseplaylistfeature.cpp
        QMessageBox::warning(NULL,
                             tr("Playlist Creation Failed"),
                             tr("An unknown error occurred while creating playlist: ")
                             + playlist);
    }
}


