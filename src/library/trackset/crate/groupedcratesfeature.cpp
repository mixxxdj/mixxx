#include "library/trackset/crate/groupedcratesfeature.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <algorithm>
#include <vector>

#include "analyzer/analyzerscheduledtrack.h"
#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/library_prefs.h"
#include "library/parser.h"
#include "library/parsercsv.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/trackset/crate/cratefeaturehelper.h"
#include "library/trackset/crate/cratesummary.h"
#include "library/treeitem.h"
#include "moc_groupedcratesfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

const bool sDebug = false;

namespace {

// constexpr int kInvalidCrateId = -1;

QString formatLabel(
        const CrateSummary& crateSummary) {
    return QStringLiteral("%1 (%2) %3")
            .arg(
                    crateSummary.getName(),
                    QString::number(crateSummary.getTrackCount()),
                    crateSummary.getTrackDurationText());
}

const ConfigKey kConfigKeyLastImportExportCrateDirectoryKey(
        "[Library]", "LastImportExportCrateDirectory");

} // anonymous namespace

using namespace mixxx::library::prefs;

GroupedCratesFeature::GroupedCratesFeature(Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary, pConfig, "GROUPEDCRATESHOME", QStringLiteral("crates")),
          m_lockedCrateIcon(":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_crateTableModel(this, pLibrary->trackCollectionManager(), pConfig) {
    initActions();

    // construct child model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
    rebuildChildModel();

    connectLibrary(pLibrary);
    connectTrackCollection();
}

void GroupedCratesFeature::initActions() {
    m_pCreateCrateAction = make_parented<QAction>(tr("Create New Crate"), this);
    connect(m_pCreateCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotCreateCrate);

    m_pRenameCrateAction = make_parented<QAction>(tr("Rename"), this);
    m_pRenameCrateAction->setShortcut(kRenameSidebarItemShortcutKey);
    connect(m_pRenameCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotRenameCrate);
    m_pDuplicateCrateAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotDuplicateCrate);
    m_pDeleteCrateAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteCrateAction->setShortcut(removeKeySequence);
    connect(m_pDeleteCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotDeleteCrate);
    m_pLockCrateAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotToggleCrateLock);

    m_pAutoDjTrackSourceAction = make_parented<QAction>(tr("Auto DJ Track Source"), this);
    m_pAutoDjTrackSourceAction->setCheckable(true);
    connect(m_pAutoDjTrackSourceAction.get(),
            &QAction::changed,
            this,
            &GroupedCratesFeature::slotAutoDjTrackSourceChanged);

    m_pAnalyzeCrateAction = make_parented<QAction>(tr("Analyze entire Crate"), this);
    connect(m_pAnalyzeCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotAnalyzeCrate);

    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotImportPlaylist);
    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pCreateImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotCreateImportCrate);
    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Crate as Playlist"), this);
    connect(m_pExportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotExportPlaylist);
    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::slotExportTrackFiles);
#ifdef __ENGINEPRIME__
    m_pExportAllCratesAction = make_parented<QAction>(tr("Export to Engine Prime"), this);
    connect(m_pExportAllCratesAction.get(),
            &QAction::triggered,
            this,
            &GroupedCratesFeature::exportAllCrates);
    m_pExportCrateAction = make_parented<QAction>(tr("Export to Engine Prime"), this);
    connect(m_pExportCrateAction.get(),
            &QAction::triggered,
            this,
            [this]() {
                CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
                if (crateId.isValid()) {
                    emit exportCrate(crateId);
                }
            });
#endif
}

void GroupedCratesFeature::connectLibrary(Library* pLibrary) {
    connect(pLibrary,
            &Library::trackSelected,
            this,
            [this](const TrackPointer& pTrack) {
                const auto trackId = pTrack ? pTrack->getId() : TrackId{};
                slotTrackSelected(trackId);
            });
    connect(pLibrary,
            &Library::switchToView,
            this,
            &GroupedCratesFeature::slotResetSelectedTrack);
}

void GroupedCratesFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new, duplicated or imported playlist to new crate
            &TrackCollection::crateInserted,
            this,
            &GroupedCratesFeature::slotCrateTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::crateUpdated,
            this,
            &GroupedCratesFeature::slotCrateTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateDeleted,
            this,
            &GroupedCratesFeature::slotCrateTableChanged);
    connect(m_pTrackCollection, // crate tracks hidden, unhidden or purged
            &TrackCollection::crateTracksChanged,
            this,
            &GroupedCratesFeature::slotCrateContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateSummaryChanged,
            this,
            &GroupedCratesFeature::slotUpdateCrateLabels);
}

QVariant GroupedCratesFeature::title() {
    return tr("GroupedCrates");
}

