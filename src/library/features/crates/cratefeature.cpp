// cratefeature.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QDesktopServices>

#include "library/features/crates/cratefeature.h"

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/crates/cratetablemodel.h"
#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/parsercsv.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "util/duration.h"
#include "util/time.h"
#include "widget/wlibrarystack.h"
#include "widget/wlibrarytextbrowser.h"

CrateFeature::CrateFeature(UserSettingsPointer pConfig,
                           Library* pLibrary,
                           QObject* parent,
                           TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_pTrackCollection(pTrackCollection),
          m_crateDao(pTrackCollection->getCrateDAO()),
          m_pCrateTableModel(nullptr) {

    m_pCreateCrateAction = new QAction(tr("Create New Crate"),this);
    connect(m_pCreateCrateAction, SIGNAL(triggered()),
            this, SLOT(slotCreateCrate()));

    m_pDeleteCrateAction = new QAction(tr("Remove"),this);
    connect(m_pDeleteCrateAction, SIGNAL(triggered()),
            this, SLOT(slotDeleteCrate()));

    m_pRenameCrateAction = new QAction(tr("Rename"),this);
    connect(m_pRenameCrateAction, SIGNAL(triggered()),
            this, SLOT(slotRenameCrate()));

    m_pLockCrateAction = new QAction(tr("Lock"),this);
    connect(m_pLockCrateAction, SIGNAL(triggered()),
            this, SLOT(slotToggleCrateLock()));

    m_pImportPlaylistAction = new QAction(tr("Import Crate"),this);
    connect(m_pImportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotImportPlaylist()));

    m_pCreateImportPlaylistAction = new QAction(tr("Import Crate"), this);
    connect(m_pCreateImportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotCreateImportCrate()));

    m_pExportPlaylistAction = new QAction(tr("Export Crate"), this);
    connect(m_pExportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotExportPlaylist()));

    m_pExportTrackFilesAction = new QAction(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction, SIGNAL(triggered()),
            this, SLOT(slotExportTrackFiles()));

    m_pDuplicateCrateAction = new QAction(tr("Duplicate"),this);
    connect(m_pDuplicateCrateAction, SIGNAL(triggered()),
            this, SLOT(slotDuplicateCrate()));

    m_pAnalyzeCrateAction = new QAction(tr("Analyze entire Crate"),this);
    connect(m_pAnalyzeCrateAction, SIGNAL(triggered()),
            this, SLOT(slotAnalyzeCrate()));

    m_pAutoDjTrackSource = new QAction(tr("Auto DJ Track Source"),this);
    m_pAutoDjTrackSource->setCheckable(true);
    connect(m_pAutoDjTrackSource, SIGNAL(changed()),
            this, SLOT(slotAutoDjTrackSourceChanged()));

    connect(&m_crateDao, SIGNAL(added(int)),
            this, SLOT(slotCrateTableChanged(int)));

    connect(&m_crateDao, SIGNAL(deleted(int)),
            this, SLOT(slotCrateTableChanged(int)));

    connect(&m_crateDao, SIGNAL(changed(int)),
            this, SLOT(slotCrateContentChanged(int)));

    connect(&m_crateDao, SIGNAL(renamed(int,QString)),
            this, SLOT(slotCrateTableRenamed(int,QString)));

    connect(&m_crateDao, SIGNAL(lockChanged(int)),
            this, SLOT(slotCrateTableChanged(int)));

    // construct child model
    TreeItem *pRootItem = new TreeItem();
    pRootItem->setLibraryFeature(this);
    m_childModel.setRootItem(pRootItem);
    constructChildModel(-1);

    connect(pLibrary, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(slotTrackSelected(TrackPointer)));
}

