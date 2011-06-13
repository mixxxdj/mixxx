#include <QtDebug>
#include <QMenu>
#include <QInputDialog>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDateTime>

#include "library/setlogfeature.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"


#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "mixxxkeyboard.h"
#include "treeitem.h"
#include "soundsourceproxy.h"
#include "playerinfo.h"

const QString SetlogFeature::m_sSetlogViewName = QString("SETLOGHOME");

SetlogFeature::SetlogFeature(QObject* parent, ConfigObject<ConfigValue>* pConfig, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
         // m_pTrackCollection(pTrackCollection),
          m_playlistDao(pTrackCollection->getPlaylistDAO()),
          m_trackDao(pTrackCollection->getTrackDAO()),
          m_pConfig(pConfig),
          m_playlistTableModel(this, pTrackCollection->getDatabase()) {
    m_pPlaylistTableModel = new PlaylistTableModel(this, pTrackCollection);

    m_pAddToAutoDJAction = new QAction(tr("Add to Auto-DJ Queue"),this);
    connect(m_pAddToAutoDJAction, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJ()));

    m_pDeletePlaylistAction = new QAction(tr("Remove"),this);
    connect(m_pDeletePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotDeletePlaylist()));

    m_pRenamePlaylistAction = new QAction(tr("Rename"),this);
    connect(m_pRenamePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotRenamePlaylist()));

    m_pLockPlaylistAction = new QAction(tr("Lock"),this);
    connect(m_pLockPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotTogglePlaylistLock()));

    m_pExportPlaylistAction = new QAction(tr("Export Playlist"), this);
    connect(m_pExportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotExportPlaylist()));


    m_oldTrackIdPlayer[0] = 0;
    m_oldTrackIdPlayer[1] = 0;

    //create a new playlist for today
    QString set_log_name_format;
    QString set_log_name;

    //set_log_name_format = QDate::currentDate().toString(Qt::ISODate) + "_%d";
    set_log_name_format = QDate::currentDate().toString(Qt::ISODate) + "_%1";
    int i = 0;
    int existingId;

    qDebug() << set_log_name_format;

    do{
    	i++;
    	// set_log_name.sprintf(set_log_name_format.toUtf8().data(),i);
    	set_log_name = set_log_name_format.arg(i);
    	existingId = m_playlistDao.getPlaylistIdFromName(set_log_name);
    	qDebug() << set_log_name;
    }
    while( existingId != -1 );

    bool playlistCreated = m_playlistDao.createPlaylist(set_log_name, PlaylistDAO::PLHT_SET_LOG);

    if(!playlistCreated){
    	qDebug() << tr("Playlist Creation Failed");
    	qDebug() << tr("An unknown error occurred while creating playlist: ") << set_log_name;
    }

    m_playlistId = m_playlistDao.getPlaylistIdFromName(set_log_name);

    // Setup the sidebar playlist model
    m_playlistTableModel.setTable("Playlists");
    m_playlistTableModel.setFilter("hidden=2"); // PLHT_SET_LOG
    m_playlistTableModel.setSort(m_playlistTableModel.fieldIndex("name"),
                                 Qt::AscendingOrder);
    m_playlistTableModel.select();

    //construct child model
    TreeItem *rootItem = new TreeItem();
    m_childModel.setRootItem(rootItem);
    constructChildModel();
}

SetlogFeature::~SetlogFeature() {
    delete m_pPlaylistTableModel;
    delete m_pDeletePlaylistAction;
    delete m_pAddToAutoDJAction;
    delete m_pRenamePlaylistAction;
    delete m_pLockPlaylistAction;
}

QVariant SetlogFeature::title() {
    return tr("Set Logs");
}

QIcon SetlogFeature::getIcon() {
    return QIcon(":/images/library/ic_library_playlist.png");
}


void SetlogFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                                 WLibrary* libraryWidget,
                                 MixxxKeyboard* keyboard) {
	Q_UNUSED(keyboard);
	Q_UNUSED(sidebarWidget);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    connect(this, SIGNAL(showPage(const QUrl&)),
            edit, SLOT(setSourc e(const QUrl&)));

    // playposition is from -0.14 to + 1.14
    m_pCOPlayPos1 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel1]", "playposition")));
    m_pCOPlayPos2 = new ControlObjectThreadMain(
                            ControlObject::getControl(ConfigKey("[Channel2]", "playposition")));

    connect(m_pCOPlayPos1, SIGNAL(valueChanged(double)),
    this, SLOT(slotPositionChanged(double)));
    connect(m_pCOPlayPos2, SIGNAL(valueChanged(double)),
    this, SLOT(slotPositionChanged(double)));

    libraryWidget->registerView(m_sSetlogViewName, edit);
    //libraryWidget->
}