QString GroupedCratesFeature::formatRootViewHtml() const {
    QString cratesTitle = tr("Crates");
    QString cratesSummary =
            tr("Crates are a great way to help organize the music you want to "
               "DJ with.");
    QString cratesSummary2 =
            tr("Make a crate for your next gig, for your favorite electrohouse "
               "tracks, or for your most requested tracks.");
    QString cratesSummary3 =
            tr("Crates let you organize your music however you'd like!");

    QString html;
    QString createCrateLink = tr("Create New Crate");
    html.append(QStringLiteral("<h2>%1</h2>").arg(cratesTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(cratesSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(cratesSummary2));
    html.append(QStringLiteral("<p>%1</p>").arg(cratesSummary3));
    // Colorize links in lighter blue, instead of QT default dark blue.
    // Links are still different from regular text, but readable on dark/light backgrounds.
    // https://github.com/mixxxdj/mixxx/issues/9103
    html.append(
            QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                    .arg(createCrateLink));
    return html;
}

std::unique_ptr<TreeItem> GroupedCratesFeature::newTreeItemForCrateSummary(
        const CrateSummary& crateSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForCrateSummary(pTreeItem.get(), crateSummary);
    return pTreeItem;
}

void GroupedCratesFeature::updateTreeItemForCrateSummary(
        TreeItem* pTreeItem, const CrateSummary& crateSummary) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(crateSummary.getId().toVariant());
    } else {
        // The data (= CrateId) is immutable once it has been set
        DEBUG_ASSERT(CrateId(pTreeItem->getData()) == crateSummary.getId());
    }
    // Update mutable properties
    // pTreeItem->setLabel(formatLabel(crateSummary));
    pTreeItem->setIcon(crateSummary.isLocked() ? m_lockedCrateIcon : QIcon());
}

bool GroupedCratesFeature::dropAcceptChild(
        const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
    CrateId crateId(crateIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return false;
    }
    // If a track is dropped onto a crate's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    // pSource != nullptr it is a drop from inside Mixxx and indicates all
    // tracks already in the DB
    QList<TrackId> trackIds =
            m_pLibrary->trackCollectionManager()->resolveTrackIdsFromUrls(urls, !pSource);
    if (trackIds.isEmpty()) {
        return false;
    }

    m_pTrackCollection->addCrateTracks(crateId, trackIds);
    return true;
}

bool GroupedCratesFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return false;
    }
    Crate crate;
    if (!m_pTrackCollection->crates().readCrateById(crateId, &crate) ||
            crate.isLocked()) {
        return false;
    }
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

void GroupedCratesFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &GroupedCratesFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void GroupedCratesFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* GroupedCratesFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void GroupedCratesFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

void GroupedCratesFeature::oldactivateChild(const QModelIndex& index) {
    qDebug() << "   GroupedCratesFeature::activateChild()" << index;
    CrateId crateId(crateIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    m_prevSiblingCrate = CrateId();
    emit saveModelState();
    // QList<QVariantMap> groupedCrates = m_crateTableModel.getGroupedCrates();
    m_crateTableModel.selectCrate(crateId);
    emit showTrackModel(&m_crateTableModel);
    emit enableCoverArtDisplay(true);
}

bool GroupedCratesFeature::activateCrate(CrateId crateId) {
    qDebug() << "GroupedCratesFeature::activateCrate()" << crateId;
    qDebug() << "EVE EVE EVE EVE EVE GroupedCratesFeature::activateCrate()" << crateId;
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return false;
    }
    if (!m_pTrackCollection->crates().readCrateSummaryById(crateId)) {
        // this may happen if called by slotCrateTableChanged()
        // and the crate has just been deleted
        return false;
    }
    QModelIndex index = indexFromCrateId(crateId);
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    m_prevSiblingCrate = CrateId();
    emit saveModelState();
    m_crateTableModel.selectCrate(crateId);
    emit showTrackModel(&m_crateTableModel);
    emit enableCoverArtDisplay(true);
    // Update selection
    emit featureSelect(this, m_lastClickedIndex);
    return true;
}

bool GroupedCratesFeature::readLastRightClickedCrate(Crate* pCrate) const {
    CrateId crateId(crateIdFromIndex(m_lastRightClickedIndex));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        qWarning() << "Failed to determine id of selected crate";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            m_pTrackCollection->crates().readCrateById(crateId, pCrate)) {
        qWarning() << "Failed to read selected crate with id" << crateId;
        return false;
    }
    return true;
}

bool GroupedCratesFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void GroupedCratesFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction.get());
#ifdef __ENGINEPRIME__
    menu.addSeparator();
    menu.addAction(m_pExportAllCratesAction.get());
#endif
    menu.exec(globalPos);
}

void GroupedCratesFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
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

    QMenu menu(m_pSidebarWidget);
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
#ifdef __ENGINEPRIME__
    menu.addAction(m_pExportCrateAction.get());
#endif
    menu.exec(globalPos);
}

void GroupedCratesFeature::slotCreateCrate() {
    CrateId crateId =
            CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptyCrate();
    if (crateId.isValid()) {
        // expand Crates and scroll to new crate
        m_pSidebarWidget->selectChildIndex(indexFromCrateId(crateId), false);
    }
}

void GroupedCratesFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteCrate();
}

void GroupedCratesFeature::slotDeleteCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isLocked()) {
            qWarning() << "Refusing to delete locked crate" << crate;
            return;
        }
        CrateId crateId = crate.getId();
        // Store sibling id to restore selection after crate was deleted
        // to avoid the scroll position being reset to Crate root item.
        m_prevSiblingCrate = CrateId();
        if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex)) {
            storePrevSiblingCrateId(crateId);
        }

        QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
                tr("Confirm Deletion"),
                tr("Do you really want to delete crate <b>%1</b>?")
                        .arg(crate.getName()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            if (m_pTrackCollection->deleteCrate(crateId)) {
                qDebug() << "Deleted crate" << crate;
                return;
            }
        } else {
            return;
        }
    }
    qWarning() << "Failed to delete selected crate";
}

void GroupedCratesFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameCrate();
}

void GroupedCratesFeature::slotRenameCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        const QString oldName = crate.getName();
        crate.resetName();
        for (;;) {
            bool ok = false;
            auto newName =
                    QInputDialog::getText(nullptr,
                            tr("Rename Crate"),
                            tr("Enter new name for crate:"),
                            QLineEdit::Normal,
                            oldName,
                            &ok)
                            .trimmed();
            if (!ok || newName.isEmpty()) {
                return;
            }
            if (newName.isEmpty()) {
                QMessageBox::warning(nullptr,
                        tr("Renaming Crate Failed"),
                        tr("A crate cannot have a blank name."));
                continue;
            }
            if (m_pTrackCollection->crates().readCrateByName(newName)) {
                QMessageBox::warning(nullptr,
                        tr("Renaming Crate Failed"),
                        tr("A crate by that name already exists."));
                continue;
            }
            crate.setName(std::move(newName));
            DEBUG_ASSERT(crate.hasName());
            break;
        }

        if (!m_pTrackCollection->updateCrate(crate)) {
            qDebug() << "Failed to rename crate" << crate;
        }
    } else {
        qDebug() << "Failed to rename selected crate";
    }
}

void GroupedCratesFeature::slotDuplicateCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        CrateId newCrateId =
                CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                        .duplicateCrate(crate);
        if (newCrateId.isValid()) {
            qDebug() << "Duplicate crate" << crate << ", new crate:" << newCrateId;
            return;
        }
    }
    qDebug() << "Failed to duplicate selected crate";
}

void GroupedCratesFeature::slotToggleCrateLock() {
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

void GroupedCratesFeature::slotAutoDjTrackSourceChanged() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isAutoDjSource() != m_pAutoDjTrackSourceAction->isChecked()) {
            crate.setAutoDjSource(m_pAutoDjTrackSourceAction->isChecked());
            m_pTrackCollection->updateCrate(crate);
        }
    }
}

QModelIndex GroupedCratesFeature::oldrebuildChildModel(CrateId selectedCrateId) {
    qDebug() << "GroupedCratesFeature::rebuildChildModel()" << selectedCrateId;

    m_lastRightClickedIndex = QModelIndex();

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    std::vector<std::unique_ptr<TreeItem>> modelRows;
    modelRows.reserve(m_pTrackCollection->crates().countCrates());

    int selectedRow = -1;
    CrateSummarySelectResult crateSummaries(
            m_pTrackCollection->crates().selectCrateSummaries());
    CrateSummary crateSummary;
    while (crateSummaries.populateNext(&crateSummary)) {
        modelRows.push_back(newTreeItemForCrateSummary(crateSummary));
        if (selectedCrateId == crateSummary.getId()) {
            // save index for selection
            selectedRow = static_cast<int>(modelRows.size()) - 1;
        }
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);

    // Update rendering of crates depending on the currently selected track
    slotTrackSelected(m_selectedTrackId);

    if (selectedRow >= 0) {
        return m_pSidebarModel->index(selectedRow, 0);
    } else {
        return QModelIndex();
    }
}

void GroupedCratesFeature::oldupdateChildModel(const QSet<CrateId>& updatedCrateIds) {
    const CrateStorage& crateStorage = m_pTrackCollection->crates();
    for (const CrateId& crateId : updatedCrateIds) {
        QModelIndex index = indexFromCrateId(crateId);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        CrateSummary crateSummary;
        VERIFY_OR_DEBUG_ASSERT(
                crateStorage.readCrateSummaryById(crateId, &crateSummary)) {
            continue;
        }
        updateTreeItemForCrateSummary(
                m_pSidebarModel->getItem(index), crateSummary);
        m_pSidebarModel->triggerRepaint(index);
    }

    if (m_selectedTrackId.isValid()) {
        // Crates containing the currently selected track might
        // have been modified.
        slotTrackSelected(m_selectedTrackId);
    }
}

CrateId GroupedCratesFeature::crateIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return CrateId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return CrateId();
    }
    return CrateId(item->getData());
}

QModelIndex GroupedCratesFeature::indexFromCrateId(CrateId crateId) const {
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return QModelIndex();
    }
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                (CrateId(pTreeItem->getData()) == crateId)) {
            return index;
        }
    }
    qDebug() << "Tree item for crate not found:" << crateId;
    return QModelIndex();
}

void GroupedCratesFeature::slotImportPlaylist() {
    // qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();

    QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }

    // Update the import/export crate directory
    QString fileDirectory(playlistFile);
    fileDirectory.truncate(playlistFile.lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory));

    CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        qDebug() << "Importing playlist file" << playlistFile << "into crate"
                 << crateId << crate;
    } else {
        qDebug() << "Importing playlist file" << playlistFile << "into crate"
                 << crateId << crate << "failed!";
        return;
    }

    slotImportPlaylistFile(playlistFile, crateId);
    activateChild(m_lastRightClickedIndex);
}

