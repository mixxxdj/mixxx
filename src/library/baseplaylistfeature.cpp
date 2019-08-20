#include "library/baseplaylistfeature.h"

#include <QInputDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"
#include "library/playlisttablemodel.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"
#include "util/assert.h"

BasePlaylistFeature::BasePlaylistFeature(QObject* parent,
                                         UserSettingsPointer pConfig,
                                         TrackCollection* pTrackCollection,
                                         QString rootViewName)
        : LibraryFeature(pConfig, parent),
          m_pTrackCollection(pTrackCollection),
          m_playlistDao(pTrackCollection->getPlaylistDAO()),
          m_trackDao(pTrackCollection->getTrackDAO()),
          m_pPlaylistTableModel(NULL),
          m_rootViewName(rootViewName) {
    m_pCreatePlaylistAction = new QAction(tr("Create New Playlist"),this);
    connect(m_pCreatePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotCreatePlaylist()));

    m_pAddToAutoDJAction = new QAction(tr("Add to Auto DJ Queue (bottom)"), this);
    connect(m_pAddToAutoDJAction, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJ()));

    m_pAddToAutoDJTopAction = new QAction(tr("Add to Auto DJ Queue (top)"), this);
    connect(m_pAddToAutoDJTopAction, SIGNAL(triggered()),
            this, SLOT(slotAddToAutoDJTop()));

    m_pDeletePlaylistAction = new QAction(tr("Remove"),this);
    connect(m_pDeletePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotDeletePlaylist()));

    m_pRenamePlaylistAction = new QAction(tr("Rename"),this);
    connect(m_pRenamePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotRenamePlaylist()));

    m_pLockPlaylistAction = new QAction(tr("Lock"),this);
    connect(m_pLockPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotTogglePlaylistLock()));

    m_pDuplicatePlaylistAction = new QAction(tr("Duplicate"), this);
    connect(m_pDuplicatePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotDuplicatePlaylist()));

    m_pImportPlaylistAction = new QAction(tr("Import Playlist"),this);
    connect(m_pImportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotImportPlaylist()));

    m_pCreateImportPlaylistAction = new QAction(tr("Import Playlist"), this);
    connect(m_pCreateImportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotCreateImportPlaylist()));

    m_pExportPlaylistAction = new QAction(tr("Export Playlist"), this);
    connect(m_pExportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotExportPlaylist()));

    m_pExportTrackFilesAction = new QAction(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction, SIGNAL(triggered()),
            this, SLOT(slotExportTrackFiles()));

    m_pAnalyzePlaylistAction = new QAction(tr("Analyze entire Playlist"), this);
    connect(m_pAnalyzePlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotAnalyzePlaylist()));

    connect(&m_playlistDao, SIGNAL(added(int)),
            this, SLOT(slotPlaylistTableChanged(int)));

    connect(&m_playlistDao, SIGNAL(deleted(int)),
            this, SLOT(slotPlaylistTableChanged(int)));

    connect(&m_playlistDao, SIGNAL(renamed(int,QString)),
            this, SLOT(slotPlaylistTableRenamed(int,QString)));

    connect(&m_playlistDao, SIGNAL(changed(int)),
            this, SLOT(slotPlaylistContentChanged(int)));

    connect(&m_playlistDao, SIGNAL(lockChanged(int)),
            this, SLOT(slotPlaylistTableChanged(int)));

    Library* pLibrary = static_cast<Library*>(parent);
    connect(pLibrary, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(slotTrackSelected(TrackPointer)));
    connect(pLibrary, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotResetSelectedTrack()));
}

BasePlaylistFeature::~BasePlaylistFeature() {
    delete m_pPlaylistTableModel;
    delete m_pCreatePlaylistAction;
    delete m_pDeletePlaylistAction;
    delete m_pImportPlaylistAction;
    delete m_pCreateImportPlaylistAction;
    delete m_pExportPlaylistAction;
    delete m_pExportTrackFilesAction;
    delete m_pDuplicatePlaylistAction;
    delete m_pAddToAutoDJAction;
    delete m_pAddToAutoDJTopAction;
    delete m_pRenamePlaylistAction;
    delete m_pLockPlaylistAction;
    delete m_pAnalyzePlaylistAction;
}

int BasePlaylistFeature::playlistIdFromIndex(QModelIndex index) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return -1;
    }

    bool ok = false;
    int playlistId = item->getData().toInt(&ok);
    if (ok) {
        return playlistId;
    } else {
        return -1;
    }
}

