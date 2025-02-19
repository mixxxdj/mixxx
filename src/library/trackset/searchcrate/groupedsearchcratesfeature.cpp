#include "library/trackset/searchcrate/groupedsearchcratesfeature.h"

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
#include "library/trackset/playlistfeature.h"
#include "library/trackset/searchcrate/searchcratefeaturehelper.h"
#include "library/trackset/searchcrate/searchcratesummary.h"
#include "library/treeitem.h"
#include "moc_groupedsearchcratesfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

constexpr int kInvalidSearchCrateId = -1;
const bool sDebug = true;

QString formatLabel(
        const SearchCrateSummary& searchCrateSummary) {
    return QStringLiteral("%1 [%2] (%3) %4")
            .arg(
                    searchCrateSummary.getName(),
                    searchCrateSummary.getSearchInput(),
                    QString::number(searchCrateSummary.getTrackCount()),
                    searchCrateSummary.getTrackDurationText());
}

const ConfigKey kConfigKeyLastImportExportSearchCrateDirectoryKey(
        "[Library]", "LastImportExportSearchCrateDirectory");

} // anonymous namespace

// added some lines to get github ci starting
// another line
// another line
// another line
// another line
// another line

using namespace mixxx::library::prefs;

GroupedSearchCratesFeature::GroupedSearchCratesFeature(
        Library* pLibrary, UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary,
                  pConfig,
                  "GROUPEDSEARCHCRATESHOME",
                  QStringLiteral("searchcrate")),
          m_lockedSearchCrateIcon(
                  ":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(
                  pLibrary->trackCollectionManager()->internalCollection()),
          m_searchCrateTableModel(this,
                  pLibrary->trackCollectionManager(),
                  pLibrary->trackCollectionManager()->internalCollection(),
                  pConfig) {
    initActions();
    // construct child model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
    rebuildChildModel();
    connectLibrary(pLibrary);
    connectTrackCollection();
}

void GroupedSearchCratesFeature::initActions() {
    m_pCreateSearchCrateAction = make_parented<QAction>(tr("Create New SearchCrate"), this);
    connect(m_pCreateSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotCreateSearchCrate);
    m_pEditSearchCrateAction = make_parented<QAction>(tr("Edit SearchCrate"), this);
    connect(m_pEditSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotEditSearchCrate);
    m_pRenameSearchCrateAction = make_parented<QAction>(tr("Rename"), this);
    m_pRenameSearchCrateAction->setShortcut(kRenameSidebarItemShortcutKey);
    connect(m_pRenameSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotRenameSearchCrate);
    m_pDuplicateSearchCrateAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotDuplicateSearchCrate);
    m_pDeleteSearchCrateAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteSearchCrateAction->setShortcut(removeKeySequence);
    connect(m_pDeleteSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotDeleteSearchCrate);
    m_pLockSearchCrateAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotToggleSearchCrateLock);

    m_pAnalyzeSearchCrateAction = make_parented<QAction>(tr("Analyze entire SearchCrate"), this);
    connect(m_pAnalyzeSearchCrateAction.get(),
            &QAction::triggered,
            this,
            &GroupedSearchCratesFeature::slotAnalyzeSearchCrate);
}

void GroupedSearchCratesFeature::connectLibrary(Library* pLibrary) {
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
            &GroupedSearchCratesFeature::slotResetSelectedTrack);
}

void GroupedSearchCratesFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new or duplicate searchCrate
            &TrackCollection::searchCrateInserted,
            this,
            &GroupedSearchCratesFeature::slotSearchCrateTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::searchCrateUpdated,
            this,
            &GroupedSearchCratesFeature::slotSearchCrateTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::searchCrateDeleted,
            this,
            &GroupedSearchCratesFeature::slotSearchCrateTableChanged);
    connect(m_pTrackCollection, // searchCrate tracks hidden, unhidden or purged
            &TrackCollection::searchCrateTracksChanged,
            this,
            &GroupedSearchCratesFeature::slotSearchCrateContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::searchCrateSummaryChanged,
            this,
            &GroupedSearchCratesFeature::slotUpdateSearchCrateLabels);
}

QVariant GroupedSearchCratesFeature::title() {
    if (m_pConfig->getValue<bool>(ConfigKey("[Library]", "IPreferSmarties"))) {
        return tr("Smarties (Grouped)");
    } else {
        return tr("SearchCrates (Grouped)");
    }
}

QString GroupedSearchCratesFeature::formatRootViewHtml() const {
    QString searchCrateTitle = "";
    if (m_pConfig->getValue<bool>(ConfigKey("[Library]", "IPreferSmarties"))) {
        searchCrateTitle = tr("Smarties (Grouped)");
    } else {
        searchCrateTitle = tr("SearchCrates (Grouped)");
    }
    QString searchCrateSummary =
            tr("SearchCrates are saved search queries, they can help you to easily "
               "find track.");
    QString searchCrateSummary2 =
            tr("Use searchCrates to have an overview of all tracks of an album or "
               "artist, with parts in the title, added before/after a date, "
               "with a corresponding comment, bpm ... ");
    QString searchCrateSummary3 =
            tr("SearchCrates let you keep your searches for later reuse!");

    QString html;
    QString createSearchCrateLink = tr("Create New SearchCrate");
    html.append(QStringLiteral("<h2>%1</h2>").arg(searchCrateTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(searchCrateSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(searchCrateSummary2));
    html.append(QStringLiteral("<p>%1</p>").arg(searchCrateSummary3));
    // Colorize links in lighter blue, instead of QT default dark blue.
    // Links are still different from regular text, but readable on dark/light backgrounds.
    // https://github.com/mixxxdj/mixxx/issues/9103
    html.append(
            QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                    .arg(createSearchCrateLink));
    return html;
}

std::unique_ptr<TreeItem> GroupedSearchCratesFeature::newTreeItemForSearchCrateSummary(
        const SearchCrateSummary& searchCrateSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForSearchCrateSummary(pTreeItem.get(), searchCrateSummary);
    return pTreeItem;
}

void GroupedSearchCratesFeature::updateTreeItemForSearchCrateSummary(
        TreeItem* pTreeItem, const SearchCrateSummary& searchCrateSummary) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(searchCrateSummary.getId().toVariant());
    } else {
        // The data (= SearchCrateId) is immutable once it has been set
        DEBUG_ASSERT(SearchCrateId(pTreeItem->getData()) == searchCrateSummary.getId());
    }
    // Update mutable properties
    // next line disabled to let the new treeitem creation format the displayname
    // pTreeItem->setLabel(formatLabel(searchCrateSummary));
    pTreeItem->setIcon(searchCrateSummary.isLocked() ? m_lockedSearchCrateIcon : QIcon());
}

void GroupedSearchCratesFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &GroupedSearchCratesFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void GroupedSearchCratesFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* GroupedSearchCratesFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void GroupedSearchCratesFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    m_lastRightClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