void GroupedCratesFeature::slotImportPlaylistFile(const QString& playlistFile, CrateId crateId) {
    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.
    // TODO(XXX): Parsing a list of track locations from a playlist file
    // is a general task and should be implemented separately.
    QList<QString> locations = Parser().parse(playlistFile);
    if (locations.empty()) {
        return;
    }

    if (crateId == m_crateTableModel.selectedCrate()) {
        // Add tracks directly to the model
        m_crateTableModel.addTracks(QModelIndex(), locations);
    } else {
        // Create a temporary table model since the main one might have another
        // crate selected which is not the crate that received the right-click.
        std::unique_ptr<CrateTableModel> pCrateTableModel =
                std::make_unique<CrateTableModel>(
                        this, m_pLibrary->trackCollectionManager(), m_pConfig);
        pCrateTableModel->selectCrate(crateId);
        pCrateTableModel->select();
        pCrateTableModel->addTracks(QModelIndex(), locations);
    }
}

void GroupedCratesFeature::slotCreateImportCrate() {
    // Get file to read
    const QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
    if (playlistFiles.isEmpty()) {
        return;
    }

    // Set last import directory
    QString fileDirectory(playlistFiles.first());
    fileDirectory.truncate(playlistFiles.first().lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory));

    CrateId lastCrateId;

    // For each selected file create a new crate
    for (const QString& playlistFile : playlistFiles) {
        const QFileInfo fileInfo(playlistFile);

        Crate crate;

        // Get a valid name
        const QString baseName = fileInfo.baseName();
        for (int i = 0;; ++i) {
            auto name = baseName;
            if (i > 0) {
                name += QStringLiteral(" %1").arg(i);
            }
            name = name.trimmed();
            if (!name.isEmpty()) {
                if (!m_pTrackCollection->crates().readCrateByName(name)) {
                    // unused crate name found
                    crate.setName(std::move(name));
                    DEBUG_ASSERT(crate.hasName());
                    break; // terminate loop
                }
            }
        }

        if (!m_pTrackCollection->insertCrate(crate, &lastCrateId)) {
            QMessageBox::warning(nullptr,
                    tr("Crate Creation Failed"),
                    tr("An unknown error occurred while creating crate: ") +
                            crate.getName());
            return;
        }

        slotImportPlaylistFile(playlistFile, lastCrateId);
    }
    activateCrate(lastCrateId);
}

void GroupedCratesFeature::slotAnalyzeCrate() {
    if (m_lastRightClickedIndex.isValid()) {
        CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
        if (crateId.isValid()) {
            QList<AnalyzerScheduledTrack> tracks;
            tracks.reserve(
                    m_pTrackCollection->crates().countCrateTracks(crateId));
            {
                CrateTrackSelectResult crateTracks(
                        m_pTrackCollection->crates().selectCrateTracksSorted(
                                crateId));
                while (crateTracks.next()) {
                    tracks.append(crateTracks.trackId());
                }
            }
            emit analyzeTracks(tracks);
        }
    }
}

void GroupedCratesFeature::slotExportPlaylist() {
    CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        qDebug() << "Exporting crate" << crateId << crate;
    } else {
        qDebug() << "Failed to export crate" << crateId;
        return;
    }

    QString lastCrateDirectory = m_pConfig->getValue(
            kConfigKeyLastImportExportCrateDirectoryKey);
    //    QString lastCrateDirectory = m_pConfig->getValue(
    //            kConfigKeyLastImportExportCrateDirectoryKey,
    //            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    // Open a dialog to let the user choose the file location for crate export.
    // The location is set to the last used directory for import/export and the file
    // name to the playlist name.
    const QString fileLocation = getFilePathWithVerifiedExtensionFromFileDialog(
            tr("Export Crate"),
            lastCrateDirectory.append("/").append(crate.getName()),
            tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;PLS Playlist "
               "(*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"),
            tr("M3U Playlist (*.m3u)"));
    // Exit method if user cancelled the open dialog.
    if (fileLocation.isEmpty()) {
        return;
    }
    // Update the import/export crate directory
    QString fileDirectory(fileLocation);
    fileDirectory.truncate(fileLocation.lastIndexOf("/"));
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // check config if relative paths are desired
    bool useRelativePath =
            m_pConfig->getValue<bool>(
                    kUseRelativePathOnExportConfigKey);

    // Create list of files of the crate
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<CrateTableModel> pCrateTableModel =
            std::make_unique<CrateTableModel>(
                    this, m_pLibrary->trackCollectionManager(), m_pConfig);
    pCrateTableModel->selectCrate(crateId);
    pCrateTableModel->select();

    if (fileLocation.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(fileLocation, pCrateTableModel.get(), useRelativePath);
    } else if (fileLocation.endsWith(".txt", Qt::CaseInsensitive)) {
        ParserCsv::writeReadableTextFile(fileLocation, pCrateTableModel.get(), false);
    } else {
        // populate a list of files of the crate
        QList<QString> playlistItems;
        int rows = pCrateTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pCrateTableModel->index(i, 0);
            playlistItems << pCrateTableModel->getTrackLocation(index);
        }
        exportPlaylistItemsIntoFile(
                fileLocation,
                playlistItems,
                useRelativePath);
    }
}