void SetlogFeature::activate() {
    emit(showPage(QUrl("qrc:/html/setlogs.html")));
    emit(switchToView(m_sSetlogViewName));
}

void SetlogFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "SetlogFeature::activateChild()" << index;

    //Switch the playlist table model's playlist.
    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);
    m_pPlaylistTableModel->setPlaylist(playlistId);
    emit(showTrackModel(m_pPlaylistTableModel));
}

void SetlogFeature::onRightClick(const QPoint& globalPos) {
	Q_UNUSED(globalPos);
    m_lastRightClickedIndex = QModelIndex();

    //Create the right-click menu
    // QMenu menu(NULL);
    // menu.addAction(m_pCreatePlaylistAction);
    // TODO add something like disable logging
    // menu.exec(globalPos);
}

void SetlogFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    QString playlistName = index.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);


    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    m_pDeletePlaylistAction->setEnabled(!locked);
    m_pRenamePlaylistAction->setEnabled(!locked);

    m_pLockPlaylistAction->setText(locked ? tr("Unlock") : tr("Lock"));


    //Create the right-click menu
    QMenu menu(NULL);
    //menu.addAction(m_pCreatePlaylistAction);
    //menu.addSeparator();
    menu.addAction(m_pAddToAutoDJAction);
    menu.addAction(m_pRenamePlaylistAction);
    if(playlistId != m_playlistId){
    	// Todays playlist should not be locked or deleted
    	menu.addAction(m_pDeletePlaylistAction);
    	menu.addAction(m_pLockPlaylistAction);
    }
    menu.addSeparator();
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}


void SetlogFeature::slotRenamePlaylist()
{
    qDebug() << "slotRenamePlaylist()";

    QString oldName = m_lastRightClickedIndex.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(oldName);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    if (locked) {
        qDebug() << "Skipping playlist rename because playlist" << playlistId << "is locked.";
        return;
    }

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


void SetlogFeature::slotTogglePlaylistLock()
{
    QString playlistName = m_lastRightClickedIndex.data().toString();
    int playlistId = m_playlistDao.getPlaylistIdFromName(playlistName);
    bool locked = !m_playlistDao.isPlaylistLocked(playlistId);

    if (!m_playlistDao.setPlaylistLocked(playlistId, locked)) {
        qDebug() << "Failed to toggle lock of playlistId " << playlistId;
    }

    TreeItem* playlistItem = m_childModel.getItem(m_lastRightClickedIndex);
    playlistItem->setIcon(locked ? QIcon(":/images/library/ic_library_locked.png") : QIcon());
}

void SetlogFeature::slotDeletePlaylist()
{
    //qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.data();
    int playlistId = m_playlistDao.getPlaylistIdFromName(m_lastRightClickedIndex.data().toString());
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    if (locked) {
        qDebug() << "Skipping playlist deletion because playlist" << playlistId << "is locked.";
        return;
    }

    if (m_lastRightClickedIndex.isValid() &&
        !m_playlistDao.isPlaylistLocked(playlistId)) {
        Q_ASSERT(playlistId >= 0);

        clearChildModel();
        m_playlistDao.deletePlaylist(playlistId);
        m_playlistTableModel.select();
        constructChildModel();
        emit(featureUpdated());
    }

}

bool SetlogFeature::dropAccept(QUrl url) {
    Q_UNUSED(url);
	return false;
}

bool SetlogFeature::dropAcceptChild(const QModelIndex& index, QUrl url){
	Q_UNUSED(url);
	Q_UNUSED(index);
	return false;
}

bool SetlogFeature::dragMoveAccept(QUrl url) {
	Q_UNUSED(url);
    return false;
}

bool SetlogFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
	Q_UNUSED(url);
	Q_UNUSED(index);
	return false;
}


