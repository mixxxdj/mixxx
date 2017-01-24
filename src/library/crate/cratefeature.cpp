#include "library/crate/cratefeature.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QDesktopServices>

#include "library/export/trackexportwizard.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"

#include "library/trackcollection.h"
#include "library/treeitem.h"
#include "widget/wlibrarytextbrowser.h"
#include "widget/wlibrary.h"
#include "controllers/keyboard/keyboardeventfilter.h"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"

CrateFeature::CrateFeature(Library* pLibrary,
                           TrackCollection* pTrackCollection,
                           UserSettingsPointer pConfig)
        : LibraryFeature(pConfig),
          m_pTrackCollection(pTrackCollection),
          m_crateTableModel(this, pTrackCollection) {

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

    connect(m_pTrackCollection, SIGNAL(crateInserted(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateUpdated(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateDeleted(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateTracksChanged(CrateId, QList<TrackId>, QList<TrackId>)),
            this, SLOT(slotCrateContentChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateSummaryChanged(QSet<CrateId>)),
            this, SLOT(slotUpdateCrateLabels()));

    // construct child model
    TreeItem *rootItem = new TreeItem();
    m_childModel.setRootItem(rootItem);
    constructChildModel();

    connect(pLibrary, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(slotTrackSelected(TrackPointer)));
    connect(pLibrary, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotResetSelectedTrack()));
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

QIcon CrateFeature::getIcon() {
    return QIcon(":/images/library/ic_library_crates.png");
}

CrateId CrateFeature::crateIdFromIndex(QModelIndex index) {
    if (!index.isValid()) {
        return CrateId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return CrateId();
    }
    return CrateId(item->getData());
}

bool CrateFeature::dropAcceptChild(const QModelIndex& index, QList<QUrl> urls,
                                   QObject* pSource) {
    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return false;
    }
    QList<QFileInfo> files = DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    QList<TrackId> trackIds;
    if (pSource) {
        trackIds = m_pTrackCollection->getTrackDAO().getTrackIds(files);
        m_pTrackCollection->unhideTracks(trackIds);
    } else {
        // Adds track, does not insert duplicates, handles unremoving logic.
        trackIds = m_pTrackCollection->getTrackDAO().addMultipleTracks(files, true);
    }
    qDebug() << "CrateFeature::dropAcceptChild adding tracks"
            << trackIds.size() << " to crate "<< crateId;
    // remove tracks that could not be added
    for (int trackIdIndex = 0; trackIdIndex < trackIds.size(); ++trackIdIndex) {
        if (!trackIds.at(trackIdIndex).isValid()) {
            trackIds.removeAt(trackIdIndex--);
        }
    }
    m_pTrackCollection->addCrateTracks(crateId, trackIds);
    return true;
}

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return false;
    }
    Crate crate;
    if (!m_pTrackCollection->crates().readCrateById(crateId, &crate) || crate.isLocked()) {
        return false;
    }
    return SoundSourceProxy::isUrlSupported(url) ||
        Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

void CrateFeature::bindWidget(WLibrary* libraryWidget,
                              KeyboardEventFilter* keyboard) {
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
    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return;
    }
    m_crateTableModel.selectCrate(crateId);
    emit(showTrackModel(&m_crateTableModel));
    emit(enableCoverArtDisplay(true));
}

void CrateFeature::activateCrate(CrateId crateId) {
    //qDebug() << "CrateFeature::activateCrate()" << crateId;
    QModelIndex index = indexFromCrateId(crateId);
    if (crateId.isValid() && index.isValid()) {
        m_crateTableModel.selectCrate(crateId);
        emit(showTrackModel(&m_crateTableModel));
        emit(enableCoverArtDisplay(true));
        // Update selection
        emit(featureSelect(this, m_lastRightClickedIndex));
        activateChild(m_lastRightClickedIndex);
    }
}


void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction);
    menu.exec(globalPos);
}

void CrateFeature::onRightClickChild(const QPoint& globalPos, QModelIndex index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return;
    }

    Crate crate;
    if (!m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        return;
    }

    m_pDeleteCrateAction->setEnabled(!crate.isLocked());
    m_pRenameCrateAction->setEnabled(!crate.isLocked());

    m_pAutoDjTrackSource->setChecked(crate.isAutoDjSource());

    m_pLockCrateAction->setText(crate.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction);
    menu.addSeparator();
    menu.addAction(m_pRenameCrateAction);
    menu.addAction(m_pDuplicateCrateAction);
    menu.addAction(m_pDeleteCrateAction);
    menu.addAction(m_pLockCrateAction);
    menu.addSeparator();
    menu.addAction(m_pAutoDjTrackSource);
    menu.addSeparator();
    menu.addAction(m_pAnalyzeCrateAction);
    menu.addSeparator();
    menu.addAction(m_pImportPlaylistAction);
    menu.addAction(m_pExportPlaylistAction);
    menu.addAction(m_pExportTrackFilesAction);
    menu.exec(globalPos);
}