void GroupedCratesFeature::slotExportTrackFiles() {
    CrateId crateId(crateIdFromIndex(m_lastRightClickedIndex));
    if (!crateId.isValid()) {
        return;
    }
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<CrateTableModel> pCrateTableModel =
            std::make_unique<CrateTableModel>(
                    this, m_pLibrary->trackCollectionManager(), m_pConfig);
    pCrateTableModel->selectCrate(crateId);
    pCrateTableModel->select();

    int rows = pCrateTableModel->rowCount();
    TrackPointerList trackpointers;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pCrateTableModel->index(i, 0);
        auto pTrack = pCrateTableModel->getTrack(index);
        VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
            continue;
        }
        trackpointers.push_back(pTrack);
    }

    if (trackpointers.isEmpty()) {
        return;
    }

    TrackExportWizard track_export(nullptr, m_pConfig, trackpointers);
    track_export.exportTracks();
}

void GroupedCratesFeature::storePrevSiblingCrateId(CrateId crateId) {
    QModelIndex actIndex = indexFromCrateId(crateId);
    m_prevSiblingCrate = CrateId();
    for (int i = (actIndex.row() + 1); i >= (actIndex.row() - 1); i -= 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_prevSiblingCrate = crateIdFromIndex(newIndex);
            }
        }
    }
}

void GroupedCratesFeature::slotCrateTableChanged(CrateId crateId) {
    Q_UNUSED(crateId);
    //    QList<QVariantMap> groupedCrates = m_crateTableModel.getGroupedCrates();
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        // If the previously selected crate was loaded to the tracks table and
        // selected in the sidebar try to activate that or a sibling
        rebuildChildModel();
        if (!activateCrate(m_crateTableModel.selectedCrate())) {
            // probably last clicked crate was deleted, try to
            // select the stored sibling
            if (m_prevSiblingCrate.isValid()) {
                activateCrate(m_prevSiblingCrate);
            }
        }
    } else {
        // No valid selection to restore
        rebuildChildModel();
    }
}

void GroupedCratesFeature::slotCrateContentChanged(CrateId crateId) {
    QSet<CrateId> updatedCrateIds;
    updatedCrateIds.insert(crateId);
    updateChildModel(updatedCrateIds);
}

void GroupedCratesFeature::slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds) {
    updateChildModel(updatedCrateIds);
}

void GroupedCratesFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreateCrate();
    } else {
        qDebug() << "Unknown crate link clicked" << link;
    }
}

void GroupedCratesFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    std::vector<CrateId> sortedTrackCrates;
    if (m_selectedTrackId.isValid()) {
        CrateTrackSelectResult trackCratesIter(
                m_pTrackCollection->crates().selectTrackCratesSorted(m_selectedTrackId));
        while (trackCratesIter.next()) {
            sortedTrackCrates.push_back(trackCratesIter.crateId());
        }
    }

    // Recursive lambda to set bold for crates and groups
    auto setBoldForItemsRecursive =
            [&](TreeItem* pItem,
                    const auto& setBoldForItemsRecursiveRef) -> bool {
        if (!pItem) {
            return false;
        }

        bool isBold = false;

        // Check if the current item is a crate and contains the selected track
        if (!pItem->children().isEmpty()) {
            // Recursively check child items
            for (TreeItem* pChild : pItem->children()) {
                isBold |= setBoldForItemsRecursiveRef(pChild, setBoldForItemsRecursiveRef);
            }
        } else {
            // Check crates directly
            isBold = m_selectedTrackId.isValid() &&
                    std::binary_search(
                            sortedTrackCrates.begin(),
                            sortedTrackCrates.end(),
                            CrateId(pItem->getData()));
        }

        // Set bold status for the current item
        pItem->setBold(isBold);
        return isBold;
    };

    // Start the recursion from the root
    for (TreeItem* pTreeItem : pRootItem->children()) {
        setBoldForItemsRecursive(pTreeItem, setBoldForItemsRecursive);
    }

    m_pSidebarModel->triggerRepaint();
}

void GroupedCratesFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}