TreeItemModel* SetlogFeature::getChildModel() {
    return &m_childModel;
}
/**
  * Purpose: When inserting or removing playlists,
  * we require the sidebar model not to reset.
  * This method queries the database and does dynamic insertion
*/
void SetlogFeature::constructChildModel()
{
    QList<TreeItem*> data_list;
    int nameColumn = m_playlistTableModel.record().indexOf("name");
    int idColumn = m_playlistTableModel.record().indexOf("id");

    //Access the invisible root item
    TreeItem* root = m_childModel.getItem(QModelIndex());
    //Create new TreeItems for the playlists in the database
    for (int row = 0; row < m_playlistTableModel.rowCount(); ++row) {
        QModelIndex ind = m_playlistTableModel.index(row, nameColumn);
        QString playlist_name = m_playlistTableModel.data(ind).toString();
        ind = m_playlistTableModel.index(row, idColumn);
        int playlist_id = m_playlistTableModel.data(ind).toInt();
        bool locked = m_playlistDao.isPlaylistLocked(playlist_id);

        //Create the TreeItem whose parent is the invisible root item
        TreeItem* item = new TreeItem(playlist_name, playlist_name, this, root);
        item->setIcon(locked ? QIcon(":/images/library/ic_library_locked.png") : QIcon());
        data_list.append(item);
    }

    //Append all the newly created TreeItems in a dynamic way to the childmodel
    m_childModel.insertRows(data_list, 0, m_playlistTableModel.rowCount());
}

/**
  * Clears the child model dynamically, but the invisible root item remains
  */
void SetlogFeature::clearChildModel()
{
    m_childModel.removeRows(0,m_playlistTableModel.rowCount());
}

void SetlogFeature::onLazyChildExpandation(const QModelIndex &index){
	Q_UNUSED(index);
    //Nothing to do because the childmodel is not of lazy nature.
}

void SetlogFeature::slotExportPlaylist(){
    qDebug() << "Export playlist" << m_lastRightClickedIndex.data();
    QString file_location = QFileDialog::getSaveFileName(NULL,
                                        tr("Export Playlist"),
                                        QDesktopServices::storageLocation(QDesktopServices::MusicLocation),
                                        tr("M3U Playlist (*.m3u);;PLS Playlist (*.pls)"));
    //Exit method if user cancelled the open dialog.
    if(file_location.isNull() || file_location.isEmpty()) return;
    //create and populate a list of files of the playlist
    QList<QString> playlist_items;
    int rows = m_pPlaylistTableModel->rowCount();
    for(int i = 0; i < rows; ++i){
        QModelIndex index = m_pPlaylistTableModel->index(i,0);
        playlist_items << m_pPlaylistTableModel->getTrackLocation(index);
    }
    //check config if relative paths are desired
    bool useRelativePath = (bool)m_pConfig->getValueString(ConfigKey("[Library]","UseRelativePathOnExport")).toInt();

    if(file_location.endsWith(".m3u", Qt::CaseInsensitive))
    {
        ParserM3u::writeM3UFile(file_location, playlist_items, useRelativePath);
    }
    else if(file_location.endsWith(".pls", Qt::CaseInsensitive))
    {
        ParserPls::writePLSFile(file_location,playlist_items, useRelativePath);
    }
    else
    {
        //default export to M3U if file extension is missing

        qDebug() << "Playlist export: No file extension specified. Appending .m3u "
                 << "and exporting to M3U.";
        file_location.append(".m3u");
        ParserM3u::writeM3UFile(file_location, playlist_items, useRelativePath);
    }

}
void SetlogFeature::slotAddToAutoDJ() {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();

    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = m_playlistDao.getPlaylistIdFromName(
            m_lastRightClickedIndex.data().toString());
        if (playlistId >= 0) {
            m_playlistDao.addToAutoDJQueue(playlistId);
        }
    }
    emit(featureUpdated());
}

void SetlogFeature::slotPositionChanged(double /*value*/){
	TrackPointer currendPlayingTrack;
	int currendPlayingTrackId = 0;

    int deck = PlayerInfo::Instance().getCurrentPlayingDeck();
    if ( deck && deck <= 2) {
        QString chan = QString("[Channel%1]").arg(deck);
        currendPlayingTrack = PlayerInfo::Instance().getTrackInfo(chan);
    	if (currendPlayingTrack )
    	{
    		currendPlayingTrackId = currendPlayingTrack->getId();
    	}
    	if( m_oldTrackIdPlayer[deck-1] != currendPlayingTrackId ){
    		// The audience listens to a new track
    		qDebug() << "The audience listens to track " << currendPlayingTrackId;
    		m_playlistDao.appendTrackToPlaylist(currendPlayingTrackId, m_playlistId);
    		if( m_pPlaylistTableModel->getPlaylistId() == m_playlistId ){
    			// View needs a refresh
    			m_pPlaylistTableModel->select();
    		}
    		m_oldTrackIdPlayer[deck-1] = currendPlayingTrackId;
    	}
    }
}