CrateFeature::~CrateFeature() {
    //delete QActions
    delete m_pExportTrackFilesAction;
    delete m_pCreateCrateAction;
    delete m_pDeleteCrateAction;
    delete m_pRenameCrateAction;
    delete m_pDuplicateCrateAction;
    delete m_pLockCrateAction;
    delete m_pImportPlaylistAction;
    delete m_pAnalyzeCrateAction;
    delete m_pAutoDjTrackSource;
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QString CrateFeature::getIconPath() {
    return ":/images/library/ic_library_crates.png";
}

QString CrateFeature::getSettingsName() const {
    return "CrateFeature";
}

bool CrateFeature::isSinglePane() const {
    return false;
}

int CrateFeature::crateIdFromIndex(QModelIndex index) {
    bool ok = false;
    int crateId = index.data(AbstractRole::RoleDataPath).toInt(&ok);
    return ok ? crateId : -1;
}

bool CrateFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
                Parser::isPlaylistFilenameSupported(url.toLocalFile());
}


bool CrateFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                                   QObject* pSource) {
    int crateId = crateIdFromIndex(index);
    
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    QList<TrackId> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
        m_pTrackCollection->getTrackDAO().unhideTracks(trackIds);
    } else {
        // Adds track, does not insert duplicates, handles unremoving logic.
        trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(files, true);
    }
    //qDebug() << "CrateFeature::dropAcceptChild adding tracks"
    //        << trackIds.size() << " to crate "<< crateId;
    // remove tracks that could not be added
    for (int trackIdIndex = 0; trackIdIndex < trackIds.size(); ++trackIdIndex) {
        if (!trackIds.at(trackIdIndex).isValid()) {
            trackIds.removeAt(trackIdIndex--);
        }
    }
    
    // Request a name for the crate if it's a new crate
    if (crateId < 0) {
        QString name = getValidCrateName();
        if (name.isNull()) {
            return false;
        }
        
        crateId = m_crateDao.createCrate(name);
        // An error happened
        if (crateId < 0) {
            return false;
        }
    }
    
    m_crateDao.addTracksToCrate(crateId, &trackIds);
    return true;
}

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    int crateId = crateIdFromIndex(index);
    bool locked = m_crateDao.isCrateLocked(crateId);
    bool formatSupported = SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
    return !locked && formatSupported;
}

QWidget* CrateFeature::createPaneWidget(KeyboardEventFilter *pKeyboard, 
                                        int paneId) {
    WLibraryStack* pContainer = new WLibraryStack(nullptr);
    m_panes[paneId] = pContainer;
    
    WLibraryTextBrowser* pEdit = new WLibraryTextBrowser(pContainer);
    pEdit->setHtml(getRootViewHtml());
    pEdit->setOpenLinks(false);
    pEdit->installEventFilter(pKeyboard);
    connect(pEdit, SIGNAL(anchorClicked(const QUrl)),
            this, SLOT(htmlLinkClicked(const QUrl)));
    
    m_idBrowse[paneId] = pContainer->addWidget(pEdit);
    
    QWidget* pTable = LibraryFeature::createPaneWidget(pKeyboard, paneId);
    m_idTable[paneId] = pContainer->addWidget(pTable);
    
    return pContainer;
}

TreeItemModel* CrateFeature::getChildModel() {
    return &m_childModel;
}

void CrateFeature::activate() {
    adoptPreselectedPane();

    auto modelIt = m_lastClickedIndex.constFind(m_featurePane);
    if (modelIt != m_lastClickedIndex.constEnd() &&  (*modelIt).isValid()) {
        activateChild(*modelIt);
        return;
    }
    
    showBrowse(m_featurePane);    
    switchToFeature();
    showBreadCrumb();
    restoreSearch(QString()); //disable search on crate home
}

void CrateFeature::activateChild(const QModelIndex& index) {
    adoptPreselectedPane();
    
    m_lastClickedIndex[m_featurePane] = index;
    int crateId = crateIdFromIndex(index);
    if (crateId == -1) {
        return;
    }
    
    m_pCrateTableModel = getTableModel(m_featurePane);
    m_pCrateTableModel->setTableModel(crateId);
    showTable(m_featurePane);
    restoreSearch("");
    showBreadCrumb(index);
    showTrackModel(m_pCrateTableModel);
}

void CrateFeature::invalidateChild() {
    m_lastClickedIndex.clear();
}