QModelIndex GroupedCratesFeature::rebuildChildModel(CrateId selectedCrateId) {
    if (sDebug) {
        qDebug() << "[GROUPEDCRATESFEATURE] -> rebuildChildModel()" << selectedCrateId;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // remember open/close state of group
    QMap<QString, bool> groupExpandedStates;
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex groupIndex = m_pSidebarModel->index(row, 0);
        if (groupIndex.isValid()) {
            TreeItem* pGroupItem = m_pSidebarModel->getItem(groupIndex);
            if (pGroupItem) {
                const QString& groupName = pGroupItem->getLabel();
                groupExpandedStates[groupName] = m_pSidebarWidget->isExpanded(groupIndex);
                if (sDebug) {
                    qDebug() << "[GroupedCratesFeature] Saved open/close state "
                                "for group:"
                             << groupName << "->"
                             << groupExpandedStates[groupName];
                }
            }
        }
    }

    m_lastRightClickedIndex = QModelIndex();
    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    QList<QVariantMap> groupedCrates = m_crateTableModel.getGroupedCrates();
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedCratesVarLengthMask"));

    if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedCratesLength")) == 0) {
        // Fixed prefix length
        QMap<QString, int> groupCounts;
        for (int i = 0; i < groupedCrates.size(); ++i) {
            const auto& crateData = groupedCrates[i];
            const QString& groupName = crateData["group_name"].toString();
            groupCounts[groupName]++;
        }

        QMap<QString, TreeItem*> groupItems;
        std::vector<std::unique_ptr<TreeItem>> modelRows;

        for (int i = 0; i < groupedCrates.size(); ++i) {
            const auto& crateData = groupedCrates[i];
            const QString& groupName = crateData["group_name"].toString();
            CrateId crateId(crateData["crate_id"]);

            CrateSummary crateSummary;
            if (!m_pTrackCollection->crates().readCrateSummaryById(crateId, &crateSummary)) {
                qWarning() << "[GROUPEDCRATESFEATURE] -> Failed to fetch summary "
                              "for crate ID:"
                           << crateId;
                continue;
            }

            const QString& crateSummaryName = formatLabel(crateSummary);

            if (groupCounts[groupName] > 1) {
                TreeItem* pGroupItem = groupItems.value(groupName, nullptr);
                if (!pGroupItem) {
                    auto newGroup = std::make_unique<TreeItem>(groupName, kInvalidCrateId);
                    pGroupItem = newGroup.get();
                    groupItems.insert(groupName, pGroupItem);
                    modelRows.push_back(std::move(newGroup));
                }

                const QString& displayCrateName =
                        crateSummaryName.mid(groupName.length()).trimmed();
                if (sDebug) {
                    qDebug() << "[GROUPEDCRATESFEATURE] -> crateSummaryName - "
                                "displayCrateName = "
                             << crateSummaryName << " - " << displayCrateName;
                }

                TreeItem* pChildItem = pGroupItem->appendChild(
                        displayCrateName, crateId.toVariant().toInt());
                pChildItem->setFullPath(groupName + delimiter + displayCrateName);
                updateTreeItemForCrateSummary(pChildItem, crateSummary);
                if (sDebug) {
                    qDebug() << "[GROUPEDCRATESFEATURE] Added CrateId to group:"
                             << crateId << "Group:" << groupName;
                }
            } else {
                auto newCrate = std::make_unique<TreeItem>(
                        crateSummaryName, crateId.toVariant().toInt());
                newCrate->setFullPath(crateSummaryName);
                updateTreeItemForCrateSummary(newCrate.get(), crateSummary);

                modelRows.push_back(std::move(newCrate));
            }
        }

        m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);
        slotTrackSelected(m_selectedTrackId);

    } else {
        // variable group prefix length with mask
        QMap<QString, QList<QVariantMap>> topLevelGroups;
        for (int i = 0; i < groupedCrates.size(); ++i) {
            const auto& crateData = groupedCrates[i];
            const QString& groupName = crateData["group_name"].toString();
            const QString& topGroup = groupName.section(delimiter, 0, 0);
            topLevelGroups[topGroup].append(crateData);
        }

        if (sDebug) {
            qDebug() << "[GROUPEDCRATESFEATURE] Top-level groups:";
            for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
                qDebug() << "Group:" << it.key() << "-> Crates:" << it.value().size();
            }
        }
        // lambda function to build tree
        std::function<void(
                const QString&, const QList<QVariantMap>&, TreeItem*)>
                buildTreeStructure;
        buildTreeStructure = [&](const QString& currentPath,
                                     const QList<QVariantMap>& crates,
                                     TreeItem* pParentItem) {
            QMap<QString, QList<QVariantMap>> subgroupedCrates;

            for (const QVariantMap& crateData : crates) {
                const QString& groupName = crateData["group_name"].toString();

                if (sDebug) {
                    qDebug() << "[GROUPEDCRATESFEATURE] Processing crate with "
                                "groupName:"
                             << groupName << "currentPath:" << currentPath;
                }

                if (!groupName.startsWith(currentPath)) {
                    if (sDebug) {
                        qDebug() << "[GROUPEDCRATESFEATURE] Skipping crate. "
                                    "Group name does not match path:"
                                 << groupName << "Current path:" << currentPath;
                    }
                    continue;
                }

                const QString& remainingPath = groupName.mid(currentPath.length());
                int delimiterPos = remainingPath.indexOf(delimiter);

                if (delimiterPos >= 0) {
                    const QString& subgroupName = remainingPath.left(delimiterPos);
                    subgroupedCrates[subgroupName].append(crateData);
                    if (sDebug) {
                        qDebug() << "[GROUPEDCRATESFEATURE] Added crate to "
                                    "subgroup:"
                                 << subgroupName
                                 << "Remaining path:" << remainingPath;
                    }
                } else {
                    CrateId crateId(crateData["crate_id"]);
                    CrateSummary crateSummary;
                    if (!m_pTrackCollection->crates().readCrateSummaryById(
                                crateId, &crateSummary)) {
                        qWarning() << "[GROUPEDCRATESFEATURE] Failed to fetch "
                                      "summary for crate ID:"
                                   << crateId;
                        continue;
                    }

                    const QString& displayCrateName =
                            formatLabel(crateSummary).mid(currentPath.length());

                    TreeItem* pChildItem = pParentItem->appendChild(
                            displayCrateName.trimmed(), crateId.toVariant());
                    pChildItem->setFullPath(currentPath + delimiter + displayCrateName);
                    updateTreeItemForCrateSummary(pChildItem, crateSummary);
                    if (sDebug) {
                        qDebug() << "[GROUPEDCRATESFEATURE] Added crate to "
                                    "parent:"
                                 << displayCrateName
                                 << "Parent:" << pParentItem->getLabel();
                    }
                }
            }

            for (auto it = subgroupedCrates.constBegin(); it != subgroupedCrates.constEnd(); ++it) {
                const QString& subgroupName = it.key();
                const QList<QVariantMap>& subgroupCrates = it.value();
                if (!subgroupCrates.isEmpty()) {
                    if (subgroupCrates.size() > 1) {
                        // subgroup has > 1 crate -> create subgroup
                        auto pNewSubgroup = std::make_unique<TreeItem>(
                                subgroupName, kInvalidCrateId);
                        TreeItem* pSubgroupItem = pNewSubgroup.get();
                        pParentItem->insertChild(pParentItem->childCount(),
                                std::move(pNewSubgroup));
                        if (sDebug) {
                            qDebug() << "[GROUPEDCRATESFEATURE] Created subgroup:" << subgroupName
                                     << "Parent:" << pParentItem->getLabel();
                        }

                        // loop into the subgroup
                        buildTreeStructure(
                                currentPath + subgroupName + delimiter,
                                subgroupCrates,
                                pSubgroupItem);
                    } else {
                        // only one crate -> directly under the parent, NO subgroup
                        const QVariantMap& crateData = subgroupCrates.first();
                        CrateId crateId(crateData["crate_id"]);
                        CrateSummary crateSummary;
                        if (!m_pTrackCollection->crates().readCrateSummaryById(
                                    crateId, &crateSummary)) {
                            qWarning() << "[GROUPEDCRATESFEATURE] Failed to "
                                          "fetch summary for crate ID:"
                                       << crateId;
                            continue;
                        }

                        const QString& displayCrateName =
                                formatLabel(crateSummary)
                                        .mid(currentPath.length());

                        TreeItem* pChildItem = pParentItem->appendChild(
                                displayCrateName.trimmed(), crateId.toVariant());
                        pChildItem->setFullPath(currentPath + delimiter + displayCrateName);
                        updateTreeItemForCrateSummary(pChildItem, crateSummary);
                        if (sDebug) {
                            qDebug() << "[GROUPEDCRATESFEATURE] Added single crate to parent:"
                                     << displayCrateName
                                     << "Parent:" << pParentItem->getLabel();
                        }
                    }
                }
            }
        };
        // building rootlevel groups
        for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
            const QString& groupName = it.key();
            const QList<QVariantMap>& crates = it.value();

            if (crates.size() > 1) {
                auto pNewGroup = std::make_unique<TreeItem>(groupName, kInvalidCrateId);
                TreeItem* pGroupItem = pNewGroup.get();
                pRootItem->insertChild(pRootItem->childCount(), std::move(pNewGroup));
                if (sDebug) {
                    qDebug() << "[GROUPEDCRATESFEATURE] Created top-level group:" << groupName;
                }

                buildTreeStructure(groupName + delimiter, crates, pGroupItem);
            } else {
                const QVariantMap& crateData = crates.first();
                CrateId crateId(crateData["crate_id"]);
                CrateSummary crateSummary;

                if (!m_pTrackCollection->crates().readCrateSummaryById(crateId, &crateSummary)) {
                    qWarning() << "[GROUPEDCRATESFEATURE] Failed to fetch "
                                  "summary for crate ID:"
                               << crateId;
                    continue;
                }

                const QString& displayCrateName = formatLabel(crateSummary);
                TreeItem* pChildItem = pRootItem->appendChild(
                        displayCrateName.trimmed(), crateId.toVariant());
                updateTreeItemForCrateSummary(pChildItem, crateSummary);
                if (sDebug) {
                    qDebug() << "[GROUPEDCRATESFEATURE] Added crate to root:" << displayCrateName;
                }
            }
        }
    }
    // store open/close state of groups
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex groupIndex = m_pSidebarModel->index(row, 0);
        if (groupIndex.isValid()) {
            TreeItem* pGroupItem = m_pSidebarModel->getItem(groupIndex);
            if (pGroupItem) {
                const QString& groupName = pGroupItem->getLabel();
                if (groupExpandedStates.contains(groupName)) {
                    m_pSidebarWidget->setExpanded(groupIndex, groupExpandedStates[groupName]);
                    if (sDebug) {
                        qDebug() << "[GROUPEDCRATESFEATURE] Restored expanded "
                                    "state for group:"
                                 << groupName << "->"
                                 << groupExpandedStates[groupName];
                    }
                }
            }
        }
    }

    if (previouslySelectedIndex.isValid()) {
        return previouslySelectedIndex;
    }

    return QModelIndex();
}

