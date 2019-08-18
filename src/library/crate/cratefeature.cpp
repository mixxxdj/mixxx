#include "library/crate/cratefeature.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QDesktopServices>

#include <algorithm>
#include <vector>

#include "library/crate/cratefeaturehelper.h"
#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"
#include "library/trackcollection.h"
#include "library/treeitem.h"

#include "sources/soundsourceproxy.h"

#include "widget/wlibrary.h"
#include "widget/wlibrarytextbrowser.h"

#include "util/dnd.h"


namespace {

QString formatLabel(
        const CrateSummary& crateSummary) {
    return QString("%1 (%2) %3").arg(
        crateSummary.getName(),
        QString::number(crateSummary.getTrackCount()),
        crateSummary.getTrackDurationText());
}

} // anonymous namespace

CrateFeature::CrateFeature(Library* pLibrary,
                           TrackCollection* pTrackCollection,
                           UserSettingsPointer pConfig)
        : LibraryFeature(pConfig),
          m_cratesIcon(":/images/library/ic_library_crates.svg"),
          m_lockedCrateIcon(":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(pTrackCollection),
          m_crateTableModel(this, pTrackCollection) {

    initActions();

    // construct child model
    m_childModel.setRootItem(std::make_unique<TreeItem>(this));
    rebuildChildModel();

    connectLibrary(pLibrary);
    connectTrackCollection();
}

CrateFeature::~CrateFeature() {
}

void CrateFeature::initActions() {
    m_pCreateCrateAction = make_parented<QAction>(tr("Create New Crate"), this);
    connect(m_pCreateCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotCreateCrate()));

    m_pDeleteCrateAction = make_parented<QAction>(tr("Remove"), this);
    connect(m_pDeleteCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotDeleteCrate()));

    m_pRenameCrateAction = make_parented<QAction>(tr("Rename"), this);
    connect(m_pRenameCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotRenameCrate()));

    m_pLockCrateAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotToggleCrateLock()));

    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pImportPlaylistAction.get(), SIGNAL(triggered()),
            this, SLOT(slotImportPlaylist()));

    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pCreateImportPlaylistAction.get(), SIGNAL(triggered()),
            this, SLOT(slotCreateImportCrate()));

    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Crate"), this);
    connect(m_pExportPlaylistAction.get(), SIGNAL(triggered()),
            this, SLOT(slotExportPlaylist()));

    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction.get(), SIGNAL(triggered()),
            this, SLOT(slotExportTrackFiles()));

    m_pDuplicateCrateAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotDuplicateCrate()));

    m_pAnalyzeCrateAction = make_parented<QAction>(tr("Analyze entire Crate"), this);
    connect(m_pAnalyzeCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotAnalyzeCrate()));

    m_pAutoDjTrackSourceAction = make_parented<QAction>(tr("Auto DJ Track Source"), this);
    m_pAutoDjTrackSourceAction->setCheckable(true);
    connect(m_pAutoDjTrackSourceAction.get(), SIGNAL(changed()),
            this, SLOT(slotAutoDjTrackSourceChanged()));
}

void CrateFeature::connectLibrary(Library* pLibrary) {
    connect(pLibrary, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(slotTrackSelected(TrackPointer)));
    connect(pLibrary, SIGNAL(switchToView(const QString&)),
            this, SLOT(slotResetSelectedTrack()));
}