void BasePlaylistFeature::activate() {
    emit(switchToView(m_rootViewName));
    emit disableSearch();
    emit(enableCoverArtDisplay(true));
}

void BasePlaylistFeature::activateChild(const QModelIndex& index) {
    //qDebug() << "BasePlaylistFeature::activateChild()" << index;
    int playlistId = playlistIdFromIndex(index);
    if (playlistId != -1 && m_pPlaylistTableModel) {
        m_pPlaylistTableModel->setTableModel(playlistId);
        emit(showTrackModel(m_pPlaylistTableModel));
        emit(enableCoverArtDisplay(true));
    }
}

void BasePlaylistFeature::activatePlaylist(int playlistId) {
    //qDebug() << "BasePlaylistFeature::activatePlaylist()" << playlistId;
    QModelIndex index = indexFromPlaylistId(playlistId);
    if (playlistId != -1 && index.isValid() && m_pPlaylistTableModel) {
        m_pPlaylistTableModel->setTableModel(playlistId);
        emit(showTrackModel(m_pPlaylistTableModel));
        emit(enableCoverArtDisplay(true));
        // Update selection
        emit(featureSelect(this, m_lastRightClickedIndex));
        activateChild(m_lastRightClickedIndex);
    }
}

void BasePlaylistFeature::slotRenamePlaylist() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == -1) {
        return;
    }
    QString oldName = m_playlistDao.getPlaylistName(playlistId);
    bool locked = m_playlistDao.isPlaylistLocked(playlistId);

    if (locked) {
        qDebug() << "Skipping playlist rename because playlist" << playlistId
                 << "is locked.";
        return;
    }
    QString newName;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        newName = QInputDialog::getText(NULL,
                                        tr("Rename Playlist"),
                                        tr("Enter new name for playlist:"),
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
        } else if (newName.isEmpty()) {
            QMessageBox::warning(NULL,
                                tr("Renaming Playlist Failed"),
                                tr("A playlist cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    m_playlistDao.renamePlaylist(playlistId, newName);
}

void BasePlaylistFeature::slotDuplicatePlaylist() {
    int oldPlaylistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (oldPlaylistId == -1) {
        return;
    }

    QString oldName = m_playlistDao.getPlaylistName(oldPlaylistId);

    QString name;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(NULL,
                                     tr("Duplicate Playlist"),
                                     tr("Enter name for new playlist:"),
                                     QLineEdit::Normal,
                                     //: Appendix to default name when duplicating a playlist
                                     oldName + tr("_copy" , "[noun]"),
                                     &ok).trimmed();
        if (!ok || oldName == name) {
            return;
        }

        int existingId = m_playlistDao.getPlaylistIdFromName(name);

        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                 tr("Playlist Creation Failed"),
                                 tr("A playlist by that name already exists."));
        } else if (name.isEmpty()) {
            QMessageBox::warning(NULL,
                                 tr("Playlist Creation Failed"),
                                 tr("A playlist cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    int newPlaylistId = m_playlistDao.createPlaylist(name);

    if (newPlaylistId != -1 &&
        m_playlistDao.copyPlaylistTracks(oldPlaylistId, newPlaylistId)) {
        activatePlaylist(newPlaylistId);
    }
}

void BasePlaylistFeature::slotTogglePlaylistLock() {
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == -1) {
        return;
    }
    bool locked = !m_playlistDao.isPlaylistLocked(playlistId);

    if (!m_playlistDao.setPlaylistLocked(playlistId, locked)) {
        qDebug() << "Failed to toggle lock of playlistId " << playlistId;
    }
}

void BasePlaylistFeature::slotCreatePlaylist() {
    if (!m_pPlaylistTableModel) {
        return;
    }

    QString name;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(NULL,
                                     tr("Create New Playlist"),
                                     tr("Enter name for new playlist:"),
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
        } else if (name.isEmpty()) {
            QMessageBox::warning(NULL,
                                 tr("Playlist Creation Failed"),
                                 tr("A playlist cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    int playlistId = m_playlistDao.createPlaylist(name);

    if (playlistId != -1) {
        activatePlaylist(playlistId);
    } else {
        QMessageBox::warning(NULL,
                             tr("Playlist Creation Failed"),
                             tr("An unknown error occurred while creating playlist: ")
                              + name);
    }
}

void BasePlaylistFeature::slotDeletePlaylist() {
    //qDebug() << "slotDeletePlaylist() row:" << m_lastRightClickedIndex.data();
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == -1) {
        return;
    }

    bool locked = m_playlistDao.isPlaylistLocked(playlistId);
    if (locked) {
        qDebug() << "Skipping playlist deletion because playlist" << playlistId << "is locked.";
        return;
    }

    if (m_lastRightClickedIndex.isValid()) {
        VERIFY_OR_DEBUG_ASSERT(playlistId >= 0) {
            return;
        }

        m_playlistDao.deletePlaylist(playlistId);
        activate();
    }
}

void BasePlaylistFeature::slotImportPlaylist() {
    //qDebug() << "slotImportPlaylist() row:" << m_lastRightClickedIndex.data();

    if (!m_pPlaylistTableModel) {
        return;
    }

    QString playlist_file = getPlaylistFile();
    if (playlist_file.isEmpty()) return;

    // Update the import/export playlist directory
    QFileInfo fileName(playlist_file);
    m_pConfig->set(ConfigKey("[Library]","LastImportExportPlaylistDirectory"),
                ConfigValue(fileName.dir().absolutePath()));

    slotImportPlaylistFile(playlist_file);
    activateChild(m_lastRightClickedIndex);
}

void BasePlaylistFeature::slotImportPlaylistFile(const QString &playlist_file) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    Parser* playlist_parser = NULL;

    if (playlist_file.endsWith(".m3u", Qt::CaseInsensitive) ||
            playlist_file.endsWith(".m3u8", Qt::CaseInsensitive)) {
        playlist_parser = new ParserM3u();
    } else if (playlist_file.endsWith(".pls", Qt::CaseInsensitive)) {
        playlist_parser = new ParserPls();
    } else if (playlist_file.endsWith(".csv", Qt::CaseInsensitive)) {
        playlist_parser = new ParserCsv();
    } else {
        return;
    }

    if (playlist_parser) {
      QStringList entries = playlist_parser->parse(playlist_file);

      // Iterate over the List that holds URLs of playlist entries
      m_pPlaylistTableModel->addTracks(QModelIndex(), entries);

      // delete the parser object
      delete playlist_parser;
    }
}

void BasePlaylistFeature::slotCreateImportPlaylist() {
    if (!m_pPlaylistTableModel) {
        return;
    }

    // Get file to read
    QStringList playlist_files = LibraryFeature::getPlaylistFiles();
    if (playlist_files.isEmpty()) {
        return;
    }

    // Set last import directory
    QFileInfo fileName(playlist_files.first());
    m_pConfig->set(ConfigKey("[Library]","LastImportExportPlaylistDirectory"),
                ConfigValue(fileName.dir().absolutePath()));

    int lastPlaylistId = -1;

    // For each selected element create a different playlist.
    for (const QString& playlistFile : playlist_files) {
        fileName = QFileInfo(playlistFile);

        // Get a valid name
        QString baseName = fileName.baseName();
        QString name;

        bool validNameGiven = false;
        int i = 0;
        while (!validNameGiven) {
            name = baseName;
            if (i != 0) {
                name += QString::number(i);
            }

            // Check name
            int existingId = m_playlistDao.getPlaylistIdFromName(name);

            validNameGiven = (existingId == -1);
            ++i;
        }

        lastPlaylistId = m_playlistDao.createPlaylist(name);
        if (lastPlaylistId != -1 && m_pPlaylistTableModel) {
            m_pPlaylistTableModel->setTableModel(lastPlaylistId);
        }
        else {
                QMessageBox::warning(NULL,
                                     tr("Playlist Creation Failed"),
                                     tr("An unknown error occurred while creating playlist: ")
                                      + name);
                return;
        }

        slotImportPlaylistFile(playlistFile);
    }
    activatePlaylist(lastPlaylistId);
}

void BasePlaylistFeature::slotExportPlaylist() {
    if (!m_pPlaylistTableModel) {
        return;
    }
    int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
    if (playlistId == -1) {
        return;
    }
    QString playlistName = m_playlistDao.getPlaylistName(playlistId);
    qDebug() << "Export playlist" << playlistName;

    QString lastPlaylistDirectory = m_pConfig->getValue(
                ConfigKey("[Library]", "LastImportExportPlaylistDirectory"),
                QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    // Open a dialog to let the user choose the file location for playlist export.
    // The location is set to the last used directory for import/export and the file
    // name to the playlist name.
    QString filefilter = tr("M3U Playlist (*.m3u)");
    QString file_location = QFileDialog::getSaveFileName(
            NULL,
            tr("Export Playlist"),
            lastPlaylistDirectory.append("/").append(playlistName),
            tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;"
            "PLS Playlist (*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"),
            &filefilter);
    // Exit method if user cancelled the open dialog.
    if (file_location.isNull() || file_location.isEmpty()) {
        return;
    }
    // Manually add extension due to bug in QFileDialog
    // via https://bugreports.qt-project.org/browse/QTBUG-27186
    // Can be removed after switch to Qt5
    QFileInfo fileName(file_location);
    if (fileName.suffix().isNull() || fileName.suffix().isEmpty()) {
        QString ext = filefilter.section(".",1,1);
        ext.chop(1);
        file_location.append(".").append(ext);
    }
    // Update the import/export playlist directory
    m_pConfig->set(ConfigKey("[Library]","LastImportExportPlaylistDirectory"),
                ConfigValue(fileName.dir().absolutePath()));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // Create a new table model since the main one might have an active search.
    // This will only export songs that we think exist on default
    QScopedPointer<PlaylistTableModel> pPlaylistTableModel(
        new PlaylistTableModel(this, m_pTrackCollection,
                               "mixxx.db.model.playlist_export"));

    pPlaylistTableModel->setTableModel(m_pPlaylistTableModel->getPlaylist());
    pPlaylistTableModel->setSort(pPlaylistTableModel->fieldIndex(
            ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    pPlaylistTableModel->select();

    // check config if relative paths are desired
    bool useRelativePath = m_pConfig->getValue<bool>(
            ConfigKey("[Library]", "UseRelativePathOnExport"));

    if (file_location.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(file_location, pPlaylistTableModel.data(), useRelativePath);
    } else if (file_location.endsWith(".txt", Qt::CaseInsensitive)) {
        if (m_playlistDao.getHiddenType(pPlaylistTableModel->getPlaylist()) == PlaylistDAO::PLHT_SET_LOG) {
            ParserCsv::writeReadableTextFile(file_location, pPlaylistTableModel.data(), true);
        } else {
            ParserCsv::writeReadableTextFile(file_location, pPlaylistTableModel.data(), false);
        }
    } else {
        // Create and populate a list of files of the playlist
        QList<QString> playlist_items;
        int rows = pPlaylistTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pPlaylistTableModel->index(i, 0);
            playlist_items << pPlaylistTableModel->getTrackLocation(index);
        }

        if (file_location.endsWith(".pls", Qt::CaseInsensitive)) {
            ParserPls::writePLSFile(file_location, playlist_items, useRelativePath);
        } else if (file_location.endsWith(".m3u8", Qt::CaseInsensitive)) {
            ParserM3u::writeM3U8File(file_location, playlist_items, useRelativePath);
        } else {
            //default export to M3U if file extension is missing
            if(!file_location.endsWith(".m3u", Qt::CaseInsensitive))
            {
                qDebug() << "Crate export: No valid file extension specified. Appending .m3u "
                         << "and exporting to M3U.";
                file_location.append(".m3u");
            }
            ParserM3u::writeM3UFile(file_location, playlist_items, useRelativePath);
        }
    }
}

void BasePlaylistFeature::slotExportTrackFiles() {
    QScopedPointer<PlaylistTableModel> pPlaylistTableModel(
        new PlaylistTableModel(this, m_pTrackCollection,
                               "mixxx.db.model.playlist_export"));

    pPlaylistTableModel->setTableModel(m_pPlaylistTableModel->getPlaylist());
    pPlaylistTableModel->setSort(pPlaylistTableModel->fieldIndex(
            ColumnCache::COLUMN_PLAYLISTTRACKSTABLE_POSITION), Qt::AscendingOrder);
    pPlaylistTableModel->select();

    int rows = pPlaylistTableModel->rowCount();
    QList<TrackPointer> tracks;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pPlaylistTableModel->index(i, 0);
        tracks.push_back(pPlaylistTableModel->getTrack(index));
    }

    TrackExportWizard track_export(nullptr, m_pConfig, tracks);
    track_export.exportTracks();
}

void BasePlaylistFeature::slotAddToAutoDJ() {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(false); // Top = True
}

void BasePlaylistFeature::slotAddToAutoDJTop() {
    //qDebug() << "slotAddToAutoDJTop() row:" << m_lastRightClickedIndex.data();
    addToAutoDJ(true); // bTop = True
}

void BasePlaylistFeature::addToAutoDJ(bool bTop) {
    //qDebug() << "slotAddToAutoDJ() row:" << m_lastRightClickedIndex.data();
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
        if (playlistId >= 0) {
            // Insert this playlist
            m_playlistDao.addPlaylistToAutoDJQueue(playlistId, bTop);
        }
    }
}

void BasePlaylistFeature::slotAnalyzePlaylist() {
    if (m_lastRightClickedIndex.isValid()) {
        int playlistId = playlistIdFromIndex(m_lastRightClickedIndex);
        if (playlistId >= 0) {
            QList<TrackId> ids = m_playlistDao.getTrackIds(playlistId);
            emit(analyzeTracks(ids));
        }
    }
}

TreeItemModel* BasePlaylistFeature::getChildModel() {
    return &m_childModel;
}

void BasePlaylistFeature::bindWidget(WLibrary* libraryWidget,
                                     KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(getRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit, SIGNAL(anchorClicked(const QUrl)),
            this, SLOT(htmlLinkClicked(const QUrl)));
    libraryWidget->registerView(m_rootViewName, edit);
}

void BasePlaylistFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreatePlaylist();
    } else {
        qDebug() << "Unknown playlist link clicked" << link.path();
    }
}