void GroupedCratesFeature::updateChildModel(const QSet<CrateId>& updatedCrateIds) {
    if (sDebug) {
        qDebug() << "[GROUPEDCRATESFEATURE] -> updateChildModel() -> Updating "
                    "crates"
                 << updatedCrateIds;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // Fetch grouped crates using CrateTableModel
    QList<QVariantMap> groupedCrates = m_crateTableModel.getGroupedCrates();

    QMap<QString, QList<QVariantMap>> groupedCratesMap;
    for (int i = 0; i < groupedCrates.size(); ++i) {
        const auto& crateData = groupedCrates[i];
        groupedCratesMap[crateData["group_name"].toString()].append(crateData);
    }

    // Update full paths recursively for all items starting from the root
    updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());

    // Update or rebuild items
    for (const CrateId& crateId : updatedCrateIds) {
        // Find the updated crate in groupedCrates
        auto updatedGroup = std::find_if(
                groupedCrates.begin(),
                groupedCrates.end(),
                [&crateId](const QVariantMap& crateData) {
                    return crateData["crate_id"].toInt() == crateId.toVariant().toInt();
                });

        if (updatedGroup != groupedCrates.end()) {
            QModelIndex index = indexFromCrateId(crateId);
            if (index.isValid()) {
                // Update the existing item
                TreeItem* pItem = m_pSidebarModel->getItem(index);
                VERIFY_OR_DEBUG_ASSERT(pItem != nullptr) {
                    continue;
                }
                pItem->setData((*updatedGroup)["crate_name"].toString());
                updateTreeItemForCrateSummary(pItem, CrateSummary(crateId));

                // Update fullPath for the entire tree under this item
                updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());

                m_pSidebarModel->triggerRepaint(index);
            } else {
                // Rebuild the group if the crate is missing
                rebuildChildModel(crateId);
            }
        }
    }
    if (previouslySelectedIndex.isValid()) {
        m_lastRightClickedIndex = previouslySelectedIndex;
    }
}