bool GroupedSearchCratesFeature::activateSearchCrate(SearchCrateId searchCrateId) {
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [ACTIVATE SEARCHCRATES] -> "
                    "searchCrateId "
                 << searchCrateId;
    }
    VERIFY_OR_DEBUG_ASSERT(searchCrateId.isValid()) {
        return false;
    }

    emit saveModelState();
    m_searchCrateTableModel.selectSearchCrate(searchCrateId);
    emit showTrackModel(&m_searchCrateTableModel);
    emit enableCoverArtDisplay(true);
    return true;
}

bool GroupedSearchCratesFeature::readLastRightClickedSearchCrate(SearchCrate* pSearchCrate) const {
    SearchCrateId searchCrateId(searchCrateIdFromIndex(m_lastRightClickedIndex));
    VERIFY_OR_DEBUG_ASSERT(searchCrateId.isValid()) {
        qWarning() << "Failed to determine id of selected searchCrate";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            m_pTrackCollection->searchCrates().readSearchCrateById(searchCrateId, pSearchCrate)) {
        qWarning() << "Failed to read selected searchCrate with id" << searchCrateId;
        return false;
    }
    return true;
}

bool GroupedSearchCratesFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void GroupedSearchCratesFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateSearchCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditSearchCrateAction.get());
    menu.addSeparator();
    menu.exec(globalPos);
}

void GroupedSearchCratesFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    SearchCrateId searchCrateId(searchCrateIdFromIndex(index));
    if (!searchCrateId.isValid()) {
        return;
    }

    SearchCrate searchCrate;
    if (!m_pTrackCollection->searchCrates().readSearchCrateById(searchCrateId, &searchCrate)) {
        return;
    }

    m_pDeleteSearchCrateAction->setEnabled(!searchCrate.isLocked());
    m_pRenameSearchCrateAction->setEnabled(!searchCrate.isLocked());
    m_pLockSearchCrateAction->setText(searchCrate.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateSearchCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditSearchCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pRenameSearchCrateAction.get());
    menu.addAction(m_pDuplicateSearchCrateAction.get());
    menu.addAction(m_pDeleteSearchCrateAction.get());
    menu.addAction(m_pLockSearchCrateAction.get());
    menu.addSeparator();
    menu.addSeparator();
    menu.addAction(m_pAnalyzeSearchCrateAction.get());
    menu.addSeparator();
    menu.exec(globalPos);
}

void GroupedSearchCratesFeature::slotCreateSearchCrateFromSearch(const QString& text) {
    SearchCrateId searchCrateId =
            SearchCrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySearchCrateFromSearch(text);

    if (searchCrateId.isValid()) {
        // expand SearchCrate and scroll to new searchCrate
        m_pSidebarWidget->selectChildIndex(indexFromSearchCrateId(searchCrateId), false);
        m_lastRightClickedIndex = indexFromSearchCrateId(searchCrateId);
        activateSearchCrate(searchCrateId);
    }
}

void GroupedSearchCratesFeature::slotCreateSearchCrateFromUI() {
    SearchCrateId searchCrateId =
            SearchCrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySearchCrateFromUI();
    if (searchCrateId.isValid()) {
        // expand SearchCrate and scroll to new searchCrate
        m_pSidebarWidget->selectChildIndex(indexFromSearchCrateId(searchCrateId), false);
    }
}

void GroupedSearchCratesFeature::slotCreateSearchCrate() {
    SearchCrateId searchCrateId =
            SearchCrateFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySearchCrate();
    if (searchCrateId.isValid()) {
        // expand SearchCrate and scroll to new searchCrate
        m_pSidebarWidget->selectChildIndex(indexFromSearchCrateId(searchCrateId), false);
    }
}

void GroupedSearchCratesFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteSearchCrate();
}

void GroupedSearchCratesFeature::slotDeleteSearchCrate() {
    SearchCrate searchCrate;
    if (readLastRightClickedSearchCrate(&searchCrate)) {
        if (searchCrate.isLocked()) {
            // qWarning() << "Refusing to delete locked searchCrate" << searchCrate;
            QMessageBox msgBox;
            msgBox.setText("Locked SearchCrate can't be deleted");
            msgBox.exec();
            return;
        }
        SearchCrateId searchCrateId = searchCrate.getId();
        // Store sibling id to restore selection after searchCrate was deleted
        // to avoid the scroll position being reset to SearchCrate root item.
        m_prevSiblingSearchCrate = SearchCrateId();
        if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex)) {
            storePrevSiblingSearchCrateId(searchCrateId);
        }

        QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
                tr("Confirm Deletion"),
                tr("Do you really want to delete searchCrate <b>%1</b>?")
                        .arg(searchCrate.getName()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            if (m_pTrackCollection->deleteSearchCrate(searchCrateId)) {
                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT DELETE SEARCHCRATES] -> "
                                "Deleted searchCrate"
                             << searchCrate;
                }
                return;
            }
        } else {
            return;
        }
    }
    qWarning() << "Failed to delete selected searchCrate";
}

void GroupedSearchCratesFeature::slotFindPreviousSearchCrate() {
    SearchCrate searchCrate;
    if (readLastRightClickedSearchCrate(&searchCrate)) {
        SearchCrateId searchCrateId = searchCrate.getId();
        m_prevSiblingSearchCrate = SearchCrateId();
        storePrevSiblingSearchCrateId(searchCrateId);
    }
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT FIND PREVIOUS SEARCHCRATES] -> "
                    "Previous searchCrate ID "
                 << m_prevSiblingSearchCrate;
    }
}

void GroupedSearchCratesFeature::slotFindNextSearchCrate() {
    SearchCrate searchCrate;
    if (readLastRightClickedSearchCrate(&searchCrate)) {
        SearchCrateId searchCrateId = searchCrate.getId();
        m_nextSiblingSearchCrate = SearchCrateId();
        storeNextSiblingSearchCrateId(searchCrateId);
    }
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT FIND NEXT SEARCHCRATES] -> Next "
                    "searchCrate ID "
                 << m_nextSiblingSearchCrate;
    }
}

void GroupedSearchCratesFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameSearchCrate();
}

