#include "library/trackset/crate/cratefeature.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QStandardPaths>
#include <algorithm>
#include <vector>

#include "analyzer/analyzerscheduledtrack.h"
#include "control/controlobject.h"
#include "control/controlproxy.h"
#include "control/pollingcontrolproxy.h"
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
#include "moc_cratefeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarypreparationwindow.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {
const bool sDebugCrateFeature = false;

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

CrateFeature::CrateFeature(Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary, pConfig, "CRATEHOME", QStringLiteral("crates")),
          m_lockedCrateIcon(":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_crateTableModel(this, pLibrary->trackCollectionManager()) {
    initActions();

    // construct child model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
    rebuildChildModel();

    connectLibrary(pLibrary);
    connectTrackCollection();
}

void CrateFeature::initActions() {
    m_pShowTrackModelInPreparationWindowAction =
            make_parented<QAction>(tr("Show in Preparation Window"), this);
    connect(m_pShowTrackModelInPreparationWindowAction,
            &QAction::triggered,
            this,
            &CrateFeature::slotShowInPreparationWindow);

    m_pCreateCrateAction = make_parented<QAction>(tr("Create New Crate"), this);
    connect(m_pCreateCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotCreateCrate);

    m_pRenameCrateAction = make_parented<QAction>(tr("Rename"), this);
    connect(m_pRenameCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotRenameCrate);
    m_pDuplicateCrateAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotDuplicateCrate);
    m_pDeleteCrateAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteCrateAction->setShortcut(removeKeySequence);
    connect(m_pDeleteCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotDeleteCrate);
    m_pLockCrateAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotToggleCrateLock);

    m_pAutoDjTrackSourceAction = make_parented<QAction>(tr("Auto DJ Track Source"), this);
    m_pAutoDjTrackSourceAction->setCheckable(true);
    connect(m_pAutoDjTrackSourceAction.get(),
            &QAction::changed,
            this,
            &CrateFeature::slotAutoDjTrackSourceChanged);

    m_pAnalyzeCrateAction = make_parented<QAction>(tr("Analyze entire Crate"), this);
    connect(m_pAnalyzeCrateAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotAnalyzeCrate);

    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotImportPlaylist);
    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Crate"), this);
    connect(m_pCreateImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotCreateImportCrate);
    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Crate as Playlist"), this);
    connect(m_pExportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotExportPlaylist);
    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::slotExportTrackFiles);
#ifdef __ENGINEPRIME__
    m_pExportAllCratesAction = make_parented<QAction>(tr("Export to Engine DJ"), this);
    connect(m_pExportAllCratesAction.get(),
            &QAction::triggered,
            this,
            &CrateFeature::exportAllCrates);
    m_pExportCrateAction = make_parented<QAction>(tr("Export to Engine DJ"), this);
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

void CrateFeature::connectLibrary(Library* pLibrary) {
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
            &CrateFeature::slotResetSelectedTrack);
}

void CrateFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new, duplicated or imported playlist to new crate
            &TrackCollection::crateInserted,
            this,
            &CrateFeature::slotCrateTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::crateUpdated,
            this,
            &CrateFeature::slotCrateTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateDeleted,
            this,
            &CrateFeature::slotCrateTableChanged);
    connect(m_pTrackCollection, // crate tracks hidden, unhidden or purged
            &TrackCollection::crateTracksChanged,
            this,
            &CrateFeature::slotCrateContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::crateSummaryChanged,
            this,
            &CrateFeature::slotUpdateCrateLabels);
}

QVariant CrateFeature::title() {
    return tr("Crates");
}

QString CrateFeature::formatRootViewHtml() const {
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
    //Colorize links in lighter blue, instead of QT default dark blue.
    //Links are still different from regular text, but readable on dark/light backgrounds.
    //https://github.com/mixxxdj/mixxx/issues/9103
    html.append(
            QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                    .arg(createCrateLink));
    return html;
}

std::unique_ptr<TreeItem> CrateFeature::newTreeItemForCrateSummary(
        const CrateSummary& crateSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForCrateSummary(pTreeItem.get(), crateSummary);
    return pTreeItem;
}

void CrateFeature::updateTreeItemForCrateSummary(
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
    pTreeItem->setLabel(formatLabel(crateSummary));
    pTreeItem->setIcon(crateSummary.isLocked() ? m_lockedCrateIcon : QIcon());
}

bool CrateFeature::dropAcceptChild(
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

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
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

void CrateFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &CrateFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void CrateFeature::bindLibraryPreparationWindowWidget(
        WLibraryPreparationWindow* libraryPreparationWindowWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryPreparationWindowWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &CrateFeature::htmlLinkClicked);
    libraryPreparationWindowWidget->registerView(m_rootViewName, edit);
}

void CrateFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* CrateFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void CrateFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