void GroupedCratesFeature::activateChild(const QModelIndex& index) {
    if (sDebug) {
        qDebug() << "[GROUPEDCRATESFEATURE] -> activateChild() -> index" << index;
    }

    CrateId crateId(crateIdFromIndex(index));

    if (crateId.toString() == "-1") {
        // Group activated
        if (sDebug) {
            qDebug() << "[GroupedCratesFeature] -> activateChild() -> Group activated";
        }
        const QString& fullPath = fullPathFromIndex(index);
        if (fullPath.isEmpty()) {
            qWarning() << "[GROUPEDCRATESFEATURE] -> activateChild() -> Group "
                          "activated: No valid full path for index: "
                       << index;
            return;
        }
        if (sDebug) {
            qDebug() << "[GroupedCratesFeature] -> activateChild() -> Group "
                        "activated -> fullPath:"
                     << fullPath;
        }

        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        m_prevSiblingCrate = CrateId();
        emit saveModelState();
        emit disableSearch();
        emit enableCoverArtDisplay(false);

        m_crateTableModel.selectCrateGroup(fullPath);
        emit featureSelect(this, m_lastClickedIndex);
        emit showTrackModel(&m_crateTableModel);
    } else {
        // Crate activated
        if (sDebug) {
            qDebug() << "[GROUPEDCRATESFEATURE] -> activateChild() -> Child "
                        "crate activated -> crateId: "
                     << crateId;
        }
        VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
            return;
        }
        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        m_prevSiblingCrate = CrateId();
        emit saveModelState();
        m_crateTableModel.selectCrate(crateId);
        emit showTrackModel(&m_crateTableModel);
        emit enableCoverArtDisplay(true);
    }
}

QString GroupedCratesFeature::fullPathFromIndex(const QModelIndex& index) const {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedCratesVarLengthMask"));
    if (!index.isValid()) {
        return QString();
    }

    TreeItem* pItem = m_pSidebarModel->getItem(index);
    if (!pItem) {
        return QString();
    }

    QString fullPath;
    TreeItem* currentItem = pItem;
    while (currentItem) {
        if (!fullPath.isEmpty()) {
            // Prepend delimiter
            fullPath.prepend(delimiter);
        }
        // Prepend current item's label
        fullPath.prepend(currentItem->getLabel());
        currentItem = currentItem->parent();
    }

    // remove the last prepended delimiter (we don't know the depth of the tree)
    // if another root level crate exists that is not in the group
    // (beginning of the name equal to root level groop name),
    // the member tracks would be added to the group,
    // with added delimiter only tracks in group member crates are added
    fullPath = fullPath.mid(delimiter.length()).append(delimiter);
    return fullPath;
}

QString GroupedCratesFeature::groupNameFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        // If index Invalid -> return an empty string
        return QString();
    }

    TreeItem* pItem = m_pSidebarModel->getItem(index);
    if (!pItem) {
        // ig no item found for this index -> return an empty string
        return QString();
    }
    // if index & label found -> return label
    return pItem->getLabel();
}

void GroupedCratesFeature::updateFullPathRecursive(TreeItem* pItem, const QString& parentPath) {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedCratesVarLengthMask"));
    if (!pItem) {
        return;
    }

    QString currentFullPath = parentPath.isEmpty()
            ? pItem->getLabel()
            : parentPath + delimiter + pItem->getLabel();
    pItem->setFullPath(currentFullPath);

    if (sDebug) {
        qDebug() << "[GROUPEDCRATESFEATURE] -> Updated full path for item: " << pItem->getLabel()
                 << " FullPath: " << currentFullPath;
    }

    for (TreeItem* pChild : pItem->children()) {
        updateFullPathRecursive(pChild, currentFullPath);
    }
}
