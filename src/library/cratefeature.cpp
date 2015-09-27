// cratefeature.cpp
// Created 10/22/2009 by RJ Ryan (rryan@mit.edu)

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QDesktopServices>

#include "library/cratefeature.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"

#include "library/cratetablemodel.h"
#include "library/trackcollection.h"
#include "library/queryutil.h"
#include "widget/wlibrarytextbrowser.h"
#include "widget/wlibrary.h"
#include "mixxxkeyboard.h"
#include "treeitem.h"
#include "soundsourceproxy.h"
#include "util/dnd.h"
#include "util/time.h"

CrateFeature::CrateFeature(Library* pLibrary,
                           TrackCollection* pTrackCollection,
                           ConfigObject<ConfigValue>* pConfig)
        : m_pTrackCollection(pTrackCollection),
          m_crateDao(pTrackCollection->getCrateDAO()),
          m_crateTableModel(this, pTrackCollection),
          m_pConfig(pConfig) {
    m_pCreateCrateAction = new QAction(tr("Create New Crate"),this);
    connect(m_pCreateCrateAction, SIGNAL(triggered()),
            this, SLOT(slotCreateNormalCrate()));

    m_pCreateCrateFolderAction = new QAction(tr("Create New Folder"),this);
    connect(m_pCreateCrateFolderAction, SIGNAL(triggered()),
            this, SLOT(slotCreateFolderOfCrates()));

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

    m_pExportPlaylistAction = new QAction(tr("Export Crate"), this);
    connect(m_pExportPlaylistAction, SIGNAL(triggered()),
            this, SLOT(slotExportPlaylist()));

    m_pDuplicateCrateAction = new QAction(tr("Duplicate"),this);
    connect(m_pDuplicateCrateAction, SIGNAL(triggered()),
            this, SLOT(slotDuplicateCrate()));
/*
    m_pCollapseAllAction = new QAction(tr("Collapse all"), this);
    connect(m_pCollapseAllAction, SIGNAL(triggered()),
            this, SLOT(slotCollapseAll()));

    m_pExpandAllAction = new QAction(tr("Expand all"), this);
    connect(m_pExpandAllAction, SIGNAL(triggered()),
            this, SLOT(slotExpandAll()));
*/
    m_pAnalyzeCrateAction = new QAction(tr("Analyze entire Crate"),this);
    connect(m_pAnalyzeCrateAction, SIGNAL(triggered()),
            this, SLOT(slotAnalyzeCrate()));

#ifdef __AUTODJCRATES__

    m_pAutoDjTrackSource = new QAction(tr("Auto DJ Track Source"),this);
    m_pAutoDjTrackSource->setCheckable(true);
    connect(m_pAutoDjTrackSource, SIGNAL(changed()),
            this, SLOT(slotAutoDjTrackSourceChanged()));

#endif // __AUTODJCRATES__

    connect(&m_crateDao, SIGNAL(added(int)),
            this, SLOT(slotCrateTableChanged(int)));

    connect(&m_crateDao, SIGNAL(deleted(int)),
            this, SLOT(slotCrateTableChanged(int)));

    connect(&m_crateDao, SIGNAL(changed(int)),
            this, SLOT(slotCrateTableChanged(int)));

    connect(&m_crateDao, SIGNAL(renamed(int,QString)),
            this, SLOT(slotCrateTableRenamed(int,QString)));

    connect(&m_crateDao, SIGNAL(lockChanged(int)),
            this, SLOT(slotCrateTableChanged(int)));

    // construct child model
    TreeItem *rootItem = new TreeItem();
    m_childModel.setRootItem(rootItem);
    refreshModelHasChildren();
    //add first crate level to model
    onLazyChildExpandation(QModelIndex());

    connect(pLibrary, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(slotTrackSelected(TrackPointer)));
    connect(pLibrary, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotResetSelectedTrack()));
}

CrateFeature::~CrateFeature() {
    //delete QActions
    delete m_pCreateCrateAction;
    delete m_pDeleteCrateAction;
    delete m_pRenameCrateAction;
    delete m_pDuplicateCrateAction;
    delete m_pLockCrateAction;
    delete m_pImportPlaylistAction;
    delete m_pAnalyzeCrateAction;
#ifdef __AUTODJCRATES__
    delete m_pAutoDjTrackSource;
#endif // __AUTODJCRATES__
    delete m_pCreateCrateFolderAction;
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QIcon CrateFeature::getIcon() {
    return QIcon(":/images/library/ic_library_crates.png");
}

int CrateFeature::crateIdFromIndex(QModelIndex index) {
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == NULL) {
        return -1;
    }

    QString dataPath = item->dataPath().toString();
    bool ok = false;
    int crateId = dataPath.toInt(&ok);
    if (!ok) {
        return -1;
    }
    return crateId;
}