void GroupedSearchCratesFeature::slotRenameSearchCrate() {
    SearchCrate searchCrate;
    if (readLastRightClickedSearchCrate(&searchCrate)) {
        const QString oldName = searchCrate.getName();
        searchCrate.resetName();
        for (;;) {
            bool ok = false;
            auto newName =
                    QInputDialog::getText(nullptr,
                            tr("Rename SearchCrate"),
                            tr("Enter new name for searchCrate:"),
                            QLineEdit::Normal,
                            oldName,
                            &ok)
                            .trimmed();
            if (!ok || newName.isEmpty()) {
                return;
            }
            if (newName.isEmpty()) {
                QMessageBox::warning(nullptr,
                        tr("Renaming SearchCrate Failed"),
                        tr("A searchCrate cannot have a blank name."));
                continue;
            }
            if (m_pTrackCollection->searchCrates().readSearchCrateByName(newName)) {
                QMessageBox::warning(nullptr,
                        tr("Renaming SearchCrate Failed"),
                        tr("A searchCrate by that name already exists."));
                continue;
            }
            searchCrate.setName(std::move(newName));
            DEBUG_ASSERT(searchCrate.hasName());
            break;
        }

        if (!m_pTrackCollection->updateSearchCrate(searchCrate)) {
            if (sDebug) {
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT RENAME SEARCHCRATES] -> "
                            "Failed to rename searchCrate"
                         << searchCrate;
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT RENAME MARTIES] -> Failed to "
                        "rename selected searchCrate";
        }
    }
}

void GroupedSearchCratesFeature::slotDuplicateSearchCrate() {
    SearchCrate searchCrate;
    if (readLastRightClickedSearchCrate(&searchCrate)) {
        SearchCrateId newSearchCrateId =
                SearchCrateFeatureHelper(m_pTrackCollection, m_pConfig)
                        .duplicateSearchCrate(searchCrate);
        if (newSearchCrateId.isValid()) {
            if (sDebug) {
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT DUPLICATE SEARCHCRATES] -> "
                            "Duplicate searchCrate"
                         << searchCrate << ", new searchCrate:" << newSearchCrateId;
            }
            return;
        }
    }
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT DUPLICATE SEARCHCRATES] -> Failed to "
                    "duplicate selected searchCrate";
    }
}

void GroupedSearchCratesFeature::SetActiveSearchCrateToLastRightClicked(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
}

void GroupedSearchCratesFeature::selectSearchCrateForEdit(SearchCrateId selectedSearchCrateId) {
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [REBUILD CHILD MODEL] -> "
                    "selectedSearchCrateId "
                 << selectedSearchCrateId;
    }
    //    return selectedSearchCrateId;
}