void CrateFeature::slotCreateCrate() {
    Crate crate;
    while (!crate.hasName()) {
        bool ok = false;
        crate.parseName(
                QInputDialog::getText(
                        nullptr,
                        tr("Create New Crate"),
                        tr("Enter name for new crate:"),
                        QLineEdit::Normal,
                        tr("New Crate"),
                        &ok));
        if (!ok) {
            return;
        }
        if (!crate.hasName()) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate cannot have a blank name."));
            continue;
        }

        if (m_pTrackCollection->crates().readCrateByName(crate.getName())) {
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("A crate by that name already exists."));
            crate.resetName();
            continue;
        }
    }

    CrateId crateId;
    if (m_pTrackCollection->insertCrate(crate, &crateId)) {
        activateCrate(crateId);
    } else {
        qDebug() << "Error creating crate with name " << crate.getName();
        QMessageBox::warning(
                nullptr,
                tr("Creating Crate Failed"),
                tr("An unknown error occurred while creating crate: ") + crate.getName());
    }
}

bool CrateFeature::readLastRightClickedCrate(Crate* pCrate) {
    CrateId crateId(crateIdFromIndex(m_lastRightClickedIndex));
    DEBUG_ASSERT_AND_HANDLE(crateId.isValid()) {
        return false;
    }

    if (!m_pTrackCollection->crates().readCrateById(crateId, pCrate)) {
        qDebug() << "Crates are out of sync -> enforcing refresh of child model";
        constructChildModel();
        return false;
    }

    return true;
}

void CrateFeature::slotDeleteCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isLocked()) {
            qDebug() << "Refusing to delete locked crate" << crate.getId();
            return;
        }

        if (m_pTrackCollection->deleteCrate(crate.getId())) {
            activate();
        } else {
            qDebug() << "Failed to delete crate" << crate;
        }
    } else {
        qDebug() << "Failed to delete selected crate";
    }
}

void CrateFeature::slotRenameCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        const QString oldName = crate.getName();
        crate.resetName();
        while (!crate.hasName()) {
            bool ok = false;
            crate.parseName(
                    QInputDialog::getText(
                            nullptr,
                            tr("Rename Crate"),
                            tr("Enter new name for crate:"),
                            QLineEdit::Normal,
                            oldName,
                            &ok));
            if (!ok || (crate.getName() == oldName)) {
                return;
            }
            if (!crate.hasName()) {
                QMessageBox::warning(
                        nullptr,
                        tr("Renaming Crate Failed"),
                        tr("A crate cannot have a blank name."));
                continue;
            }
            if (m_pTrackCollection->crates().readCrateByName(crate.getName())) {
                QMessageBox::warning(
                        nullptr,
                        tr("Renaming Crate Failed"),
                        tr("A crate by that name already exists."));
                crate.resetName();
                continue;
            }
        }

        if (!m_pTrackCollection->updateCrate(crate)) {
            qDebug() << "Failed to rename crate" << crate;
        }
    } else {
        qDebug() << "Failed to rename selected crate";
    }
}

void CrateFeature::slotDuplicateCrate() {
    Crate oldCrate;
    if (readLastRightClickedCrate(&oldCrate)) {
        Crate crate;
        while (!crate.hasName()) {
            bool ok = false;
            crate.parseName(
                    QInputDialog::getText(
                            nullptr,
                             tr("Duplicate Crate"),
                             tr("Enter name for new crate:"),
                             QLineEdit::Normal,
                             //: Appendix to default name when duplicating a crate
                             oldCrate.getName() + tr("_copy" , "[noun]"),
                             &ok));
            if (!ok || (crate.getName() == oldCrate.getName())) {
                return;
            }
            if (!crate.hasName()) {
                QMessageBox::warning(
                        nullptr,
                        tr("Duplicating Crate Failed"),
                        tr("A crate cannot have a blank name."));
                continue;
            }
            if (m_pTrackCollection->crates().readCrateByName(crate.getName())) {
                QMessageBox::warning(
                        nullptr,
                        tr("Duplicating Crate Failed"),
                        tr("A crate by that name already exists."));
                crate.resetName();
                continue;
            }
        }

        CrateId crateId;
        if (m_pTrackCollection->insertCrate(crate, &crateId)) {
            QList<TrackId> trackIds;
            trackIds.reserve(
                    m_pTrackCollection->crates().countCrateTracks(oldCrate.getId()));
            {
                CrateTrackSelectIterator crateTracks(
                        m_pTrackCollection->crates().selectCrateTracks(oldCrate.getId()));
                while (crateTracks.next()) {
                    trackIds.append(crateTracks.trackId());
                }
            }
            m_pTrackCollection->addCrateTracks(crateId, trackIds);
            activateCrate(crateId);
        } else {
            qDebug() << "Error creating crate with name " << crate.getName();
            QMessageBox::warning(
                    nullptr,
                    tr("Creating Crate Failed"),
                    tr("An unknown error occurred while creating crate: ") + crate.getName());
        }
    } else {
        qDebug() << "Failed to duplicate selected crate";
    }
}