/**
  * Purpose: When inserting or removing playlists,
  * we require the sidebar model not to reset.
  * This method queries the database and does dynamic insertion
*/
QModelIndex BasePlaylistFeature::constructChildModel(int selected_id) {
    QList<TreeItem*> data_list;
    int selected_row = -1;

    int row = 0;
    for (const IdAndLabel& idAndLabel : createPlaylistLabels()) {
        int playlistId = idAndLabel.id;
        QString playlistLabel = idAndLabel.label;

        if (selected_id == playlistId) {
            // save index for selection
            selected_row = row;
        }

        // Create the TreeItem whose parent is the invisible root item
        TreeItem* item = new TreeItem(this, playlistLabel, playlistId);
        item->setBold(m_playlistsSelectedTrackIsIn.contains(playlistId));

        decorateChild(item, playlistId);
        data_list.append(item);

        ++row;
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_childModel.insertTreeItemRows(data_list, 0);
    if (selected_row == -1) {
        return QModelIndex();
    }
    return m_childModel.index(selected_row, 0);
}

void BasePlaylistFeature::updateChildModel(int playlistId) {
    QString playlistLabel = fetchPlaylistLabel(playlistId);

    QVariant variantId = QVariant(playlistId);

    for (int row = 0; row < m_childModel.rowCount(); ++row) {
        QModelIndex index = m_childModel.index(row, 0);
        TreeItem* pTreeItem = m_childModel.getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                pTreeItem->getData() == variantId) {
            pTreeItem->setLabel(playlistLabel);
            decorateChild(pTreeItem, playlistId);
        }
    }
}