void GroupedSearchCratesFeature::slotEditSearchCrate() {
    QMutex mutex;
    mutex.lock();

    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                    "[START] -> slotEditSearchCrate";
    }
    SearchCrate searchCrate;
    readLastRightClickedSearchCrate(&searchCrate);
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> [START] -> "
                    "m_lastRightClickedIndex  = "
                 << m_lastRightClickedIndex;
    }
    // Load data into QVariant
    searchCrateData.clear();
    m_searchCrateTableModel.selectSearchCrate2QVL(
            searchCrateIdFromIndex(m_lastRightClickedIndex), searchCrateData);
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> [START] -> "
                    "SearchCrate data loaded into QVariantList:"
                 << searchCrateData;
    }

    QVariantList playlistsCratesData;
    m_searchCrateTableModel.selectPlaylistsCrates2QVL(playlistsCratesData);
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> [START] -> "
                    "Playlists & Crates data loaded into QVariantList:"
                 << playlistsCratesData;
    }

    if (readLastRightClickedSearchCrate(&searchCrate)) {
        SearchCrateId searchCrateId = searchCrateIdFromIndex(m_lastRightClickedIndex);
        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> [START] -> "
                        "SlotEditSearchCrate -> searchCrateID = "
                     << searchCrateId;
        }
        // Pass this to provide the GroupedSearchCratesFeature instance
        dlgGroupedSearchCratesInfo infoDialog(this);

        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> [START] -> "
                        "[INIT DIALOG] -> INIT DIALOG ";
        }

        infoDialog.init(searchCrateData, playlistsCratesData);
        // DLG -> Update SearchCrate on 'Apply'
        connect(&infoDialog,
                &dlgGroupedSearchCratesInfo::dataUpdated,
                this,
                [this, searchCrateId](const QVariantList& updatedData) mutable {
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                    "[UPDATE] -> START Request UPDATE SearchCrate "
                                    "searchCrateId "
                                 << searchCrateId;
                    }
                    searchCrateData = updatedData; // Capture the updated data from the UI
                    // current searchCrateId @ 0 prev/bof/next/eof pointers @ 56
                    SearchCrateId searchCrateId(searchCrateData[0]);
                    // SearchCrateId previousSearchCrateId(searchCrateData[56]);
                    // bool currentSearchCrateIdBOF(searchCrateData[57].toString() == "true");
                    // SearchCrateId nextSearchCrateId(searchCrateData[58]);
                    // bool currentSearchCrateIdEOF(searchCrateData[59].toString() == "true");
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] extracted "
                                    "searchCrateId from searchCrateData: "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current searchCrateId "
                                 << searchCrateId;
                        // qDebug() << "[GROUPEDSEARCHCRATESFEATURE] previous SearchCrateId: "
                        //          << previousSearchCrateId;
                        // qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId BOF: "
                        //          << currentSearchCrateIdBOF;
                        // qDebug() << "[GROUPEDSEARCHCRATESFEATURE] next SearchCrateId: "
                        //          << nextSearchCrateId;
                        // qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId EOF: "
                        //          << currentSearchCrateIdEOF;
                    }
                    if (searchCrateId.isValid()) {
                        // Store updated data
                        m_searchCrateTableModel.saveQVL2SearchCrate(searchCrateId, searchCrateData);
                        // Send updated data back to ui-> adapted sql
                        m_searchCrateTableModel.selectSearchCrate2QVL(
                                searchCrateIdFromIndex(m_lastRightClickedIndex),
                                searchCrateData);
                        activateSearchCrate(searchCrateId);
                        m_lastClickedIndex = indexFromSearchCrateId(searchCrateId);
                        m_lastRightClickedIndex = indexFromSearchCrateId(searchCrateId);
                        slotSearchCrateTableChanged(searchCrateId);
                        if (sDebug) {
                            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                        "[UPDATE] -> END UPDATE searchCrateId "
                                     << searchCrateId;
                        }
                        emit updateSearchCrateData(searchCrateData);
                    } else {
                        return;
                    }
                });
        // DLG -> Delete SearchCrate on 'Delete'
        connect(&infoDialog,
                &dlgGroupedSearchCratesInfo::requestDeleteSearchCrate,
                this,
                // [this, &searchCrateId]() {
                // [this, &searchCrateId]() {
                [this]() {
                    // current searchCrateId @ 0 prev/bof/next/eof pointers @ 56
                    SearchCrateId searchCrateId(searchCrateData[0]);
                    SearchCrateId previousSearchCrateId(searchCrateData[56]);
                    bool currentSearchCrateIdBOF(searchCrateData[57].toString() == "true");
                    SearchCrateId nextSearchCrateId(searchCrateData[58]);
                    bool currentSearchCrateIdEOF(searchCrateData[59].toString() == "true");
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                    "[DELETE] -> START Request DELETE SearchCrate "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] extracted "
                                    "searchCrateId from searchCrateData: "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current searchCrateId "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] previous SearchCrateId: "
                                 << previousSearchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId BOF: "
                                 << currentSearchCrateIdBOF;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] next SearchCrateId: "
                                 << nextSearchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId EOF: "
                                 << currentSearchCrateIdEOF;
                    }
                    if (!searchCrateId.isValid()) {
                        if (sDebug) {
                            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT "
                                        "SEARCHCRATES] -> "
                                        "[DELETE] -> Invalid SearchCrateId. : "
                                     << searchCrateId;
                        }
                        return;
                    } else {
                        slotDeleteSearchCrate();
                        if (currentSearchCrateIdBOF && !currentSearchCrateIdEOF) {
                            if (nextSearchCrateId.isValid()) {
                                m_searchCrateTableModel.selectSearchCrate2QVL(
                                        nextSearchCrateId,
                                        searchCrateData);
                                emit updateSearchCrateData(searchCrateData);
                                if (sDebug) {
                                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] "
                                                "[SLOT EDIT SEARCHCRATES] -> "
                                                "[DELETE] -> SearchCrate "
                                                "DELETED, new active "
                                                "searchCrate: "
                                             << nextSearchCrateId;
                                }
                                m_lastRightClickedIndex = indexFromSearchCrateId(nextSearchCrateId);
                                activateSearchCrate(nextSearchCrateId);
                            } else {
                                return;
                            }
                        } else {
                            if (previousSearchCrateId.isValid()) {
                                m_searchCrateTableModel.selectSearchCrate2QVL(
                                        previousSearchCrateId,
                                        searchCrateData);
                                emit updateSearchCrateData(searchCrateData);
                                if (sDebug) {
                                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] "
                                                "[SLOT EDIT SEARCHCRATES] -> "
                                                "[DELETE] -> SearchCrate "
                                                "DELETED, new active "
                                                "searchCrate: "
                                             << previousSearchCrateId;
                                }
                                m_lastRightClickedIndex =
                                        indexFromSearchCrateId(
                                                previousSearchCrateId);
                                activateSearchCrate(previousSearchCrateId);
                            } else {
                                return;
                            }
                        }
                        slotSearchCrateTableChanged(previousSearchCrateId);
                    }
                });
        // DLG -> New SearchCrate on 'New'
        connect(&infoDialog,
                &dlgGroupedSearchCratesInfo::requestNewSearchCrate,
                this,
                // [this, &searchCrateId]() {
                [this, searchCrateId]() {
                
                    //            emit setBlockerOff("new");
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                    "[NEW] "
                                    "-> START Request NEW SearchCrate searchCrateId "
                                 << searchCrateId;
                    }
                    SearchCrateId searchCrateId =
                            SearchCrateFeatureHelper(m_pTrackCollection, m_pConfig)
                                    .createEmptySearchCrateFromUI();
                    if (!searchCrateId.isValid()) {
                        if (sDebug) {
                            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT "
                                        "SEARCHCRATES] -> "
                                        "[NEW] -> Creation failed.";
                        }
                        return;
                    } else {
                        if (sDebug) {
                            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT "
                                        "SEARCHCRATES] -> "
                                        "[NEW] -> New searchCrate created. "
                                        "searchCrateId "
                                     << searchCrateId;
                        }
                    }
                    activateSearchCrate(searchCrateId);
                    searchCrateData.clear();
                    m_searchCrateTableModel.selectSearchCrate2QVL(
                            searchCrateId, searchCrateData);
                    slotSearchCrateTableChanged(searchCrateId);
                    m_lastRightClickedIndex = indexFromSearchCrateId(searchCrateId);
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                    "[NEW] "
                                    "-> END SearchCrate created searchCrateId "
                                 << searchCrateId;
                    }
                    emit updateSearchCrateData(searchCrateData);
                });
        // DLG -> Previous SearchCrate on 'Previous'
        connect(&infoDialog,
                &dlgGroupedSearchCratesInfo::requestPreviousSearchCrate,
                this,
                [this]() {
                    // [this, &searchCrateId]() {
                    // current searchCrateId @ 0 prev/bof/next/eof pointers @ 56
                    SearchCrateId searchCrateId(searchCrateData[0]);
                    SearchCrateId previousSearchCrateId(searchCrateData[56]);
                    bool currentSearchCrateIdBOF(searchCrateData[57].toString() == "true");
                    SearchCrateId nextSearchCrateId(searchCrateData[58]);
                    bool currentSearchCrateIdEOF(searchCrateData[59].toString() == "true");
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                    "[PREVIOUS] -> START Request PREVIOUS SearchCrate "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] extracted "
                                    "searchCrateId from searchCrateData: "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current searchCrateId "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] previous SearchCrateId: "
                                 << previousSearchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId BOF: "
                                 << currentSearchCrateIdBOF;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] next SearchCrateId: "
                                 << nextSearchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId EOF: "
                                 << currentSearchCrateIdEOF;
                    }
                    if (currentSearchCrateIdBOF && !currentSearchCrateIdEOF) {
                        if (nextSearchCrateId.isValid()) {
                            m_searchCrateTableModel.selectSearchCrate2QVL(
                                    nextSearchCrateId,
                                    searchCrateData);
                            emit updateSearchCrateData(searchCrateData);
                            if (sDebug) {
                                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] "
                                            "[SLOT EDIT SEARCHCRATES] -> "
                                            "[PREVIOUS] -> new active "
                                            "searchCrate: "
                                         << nextSearchCrateId;
                            }
                            m_lastRightClickedIndex = indexFromSearchCrateId(nextSearchCrateId);
                            activateSearchCrate(nextSearchCrateId);
                            slotSearchCrateTableChanged(nextSearchCrateId);
                        } else {
                            return;
                        }
                    } else {
                        if (previousSearchCrateId.isValid()) {
                            m_searchCrateTableModel.selectSearchCrate2QVL(
                                    previousSearchCrateId,
                                    searchCrateData);
                            emit updateSearchCrateData(searchCrateData);
                            if (sDebug) {
                                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] "
                                            "[SLOT EDIT SEARCHCRATES] -> "
                                            "[PREVIOUS] -> new active "
                                            "searchCrate: "
                                         << previousSearchCrateId;
                            }
                            m_lastRightClickedIndex = indexFromSearchCrateId(previousSearchCrateId);
                            activateSearchCrate(previousSearchCrateId);
                            slotSearchCrateTableChanged(previousSearchCrateId);
                        } else {
                            return;
                        }
                    }
                });
        // DLG -> Next SearchCrate on 'Next'
        connect(&infoDialog,
                &dlgGroupedSearchCratesInfo::requestNextSearchCrate,
                this,
                [this]() {
                    // [this, &searchCrateId]() {
                    // current searchCrateId @ 0 prev/bof/next/eof pointers @ 56
                    SearchCrateId searchCrateId(searchCrateData[0]);
                    SearchCrateId previousSearchCrateId(searchCrateData[56]);
                    bool currentSearchCrateIdBOF(searchCrateData[57].toString() == "true");
                    SearchCrateId nextSearchCrateId(searchCrateData[58]);
                    bool currentSearchCrateIdEOF(searchCrateData[59].toString() == "true");
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                                    "[NEXT] -> START Request NEXT SearchCrate "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] extracted "
                                    "searchCrateId from searchCrateData: "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current searchCrateId "
                                 << searchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] previous SearchCrateId: "
                                 << previousSearchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId BOF: "
                                 << currentSearchCrateIdBOF;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] next SearchCrateId: "
                                 << nextSearchCrateId;
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId EOF: "
                                 << currentSearchCrateIdEOF;
                    }
                    if (currentSearchCrateIdEOF && !currentSearchCrateIdBOF) {
                        if (previousSearchCrateId.isValid()) {
                            m_searchCrateTableModel.selectSearchCrate2QVL(
                                    previousSearchCrateId,
                                    searchCrateData);
                            emit updateSearchCrateData(searchCrateData);
                            if (sDebug) {
                                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] "
                                            "[SLOT EDIT SEARCHCRATES] -> "
                                            "[NEXT] -> new active searchCrate: "
                                         << previousSearchCrateId;
                            }
                            m_lastRightClickedIndex = indexFromSearchCrateId(previousSearchCrateId);
                            activateSearchCrate(previousSearchCrateId);
                            slotSearchCrateTableChanged(previousSearchCrateId);
                        } else {
                            return;
                        }
                    } else {
                        if (nextSearchCrateId.isValid()) {
                            m_searchCrateTableModel.selectSearchCrate2QVL(
                                    nextSearchCrateId,
                                    searchCrateData);
                            emit updateSearchCrateData(searchCrateData);
                            if (sDebug) {
                                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] "
                                            "[SLOT EDIT SEARCHCRATES] -> "
                                            "[NEXT] -> new active searchCrate: "
                                         << nextSearchCrateId;
                            }
                            m_lastRightClickedIndex = indexFromSearchCrateId(nextSearchCrateId);
                            activateSearchCrate(nextSearchCrateId);
                            slotSearchCrateTableChanged(nextSearchCrateId);
                        } else {
                            return;
                        }
                    }
                });
        // Execute & close the dialog
        if (infoDialog.exec() == QDialog::Accepted) {
            // Extract SearchCrateId from searchCrateData
            // current searchCrateId @ 0 prev/bof/next/eof pointers @ 56
            SearchCrateId searchCrateId(searchCrateData[0]);
            SearchCrateId previousSearchCrateId(searchCrateData[56]);
            bool currentSearchCrateIdBOF(searchCrateData[57].toString() == "true");
            SearchCrateId nextSearchCrateId(searchCrateData[58]);
            bool currentSearchCrateIdEOF(searchCrateData[59].toString() == "true");
            if (sDebug) {
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> "
                            "[NEXT] -> START Request NEXT SearchCrate "
                         << searchCrateId;
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] extracted "
                            "searchCrateId from searchCrateData: "
                         << searchCrateId;
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current searchCrateId "
                         << searchCrateId;
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] previous SearchCrateId: "
                         << previousSearchCrateId;
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId BOF: "
                         << currentSearchCrateIdBOF;
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] next SearchCrateId: "
                         << nextSearchCrateId;
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] current SearchCrateId EOF: "
                         << currentSearchCrateIdEOF;
            }

            if (searchCrateId.isValid()) {
                // Store updated data
                m_searchCrateTableModel.saveQVL2SearchCrate(searchCrateId, searchCrateData);
                // Send updated data back to ui-> adapted sql
                m_searchCrateTableModel.selectSearchCrate2QVL(
                        searchCrateIdFromIndex(m_lastRightClickedIndex),
                        searchCrateData);
                activateSearchCrate(searchCrateId);
                m_lastClickedIndex = indexFromSearchCrateId(searchCrateId);
                m_lastRightClickedIndex = indexFromSearchCrateId(searchCrateId);
                slotSearchCrateTableChanged(searchCrateId);
                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT EDIT SEARCHCRATES] -> [CLOSE "
                                "DIALOG] -> SearchCrate data saved from QVariantList "
                                "to database for "
                                "SearchCrateId:"
                             << searchCrateId;
                }
                emit updateSearchCrateData(searchCrateData);
            } else {
                return;
            }
        }
    }
    mutex.unlock();
}