void CrateFeature::slotToggleCrateLock() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        crate.setLocked(!crate.isLocked());
        if (!m_pTrackCollection->updateCrate(crate)) {
            qDebug() << "Failed to toggle lock of crate" << crate;
        }
    } else {
        qDebug() << "Failed to toggle lock of selected crate";
    }
}

void CrateFeature::slotAutoDjTrackSourceChanged() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isAutoDjSource() != m_pAutoDjTrackSource->isChecked()) {
            crate.setAutoDjSource(m_pAutoDjTrackSource->isChecked());
            m_pTrackCollection->updateCrate(crate);
        }
    }
}

QVector<CrateSummary> CrateFeature::buildCrateList() {
    m_crateList.clear();
    QVector<CrateSummary> result;

    // Reserve memory in advance to avoid expensive reallocations
    // while collecting the results. This extra query might cause
    // a minor performance hit for small result sets, but should
    // improve the performance for big result sets were it actually
    // matters.
    uint numCrates = m_pTrackCollection->crates().countCrates();
    m_crateList.reserve(numCrates);
    result.reserve(numCrates);

    CrateSummarySelectIterator crateSummaries(
            m_pTrackCollection->crates().selectCrateSummaries());
    CrateSummary crateSummary;
    while (crateSummaries.readNext(&crateSummary)) {
        m_crateList.append(qMakePair(
                crateSummary.getId(), QString("%1 (%2) %3").arg(
                        crateSummary.getName(),
                        QString::number(crateSummary.getTrackCount()),
                        crateSummary.getTrackDurationText())));
        result.push_back(std::move(crateSummary));
    }

    return result;
}