QModelIndex CrateFeature::indexFromCrateId(int id) {

    QStack<QModelIndex> cratesToProcess;
    QModelIndex crate;
    cratesToProcess.push(QModelIndex());
    while (! cratesToProcess.empty()) {
        crate = cratesToProcess.pop();

        if (id == crateIdFromIndex(crate)) {
            qDebug() << "Found index of crate found id: " << id;
            return crate;
        }

        for(int row = 0; row < m_childModel.rowCount(crate); row++) {
            cratesToProcess.push(m_childModel.index(row, 0, crate));
        }
    }
    qDebug() << "Failed search for index of crate id: " << id;
    return QModelIndex();
}

// updates hasChildren function in childModel
void CrateFeature::refreshModelHasChildren() { 
    //put the set of folders to variable to be lambda-captureable
    QSet<int> folderSet = m_crateDao.getFolderSet();
    m_childModel.setHasChildrenFunction( 
        [folderSet] (QVariant crateId) {
             return folderSet.contains(crateId.toInt());
        }
    );
}

void CrateFeature::slotCrateTableChanged(int crateId) {
    qDebug() << "slotCrateTableChanged() playlistId:" << crateId;

    onLazyChildExpandation(QModelIndex());
    QModelIndex index;
    foreach(int folderId, m_openFolders) {
        index = indexFromCrateId(folderId);
        if (index.isValid()) {
            qDebug() << "Restored folder id: " << folderId;
            emit(featureSelect(this, index));
        } else {
            qDebug() << "Failed restoring folder id: " << folderId;
        }
    }
    if (indexFromCrateId(crateId).isValid())
        emit(featureSelect(this, indexFromCrateId(crateId)));

    refreshModelHasChildren();

    qDebug() << "slotCrateTableChanged finished";
}

bool CrateFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                                   QObject* pSource) {
    int crateId = crateIdFromIndex(index);
    if (crateId == -1 || m_crateDao.isFolder(crateIdFromIndex(index))) {
        return false;
    }
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    QList<TrackId> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
        m_pTrackCollection->getTrackDAO().unhideTracks(trackIds);
    } else {
        // Adds track, does not insert duplicates, handles unremoving logic.
        trackIds = m_pTrackCollection->getTrackDAO().addTracks(files, true);
    }
    qDebug() << "CrateFeature::dropAcceptChild adding tracks"
            << trackIds.size() << " to crate "<< crateId;
    // remove tracks that could not be added
    for (int trackIdIndex = 0; trackIdIndex < trackIds.size(); ++trackIdIndex) {
        if (!trackIds.at(trackIdIndex).isValid()) {
            trackIds.removeAt(trackIdIndex--);
        }
    }
    m_crateDao.addTracksToCrate(crateId, &trackIds);
    return true;
}

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    int crateId = crateIdFromIndex(index);
    if (crateId == -1 || m_crateDao.isFolder(crateIdFromIndex(index))) {
        return false;
    }
    bool locked = m_crateDao.isCrateLocked(crateId);
    bool formatSupported = SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
    return !locked && formatSupported;
}

void CrateFeature::bindWidget(WLibrary* libraryWidget,
                              MixxxKeyboard* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(getRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit, SIGNAL(anchorClicked(const QUrl)),
            this, SLOT(htmlLinkClicked(const QUrl)));
    libraryWidget->registerView("CRATEHOME", edit);
}

TreeItemModel* CrateFeature::getChildModel() {
    return &m_childModel;
}

void CrateFeature::activate() {
    emit(switchToView("CRATEHOME"));
    emit(restoreSearch(QString())); //disable search on crate home
    emit(enableCoverArtDisplay(true));
}

void CrateFeature::activateChild(const QModelIndex& index) {
    if (!index.isValid())
        return;
    int crateId = crateIdFromIndex(index);
    if (crateId == -1) {
        return;
    }
    m_crateTableModel.setTableModel(crateId);
    emit(showTrackModel(&m_crateTableModel));
    emit(enableCoverArtDisplay(true));
}

void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addAction(m_pCreateCrateFolderAction);
    menu.exec(globalPos);
}

void CrateFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    qDebug() << "onRightClickChild";
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    int crateId = crateIdFromIndex(index);
    if (crateId == -1) {
        return;
    }

    bool locked = m_crateDao.isCrateLocked(crateId);
    m_pDeleteCrateAction->setEnabled(!(locked ));

#ifdef __AUTODJCRATES__
    bool bAutoDj = m_crateDao.isCrateInAutoDj(crateId);
    m_pAutoDjTrackSource->setChecked(bAutoDj);
#endif // __AUTODJCRATES__

    m_pLockCrateAction->setText(locked ? tr("Unlock") : tr("Lock"));

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addAction(m_pCreateCrateFolderAction);
    menu.addSeparator();
    menu.addAction(m_pRenameCrateAction);
    menu.addAction(m_pDuplicateCrateAction);
    menu.addAction(m_pDeleteCrateAction);
    menu.addAction(m_pLockCrateAction);
    menu.addSeparator();
#ifdef __AUTODJCRATES__
    menu.addAction(m_pAutoDjTrackSource);
    menu.addSeparator();
#endif // __AUTODJCRATES__
    menu.addAction(m_pAnalyzeCrateAction);
    menu.addSeparator();
    menu.addAction(m_pImportPlaylistAction);
    menu.addAction(m_pExportPlaylistAction);
    menu.exec(globalPos);
}


