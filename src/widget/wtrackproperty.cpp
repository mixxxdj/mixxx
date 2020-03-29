#include "widget/wtrackproperty.h"

// Qt includes
#include <QMenu>
#include <QtCore/QItemSelectionModel>
#include <QtWidgets/QInputDialog>

// std includes
#include <utility>

// Project includes
#include "control/controlobject.h"
#include "library/dao/playlistdao.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackmodel.h"
#include "track/track.h"
#include "util/dnd.h"
#include "util/desktophelper.h"

WTrackProperty::WTrackProperty(const char* group,
                               UserSettingsPointer pConfig,
                               QWidget* pParent,
                               TrackCollectionManager* pTrackCollectionManager)
        : WLabel(pParent),
          m_pGroup(group),
          m_pConfig(std::move(pConfig)),
          m_pTrackCollectionManager(pTrackCollectionManager),
          m_bPlaylistMenuLoaded(false) {
    setAcceptDrops(true);

    // Setup context menu
    m_pMenu = new QMenu(this);
    m_pPlaylistMenu = new QMenu(this);
    m_pPlaylistMenu->setTitle(tr("Add to Playlist"));
    connect(m_pPlaylistMenu, SIGNAL(aboutToShow()),
            this, SLOT(slotPopulatePlaylistMenu()));

    // Create all the context m_pMenu->actions (stuff that shows up when you
    // right-click)
    createContextMenuActions();
}

WTrackProperty::~WTrackProperty() {
    delete m_pMenu;
    delete m_pFileBrowserAct;
    delete m_pPlaylistMenu;
}

void WTrackProperty::setup(const QDomNode& node, const SkinContext& context) {
    WLabel::setup(node, context);

    m_property = context.selectString(node, "Property");
}

void WTrackProperty::slotTrackLoaded(TrackPointer track) {
    if (track) {
        m_pCurrentTrack = track;
        connect(track.get(),
                &Track::changed,
                this,
                &WTrackProperty::slotTrackChanged);
        updateLabel();
    }
}

void WTrackProperty::slotLoadingTrack(TrackPointer pNewTrack, TrackPointer pOldTrack) {
    Q_UNUSED(pNewTrack);
    Q_UNUSED(pOldTrack);
    if (m_pCurrentTrack) {
        disconnect(m_pCurrentTrack.get(), nullptr, this, nullptr);
    }
    m_pCurrentTrack.reset();
    updateLabel();
}

void WTrackProperty::slotTrackChanged(TrackId trackId) {
    Q_UNUSED(trackId);
    updateLabel();
}

void WTrackProperty::updateLabel() {
    if (m_pCurrentTrack) {
        QVariant property = m_pCurrentTrack->property(m_property.toUtf8().constData());
        if (property.isValid() && property.canConvert(QMetaType::QString)) {
            setText(property.toString());
            return;
        }
    }
    setText("");
}

void WTrackProperty::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && m_pCurrentTrack) {
        DragAndDropHelper::dragTrack(m_pCurrentTrack, this, m_pGroup);
    }
}

void WTrackProperty::dragEnterEvent(QDragEnterEvent *event) {
    DragAndDropHelper::handleTrackDragEnterEvent(event, m_pGroup, m_pConfig);
}

void WTrackProperty::dropEvent(QDropEvent *event) {
    DragAndDropHelper::handleTrackDropEvent(event, *this, m_pGroup, m_pConfig);
}

void WTrackProperty::slotOpenInFileBrowser() {
    QString trackLocation = m_pCurrentTrack->getLocation();
    mixxx::DesktopHelper::openInFileBrowser(QStringList(trackLocation));
}

void WTrackProperty::slotPopulatePlaylistMenu() {
    // The user may open the Playlist submenu, move their cursor away, then
    // return to the Playlist submenu before exiting the track context menu.
    // Avoid querying the database multiple times in that case.
    if (m_bPlaylistMenuLoaded) {
        return;
    }
    m_pPlaylistMenu->clear();
    PlaylistDAO& playlistDao = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO();
    QMap<QString,int> playlists;
    int numPlaylists = playlistDao.playlistCount();
    for (int i = 0; i < numPlaylists; ++i) {
        int iPlaylistId = playlistDao.getPlaylistId(i);
        playlists.insert(playlistDao.getPlaylistName(iPlaylistId), iPlaylistId);
    }
    QMapIterator<QString, int> it(playlists);
    while (it.hasNext()) {
        it.next();
        if (!playlistDao.isHidden(it.value())) {
            // No leak because making the menu the parent means they will be
            // auto-deleted
            auto pAction = new QAction(it.key(), m_pPlaylistMenu);
            bool locked = playlistDao.isPlaylistLocked(it.value());
            pAction->setEnabled(!locked);
            m_pPlaylistMenu->addAction(pAction);
            int iPlaylistId = it.value();
            connect(pAction, &QAction::triggered,
                    this, [this, iPlaylistId] { slotAddToPlaylist(iPlaylistId); });
        }
    }
    m_pPlaylistMenu->addSeparator();
    auto* newPlaylistAction = new QAction(tr("Create New Playlist"), m_pPlaylistMenu);
    m_pPlaylistMenu->addAction(newPlaylistAction);
    connect(newPlaylistAction, &QAction::triggered,
            this, [this] { slotAddToPlaylist(-1); });
    m_bPlaylistMenuLoaded = true;
}

void WTrackProperty::slotAddToPlaylist(int iPlaylistId) {
    const TrackId trackId = m_pCurrentTrack->getId();

    PlaylistDAO& playlistDao = m_pTrackCollectionManager->internalCollection()->getPlaylistDAO();

    if (iPlaylistId == -1) { // i.e. a new playlist is suppose to be created
        QString name;
        bool validNameGiven = false;

        do {
            bool ok = false;
            name = QInputDialog::getText(nullptr,
                                         tr("Create New Playlist"),
                                         tr("Enter name for new playlist:"),
                                         QLineEdit::Normal,
                                         tr("New Playlist"),
                                         &ok).trimmed();
            if (!ok) {
                return;
            }
            if (playlistDao.getPlaylistIdFromName(name) != -1) {
                QMessageBox::warning(nullptr,
                                     tr("Playlist Creation Failed"),
                                     tr("A playlist by that name already exists."));
            } else if (name.isEmpty()) {
                QMessageBox::warning(nullptr,
                                     tr("Playlist Creation Failed"),
                                     tr("A playlist cannot have a blank name."));
            } else {
                validNameGiven = true;
            }
        } while (!validNameGiven);
        iPlaylistId = playlistDao.createPlaylist(name);//-1 is changed to the new playlist ID return from the DAO
        if (iPlaylistId == -1) {
            QMessageBox::warning(nullptr,
                                 tr("Playlist Creation Failed"),
                                 tr("An unknown error occurred while creating playlist: ")
                                 +name);
            return;
        }
    }

    playlistDao.appendTrackToPlaylist(trackId, iPlaylistId);
}

void WTrackProperty::createContextMenuActions() {
    m_pFileBrowserAct = new QAction(tr("Open in File Browser"), this);
    connect(m_pFileBrowserAct, SIGNAL(triggered()),
            this, SLOT(slotOpenInFileBrowser()));
}

void WTrackProperty::contextMenuEvent(QContextMenuEvent *event) {
    if (m_pCurrentTrack) {
        m_pMenu->addAction(m_pFileBrowserAct);
        m_pMenu->addMenu(m_pPlaylistMenu);

        // Create the right-click menu
        m_pMenu->popup(event->globalPos());
    }
}