void GroupedSearchCratesFeature::slotToggleSearchCrateLock() {
    SearchCrate searchCrate;
    if (readLastRightClickedSearchCrate(&searchCrate)) {
        searchCrate.setLocked(!searchCrate.isLocked());
        if (!m_pTrackCollection->updateSearchCrate(searchCrate)) {
            if (sDebug) {
                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT TOGGLE LOCK] -> Failed to "
                            "toggle lock of searchCrate"
                         << searchCrate;
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [SLOT TOGGLE LOCK] -> Failed to "
                        "toggle lock of selected searchCrate";
        }
    }
}

SearchCrateId GroupedSearchCratesFeature::searchCrateIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return SearchCrateId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return SearchCrateId();
    }
    return SearchCrateId(item->getData());
}

QModelIndex GroupedSearchCratesFeature::indexFromSearchCrateId(SearchCrateId searchCrateId) const {
    VERIFY_OR_DEBUG_ASSERT(searchCrateId.isValid()) {
        return QModelIndex();
    }
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                (SearchCrateId(pTreeItem->getData()) == searchCrateId)) {
            return index;
        }
    }
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [INDEX FROM SEARCHCRATESID] -> Tree item "
                    "for searchCrate not found:"
                 << searchCrateId;
    }
    return QModelIndex();
}

void GroupedSearchCratesFeature::slotAnalyzeSearchCrate() {
    if (m_lastRightClickedIndex.isValid()) {
        SearchCrateId searchCrateId = searchCrateIdFromIndex(m_lastRightClickedIndex);
        if (searchCrateId.isValid()) {
            QList<AnalyzerScheduledTrack> tracks;
            tracks.reserve(
                    m_pTrackCollection->searchCrates().countSearchCrateTracks(searchCrateId));
            {
                SearchCrateTrackSelectResult searchCrateTracks(
                        m_pTrackCollection->searchCrates().selectSearchCrateTracksSorted(
                                searchCrateId));
                while (searchCrateTracks.next()) {
                    tracks.append(searchCrateTracks.trackId());
                }
            }
            emit analyzeTracks(tracks);
        }
    }
}

void GroupedSearchCratesFeature::storePrevSiblingSearchCrateId(SearchCrateId searchCrateId) {
    QModelIndex actIndex = indexFromSearchCrateId(searchCrateId);
    m_prevSiblingSearchCrate = SearchCrateId();
    for (int i = (actIndex.row() + 1); i >= (actIndex.row() - 1); i -= 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_prevSiblingSearchCrate = searchCrateIdFromIndex(newIndex);
            }
        }
    }
}