void CrateFeature::connectTrackCollection() {
    connect(m_pTrackCollection, SIGNAL(crateInserted(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateUpdated(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateDeleted(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateTracksChanged(CrateId, QList<TrackId>, QList<TrackId>)),
            this, SLOT(slotCrateContentChanged(CrateId)));
    connect(m_pTrackCollection, SIGNAL(crateSummaryChanged(QSet<CrateId>)),
            this, SLOT(slotUpdateCrateLabels(QSet<CrateId>)));
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QIcon CrateFeature::getIcon() {
    return m_cratesIcon;
}

QString CrateFeature::formatRootViewHtml() const {
    QString cratesTitle = tr("Crates");
    QString cratesSummary = tr("Crates are a great way to help organize the music you want to DJ with.");
    QString cratesSummary2 = tr("Make a crate for your next gig, for your favorite electrohouse tracks, or for your most requested songs.");
    QString cratesSummary3 = tr("Crates let you organize your music however you'd like!");

    QString html;
    QString createCrateLink = tr("Create New Crate");
    html.append(QString("<h2>%1</h2>").arg(cratesTitle));
    html.append(QString("<p>%1</p>").arg(cratesSummary));
    html.append(QString("<p>%1</p>").arg(cratesSummary2));
    html.append(QString("<p>%1</p>").arg(cratesSummary3));
    //Colorize links in lighter blue, instead of QT default dark blue.
    //Links are still different from regular text, but readable on dark/light backgrounds.
    //https://bugs.launchpad.net/mixxx/+bug/1744816
    html.append(QString("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                .arg(createCrateLink));
    return html;
}

std::unique_ptr<TreeItem> CrateFeature::newTreeItemForCrateSummary(
        const CrateSummary& crateSummary) {
    auto pTreeItem = std::make_unique<TreeItem>(this);
    updateTreeItemForCrateSummary(pTreeItem.get(), crateSummary);
    return pTreeItem;
}

void CrateFeature::updateTreeItemForCrateSummary(
        TreeItem* pTreeItem,
        const CrateSummary& crateSummary) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(crateSummary.getId().toVariant());
    } else {
        // The data (= CrateId) is immutable once it has been set
        DEBUG_ASSERT(CrateId(pTreeItem->getData()) == crateSummary.getId());
    }
    // Update mutable properties
    pTreeItem->setLabel(formatLabel(crateSummary));
    pTreeItem->setIcon(crateSummary.isLocked() ? m_lockedCrateIcon : QIcon());
}

namespace {

void updateTreeItemForTrackSelection(
        TreeItem* pTreeItem,
        TrackId selectedTrackId,
        const std::vector<CrateId>& sortedTrackCrates) {
    DEBUG_ASSERT(pTreeItem != nullptr);
    bool crateContainsSelectedTrack =
            selectedTrackId.isValid() &&
            std::binary_search(
                    sortedTrackCrates.begin(),
                    sortedTrackCrates.end(),
                    CrateId(pTreeItem->getData()));
    pTreeItem->setBold(crateContainsSelectedTrack);
}

} // anonymous namespace

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
    edit->setHtml(formatRootViewHtml());
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
    emit disableSearch();
    emit(enableCoverArtDisplay(true));
}

void CrateFeature::activateChild(const QModelIndex& index) {
    CrateId crateId(crateIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return;
    }
    m_crateTableModel.selectCrate(crateId);
    emit(showTrackModel(&m_crateTableModel));
    emit(enableCoverArtDisplay(true));
}

bool CrateFeature::activateCrate(CrateId crateId) {
    qDebug() << "CrateFeature::activateCrate()" << crateId;
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return false;
    }
    QModelIndex index = indexFromCrateId(crateId);
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    m_lastRightClickedIndex = index;
    m_crateTableModel.selectCrate(crateId);
    emit(showTrackModel(&m_crateTableModel));
    emit(enableCoverArtDisplay(true));
    // Update selection
    emit(featureSelect(this, m_lastRightClickedIndex));
    activateChild(m_lastRightClickedIndex);
    return true;
}

bool CrateFeature::readLastRightClickedCrate(Crate* pCrate) const {
    CrateId crateId(crateIdFromIndex(m_lastRightClickedIndex));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        qWarning() << "Failed to determine id of selected crate";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pTrackCollection->crates().readCrateById(crateId, pCrate)) {
        qWarning() << "Failed to read selected crate with id" << crateId;
        return false;
    }
    return true;
}

void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction.get());
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

    m_pAutoDjTrackSourceAction->setChecked(crate.isAutoDjSource());

    m_pLockCrateAction->setText(crate.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pRenameCrateAction.get());
    menu.addAction(m_pDuplicateCrateAction.get());
    menu.addAction(m_pDeleteCrateAction.get());
    menu.addAction(m_pLockCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pAutoDjTrackSourceAction.get());
    menu.addSeparator();
    menu.addAction(m_pAnalyzeCrateAction.get());
    menu.addSeparator();
    if (!crate.isLocked()) {
        menu.addAction(m_pImportPlaylistAction.get());
    }
    menu.addAction(m_pExportPlaylistAction.get());
    menu.addAction(m_pExportTrackFilesAction.get());
    menu.exec(globalPos);
}

void CrateFeature::slotCreateCrate() {
    CrateId crateId = CrateFeatureHelper(
            m_pTrackCollection, m_pConfig).createEmptyCrate();
    if (crateId.isValid()) {
        activateCrate(crateId);
    }
}

void CrateFeature::slotDeleteCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isLocked()) {
            qWarning() << "Refusing to delete locked crate" << crate;
            return;
        }
        if (m_pTrackCollection->deleteCrate(crate.getId())) {
            qDebug() << "Deleted crate" << crate;
            return;
        }
    }
    qWarning() << "Failed to delete selected crate";
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
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        CrateId crateId = CrateFeatureHelper(
                m_pTrackCollection, m_pConfig).duplicateCrate(crate);
        if (crateId.isValid()) {
            activateCrate(crateId);
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
        if (crate.isAutoDjSource() != m_pAutoDjTrackSourceAction->isChecked()) {
            crate.setAutoDjSource(m_pAutoDjTrackSourceAction->isChecked());
            m_pTrackCollection->updateCrate(crate);
        }
    }
}

