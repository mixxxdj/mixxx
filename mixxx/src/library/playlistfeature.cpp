#include <QtDebug>
#include <QMenu>
#include <QInputDialog>

#include "library/playlistfeature.h"

#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextedit.h"
#include "library/trackcollection.h"
#include "library/playlisttablemodel.h"
#include "library/proxytrackmodel.h"

PlaylistFeature::PlaylistFeature(QObject* parent, TrackCollection* pTrackCollection)
        : LibraryFeature(parent),
          m_pTrackCollection(pTrackCollection),
          m_playlistTableModel(this, pTrackCollection->getDatabase()) {
    m_pPlaylistTableModel = new PlaylistTableModel(NULL, pTrackCollection);
    m_pPlaylistModelProxy = new ProxyTrackModel(m_pPlaylistTableModel, false);
    m_pPlaylistModelProxy->setSortCaseSensitivity(Qt::CaseInsensitive);

    m_pCreatePlaylistAction = new QAction(tr("New Playlist"),this);
    connect(m_pCreatePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotCreatePlaylist()));

    m_pDeletePlaylistAction = new QAction(tr("Remove"),this);
    connect(m_pDeletePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotDeletePlaylist()));

    m_playlistTableModel.setTable("Playlists");
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
    return QIcon(":/images/library/rhythmbox.png");
}


void PlaylistFeature::bindWidget(WLibrarySidebar* sidebarWidget,
                                 WLibrary* libraryWidget) {
    WLibraryTextEdit* edit = new WLibraryTextEdit(libraryWidget);
    // connect(this, SIGNAL(showText(const QString&)),
    //         edit, SLOT(setText(const QString&)));
    edit->setText("Playlist help page goes here.");
    libraryWidget->registerView("PLAYLISTHOME", edit);
}

void PlaylistFeature::activate() {
    qDebug("PlaylistFeature::activate()");
    emit(switchToView("PLAYLISTHOME"));
}

void PlaylistFeature::activateChild(const QModelIndex& index) {
    qDebug() << "PlaylistFeature::activateChild()" << index;

    //Switch the playlist table model's playlist.
    QString playlistName = index.data().toString();
    int playlistId = m_pTrackCollection->getPlaylistIdFromName(playlistName);
    m_pPlaylistTableModel->setPlaylist(playlistId);
    emit(showTrackModel(m_pPlaylistModelProxy));
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
        m_pTrackCollection->createPlaylist(name);
        m_playlistTableModel.select();
    }
    emit(featureUpdated());

    //Switch the view to the new playlist.
    int playlistId = m_pTrackCollection->getPlaylistIdFromName(name);
    m_pPlaylistTableModel->setPlaylist(playlistId);
    emit(showTrackModel(m_pPlaylistModelProxy));
}

void PlaylistFeature::slotDeletePlaylist()
{
    qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.data();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = m_pTrackCollection->getPlaylistIdFromName(m_lastRightClickedIndex.data().toString());
        Q_ASSERT(playlistId >= 0);
        m_pTrackCollection->deletePlaylist(playlistId);
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
    int playlistId = m_pTrackCollection->getPlaylistIdFromName(playlistName);
    m_pTrackCollection->appendTrackToPlaylist(url.toLocalFile(), playlistId);
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