void GroupedSearchCratesFeature::storeNextSiblingSearchCrateId(SearchCrateId searchCrateId) {
    QModelIndex actIndex = indexFromSearchCrateId(searchCrateId);
    m_nextSiblingSearchCrate = SearchCrateId();
    for (int i = (actIndex.row() - 1); i <= (actIndex.row() + 1); i += 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_nextSiblingSearchCrate = searchCrateIdFromIndex(newIndex);
            }
        }
    }
}

void GroupedSearchCratesFeature::slotSearchCrateTableChanged(SearchCrateId searchCrateId) {
    if (searchCrateId.isValid()) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> slotSearchCrateTableChanged -> searchCrateId"
                 << searchCrateId
                 << " is VALID ";
        rebuildChildModel(searchCrateId);

        //        // Recall the last clicked or right-clicked smatries
        //        activateSearchCrate(searchCrateId);
    } else {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> slotSearchCrateTableChanged -> searchCrateId"
                 << searchCrateId
                 << " is NOT VALID ";
        if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
            SearchCrateId selectedCrate = m_searchCrateTableModel.selectedSearchCrate();
            if (!selectedCrate.isValid()) {
                qCritical() << "[GROUPEDSEARCHCRATESFEATURE] -> ERROR: "
                               "selectedSearchCrate() returned INVALID ID!";
            }
            if (!activateSearchCrate(selectedCrate)) {
                if (m_prevSiblingSearchCrate.isValid()) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> Activating previous sibling crate:"
                             << m_prevSiblingSearchCrate;

                    // Recall the last clicked or right-clicked smatries
                    activateSearchCrate(m_prevSiblingSearchCrate);
                } else {
                    qWarning() << "[GROUPEDSEARCHCRATESFEATURE] -> No valid "
                                  "previous sibling crate found.";
                }
            }
        } else {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> Rebuilding child "
                        "model with no specific searchCrateId";
            rebuildChildModel();
        }
    }
}

void GroupedSearchCratesFeature::slotSearchCrateContentChanged(SearchCrateId searchCrateId) {
    QSet<SearchCrateId> updatedSearchCrateIds;
    updatedSearchCrateIds.insert(searchCrateId);
    updateChildModel(updatedSearchCrateIds);
}

void GroupedSearchCratesFeature::slotUpdateSearchCrateLabels(
        const QSet<SearchCrateId>& updatedSearchCrateIds) {
    updateChildModel(updatedSearchCrateIds);
}

void GroupedSearchCratesFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreateSearchCrate();
    } else {
        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] [HTML LINK CLICKED ] -> Unknown "
                        "searchCrate link clicked"
                     << link;
        }
    }
}

void GroupedSearchCratesFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    std::vector<SearchCrateId> sortedTrackSearchCrate;
    if (m_selectedTrackId.isValid()) {
        SearchCrateTrackSelectResult trackSearchCrateIter(
                m_pTrackCollection->searchCrates().selectTrackSearchCrateSorted(m_selectedTrackId));
        while (trackSearchCrateIter.next()) {
            sortedTrackSearchCrate.push_back(trackSearchCrateIter.searchCrateId());
        }
    }

    // Set all searchCrates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem : pRootItem->children()) {
        DEBUG_ASSERT(pTreeItem != nullptr);
        bool searchCrateContainsSelectedTrack =
                m_selectedTrackId.isValid() &&
                std::binary_search(
                        sortedTrackSearchCrate.begin(),
                        sortedTrackSearchCrate.end(),
                        SearchCrateId(pTreeItem->getData()));
        pTreeItem->setBold(searchCrateContainsSelectedTrack);
    }

    m_pSidebarModel->triggerRepaint();
}

void GroupedSearchCratesFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}