void CrateFeature::activateCrate(int crateId) {
    //qDebug() << "CrateFeature::activateCrate()" << crateId;
    m_pCrateTableModel = getTableModel(m_featurePane);
    
    QModelIndex index = indexFromCrateId(crateId);
    if (crateId != -1 && index.isValid()) {
        m_pCrateTableModel->setTableModel(crateId);
        showTrackModel(m_pCrateTableModel);
        // Update selection
        emit(featureSelect(this, m_lastRightClickedIndex));
        activateChild(m_lastRightClickedIndex);
    }
}


void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(nullptr);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction);
    menu.exec(globalPos);
}

void CrateFeature::onRightClickChild(const QPoint& globalPos, const QModelIndex& index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    int crateId = crateIdFromIndex(index);

    bool locked = m_crateDao.isCrateLocked(crateId);

    m_pDeleteCrateAction->setEnabled(!locked);
    m_pRenameCrateAction->setEnabled(!locked);

    bool bAutoDj = m_crateDao.isCrateInAutoDj(crateId);
    m_pAutoDjTrackSource->setChecked(bAutoDj);

    m_pLockCrateAction->setText(locked ? tr("Unlock") : tr("Lock"));

    QMenu menu(nullptr);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    if (crateId >= 0) {
        menu.addAction(m_pRenameCrateAction);
        menu.addAction(m_pDuplicateCrateAction);
        menu.addAction(m_pDeleteCrateAction);
        menu.addAction(m_pLockCrateAction);
        menu.addSeparator();
        menu.addAction(m_pAutoDjTrackSource);
        menu.addSeparator();
        menu.addAction(m_pAnalyzeCrateAction);
        menu.addSeparator();
    }
    menu.addAction(m_pImportPlaylistAction);
    if (crateId >= 0) {
        menu.addAction(m_pExportPlaylistAction);
        menu.addAction(m_pExportTrackFilesAction);
    }
    menu.exec(globalPos);
}

void CrateFeature::slotCreateCrate() {
    QString name = getValidCrateName();
    if (name.isNull()) {
        // The user canceled
        return;
    }

    int crateId = m_crateDao.createCrate(name);
    if (crateId != -1) {
        activateCrate(crateId);
    } else {
        qDebug() << "Error creating crate with name " << name;
        QMessageBox::warning(nullptr,
                             tr("Creating Crate Failed"),
                             tr("An unknown error occurred while creating crate: ")
                             + name);
    }
}

void CrateFeature::slotDeleteCrate() {
    int crateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (crateId == -1) {
        return;
    }

    bool locked = m_crateDao.isCrateLocked(crateId);
    if (locked) {
        qDebug() << "Skipping crate deletion because crate" << crateId << "is locked.";
        return;
    }

    bool deleted = m_crateDao.deleteCrate(crateId);

    if (deleted) {
        // This avoids a bug where the m_lastChildClicked index is still a valid
        // index but it's not true since we just deleted it
        for (auto it = m_crateTableModel.begin();
                it != m_crateTableModel.end(); ++it) {
            if ((*it)->getCrate() == crateId) {
                // Show the browse widget, this avoids a problem when the same
                // playlist is shown twice and gets deleted. One of the panes
                // gets still showing the unexisting crate.
                m_lastClickedIndex[it.key()] = QModelIndex();
                showBrowse(it.key());
            }
        }
        
        activate();
    } else {
        qDebug() << "Failed to delete crateId" << crateId;
    }
}

