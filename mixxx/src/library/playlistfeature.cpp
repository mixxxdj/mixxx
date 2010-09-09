#include <QtDebug>
#include <QMenu>
#include <QInputDialog>

#include "library/playlistfeature.h"

#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "mixxxkeyboard.h"

PlaylistFeature::PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
         // m_pTrackCollection(pTrackCollection),
          m_playlistDao(pTrackCollection->getPlaylistDAO()),
          m_trackDao(pTrackCollection->getTrackDAO()),
          m_playlistTableModel(this, pTrackCollection->getDatabase()) {
    m_pPlaylistTableModel = new PlaylistTableModel(this, pTrackCollection);

    m_pCreatePlaylistAction = new QAction(tr("New Playlist"),this);
    connect(m_pCreatePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotCreatePlaylist()));

    m_pDeletePlaylistAction = new QAction(tr("Remove"),this);
    connect(m_pDeletePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotDeletePlaylist()));

    // Setup the sidebar playlist model
    m_playlistTableModel.setTable("Playlists");
    m_playlistTableModel.setFilter("hidden=0");
    m_playlistTableModel.removeColumn(m_playlistTableModel.fieldIndex("id"));
    m_playlistTableModel.removeColumn(m_playlistTableModel.fieldIndex("position"));
    m_playlistTableModel.removeColumn(m_playlistTableModel.fieldIndex("date_created"));
    m_playlistTableModel.removeColumn(m_playlistTableModel.fieldIndex("date_modified"));
    m_playlistTableModel.setSort(m_playlistTableModel.fieldIndex("position"),
                                 Qt::AscendingOrder);
    m_playlistTableModel.select();
}

PlaylistFeature::~PlaylistFeature() {
    delete m_pPlaylistTableModel;
    delete m_pCreatePlaylistAction;
    delete m_pDeletePlaylistAction;
}

QVariant PlaylistFeature::title() {
    return "Playlists";
}

QIcon PlaylistFeature::getIcon() {
    return QIcon(":/images/library/ic_library_playlist.png");
}


void PlaylistFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                                 WLibrary* libraryWidget,
                                 MixxxKeyboard* keyboard) {
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    connect(this, SIGNAL(showPage(const QUrl&)),
            edit, SLOT(setSource(const QUrl&)));
    libraryWidget->registerView("PLAYLISTHOME", edit);
}

void PlaylistFeature::activate() {
    emit(showPage(QUrl("qrc:/html/playlists.html")));
    emit(switchToView("PLAYLISTHOME"));
}

void PlaylistFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "PlaylistFeature::activateChild()" << index;

    //Switch the playlist table model's playlist.
    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);
    m_pPlaylistTableModel->setPlaylist(playlistId);
    emit(showTrackModel(m_pPlaylistTableModel));
}

void PlaylistFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();

    //Create the right-click menu
    QMenu menu(NULL);
    menu.addAction(m_pCreatePlaylistAction);
    menu.exec(globalPos);
}

void PlaylistFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    //Create the right-click menu
    QMenu menu(NULL);
    menu.addAction(m_pCreatePlaylistAction);
    menu.addSeparator();
    menu.addAction(m_pDeletePlaylistAction);
    menu.exec(globalPos);
}

void PlaylistFeature::slotCreatePlaylist() {
    QString name = QInputDialog::getText(NULL, tr("New Playlist"), tr("Playlist name:"), QLineEdit::Normal, tr("New Playlist"));
    if (name == "")
        return;
    else {
        m_playlistDao.createPlaylist(name);
        m_playlistTableModel.select();
    }
    emit(featureUpdated());

    //Switch the view to the new playlist.
    int playlistId = m_playlistDao.getPlaylistIdFromName(name);
    m_pPlaylistTableModel->setPlaylist(playlistId);
    // TODO(XXX) set sidebar selection
    emit(showTrackModel(m_pPlaylistTableModel));
}

void PlaylistFeature::slotDeletePlaylist()
{
    //qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.data();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = m_playlistDao.getPlaylistIdFromName(m_lastRightClickedIndex.data().toString());
        Q_ASSERT(playlistId >= 0);
        m_playlistDao.deletePlaylist(playlistId);
        m_playlistTableModel.select();
    }

    emit(featureUpdated());
}

bool PlaylistFeature::dropAccept(QUrl url) {
    return false;
}

bool PlaylistFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    //TODO: Filter by supported formats regex and reject anything that doesn't match.
    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);
    //m_playlistDao.appendTrackToPlaylist(url.toLocalFile(), playlistId);

    //If a track is dropped onto a playlist's name, but the track isn't in the library,
    //then add the track to the library before adding it to the playlist.
    QString location = url.toString();

    //XXX: Possible WTF alert - Windows needs .toString() in the above in order
    //     for drag and drop to work, at least for attached drives.
    //     The code was .toLocalFile() in 1.8.0 Beta2, and that totally broke
    //     drag and drop on Windows, but I don't know if there was a particular
    //     reason why we used .toLocalFile() in the first place.
    //     If you find that you need to change this to fix drag and drop for
    //     a particular platform, please comment and/or platform #ifdef it.
    //      -- Albert, July 05/2010

    if (!m_trackDao.trackExistsInDatabase(location))
    {
        m_trackDao.addTrack(location);
    }
    //Get id of track
    int trackId = m_trackDao.getTrackId(location);

    m_playlistDao.appendTrackToPlaylist(trackId, playlistId);

    return true;
}

bool PlaylistFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool PlaylistFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    //TODO: Filter by supported formats regex and reject anything that doesn't match.
    return true;
}

QAbstractItemModel* PlaylistFeature::getChildModel() {
    return &m_playlistTableModel;
}