QModelIndex GroupedSearchCratesFeature::rebuildChildModel(SearchCrateId selectedSearchCrateId) {
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> rebuildChildModel()" << selectedSearchCrateId;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // remember open/close state of group
    // QMap<QString, bool> groupExpandedStates;
    // for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
    //    QModelIndex groupIndex = m_pSidebarModel->index(row, 0);
    //    if (groupIndex.isValid()) {
    //        TreeItem* pGroupItem = m_pSidebarModel->getItem(groupIndex);
    //        if (pGroupItem) {
    //            const QString& groupName = pGroupItem->getLabel();
    //            groupExpandedStates[groupName] = m_pSidebarWidget->isExpanded(groupIndex);
    //            if (sDebug) {
    //                qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Saved open/close state "
    //                            "for group:"
    //                         << groupName << "->"
    //                         << groupExpandedStates[groupName];
    //            }
    //        }
    //    }
    //}

    m_lastRightClickedIndex = QModelIndex();
    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    QList<QVariantMap> groupedSearchCrates = m_searchCrateTableModel.getGroupedSearchCrates();
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedSearchCratesVarLengthMask"));

    if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedSearchCratesLength")) == 0) {
        // Fixed prefix length
        QMap<QString, int> groupCounts;
        for (int i = 0; i < groupedSearchCrates.size(); ++i) {
            const auto& searchCrateData = groupedSearchCrates[i];
            const QString& groupName = searchCrateData["group_name"].toString();
            groupCounts[groupName]++;
        }

        QMap<QString, TreeItem*> groupItems;
        std::vector<std::unique_ptr<TreeItem>> modelRows;

        for (int i = 0; i < groupedSearchCrates.size(); ++i) {
            const auto& searchCrateData = groupedSearchCrates[i];
            const QString& groupName = searchCrateData["group_name"].toString();
            SearchCrateId searchCrateId(searchCrateData["searchCrate_id"]);

            SearchCrateSummary searchCrateSummary;
            if (!m_pTrackCollection->searchCrates().readSearchCrateSummaryById(
                        searchCrateId, &searchCrateSummary)) {
                qWarning() << "[GROUPEDSEARCHCRATESFEATURE] -> Failed to fetch summary "
                              "for crate ID:"
                           << searchCrateId;
                continue;
            }

            const QString& searchCrateSummaryName = formatLabel(searchCrateSummary);

            if (groupCounts[groupName] > 1) {
                TreeItem* pGroupItem = groupItems.value(groupName, nullptr);
                if (!pGroupItem) {
                    auto newGroup = std::make_unique<TreeItem>(groupName, kInvalidSearchCrateId);
                    pGroupItem = newGroup.get();
                    groupItems.insert(groupName, pGroupItem);
                    modelRows.push_back(std::move(newGroup));
                }

                const QString& displaySearchCrateName =
                        searchCrateSummaryName.mid(groupName.length()).trimmed();
                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> searchCrateSummaryName - "
                                "displaySearchCrateName = "
                             << searchCrateSummaryName << " - " << displaySearchCrateName;
                }

                TreeItem* pChildItem = pGroupItem->appendChild(
                        displaySearchCrateName, searchCrateId.toVariant().toInt());
                pChildItem->setFullPath(groupName + delimiter + displaySearchCrateName);
                updateTreeItemForSearchCrateSummary(pChildItem, searchCrateSummary);
                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Added SearchCrateId to group:"
                             << searchCrateId << "Group:" << groupName;
                }
            } else {
                auto newSearchCrate = std::make_unique<TreeItem>(
                        searchCrateSummaryName, searchCrateId.toVariant().toInt());
                newSearchCrate->setFullPath(searchCrateSummaryName);
                updateTreeItemForSearchCrateSummary(newSearchCrate.get(), searchCrateSummary);

                modelRows.push_back(std::move(newSearchCrate));
            }
        }

        m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);
        slotTrackSelected(m_selectedTrackId);

    } else {
        // variable group prefix length with mask
        QMap<QString, QList<QVariantMap>> topLevelGroups;
        for (int i = 0; i < groupedSearchCrates.size(); ++i) {
            const auto& searchCrateData = groupedSearchCrates[i];
            const QString& groupName = searchCrateData["group_name"].toString();
            const QString& topGroup = groupName.section(delimiter, 0, 0);
            topLevelGroups[topGroup].append(searchCrateData);
        }

        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Top-level groups:";
            for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
                qDebug() << "Group:" << it.key() << "-> SearchCrates:" << it.value().size();
            }
        }
        // lambda function to build tree
        std::function<void(
                const QString&, const QList<QVariantMap>&, TreeItem*)>
                buildTreeStructure;
        buildTreeStructure = [&](const QString& currentPath,
                                     const QList<QVariantMap>& searchCrates,
                                     TreeItem* pParentItem) {
            QMap<QString, QList<QVariantMap>> subgroupedSearchCrates;

            for (const QVariantMap& searchCrateData : searchCrates) {
                const QString& groupName = searchCrateData["group_name"].toString();

                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Processing searchCrate with "
                                "groupName:"
                             << groupName << "currentPath:" << currentPath;
                }

                if (!groupName.startsWith(currentPath)) {
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Skipping searchCrate. "
                                    "Group name does not match path:"
                                 << groupName << "Current path:" << currentPath;
                    }
                    continue;
                }

                const QString& remainingPath = groupName.mid(currentPath.length());
                int delimiterPos = remainingPath.indexOf(delimiter);

                if (delimiterPos >= 0) {
                    const QString& subgroupName = remainingPath.left(delimiterPos);
                    subgroupedSearchCrates[subgroupName].append(searchCrateData);
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Added searchCrate to "
                                    "subgroup:"
                                 << subgroupName
                                 << "Remaining path:" << remainingPath;
                    }
                } else {
                    SearchCrateId searchCrateId(searchCrateData["searchCrate_id"]);
                    SearchCrateSummary searchCrateSummary;
                    if (!m_pTrackCollection->searchCrates().readSearchCrateSummaryById(
                                searchCrateId, &searchCrateSummary)) {
                        qWarning() << "[GROUPEDSEARCHCRATESFEATURE] Failed to fetch "
                                      "summary for crate ID:"
                                   << searchCrateId;
                        continue;
                    }

                    const QString& displaySearchCrateName =
                            formatLabel(searchCrateSummary).mid(currentPath.length());

                    TreeItem* pChildItem = pParentItem->appendChild(
                            displaySearchCrateName.trimmed(), searchCrateId.toVariant());
                    pChildItem->setFullPath(currentPath + delimiter + displaySearchCrateName);
                    updateTreeItemForSearchCrateSummary(pChildItem, searchCrateSummary);
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Added crate to "
                                    "parent:"
                                 << displaySearchCrateName
                                 << "Parent:" << pParentItem->getLabel();
                    }
                }
            }

            for (auto it = subgroupedSearchCrates.constBegin();
                    it != subgroupedSearchCrates.constEnd();
                    ++it) {
                const QString& subgroupName = it.key();
                const QList<QVariantMap>& subgroupSearchCrates = it.value();
                if (!subgroupSearchCrates.isEmpty()) {
                    if (subgroupSearchCrates.size() > 1) {
                        // subgroup has > 1 searchCrate -> create subgroup
                        auto pNewSubgroup = std::make_unique<TreeItem>(
                                subgroupName, kInvalidSearchCrateId);
                        TreeItem* pSubgroupItem = pNewSubgroup.get();
                        pParentItem->insertChild(pParentItem->childCount(),
                                std::move(pNewSubgroup));
                        if (sDebug) {
                            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Created "
                                        "subgroup:"
                                     << subgroupName
                                     << "Parent:" << pParentItem->getLabel();
                        }

                        // loop into the subgroup
                        buildTreeStructure(
                                currentPath + subgroupName + delimiter,
                                subgroupSearchCrates,
                                pSubgroupItem);
                    } else {
                        // only one crate -> directly under the parent, NO subgroup
                        const QVariantMap& searchCrateData = subgroupSearchCrates.first();
                        SearchCrateId searchCrateId(searchCrateData["searchCrate_id"]);
                        SearchCrateSummary searchCrateSummary;
                        if (!m_pTrackCollection->searchCrates().readSearchCrateSummaryById(
                                    searchCrateId, &searchCrateSummary)) {
                            qWarning() << "[GROUPEDSEARCHCRATESFEATURE] Failed to "
                                          "fetch summary for searchCrate ID:"
                                       << searchCrateId;
                            continue;
                        }

                        const QString& displaySearchCrateName =
                                formatLabel(searchCrateSummary)
                                        .mid(currentPath.length());

                        TreeItem* pChildItem = pParentItem->appendChild(
                                displaySearchCrateName.trimmed(), searchCrateId.toVariant());
                        pChildItem->setFullPath(currentPath + delimiter + displaySearchCrateName);
                        updateTreeItemForSearchCrateSummary(pChildItem, searchCrateSummary);
                        if (sDebug) {
                            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Added "
                                        "single searchCrate to parent:"
                                     << displaySearchCrateName
                                     << "Parent:" << pParentItem->getLabel();
                        }
                    }
                }
            }
        };
        // building rootlevel groups
        for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
            const QString& groupName = it.key();
            const QList<QVariantMap>& searchCrates = it.value();

            if (searchCrates.size() > 1) {
                auto pNewGroup = std::make_unique<TreeItem>(groupName, kInvalidSearchCrateId);
                TreeItem* pGroupItem = pNewGroup.get();
                pRootItem->insertChild(pRootItem->childCount(), std::move(pNewGroup));
                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Created "
                                "top-level group:"
                             << groupName;
                }

                buildTreeStructure(groupName + delimiter, searchCrates, pGroupItem);
            } else {
                const QVariantMap& searchCrateData = searchCrates.first();
                SearchCrateId searchCrateId(searchCrateData["searchCrate_id"]);
                SearchCrateSummary searchCrateSummary;

                if (!m_pTrackCollection->searchCrates()
                                .readSearchCrateSummaryById(
                                        searchCrateId, &searchCrateSummary)) {
                    qWarning() << "[GROUPEDSEARCHCRATESFEATURE] Failed to fetch "
                                  "summary for searchCrate ID:"
                               << searchCrateId;
                    continue;
                }

                const QString& displaySearchCrateName = formatLabel(searchCrateSummary);
                TreeItem* pChildItem = pRootItem->appendChild(
                        displaySearchCrateName.trimmed(), searchCrateId.toVariant());
                updateTreeItemForSearchCrateSummary(pChildItem, searchCrateSummary);
                if (sDebug) {
                    qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Added crate to "
                                "root:"
                             << displaySearchCrateName;
                }
            }
        }
    }
    // store open/close state of groups
    /*for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex groupIndex = m_pSidebarModel->index(row, 0);
        if (groupIndex.isValid()) {
            TreeItem* pGroupItem = m_pSidebarModel->getItem(groupIndex);
            if (pGroupItem) {
                const QString& groupName = pGroupItem->getLabel();
                if (groupExpandedStates.contains(groupName)) {
                    m_pSidebarWidget->setExpanded(groupIndex, groupExpandedStates[groupName]);
                    if (sDebug) {
                        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] Restored expanded "
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
    }*/

    return QModelIndex();
}