/**
  * Clears the child model dynamically, but the invisible root item remains
  */
void BasePlaylistFeature::clearChildModel() {
    m_childModel.removeRows(0, m_childModel.rowCount());
}

QModelIndex BasePlaylistFeature::indexFromPlaylistId(int playlistId) {
    QVariant variantId = QVariant(playlistId);
    for (int row = 0; row < m_childModel.rowCount(); ++row) {
        QModelIndex index = m_childModel.index(row, 0);
        TreeItem* pTreeItem = m_childModel.getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                pTreeItem->getData() == variantId) {
            return index;
        }
    }
    return QModelIndex();
}

void BasePlaylistFeature::slotTrackSelected(TrackPointer pTrack) {
    m_pSelectedTrack = pTrack;
    TrackId trackId;
    if (pTrack) {
        trackId = pTrack->getId();
    }
    m_playlistDao.getPlaylistsTrackIsIn(trackId, &m_playlistsSelectedTrackIsIn);

    // Set all playlists the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (int row = 0; row < m_childModel.rowCount(); ++row) {
        QModelIndex index = m_childModel.index(row, 0);
        TreeItem* pTreeItem = m_childModel.getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren()) { // leaf node
            bool ok;
            int playlistId = pTreeItem->getData().toInt(&ok);
            VERIFY_OR_DEBUG_ASSERT(ok) {
                continue;
            }
            bool shouldBold = m_playlistsSelectedTrackIsIn.contains(playlistId);
            pTreeItem->setBold(shouldBold);
        }
    }

    m_childModel.triggerRepaint();
}


void BasePlaylistFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackPointer());
}