void CrateFeature::slotRenameCrate() {
    int crateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (crateId == -1) {
        return;
    }
    QString oldName = m_crateDao.crateName(crateId);

    bool locked = m_crateDao.isCrateLocked(crateId);
    if (locked) {
        qDebug() << "Skipping crate rename because crate" << crateId << "is locked.";
        return;
    }

    QString newName;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        newName = QInputDialog::getText(NULL,
                                        tr("Rename Crate"),
                                        tr("Enter new name for crate:"),
                                        QLineEdit::Normal,
                                        oldName,
                                        &ok).trimmed();

        if (!ok || newName == oldName) {
            return;
        }

        int existingId = m_crateDao.getCrateIdByName(newName);

        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                tr("Renaming Crate Failed"),
                                tr("A crate by that name already exists."));
        } else if (newName.isEmpty()) {
            QMessageBox::warning(NULL,
                                tr("Renaming Crate Failed"),
                                tr("A crate cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    if (!m_crateDao.renameCrate(crateId, newName)) {
        qDebug() << "Failed to rename crateId" << crateId;
    }
}

void CrateFeature::slotDuplicateCrate() {
    int oldCrateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (oldCrateId == -1) {
        return;
    }
    QString oldName = m_crateDao.crateName(oldCrateId);

    QString name;
    bool validNameGiven = false;
    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(NULL,
                                     tr("Duplicate Crate"),
                                     tr("Enter name for new crate:"),
                                     QLineEdit::Normal,
                                     //: Appendix to default name when duplicating a crate
                                     oldName + tr("_copy" , "[noun]"),
                                     &ok).trimmed();

        if (!ok || name == oldName) {
            return;
        }

        int existingId = m_crateDao.getCrateIdByName(name);
        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                tr("Renaming Crate Failed"),
                                tr("A crate by that name already exists."));
        } else if (name.isEmpty()) {
            QMessageBox::warning(NULL,
                                tr("Renaming Crate Failed"),
                                tr("A crate cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    int newCrateId = m_crateDao.createCrate(name);
    m_crateDao.copyCrateTracks(oldCrateId, newCrateId);

    if (newCrateId != -1) {
        activateCrate(newCrateId);
    } else {
        qDebug() << "Error creating crate with name " << name;
        QMessageBox::warning(NULL,
                             tr("Creating Crate Failed"),
                             tr("An unknown error occurred while creating crate: ")
                             + name);
    }
}

void CrateFeature::slotToggleCrateLock() {
    int crateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (crateId == -1) {
        return;
    }
    QString crateName = m_crateDao.crateName(crateId);
    bool locked = !m_crateDao.isCrateLocked(crateId);

    if (!m_crateDao.setCrateLocked(crateId, locked)) {
        qDebug() << "Failed to toggle lock of crateId " << crateId;
    }
}

void CrateFeature::slotAutoDjTrackSourceChanged() {
    int crateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (crateId != -1) {
        m_crateDao.setCrateInAutoDj(crateId, m_pAutoDjTrackSource->isChecked());
    }
}

void CrateFeature::buildCrateList() {
    m_crateList.clear();

    QString queryString = QString(
        "CREATE TEMPORARY VIEW IF NOT EXISTS CratesCountsDurations "
        "AS SELECT "
        "  crates.id AS id, "
        "  crates.name AS name, "
        "  LOWER(crates.name) AS sort_name, "
        "  COUNT(case library.mixxx_deleted when 0 then 1 else null end) AS count, "
        "  SUM(case library.mixxx_deleted when 0 then library.duration else 0 end) AS durationSeconds "
        "FROM crates "
        "LEFT JOIN crate_tracks ON crate_tracks.crate_id = crates.id "
        "LEFT JOIN library ON crate_tracks.track_id = library.id "
        "WHERE crates.show=1 "
        "GROUP BY crates.id;");
    QSqlQuery query(m_pTrackCollection->getDatabase());
    if (!query.exec(queryString)) {
        LOG_FAILED_QUERY(query);
    }

    QSqlTableModel crateListTableModel(this, m_pTrackCollection->getDatabase());
    crateListTableModel.setTable("CratesCountsDurations");
    crateListTableModel.setSort(crateListTableModel.fieldIndex("sort_name"),
                                Qt::AscendingOrder);
    crateListTableModel.select();
    while (crateListTableModel.canFetchMore()) {
        crateListTableModel.fetchMore();
    }
    QSqlRecord record = crateListTableModel.record();
    int nameColumn = record.indexOf("name");
    int idColumn = record.indexOf("id");
    int countColumn = record.indexOf("count");
    int durationColumn = record.indexOf("durationSeconds");

    for (int row = 0; row < crateListTableModel.rowCount(); ++row) {
        int id = crateListTableModel.data(
            crateListTableModel.index(row, idColumn)).toInt();
        QString name = crateListTableModel.data(
            crateListTableModel.index(row, nameColumn)).toString();
        int count = crateListTableModel.data(
            crateListTableModel.index(row, countColumn)).toInt();
        int duration = crateListTableModel.data(
            crateListTableModel.index(row, durationColumn)).toInt();
        m_crateList.append(qMakePair(id, QString("%1 (%2) %3")
                                     .arg(name, QString::number(count),
                                             mixxx::Duration::formatSeconds(duration))));
    }
}

/**
  * Purpose: When inserting or removing playlists,
  * we require the sidebar model not to reset.
  * This method queries the database and does dynamic insertion
*/
QModelIndex CrateFeature::constructChildModel(int selected_id) {
    buildCrateList();
    QList<TreeItem*> data_list;
    int selected_row = -1;
    // Access the invisible root item
    TreeItem* root = m_childModel.getItem(QModelIndex());

    int row = 0;
    for (QList<QPair<int, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        int crate_id = it->first;
        QString crate_name = it->second;

        if (selected_id == crate_id) {
            // save index for selection
            selected_row = row;
            m_childModel.index(selected_row, 0);
        }

        // Create the TreeItem whose parent is the invisible root item
        TreeItem* item = new TreeItem(crate_name, QString::number(crate_id), this, root);
        bool locked = m_crateDao.isCrateLocked(crate_id);
        item->setIcon(locked ? QIcon(":/images/library/ic_library_locked.png") : QIcon());
        item->setBold(m_cratesSelectedTrackIsIn.contains(crate_id));
        data_list.append(item);
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_childModel.insertRows(data_list, 0, m_crateList.size());
    if (selected_row == -1) {
        return QModelIndex();
    }
    return m_childModel.index(selected_row, 0);
}

void CrateFeature::updateChildModel(int selected_id) {
    buildCrateList();

    int row = 0;
    for (QList<QPair<int, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        int crate_id = it->first;
        QString crate_name = it->second;

        if (selected_id == crate_id) {
            TreeItem* item = m_childModel.getItem(indexFromCrateId(crate_id));
            item->setData(crate_name, QString::number(crate_id));
            bool locked = m_crateDao.isCrateLocked(crate_id);
            item->setIcon(locked ? QIcon(":/images/library/ic_library_locked.png") : QIcon());

        }

    }
}

/**
  * Clears the child model dynamically
  */
void CrateFeature::clearChildModel() {
    m_childModel.removeRows(0, m_crateList.size());
    m_crateList.clear();
}

void CrateFeature::slotImportPlaylist() {
    //qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();

    QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }

    // Update the import/export crate directory
    QFileInfo fileName(playlistFile);
    m_pConfig->set(ConfigKey("[Library]","LastImportExportCrateDirectory"),
                   ConfigValue(fileName.dir().absolutePath()));

    slotImportPlaylistFile(playlistFile);
    activateChild(m_lastRightClickedIndex);
}

void CrateFeature::slotImportPlaylistFile(const QString &playlist_file) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    Parser* playlist_parser = NULL;

    if (playlist_file.endsWith(".m3u", Qt::CaseInsensitive) ||
        playlist_file.endsWith(".m3u8", Qt::CaseInsensitive)) {
        // .m3u8 is Utf8 representation of an m3u playlist
        playlist_parser = new ParserM3u();
    } else if (playlist_file.endsWith(".pls", Qt::CaseInsensitive)) {
        playlist_parser = new ParserPls();
    } else if (playlist_file.endsWith(".csv", Qt::CaseInsensitive)) {
        playlist_parser = new ParserCsv();
    } else {
        return;
    }

    if (playlist_parser) {
      QList<QString> entries = playlist_parser->parse(playlist_file);
      //qDebug() << "Size of Imported Playlist: " << entries.size();

      //Iterate over the List that holds URLs of playlist entires
      m_pCrateTableModel->addTracks(QModelIndex(), entries);

      //delete the parser object
      delete playlist_parser;
    }
}

void CrateFeature::slotCreateImportCrate() {

    // Get file to read
    QStringList playlist_files = LibraryFeature::getPlaylistFiles();
    if (playlist_files.isEmpty()) {
        return;
    }


    // Set last import directory
    QFileInfo fileName(playlist_files.first());
    m_pConfig->set(ConfigKey("[Library]","LastImportExportCrateDirectory"),
                ConfigValue(fileName.dir().absolutePath()));

    int lastCrateId = -1;

    // For each selected file
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
            int existingId = m_crateDao.getCrateIdByName(name);

            validNameGiven = (existingId == -1);
            ++i;
        }

        lastCrateId = m_crateDao.createCrate(name);

        if (lastCrateId != -1) {
            m_pCrateTableModel->setTableModel(lastCrateId);
        }
        else {
                QMessageBox::warning(NULL,
                                     tr("Crate Creation Failed"),
                                     tr("An unknown error occurred while creating crate: ")
                                      + name);
                return;
        }

        slotImportPlaylistFile(playlistFile);
    }
    activateCrate(lastCrateId);
}