QModelIndex CrateFeature::rebuildChildModel(CrateId selectedCrateId) {
    qDebug() << "CrateFeature::rebuildChildModel()" << selectedCrateId;

    TreeItem* pRootItem = m_childModel.getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_childModel.removeRows(0, pRootItem->childRows());

    QList<TreeItem*> modelRows;
    modelRows.reserve(m_pTrackCollection->crates().countCrates());

    int selectedRow = -1;
    CrateSummarySelectResult crateSummaries(
            m_pTrackCollection->crates().selectCrateSummaries());
    CrateSummary crateSummary;
    while (crateSummaries.populateNext(&crateSummary)) {
        auto pTreeItem = newTreeItemForCrateSummary(crateSummary);
        modelRows.append(pTreeItem.get());
        pTreeItem.release();
        if (selectedCrateId == crateSummary.getId()) {
            // save index for selection
            selectedRow = modelRows.size() - 1;
        }
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_childModel.insertTreeItemRows(modelRows, 0);

    // Update rendering of crates depending on the currently selected track
    slotTrackSelected(m_pSelectedTrack);

    if (selectedRow >= 0) {
        return m_childModel.index(selectedRow, 0);
    } else {
        return QModelIndex();
    }
}

void CrateFeature::updateChildModel(const QSet<CrateId>& updatedCrateIds) {
    const CrateStorage& crateStorage = m_pTrackCollection->crates();
    for (const CrateId& crateId: updatedCrateIds) {
        QModelIndex index = indexFromCrateId(crateId);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        CrateSummary crateSummary;
        VERIFY_OR_DEBUG_ASSERT(crateStorage.readCrateSummaryById(crateId, &crateSummary)) {
            continue;
        }
        updateTreeItemForCrateSummary(m_childModel.getItem(index), crateSummary);
        m_childModel.triggerRepaint(index);
    }
    if (m_pSelectedTrack) {
        // Crates containing the currently selected track might
        // have been modified.
        slotTrackSelected(m_pSelectedTrack);
    }
}

CrateId CrateFeature::crateIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return CrateId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return CrateId();
    }
    return CrateId(item->getData());
}

QModelIndex CrateFeature::indexFromCrateId(CrateId crateId) const {
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return QModelIndex();
    }
    for (int row = 0; row < m_childModel.rowCount(); ++row) {
        QModelIndex index = m_childModel.index(row, 0);
        TreeItem* pTreeItem = m_childModel.getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                (CrateId(pTreeItem->getData()) == crateId)) {
            return index;
        }
    }
    qDebug() << "Tree item for crate not found:" << crateId;
    return QModelIndex();
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

void CrateFeature::slotImportPlaylistFile(const QString& playlist_file) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.
    // TODO(XXX): Parsing a list of track locations from a playlist file
    // is a general task and should be implemented separately.
    QList<QString> entries;
    if (playlist_file.endsWith(".m3u", Qt::CaseInsensitive) ||
        playlist_file.endsWith(".m3u8", Qt::CaseInsensitive)) {
        // .m3u8 is Utf8 representation of an m3u playlist
        entries = ParserM3u().parse(playlist_file);
    } else if (playlist_file.endsWith(".pls", Qt::CaseInsensitive)) {
        entries = ParserPls().parse(playlist_file);
    } else if (playlist_file.endsWith(".csv", Qt::CaseInsensitive)) {
        entries = ParserCsv().parse(playlist_file);
    } else {
        return;
    }
    m_crateTableModel.addTracks(QModelIndex(), entries);
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
                name += QString(" %1").arg(i);
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
                CrateTrackSelectResult crateTracks(
                        m_pTrackCollection->crates().selectCrateTracksSorted(crateId));
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
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

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
    if (m_lastRightClickedIndex.isValid() &&
            (crateIdFromIndex(m_lastRightClickedIndex) == crateId)) {
        // Preserve crate selection
        m_lastRightClickedIndex = rebuildChildModel(crateId);
        if (m_lastRightClickedIndex.isValid()) {
            activateCrate(crateId);
        }
    } else {
        // Discard crate selection
        rebuildChildModel();
    }
}

void CrateFeature::slotCrateContentChanged(CrateId crateId) {
    QSet<CrateId> updatedCrateIds;
    updatedCrateIds.insert(crateId);
    updateChildModel(updatedCrateIds);
}

void CrateFeature::slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds) {
    updateChildModel(updatedCrateIds);
}

void CrateFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path())=="create") {
        slotCreateCrate();
    } else {
        qDebug() << "Unknown crate link clicked" << link;
    }
}

void CrateFeature::slotTrackSelected(TrackPointer pTrack) {
    m_pSelectedTrack = std::move(pTrack);

    TreeItem* pRootItem = m_childModel.getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    TrackId selectedTrackId;
    std::vector<CrateId> sortedTrackCrates;
    if (m_pSelectedTrack) {
        selectedTrackId = m_pSelectedTrack->getId();
        CrateTrackSelectResult trackCratesIter(
                m_pTrackCollection->crates().selectTrackCratesSorted(selectedTrackId));
        while (trackCratesIter.next()) {
            sortedTrackCrates.push_back(trackCratesIter.crateId());
        }
    }

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem: pRootItem->children()) {
        updateTreeItemForTrackSelection(pTreeItem, selectedTrackId, sortedTrackCrates);
    }

    m_childModel.triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackPointer());
}