void CrateFeature::activateChild(const QModelIndex& index) {
    qDebug() << "   CrateFeature::activateChild()" << index;
    CrateId crateId(crateIdFromIndex(index));
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

bool CrateFeature::activateCrate(CrateId crateId) {
    qDebug() << "CrateFeature::activateCrate()" << crateId;
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

void CrateFeature::slotShowInPreparationWindow() {
    CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
    if (sDebugCrateFeature) {
        qDebug() << "   CrateFeature::slotShowInPreparationWindow()" << crateId;
    }

    if (ControlObject::exists(ConfigKey("[Skin]", "show_preparation_window"))) {
        auto proxy = std::make_unique<PollingControlProxy>("[Skin]", "show_preparation_window");
        proxy->set(1);
    }

    emit saveModelState();
    m_crateTableModel.selectCrate(crateId);
    emit showTrackModelInPreparationWindow(&m_crateTableModel);
    emit enableCoverArtDisplay(true);
    emit featureSelect(this, m_lastClickedIndex);
}

bool CrateFeature::readLastRightClickedCrate(Crate* pCrate) const {
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

bool CrateFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void CrateFeature::onRightClick(const QPoint& globalPos) {
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

void CrateFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
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
    m_pImportPlaylistAction->setEnabled(!crate.isLocked());

    m_pAutoDjTrackSourceAction->setChecked(crate.isAutoDjSource());

    m_pLockCrateAction->setText(crate.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pShowTrackModelInPreparationWindowAction);
    menu.addSeparator();
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
    menu.addAction(m_pImportPlaylistAction.get());
    menu.addAction(m_pExportPlaylistAction.get());
    menu.addAction(m_pExportTrackFilesAction.get());
#ifdef __ENGINEPRIME__
    menu.addAction(m_pExportCrateAction.get());
#endif
    menu.exec(globalPos);
}

void CrateFeature::slotCreateCrate() {
    CrateId crateId =
            CrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptyCrate();
    if (crateId.isValid()) {
        // expand Crates and scroll to new crate
        m_pSidebarWidget->selectChildIndex(indexFromCrateId(crateId), false);
    }
}

void CrateFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteCrate();
}

void CrateFeature::slotDeleteCrate() {
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

void CrateFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameCrate();
}

void CrateFeature::slotRenameCrate() {
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

void CrateFeature::slotDuplicateCrate() {
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

void CrateFeature::updateChildModel(const QSet<CrateId>& updatedCrateIds) {
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

void CrateFeature::slotImportPlaylist() {
    //qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();

    QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }

    // Update the import/export crate directory
    QFileInfo fileDirectory(playlistFile);
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

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

void CrateFeature::slotImportPlaylistFile(const QString& playlistFile, CrateId crateId) {
    Crate crate;
    if (!m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        return;
    }
    if (crate.isLocked()) {
        return;
    }
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
                std::make_unique<CrateTableModel>(this, m_pLibrary->trackCollectionManager());
        pCrateTableModel->selectCrate(crateId);
        pCrateTableModel->select();
        pCrateTableModel->addTracks(QModelIndex(), locations);
    }
}

void CrateFeature::slotCreateImportCrate() {
    // Get file to read
    const QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
    if (playlistFiles.isEmpty()) {
        return;
    }

    // Set last import directory
    QFileInfo fileDirectory(playlistFiles.first());
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    CrateId lastCrateId;

    // For each selected file create a new crate
    for (const QString& playlistFile : playlistFiles) {
        const QFileInfo fileInfo(playlistFile);

        Crate crate;

        // Get a valid name
        const QString baseName = fileInfo.completeBaseName();
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

void CrateFeature::slotAnalyzeCrate() {
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

void CrateFeature::slotExportPlaylist() {
    CrateId crateId = crateIdFromIndex(m_lastRightClickedIndex);
    Crate crate;
    if (m_pTrackCollection->crates().readCrateById(crateId, &crate)) {
        qDebug() << "Exporting crate" << crateId << crate;
    } else {
        qDebug() << "Failed to export crate" << crateId;
        return;
    }

    QString lastCrateDirectory = m_pConfig->getValue(
            kConfigKeyLastImportExportCrateDirectoryKey,
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

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
    QFileInfo fileDirectory(fileLocation);
    m_pConfig->set(kConfigKeyLastImportExportCrateDirectoryKey,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

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
            std::make_unique<CrateTableModel>(this, m_pLibrary->trackCollectionManager());
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

void CrateFeature::slotExportTrackFiles() {
    CrateId crateId(crateIdFromIndex(m_lastRightClickedIndex));
    if (!crateId.isValid()) {
        return;
    }
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<CrateTableModel> pCrateTableModel =
            std::make_unique<CrateTableModel>(this, m_pLibrary->trackCollectionManager());
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

void CrateFeature::storePrevSiblingCrateId(CrateId crateId) {
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

void CrateFeature::slotCrateTableChanged(CrateId crateId) {
    Q_UNUSED(crateId);
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

void CrateFeature::slotCrateContentChanged(CrateId crateId) {
    QSet<CrateId> updatedCrateIds;
    updatedCrateIds.insert(crateId);
    updateChildModel(updatedCrateIds);
}

void CrateFeature::slotUpdateCrateLabels(const QSet<CrateId>& updatedCrateIds) {
    updateChildModel(updatedCrateIds);
}

void CrateFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreateCrate();
    } else {
        qDebug() << "Unknown crate link clicked" << link;
    }
}

void CrateFeature::slotTrackSelected(TrackId trackId) {
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

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem : pRootItem->children()) {
        DEBUG_ASSERT(pTreeItem != nullptr);
        bool crateContainsSelectedTrack =
                m_selectedTrackId.isValid() &&
                std::binary_search(
                        sortedTrackCrates.begin(),
                        sortedTrackCrates.end(),
                        CrateId(pTreeItem->getData()));
        pTreeItem->setBold(crateContainsSelectedTrack);
    }

    m_pSidebarModel->triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}