void CrateFeature::slotAnalyzeCrate() {
    if (m_lastRightClickedIndex.isValid()) {
        int crateId = crateIdFromIndex(m_lastRightClickedIndex);
        if (crateId >= 0) {
            QList<TrackId> ids = m_crateDao.getTrackIds(crateId);
            emit(analyzeTracks(ids));
        }
    }
}

void CrateFeature::slotExportPlaylist() {
    int crateId = m_pCrateTableModel->getCrate();
    QString crateName = m_crateDao.crateName(crateId);
    qDebug() << "Export crate" << crateId << crateName;

    QString lastCrateDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportCrateDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString file_location = QFileDialog::getSaveFileName(
        NULL,
        tr("Export Crate"),
        lastCrateDirectory.append("/").append(crateName),
        tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;PLS Playlist (*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"));
    // Exit method if user cancelled the open dialog.
    if (file_location.isNull() || file_location.isEmpty()) {
        return;
    }

    // Update the import/export crate directory
    QFileInfo fileName(file_location);
    m_pConfig->set(ConfigKey("[Library]","LastImportExportCrateDirectory"),
                ConfigValue(fileName.dir().absolutePath()));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // check config if relative paths are desired
    bool useRelativePath = static_cast<bool>(
        m_pConfig->getValueString(
            ConfigKey("[Library]", "UseRelativePathOnExport")).toInt());

    // Create list of files of the crate
    QList<QString> playlist_items;
    // Create a new table model since the main one might have an active search.
    QScopedPointer<CrateTableModel> pCrateTableModel(
        new CrateTableModel(this, m_pTrackCollection));
    pCrateTableModel->setTableModel(m_pCrateTableModel->getCrate());
    pCrateTableModel->select();

    if (file_location.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(file_location, pCrateTableModel.data(), useRelativePath);
    } else if (file_location.endsWith(".txt", Qt::CaseInsensitive)) {
        ParserCsv::writeReadableTextFile(file_location, pCrateTableModel.data(), false);
    } else{
        // populate a list of files of the crate
        QList<QString> playlist_items;
        int rows = pCrateTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = m_pCrateTableModel->index(i, 0);
            playlist_items << m_pCrateTableModel->getTrackLocation(index);
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

void CrateFeature::slotExportTrackFiles() {
    // Create a new table model since the main one might have an active search.
    QScopedPointer<CrateTableModel> pCrateTableModel(
        new CrateTableModel(this, m_pTrackCollection));
    pCrateTableModel->setTableModel(m_pCrateTableModel->getCrate());
    pCrateTableModel->select();

    int rows = pCrateTableModel->rowCount();
    QList<TrackPointer> trackpointers;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = m_pCrateTableModel->index(i, 0);
        trackpointers.push_back(m_pCrateTableModel->getTrack(index));
    }

    TrackExportWizard trackExport(nullptr, m_pConfig, trackpointers);
    trackExport.exportTracks();
}

void CrateFeature::slotCrateTableChanged(int crateId) {
    //qDebug() << "slotCrateTableChanged() crateId:" << crateId;
    clearChildModel();
    m_lastRightClickedIndex = constructChildModel(crateId);
}

void CrateFeature::slotCrateContentChanged(int crateId) {
    //qDebug() << "slotCrateContentChanged()crateId:" << crateId;
    updateChildModel(crateId);
}

void CrateFeature::slotCrateTableRenamed(int a_iCrateId,
                                         QString /* a_strName */) {
    activateCrate(a_iCrateId);
}

void CrateFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path())=="create") {
        slotCreateCrate();
    } else {
        qDebug() << "Unknown crate link clicked" << link;
    }
}

