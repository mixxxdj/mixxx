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
#include "treeitem.h"

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

    m_pRenamePlaylistAction = new QAction(tr("Rename"),this);
    connect(m_pRenamePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotRenamePlaylist()));

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
    
	//construct child model
    TreeItem *rootItem = new TreeItem("$root","$root", this);

    int idColumn = m_playlistTableModel.record().indexOf("name");
    for (int row = 0; row < m_playlistTableModel.rowCount(); ++row) {
            QModelIndex ind = m_playlistTableModel.index(row, idColumn);
            QString playlist_name = m_playlistTableModel.data(ind).toString();
            TreeItem *playlist_item = new TreeItem(playlist_name, playlist_name, this, rootItem);
            rootItem->appendChild(playlist_item);
            
    }
    m_childModel.setRootItem(rootItem);
}

PlaylistFeature::~PlaylistFeature() {
    delete m_pPlaylistTableModel;
    delete m_pCreatePlaylistAction;
    delete m_pDeletePlaylistAction;
}

QVariant PlaylistFeature::title() {
    return tr("Playlists");
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
    menu.addAction(m_pRenamePlaylistAction);
    menu.addAction(m_pDeletePlaylistAction);
    menu.exec(globalPos);
}

void PlaylistFeature::slotCreatePlaylist() {
    QString name;
    bool validNameGiven = false;

    do {
        bool ok = false;
        name = QInputDialog::getText(NULL,
                                     tr("New Playlist"),
                                     tr("Playlist name:"),
                                     QLineEdit::Normal,
                                     tr("New Playlist"),
                                     &ok).trimmed();
                                             
        if (!ok)
            return;

        int existingId = m_playlistDao.getPlaylistIdFromName(name);
        
        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                 tr("Playlist Creation Failed"),
                                 tr("A playlist by that name already exists."));
        }
        else if (name.isEmpty()) {
            QMessageBox::warning(NULL,
                                 tr("Playlist Creation Failed"),
                                 tr("A playlist cannot have a blank name."));
        }
        else {
            validNameGiven = true;
        }
                                             
    } while (!validNameGiven);

    bool playlistCreated = m_playlistDao.createPlaylist(name);

    if (playlistCreated) {
        clearChildModel();
        m_playlistTableModel.select();
        constructChildModel();
        emit(featureUpdated());
        //Switch the view to the new playlist.
        int playlistId = m_playlistDao.getPlaylistIdFromName(name);
        m_pPlaylistTableModel->setPlaylist(playlistId);
        // TODO(XXX) set sidebar selection
        emit(showTrackModel(m_pPlaylistTableModel));
    }
    else {
        QMessageBox::warning(NULL,
                             tr("Playlist Creation Failed"),
                             tr("An unknown error occurred while creating playlist: ")
                              + name);
    }
}

void PlaylistFeature::slotRenamePlaylist()
{
    qDebug() << "slotRenamePlaylist()";

    QString oldName = m_lastRightClickedIndex.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(oldName);
    
    QString newName;
    bool validNameGiven = false;
    
    do {
        bool ok = false;
        newName = QInputDialog::getText(NULL,
                                        tr("Rename Playlist"),
                                        tr("New playlist name:"),
                                        QLineEdit::Normal,
                                        oldName,
                                        &ok).trimmed();
  
        if (!ok || oldName == newName) {
            return;
        }

        int existingId = m_playlistDao.getPlaylistIdFromName(newName);
        
        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                tr("Renaming Playlist Failed"),
                                tr("A playlist by that name already exists."));
        }
        else if (newName.isEmpty()) {
            QMessageBox::warning(NULL,
                                tr("Renaming Playlist Failed"),
                                tr("A playlist cannot have a blank name."));
        }
        else {
            validNameGiven = true;
        }
    } while (!validNameGiven);

    m_playlistDao.renamePlaylist(playlistId, newName);
    clearChildModel();
    m_playlistTableModel.select();
    constructChildModel();
    emit(featureUpdated());
    m_pPlaylistTableModel->setPlaylist(playlistId);
}

void PlaylistFeature::slotDeletePlaylist()
{
    //qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.data();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = m_playlistDao.getPlaylistIdFromName(m_lastRightClickedIndex.data().toString());
        Q_ASSERT(playlistId >= 0);
        
        clearChildModel();
        m_playlistDao.deletePlaylist(playlistId);
        m_playlistTableModel.select();
        constructChildModel();
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

    // If a track is dropped onto a playlist's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    QFileInfo file(url.toLocalFile());
    QString location = file.absoluteFilePath();

    // XXX: Possible WTF alert - Previously we thought we needed toString() here
    // but what you actually want in any case when converting a QUrl to a file
    // system path is QUrl::toLocalFile(). This is the second time we have
    // flip-flopped on this, but I think toLocalFile() should work in any
    // case. toString() absolutely does not work when you pass the result to a
    // QFileInfo. rryan 9/2010

    int trackId = m_trackDao.getTrackId(location);

    if (trackId == -1) {
        trackId = m_trackDao.addTrack(file);
    }

    if (trackId == -1)
        return false;

    // appendTrackToPlaylist doesn't return whether it succeeded, so assume it
    // did.
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

TreeItemModel* PlaylistFeature::getChildModel() {
    return &m_childModel;
}
/**
  * Purpose: When inserting or removing playlists,
  * we require the sidebar model not to reset.
  * This method queries the database and does dynamic insertion 
*/
void PlaylistFeature::constructChildModel()
{
    QList<QString> data_list;
    int idColumn = m_playlistTableModel.record().indexOf("name");
    for (int row = 0; row < m_playlistTableModel.rowCount(); ++row) {
            QModelIndex ind = m_playlistTableModel.index(row, idColumn);
            QString playlist_name = m_playlistTableModel.data(ind).toString();
            data_list.append(playlist_name);
    }    
    m_childModel.insertRows(data_list, 0, m_playlistTableModel.rowCount());  
}
/**
  * Clears the child model dynamically
  */
void PlaylistFeature::clearChildModel()
{
    m_childModel.removeRows(0,m_playlistTableModel.rowCount());
}