void CrateFeature::onLazyChildExpandation(const QModelIndex& index) {
    qDebug() << "\e[31mEntering CrateFeature::onLazyChildExpandation\e[0m";
    TreeItem* parent = m_childModel.getItem(index);

    int parentId = parent->dataPath().toInt();

    qDebug() << "\e[33mnode crate id: " << parentId << "\e[0m";

    QList<QPair<int, QString>> childrenList =
        m_crateDao.getChildren(parentId);

    // if node doesn't have any children we have nothing to add
    if (childrenList.size() == 0 ) { return ; }

    //Before we populate the subtree, we need to delete old subtrees
    m_childModel.removeRows(0, parent->childCount(), index);

    //Sort the childrenList by name (using lambda to do the trick)
    std::sort(
        childrenList.begin(), 
        childrenList.end(),
        [] (QPair<int, QString> fst, QPair<int, QString> snd) {
            return fst.second < snd.second;
        }
    );

    QList<TreeItem*> newNodes;

    int crateId;
    QString crateName;
    foreach(const auto& child, childrenList) {
        crateId = child.first;
        crateName = child.second;

        qDebug() << "\e[32mAdding crate id: " << crateId << " title: "<< crateName<<"\e[0m";

        // Create the TreeItem with parent from argument
        TreeItem* item = new TreeItem(
                    crateName, QString::number(crateId), this, parent);
        bool locked = m_crateDao.isCrateLocked(crateId);
        item->setIcon(locked ? 
                    QIcon(":/images/library/ic_library_locked.png") : QIcon());
        item->setBold(m_cratesSelectedTrackIsIn.contains(crateId));
        newNodes << item;
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_childModel.insertRows(newNodes, 0, newNodes.size(), index);
    if (!m_openFolders.contains(parentId))
            m_openFolders.push_back(parentId);
    qDebug() << "\e[31mLeaving CrateFeature::onLazyChildExpandation\e[0m";
}

void CrateFeature::itemCollapsed(const QModelIndex& index) {
    qDebug() << "Closing folder id: " << crateIdFromIndex(index);
    m_openFolders.removeAll(crateIdFromIndex(index));
}

void CrateFeature::slotCreateCrate(int crateType) {
    qDebug() << "CrateFeature::slotCreateCrate with type: " << crateType;
    QString name;
    bool validNameGiven = false;

    QString text;
    switch (crateType) {
        case 0: text = tr("Enter name for new crate:") ; break;
        case 1: text = tr("Enter name for new folder:") ; break;
    }

    while (!validNameGiven) {
        bool ok = false;
        name = QInputDialog::getText(NULL,
                                     tr("Create New Crate"),
                                     text,
                                     QLineEdit::Normal, tr("New Crate"),
                                     &ok).trimmed();

        if (!ok)
            return;

        int existingId = m_crateDao.getCrateIdByName(name);

        if (existingId != -1) {
            QMessageBox::warning(NULL,
                                 tr("Creating Crate Failed"),
                                 tr("A crate by that name already exists."));
        } else if (name.isEmpty()) {
            QMessageBox::warning(NULL,
                                 tr("Creating Crate Failed"),
                                 tr("A crate cannot have a blank name."));
        } else {
            validNameGiven = true;
        }
    }

    // If we clicked on folder we create new crate in it,
    // otherwise in crate's parent folder
    int parentId;
    int lastRightClickedId = crateIdFromIndex(m_lastRightClickedIndex);
    if (m_crateDao.isFolder(lastRightClickedId)) {
        parentId = lastRightClickedId;
    } else {
        parentId = m_crateDao.getParentsId(lastRightClickedId);
    }

    int crateId = m_crateDao.createCrate(name, parentId, crateType);

    if (crateId != -1) {
        emit(showTrackModel(&m_crateTableModel));
    } else {
        qDebug() << "Error creating crate with name " << name;
        QMessageBox::warning(NULL,
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
    //TODO(vlada-dudr) should we check lock of parents?
    bool locked = m_crateDao.isCrateLocked(crateId);
    if (locked) {
        qDebug() << "Skipping crate deletion because crate" << crateId << "is locked.";
        return;
    }

    bool deleted = m_crateDao.deleteCrate(crateId);

    if (deleted) {
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

    //TODO(vlada-dudr) should we check lock of parents?
    bool locked = m_crateDao.isCrateLocked(crateId);
    if (locked) {
        qDebug() << "Skipping crate rename because crate" << crateId << "is locked.";
        return;
    }

    QString newName;
    bool validNameGiven = false;

    while (!validNameGiven) {
        m_pRenameCrateAction->setEnabled(!locked);

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

        //TODO(vlada-dudr) cannot use this as name is not uniqe
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

    onLazyChildExpandation(m_lastRightClickedIndex);
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

    int newCrateId = m_crateDao.createCrate(name, m_crateDao.getParentsId(oldCrateId),0);
    m_crateDao.copyCrateTracks(oldCrateId, newCrateId);

    if (newCrateId != -1) {
        emit(showTrackModel(&m_crateTableModel));
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
#ifdef __AUTODJCRATES__
    int crateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (crateId != -1) {
        m_crateDao.setCrateInAutoDj(crateId, m_pAutoDjTrackSource->isChecked());
    }
#endif // __AUTODJCRATES__
}

void CrateFeature::slotImportPlaylist() {
    qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();

    QString lastCrateDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportCrateDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString playlist_file = QFileDialog::getOpenFileName(
        NULL,
        tr("Import Playlist"),
        lastCrateDirectory,
        tr("Playlist Files (*.m3u *.m3u8 *.pls *.csv)"));
    // Exit method if user cancelled the open dialog.
    if (playlist_file.isNull() || playlist_file.isEmpty()) return;

    // Update the import/export crate directory
    QFileInfo fileName(playlist_file);
    m_pConfig->set(ConfigKey("[Library]","LastImportExportCrateDirectory"),
                   ConfigValue(fileName.dir().absolutePath()));

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
      m_crateTableModel.addTracks(QModelIndex(), entries);

      //delete the parser object
      delete playlist_parser;
    }
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
    qDebug() << "Export crate" << m_lastRightClickedIndex.data();

    QString lastCrateDirectory = m_pConfig->getValueString(
            ConfigKey("[Library]", "LastImportExportCrateDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString file_location = QFileDialog::getSaveFileName(
        NULL,
        tr("Export Crate"),
        lastCrateDirectory,
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
    pCrateTableModel->setTableModel(m_crateTableModel.getCrate());
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
            QModelIndex index = m_crateTableModel.index(i, 0);
            playlist_items << m_crateTableModel.getTrackLocation(index);
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

void CrateFeature::slotCrateTableRenamed(int a_iCrateId,
                                         QString /* a_strName */) {
    slotCrateTableChanged(a_iCrateId);
}

void CrateFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path())=="create") {
        //@TODO(vlada-dudr): put something more creative, then 0 in there
        slotCreateCrate(0);
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
    TrackId trackId(pTrack.isNull() ? TrackId() : pTrack->getId());
    m_crateDao.getCratesTrackIsIn(trackId, &m_cratesSelectedTrackIsIn);

    TreeItem* rootItem = m_childModel.getItem(QModelIndex());
    if (rootItem == nullptr) {
        return;
    }

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    QStack<TreeItem*> cratesToProcess;
    TreeItem* crate;
    cratesToProcess.push(rootItem);
    while (! cratesToProcess.empty()) {
        crate = cratesToProcess.pop();
        
        crate->setBold(
                m_cratesSelectedTrackIsIn.contains(
                    crate->dataPath().toInt()));

        for(int row = 0; row < crate->childCount(); row++) {
            cratesToProcess.push(crate->child(row));
        }
    }

    m_childModel.triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackPointer());
}