QString CrateFeature::getRootViewHtml() const {
    QString cratesTitle = tr("Crates");
    QString cratesSummary = tr("Crates are a great way to help organize the music you want to DJ with.");
    QString cratesSummary2 = tr("Make a crate for your next gig, for your favorite electrohouse tracks, or for your most requested songs.");
    QString cratesSummary3 = tr("Crates let you organize your music however you'd like!");

    QString html;
    QString createCrateLink = tr("Create New Crate");
    html.append(QString("<h2>%1</h2>").arg(cratesTitle));
    html.append("<table border=\"0\" cellpadding=\"5\"><tr><td>");
    html.append(QString("<p>%1</p>").arg(cratesSummary));
    html.append(QString("<p>%1</p>").arg(cratesSummary2));
    html.append(QString("<p>%1</p>").arg(cratesSummary3));
    html.append("</td><td rowspan=\"2\">");
    html.append("<img src=\"qrc:/images/library/crates_art.png\">");
    html.append("</td></tr>");
    html.append(QString("<tr><td><a href=\"create\">%1</a>")
                .arg(createCrateLink));
    html.append("</td></tr></table>");
    return html;
}

void CrateFeature::slotTrackSelected(TrackPointer pTrack) {
    m_pSelectedTrack = pTrack;
    TrackId trackId(pTrack ? pTrack->getId() : TrackId());
    m_crateDao.getCratesTrackIsIn(trackId, &m_cratesSelectedTrackIsIn);

    TreeItem* rootItem = m_childModel.getItem(QModelIndex());
    if (rootItem == nullptr) {
        return;
    }

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    int row = 0;
    for (QList<QPair<int, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        TreeItem* crate = rootItem->child(row);
        if (crate == nullptr) {
            continue;
        }
        int crateId = it->first;
        bool shouldBold = m_cratesSelectedTrackIsIn.contains(crateId);
        crate->setBold(shouldBold);
    }

    m_childModel.triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackPointer());
}