/**
  * Purpose: When inserting or removing playlists,
  * we require the sidebar model not to reset.
  * This method queries the database and does dynamic insertion
*/
QModelIndex CrateFeature::constructChildModel(CrateId selected_id) {
    QVector<CrateSummary> crates = buildCrateList();
    QList<TreeItem*> data_list;
    int selected_row = -1;
    // Access the invisible root item
    TreeItem* root = m_childModel.getItem(QModelIndex());

    int row = 0;
    for (QList<QPair<CrateId, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        CrateId crate_id = it->first;
        QString crate_name = it->second;

        if (selected_id == crate_id) {
            // save index for selection
            selected_row = row;
            m_childModel.index(selected_row, 0);
        }

        // Create the TreeItem whose parent is the invisible root item
        TreeItem* item = new TreeItem(crate_name, crate_id.toString(), this, root);
        const Crate& crate = crates[row];
        DEBUG_ASSERT(crate.getId() == it->first);
        item->setIcon(crate.isLocked() ? QIcon(":/images/library/ic_library_locked.png") : QIcon());
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

void CrateFeature::updateChildModel(CrateId selected_id) {
    QVector<CrateSummary> crates = buildCrateList();

    int row = 0;
    for (QList<QPair<CrateId, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        CrateId crate_id = it->first;
        QString crate_name = it->second;

        if (selected_id == crate_id) {
            TreeItem* item = m_childModel.getItem(indexFromCrateId(crate_id));
            item->setData(crate_name, crate_id.toString());
            const Crate& crate = crates[row];
            DEBUG_ASSERT(crate.getId() == it->first);
            item->setIcon(crate.isLocked() ? QIcon(":/images/library/ic_library_locked.png") : QIcon());

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

    QString playlist_file = getPlaylistFile();
    if (playlist_file.isEmpty()) return;

    // Update the import/export crate directory
    QFileInfo fileName(playlist_file);
    m_pConfig->set(ConfigKey("[Library]","LastImportExportCrateDirectory"),
                   ConfigValue(fileName.dir().absolutePath()));

    slotImportPlaylistFile(playlist_file);
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
      m_crateTableModel.addTracks(QModelIndex(), entries);

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

    CrateId lastCrateId;

    // For each selected file
    for (const QString& playlistFile : playlist_files) {
        fileName = QFileInfo(playlistFile);

        Crate crate;

        // Get a valid name
        QString baseName = fileName.baseName();
        for (int i = 0;; ++i) {
            QString name = baseName;
            if (i > 0) {
                name += QString::number(i);
            }

            if (crate.parseName(name)) {
                DEBUG_ASSERT(crate.hasName());
                if (!m_pTrackCollection->crates().readCrateByName(crate.getName())) {
                    // unused crate name found
                    break; // terminate loop
                }
            }
        }

        if (m_pTrackCollection->insertCrate(crate, &lastCrateId)) {
            m_crateTableModel.selectCrate(lastCrateId);
        } else {
            QMessageBox::warning(
                    nullptr,
                    tr("Crate Creation Failed"),
                    tr("An unknown error occurred while creating crate: ") + crate.getName());
            return;
        }

        slotImportPlaylistFile(playlistFile);
    }
    activateCrate(lastCrateId);
}

void CrateFeature::slotAnalyzeCrate() {
    if (m_lastRightClickedIndex.isValid()) {
        CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
        if (crateId.isValid()) {
            QList<TrackId> trackIds;
            trackIds.reserve(
                    m_pTrackCollection->crates().countCrateTracks(crateId));
            {
                CrateTrackSelectIterator crateTracks(
                        m_pTrackCollection->crates().selectCrateTracks(crateId));
                while (crateTracks.next()) {
                    trackIds.append(crateTracks.trackId());
                }
            }
            emit(analyzeTracks(trackIds));
        }
    }
}

void CrateFeature::slotExportPlaylist() {
    CrateId crateId = m_crateTableModel.selectedCrate();
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        qDebug() << "Exporting crate" << crate;
    } else {
        qDebug() << "Failed to export crate" << crateId;
    }

    QString lastCrateDirectory = m_pConfig->getValue(
            ConfigKey("[Library]", "LastImportExportCrateDirectory"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation));

    QString file_location = QFileDialog::getSaveFileName(
        NULL,
        tr("Export Crate"),
        lastCrateDirectory.append("/").append(crate.getName()),
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
    bool useRelativePath =
        m_pConfig->getValue<bool>(
            ConfigKey("[Library]", "UseRelativePathOnExport"));

    // Create list of files of the crate
    QList<QString> playlist_items;
    // Create a new table model since the main one might have an active search.
    QScopedPointer<CrateTableModel> pCrateTableModel(
        new CrateTableModel(this, m_pTrackCollection));
    pCrateTableModel->selectCrate(m_crateTableModel.selectedCrate());
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

void CrateFeature::slotExportTrackFiles() {
    // Create a new table model since the main one might have an active search.
    QScopedPointer<CrateTableModel> pCrateTableModel(
        new CrateTableModel(this, m_pTrackCollection));
    pCrateTableModel->selectCrate(m_crateTableModel.selectedCrate());
    pCrateTableModel->select();

    int rows = pCrateTableModel->rowCount();
    QList<TrackPointer> trackpointers;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = m_crateTableModel.index(i, 0);
        trackpointers.push_back(m_crateTableModel.getTrack(index));
    }

    TrackExportWizard track_export(nullptr, m_pConfig, trackpointers);
    track_export.exportTracks();
}

void CrateFeature::slotCrateTableChanged(CrateId crateId) {
    //qDebug() << "slotCrateTableChanged() crateId:" << crateId;
    clearChildModel();
    m_lastRightClickedIndex = constructChildModel(crateId);
}

void CrateFeature::slotCrateContentChanged(CrateId crateId) {
    //qDebug() << "slotCrateContentChanged()crateId:" << crateId;
    updateChildModel(crateId);
}

void CrateFeature::slotUpdateCrateLabels() {
    clearChildModel();
    constructChildModel(CrateId());
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
    m_cratesSelectedTrackIsIn = m_pTrackCollection->crates().collectTrackCrates(trackId);

    TreeItem* rootItem = m_childModel.getItem(QModelIndex());
    if (rootItem == nullptr) {
        return;
    }

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    int row = 0;
    for (QList<QPair<CrateId, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        TreeItem* crate = rootItem->child(row);
        if (crate == nullptr) {
            continue;
        }
        bool shouldBold = m_cratesSelectedTrackIsIn.contains(it->first);
        crate->setBold(shouldBold);
    }

    m_childModel.triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackPointer());
}

QModelIndex CrateFeature::indexFromCrateId(CrateId crateId) {
    if (!crateId.isValid()) {
        return QModelIndex();
    }
    int row = 0;
    for (QList<QPair<CrateId, QString> >::const_iterator it = m_crateList.begin();
         it != m_crateList.end(); ++it, ++row) {
        if (crateId == it->first) {
            return m_childModel.index(row, 0);
        }
    }
    return QModelIndex();
}