void GroupedSearchCratesFeature::updateChildModel(
        const QSet<SearchCrateId>& updatedSearchCrateIds) {
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> updateChildModel() -> Updating "
                    "crates"
                 << updatedSearchCrateIds;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // Fetch grouped crates using CrateTableModel
    QList<QVariantMap> groupedSearchCrates = m_searchCrateTableModel.getGroupedSearchCrates();

    QMap<QString, QList<QVariantMap>> groupedSearchCratesMap;
    for (int i = 0; i < groupedSearchCrates.size(); ++i) {
        const auto& searchCrateData = groupedSearchCrates[i];
        groupedSearchCratesMap[searchCrateData["group_name"].toString()].append(searchCrateData);
    }

    // Update full paths recursively for all items starting from the root
    updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());

    // Update or rebuild items
    for (const SearchCrateId& searchCrateId : updatedSearchCrateIds) {
        // Find the updated searchCrate in groupedSearchCrates

        auto updatedGroup = std::find_if(groupedSearchCrates.begin(),
                groupedSearchCrates.end(),
                [&searchCrateId](const QVariantMap& searchCrateData) {
                    return searchCrateData["searchCrate_id"].toInt() ==
                            searchCrateId.toVariant().toInt();
                });
        if (updatedGroup == groupedSearchCrates.end()) {
            qWarning() << "No match found for searchCrateId:" << searchCrateId.toVariant().toInt();
        }
        if (updatedGroup != groupedSearchCrates.end()) {
            QModelIndex index = indexFromSearchCrateId(searchCrateId);
            if (index.isValid()) {
                // Update the existing item
                TreeItem* pItem = m_pSidebarModel->getItem(index);
                VERIFY_OR_DEBUG_ASSERT(pItem != nullptr) {
                    continue;
                }

                SearchCrateSummary searchCrateSummary;
                if (!m_pTrackCollection->searchCrates().readSearchCrateSummaryById(
                            searchCrateId, &searchCrateSummary)) {
                    qWarning() << "[GROUPEDSEARCHCRATESFEATURE] Failed to "
                                  "fetch summary for searchCrate ID:"
                               << searchCrateId;
                    continue;
                }

                updateTreeItemForSearchCrateSummary(pItem, searchCrateSummary);
                // Update fullPath for the entire tree under this item
                updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());

                m_pSidebarModel->triggerRepaint(index);
            } else {
                // Rebuild the group if the searchCrate is missing
                rebuildChildModel(searchCrateId);
            }
        }
    }
    if (previouslySelectedIndex.isValid()) {
        m_lastRightClickedIndex = previouslySelectedIndex;
    }
}

void GroupedSearchCratesFeature::activateChild(const QModelIndex& index) {
    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> activateChild() -> index" << index;
    }

    SearchCrateId searchCrateId(searchCrateIdFromIndex(index));

    if (searchCrateId.toString() == "-1") {
        // Group activated
        if (sDebug) {
            qDebug() << "[GroupedSearchCratesFeature] -> activateChild() -> Group activated";
        }
        const QString& fullPath = fullPathFromIndex(index);
        if (fullPath.isEmpty()) {
            qWarning() << "[GROUPEDSEARCHCRATESFEATURE] -> activateChild() -> Group "
                          "activated: No valid full path for index: "
                       << index;
            return;
        }
        if (sDebug) {
            qDebug() << "[GroupedSearchCratesFeature] -> activateChild() -> Group "
                        "activated -> fullPath:"
                     << fullPath;
        }

        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        m_prevSiblingSearchCrate = SearchCrateId();
        emit saveModelState();
        emit disableSearch();
        emit enableCoverArtDisplay(false);

        m_searchCrateTableModel.selectSearchCrateGroup(fullPath);
        emit featureSelect(this, m_lastClickedIndex);
        emit showTrackModel(&m_searchCrateTableModel);
    } else {
        // searchCrate activated
        if (sDebug) {
            qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> activateChild() -> Child "
                        "searchCrate activated -> crateId: "
                     << searchCrateId;
        }
        VERIFY_OR_DEBUG_ASSERT(searchCrateId.isValid()) {
            return;
        }
        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        m_prevSiblingSearchCrate = SearchCrateId();
        emit saveModelState();
        m_searchCrateTableModel.selectSearchCrate(searchCrateId);
        emit showTrackModel(&m_searchCrateTableModel);
        emit enableCoverArtDisplay(true);
    }
}

QString GroupedSearchCratesFeature::fullPathFromIndex(const QModelIndex& index) const {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedSearchCratesVarLengthMask"));
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

QString GroupedSearchCratesFeature::groupNameFromIndex(const QModelIndex& index) const {
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

void GroupedSearchCratesFeature::updateFullPathRecursive(
        TreeItem* pItem, const QString& parentPath) {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedSearchCratesVarLengthMask"));
    if (!pItem) {
        return;
    }

    QString currentFullPath = parentPath.isEmpty()
            ? pItem->getLabel()
            : parentPath + delimiter + pItem->getLabel();
    pItem->setFullPath(currentFullPath);

    if (sDebug) {
        qDebug() << "[GROUPEDSEARCHCRATESFEATURE] -> Updated full path for "
                    "item: "
                 << pItem->getLabel() << " FullPath: " << currentFullPath;
    }

    for (TreeItem* pChild : pItem->children()) {
        updateFullPathRecursive(pChild, currentFullPath);
    }
}