QString CrateFeature::getValidCrateName() {
    QString name;
    bool validNameGiven = false;

    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(nullptr,
                                     tr("Create New Crate"),
                                     tr("Enter name for new crate:"),
                                     QLineEdit::Normal, tr("New Crate"),
                                     &ok).trimmed();

        if (!ok) {
            return QString();
        }

        int existingId = m_crateDao.getCrateIdByName(name);

        if (existingId != -1) {
            QMessageBox::warning(nullptr,
                                 tr("Creating Crate Failed"),
                                 tr("A crate by that name already exists."));
        } else if (name.isEmpty()) {
            QMessageBox::warning(nullptr,
                                 tr("Creating Crate Failed"),
                                 tr("A crate cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }
    return name;
}

QModelIndex CrateFeature::indexFromCrateId(int crateId) {
    int row = 0;
    for (QList<QPair<int, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        int current_id = it->first;
        QString crate_name = it->second;

        if (crateId == current_id) {
            return m_childModel.index(row, 0);
        }
    }
    return QModelIndex();
}

QPointer<CrateTableModel> CrateFeature::getTableModel(int paneId) {
    auto it = m_crateTableModel.find(paneId);
    if (it == m_crateTableModel.end() || it->isNull()) {
        it = m_crateTableModel.insert(paneId,
                                      new CrateTableModel(this, m_pTrackCollection));
    }
    return *it;
}

void CrateFeature::showBrowse(int paneId) {
    auto it = m_panes.find(paneId);
    auto itId = m_idBrowse.find(paneId);
    if (it != m_panes.end() && !it->isNull() && itId != m_idBrowse.end()) {
        (*it)->setCurrentIndex(*itId);
    }
}

void CrateFeature::showTable(int paneId) {
    auto it = m_panes.find(paneId);
    auto itId = m_idTable.find(paneId);
    if (it != m_panes.end() && !it->isNull() && itId != m_idTable.end()) {
        (*it)->setCurrentIndex(*itId);
    }
}
