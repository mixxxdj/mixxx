#include "library/trackset/smarties/groupedsmartiesfeature.h"

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
#include "library/trackset/smarties/smartiesfeaturehelper.h"
#include "library/trackset/smarties/smartiessummary.h"
#include "library/treeitem.h"
#include "moc_groupedsmartiesfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

constexpr int kInvalidSmartiesId = -1;
const bool sDebug = false;

QString formatLabel(
        const SmartiesSummary& smartiesSummary) {
    return QStringLiteral("%1 [%2] (%3) %4")
            .arg(
                    smartiesSummary.getName(),
                    smartiesSummary.getSearchInput(),
                    QString::number(smartiesSummary.getTrackCount()),
                    smartiesSummary.getTrackDurationText());
}

const ConfigKey kConfigKeyLastImportExportSmartiesDirectoryKey(
        "[Library]", "LastImportExportSmartiesDirectory");

} // anonymous namespace

using namespace mixxx::library::prefs;

GroupedSmartiesFeature::GroupedSmartiesFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary,
                  pConfig,
                  "GROUPEDSMARTIESHOME",
                  QStringLiteral("smarties")),
          m_lockedSmartiesIcon(
                  ":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(
                  pLibrary->trackCollectionManager()->internalCollection()),
          m_smartiesTableModel(this,
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

void GroupedSmartiesFeature::initActions() {
    m_pCreateSmartiesAction = make_parented<QAction>(tr("Create New Smarties"), this);
    connect(m_pCreateSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotCreateSmarties);
    m_pEditSmartiesAction = make_parented<QAction>(tr("Edit Smarties"), this);
    connect(m_pEditSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotEditSmarties);
    m_pRenameSmartiesAction = make_parented<QAction>(tr("Rename"), this);
    m_pRenameSmartiesAction->setShortcut(kRenameSidebarItemShortcutKey);
    connect(m_pRenameSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotRenameSmarties);
    m_pDuplicateSmartiesAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotDuplicateSmarties);
    m_pDeleteSmartiesAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteSmartiesAction->setShortcut(removeKeySequence);
    connect(m_pDeleteSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotDeleteSmarties);
    m_pLockSmartiesAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotToggleSmartiesLock);

    m_pAnalyzeSmartiesAction = make_parented<QAction>(tr("Analyze entire Smarties"), this);
    connect(m_pAnalyzeSmartiesAction.get(),
            &QAction::triggered,
            this,
            &GroupedSmartiesFeature::slotAnalyzeSmarties);
}

void GroupedSmartiesFeature::connectLibrary(Library* pLibrary) {
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
            &GroupedSmartiesFeature::slotResetSelectedTrack);
}

void GroupedSmartiesFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new or duplicate smarties
            &TrackCollection::smartiesInserted,
            this,
            &GroupedSmartiesFeature::slotSmartiesTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::smartiesUpdated,
            this,
            &GroupedSmartiesFeature::slotSmartiesTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::smartiesDeleted,
            this,
            &GroupedSmartiesFeature::slotSmartiesTableChanged);
    connect(m_pTrackCollection, // smarties tracks hidden, unhidden or purged
            &TrackCollection::smartiesTracksChanged,
            this,
            &GroupedSmartiesFeature::slotSmartiesContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::smartiesSummaryChanged,
            this,
            &GroupedSmartiesFeature::slotUpdateSmartiesLabels);
}

QVariant GroupedSmartiesFeature::title() {
    return tr("Smarties (Grouped)");
}

QString GroupedSmartiesFeature::formatRootViewHtml() const {
    QString smartiesTitle = tr("Smarties (Grouped)");
    QString smartiesSummary =
            tr("Smarties are saved search queries, they can help you to easily "
               "find track.");
    QString smartiesSummary2 =
            tr("Use smarties to have an overview of all tracks of an album or "
               "artist, with parts in the title, added before/after a date, "
               "with a corresponding comment, bpm ... ");
    QString smartiesSummary3 =
            tr("Smarties let you keep your searches for later reuse!");

    QString html;
    QString createSmartiesLink = tr("Create New Smarties");
    html.append(QStringLiteral("<h2>%1</h2>").arg(smartiesTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(smartiesSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(smartiesSummary2));
    html.append(QStringLiteral("<p>%1</p>").arg(smartiesSummary3));
    // Colorize links in lighter blue, instead of QT default dark blue.
    // Links are still different from regular text, but readable on dark/light backgrounds.
    // https://github.com/mixxxdj/mixxx/issues/9103
    html.append(
            QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                    .arg(createSmartiesLink));
    return html;
}

std::unique_ptr<TreeItem> GroupedSmartiesFeature::newTreeItemForSmartiesSummary(
        const SmartiesSummary& smartiesSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForSmartiesSummary(pTreeItem.get(), smartiesSummary);
    return pTreeItem;
}

void GroupedSmartiesFeature::updateTreeItemForSmartiesSummary(
        TreeItem* pTreeItem, const SmartiesSummary& smartiesSummary) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(smartiesSummary.getId().toVariant());
    } else {
        // The data (= SmartiesId) is immutable once it has been set
        DEBUG_ASSERT(SmartiesId(pTreeItem->getData()) == smartiesSummary.getId());
    }
    // Update mutable properties
    // next line disabled to let the new treeitem creation format the displayname
    // pTreeItem->setLabel(formatLabel(smartiesSummary));
    pTreeItem->setIcon(smartiesSummary.isLocked() ? m_lockedSmartiesIcon : QIcon());
}

void GroupedSmartiesFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &GroupedSmartiesFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void GroupedSmartiesFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* GroupedSmartiesFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void GroupedSmartiesFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    m_lastRightClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

bool GroupedSmartiesFeature::activateSmarties(SmartiesId smartiesId) {
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [ACTIVATE SMARTIES] -> smartiesId " << smartiesId;
    }
    VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
        return false;
    }
    if (!m_pTrackCollection->smarties().readSmartiesSummaryById(smartiesId)) {
        // this may happen if called by slotSmartiesTableChanged()
        // and the smarties has just been deleted
        return false;
    }
    QModelIndex index = indexFromSmartiesId(smartiesId);
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    m_prevSiblingSmarties = SmartiesId();
    emit saveModelState();
    m_smartiesTableModel.selectSmarties(smartiesId);
    emit showTrackModel(&m_smartiesTableModel);
    emit enableCoverArtDisplay(true);
    // Update selection
    emit featureSelect(this, m_lastClickedIndex);
    return true;
}

bool GroupedSmartiesFeature::readLastRightClickedSmarties(Smarties* pSmarties) const {
    SmartiesId smartiesId(smartiesIdFromIndex(m_lastRightClickedIndex));
    VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
        qWarning() << "Failed to determine id of selected smarties";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            m_pTrackCollection->smarties().readSmartiesById(smartiesId, pSmarties)) {
        qWarning() << "Failed to read selected smarties with id" << smartiesId;
        return false;
    }
    return true;
}

bool GroupedSmartiesFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void GroupedSmartiesFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateSmartiesAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditSmartiesAction.get());
    menu.addSeparator();
    menu.exec(globalPos);
}

void GroupedSmartiesFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    SmartiesId smartiesId(smartiesIdFromIndex(index));
    if (!smartiesId.isValid()) {
        return;
    }

    Smarties smarties;
    if (!m_pTrackCollection->smarties().readSmartiesById(smartiesId, &smarties)) {
        return;
    }

    m_pDeleteSmartiesAction->setEnabled(!smarties.isLocked());
    m_pRenameSmartiesAction->setEnabled(!smarties.isLocked());
    m_pLockSmartiesAction->setText(smarties.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateSmartiesAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditSmartiesAction.get());
    menu.addSeparator();
    menu.addAction(m_pRenameSmartiesAction.get());
    menu.addAction(m_pDuplicateSmartiesAction.get());
    menu.addAction(m_pDeleteSmartiesAction.get());
    menu.addAction(m_pLockSmartiesAction.get());
    menu.addSeparator();
    menu.addSeparator();
    menu.addAction(m_pAnalyzeSmartiesAction.get());
    menu.addSeparator();
    menu.exec(globalPos);
}

void GroupedSmartiesFeature::slotCreateSmartiesFromSearch(const QString& text) {
    SmartiesId smartiesId =
            SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySmartiesFromSearch(text);

    if (smartiesId.isValid()) {
        // expand Smarties and scroll to new smarties
        m_pSidebarWidget->selectChildIndex(indexFromSmartiesId(smartiesId), false);
        m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
        activateSmarties(smartiesId);
    }
}

void GroupedSmartiesFeature::slotCreateSmartiesFromUI() {
    SmartiesId smartiesId =
            SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySmartiesFromUI();
    if (smartiesId.isValid()) {
        // expand Smarties and scroll to new smarties
        m_pSidebarWidget->selectChildIndex(indexFromSmartiesId(smartiesId), false);
    }
}

void GroupedSmartiesFeature::slotCreateSmarties() {
    SmartiesId smartiesId =
            SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySmarties();
    if (smartiesId.isValid()) {
        // expand Smarties and scroll to new smarties
        m_pSidebarWidget->selectChildIndex(indexFromSmartiesId(smartiesId), false);
    }
}

void GroupedSmartiesFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteSmarties();
}

void GroupedSmartiesFeature::slotDeleteSmarties() {
    Smarties smarties;
    if (readLastRightClickedSmarties(&smarties)) {
        if (smarties.isLocked()) {
            // qWarning() << "Refusing to delete locked smarties" << smarties;
            QMessageBox msgBox;
            msgBox.setText("Locked Smarties can't be deleted");
            msgBox.exec();
            return;
        }
        SmartiesId smartiesId = smarties.getId();
        // Store sibling id to restore selection after smarties was deleted
        // to avoid the scroll position being reset to Smarties root item.
        m_prevSiblingSmarties = SmartiesId();
        if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex)) {
            storePrevSiblingSmartiesId(smartiesId);
        }

        QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
                tr("Confirm Deletion"),
                tr("Do you really want to delete smarties <b>%1</b>?")
                        .arg(smarties.getName()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            if (m_pTrackCollection->deleteSmarties(smartiesId)) {
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT DELETE SMARTIES] -> "
                                "Deleted smarties"
                             << smarties;
                }
                return;
            }
        } else {
            return;
        }
    }
    qWarning() << "Failed to delete selected smarties";
}

void GroupedSmartiesFeature::slotFindPreviousSmarties() {
    Smarties smarties;
    if (readLastRightClickedSmarties(&smarties)) {
        SmartiesId smartiesId = smarties.getId();
        m_prevSiblingSmarties = SmartiesId();
        storePrevSiblingSmartiesId(smartiesId);
    }
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT FIND PREVIOUS SMARTIES] -> "
                    "Previous smarties ID "
                 << m_prevSiblingSmarties;
    }
}

void GroupedSmartiesFeature::slotFindNextSmarties() {
    Smarties smarties;
    if (readLastRightClickedSmarties(&smarties)) {
        SmartiesId smartiesId = smarties.getId();
        m_nextSiblingSmarties = SmartiesId();
        storeNextSiblingSmartiesId(smartiesId);
    }
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT FIND NEXT SMARTIES] -> Next "
                    "smarties ID "
                 << m_nextSiblingSmarties;
    }
}

void GroupedSmartiesFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameSmarties();
}

void GroupedSmartiesFeature::slotRenameSmarties() {
    Smarties smarties;
    if (readLastRightClickedSmarties(&smarties)) {
        const QString oldName = smarties.getName();
        smarties.resetName();
        for (;;) {
            bool ok = false;
            auto newName =
                    QInputDialog::getText(nullptr,
                            tr("Rename Smarties"),
                            tr("Enter new name for smarties:"),
                            QLineEdit::Normal,
                            oldName,
                            &ok)
                            .trimmed();
            if (!ok || newName.isEmpty()) {
                return;
            }
            if (newName.isEmpty()) {
                QMessageBox::warning(nullptr,
                        tr("Renaming Smarties Failed"),
                        tr("A smarties cannot have a blank name."));
                continue;
            }
            if (m_pTrackCollection->smarties().readSmartiesByName(newName)) {
                QMessageBox::warning(nullptr,
                        tr("Renaming Smarties Failed"),
                        tr("A smarties by that name already exists."));
                continue;
            }
            smarties.setName(std::move(newName));
            DEBUG_ASSERT(smarties.hasName());
            break;
        }

        if (!m_pTrackCollection->updateSmarties(smarties)) {
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT RENAME SMARTIES] -> "
                            "Failed to rename smarties"
                         << smarties;
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[SMARTIESFEATURE] [SLOT RENAME MARTIES] -> Failed to "
                        "rename selected smarties";
        }
    }
}

void GroupedSmartiesFeature::slotDuplicateSmarties() {
    Smarties smarties;
    if (readLastRightClickedSmarties(&smarties)) {
        SmartiesId newSmartiesId =
                SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                        .duplicateSmarties(smarties);
        if (newSmartiesId.isValid()) {
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT DUPLICATE SMARTIES] -> "
                            "Duplicate smarties"
                         << smarties << ", new smarties:" << newSmartiesId;
            }
            return;
        }
    }
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT DUPLICATE SMARTIES] -> Failed to "
                    "duplicate selected smarties";
    }
}

void GroupedSmartiesFeature::SetActiveSmartiesToLastRightClicked(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
}

void GroupedSmartiesFeature::selectSmartiesForEdit(SmartiesId selectedSmartiesId) {
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [REBUILD CHILD MODEL] -> "
                    "selectedSmartiesId "
                 << selectedSmartiesId;
    }
    //    return selectedSmartiesId;
}

void GroupedSmartiesFeature::slotEditSmarties() {
    QMutex mutex;
    //    mutex.lock();
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> slotEditSmarties";
    }
    Smarties smarties;
    readLastRightClickedSmarties(&smarties);
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> "
                    "m_lastRightClickedIndex  = "
                 << m_lastRightClickedIndex;
    }
    // Load data into QVariant
    smartiesData.clear();
    m_smartiesTableModel.selectSmarties2QVL(
            smartiesIdFromIndex(m_lastRightClickedIndex), smartiesData);
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> "
                    "Smarties data loaded into QVariantList:"
                 << smartiesData;
    }

    QVariantList playlistsCratesData;
    m_smartiesTableModel.selectPlaylistsCrates2QVL(playlistsCratesData);
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> "
                    "Playlists & Crates data loaded into QVariantList:"
                 << playlistsCratesData;
    }

    if (readLastRightClickedSmarties(&smarties)) {
        SmartiesId smartiesId = smartiesIdFromIndex(m_lastRightClickedIndex);
        if (sDebug) {
            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> "
                        "SlotEditSmarties -> smartiesID = "
                     << smartiesId;
        }
        dlgGroupedSmartiesInfo infoDialog(
                this); // Pass this to provide the GroupedSmartiesFeature
                       // instance
        if (sDebug) {
            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> "
                        "[INIT DIALOG] -> INIT DIALOG ";
        }
        infoDialog.init(smartiesData, playlistsCratesData);

        // Connect the dataUpdated signal to update smartiesData when Apply is clicked
        connect(&infoDialog,
                &dlgGroupedSmartiesInfo::dataUpdated,
                this,
                [this, &smartiesId](const QVariantList& updatedData) {
                    //            emit setBlockerOff("update");
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[UPDATE] -> START Request UPDATE Smarties "
                                    "smartiesId "
                                 << smartiesId;
                    }
                    smartiesData =
                            updatedData; // Capture the updated data from the UI
                    smartiesId = smartiesIdFromIndex(m_lastRightClickedIndex);
                    // m_lastRightClickedIndex =
                    // indexFromSmartiesId(smartiesId);
                    activateSmarties(smartiesId);
                    m_smartiesTableModel.saveQVL2Smarties(
                            smartiesId, smartiesData);
                    m_smartiesTableModel.selectSmarties2QVL(
                            smartiesIdFromIndex(m_lastRightClickedIndex),
                            smartiesData);
                    slotSmartiesTableChanged(smartiesId);
                    //            emit setBlockerOff("update");
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[UPDATE] -> END UPDATE smartiesId "
                                 << smartiesId;
                    }
                    m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                    emit updateSmartiesData(smartiesData);
                });

        connect(&infoDialog,
                &dlgGroupedSmartiesInfo::requestDeleteSmarties,
                this,
                [this, &smartiesId]() {
                    //            emit setBlockerOff("delete");
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[DELETE] "
                                    "-> START Request DELETE Smarties "
                                    "smartiesId "
                                 << smartiesId;
                    }
                    m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                    if (!smartiesId.isValid()) {
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[DELETE] -> Invalid SmartiesId. : "
                                     << smartiesId;
                        }
                        return;
                    } else {
                        slotFindPreviousSmarties();
                        auto smartiesJustAbove = m_prevSiblingSmarties;
                        slotDeleteSmarties();
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[DELETE] -> smarties is deleted -> "
                                        "new active "
                                        "smarties.";
                        }
                        activateSmarties(smartiesJustAbove);
                        smartiesId = smartiesJustAbove;
                        smartiesData.clear();
                        m_smartiesTableModel.selectSmarties2QVL(
                                smartiesId, smartiesData);
                        m_lastRightClickedIndex =
                                indexFromSmartiesId(smartiesId);
                        activateSmarties(smartiesId);
                        //                emit setBlockerOff("delete");
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[DELETE] -> END DELETE Smarties "
                                        "smartiesId "
                                     << smartiesId;
                        }
                        m_lastRightClickedIndex =
                                indexFromSmartiesId(smartiesId);
                        emit updateSmartiesData(smartiesData);
                    }
                });

        connect(&infoDialog,
                &dlgGroupedSmartiesInfo::requestNewSmarties,
                this,
                [this, &smartiesId]() {
                    //            emit setBlockerOff("new");
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[NEW] "
                                    "-> START Request NEW Smarties smartiesId "
                                 << smartiesId;
                    }
                    SmartiesId smartiesId =
                            SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                                    .createEmptySmartiesFromUI();
                    if (!smartiesId.isValid()) {
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[NEW] -> Creation failed.";
                        }
                        return;
                    } else {
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[NEW] -> New smarties created. "
                                        "smartiesId "
                                     << smartiesId;
                        }
                    }
                    activateSmarties(smartiesId);
                    smartiesData.clear();
                    m_smartiesTableModel.selectSmarties2QVL(
                            smartiesId, smartiesData);
                    slotSmartiesTableChanged(smartiesId);
                    m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[NEW] "
                                    "-> END Smarties created smartiesId "
                                 << smartiesId;
                    }
                    emit updateSmartiesData(smartiesData);
                    //            emit setBlockerOff("new");
                });

        connect(&infoDialog,
                &dlgGroupedSmartiesInfo::requestPreviousSmarties,
                this,
                [this, &smartiesId]() {
                    //            emit setBlockerOff("previous");
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[PREVIOUS] -> START Request PREVIOUS "
                                    "Smarties smartiesId "
                                 << smartiesId;
                    }
                    m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                    if (!smartiesId.isValid()) {
                        if (sDebug) {
                            qDebug()
                                    << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] "
                                       "-> [PREVIOUS] -> Invalid SmartiesId. : "
                                    << smartiesId;
                        }
                        return;
                    } else {
                        slotFindPreviousSmarties();
                        auto smartiesJustAbove = m_prevSiblingSmarties;
                        activateSmarties(smartiesJustAbove);
                        smartiesId = smartiesJustAbove;
                        if (sDebug) {
                            qDebug()
                                    << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] "
                                       "-> [PREVIOUS] -> Previous smarties ID"
                                    << m_prevSiblingSmarties;
                        }
                        smartiesData.clear();
                        m_smartiesTableModel.selectSmarties2QVL(
                                smartiesId, smartiesData);
                        m_lastRightClickedIndex =
                                indexFromSmartiesId(smartiesId);
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> [PREVIOUS] -> Sibling "
                                        "Smarties loaded smartiesId."
                                     << smartiesId;
                            //                emit setBlockerOff("previous");
                            qDebug()
                                    << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] "
                                       "-> [PREVIOUS] -> END Sibling Smarties "
                                       "loaded m_lastRightClickedIndex."
                                    << m_lastRightClickedIndex;
                        }
                        emit updateSmartiesData(smartiesData);
                    }
                });
        connect(&infoDialog,
                &dlgGroupedSmartiesInfo::requestNextSmarties,
                this,
                [this, &smartiesId]() {
                    //            emit setBlockerOff("next");
                    if (sDebug) {
                        qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                    "[NEXT] "
                                    "-> START Request NEXT Smarties smartiesId "
                                 << smartiesId;
                    }
                    m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                    if (!smartiesId.isValid()) {
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[NEXT] -> Invalid SmartiesId. : "
                                     << smartiesId;
                        }
                        return;
                    } else {
                        slotFindNextSmarties();
                        auto smartiesJustBelow = m_nextSiblingSmarties;
                        activateSmarties(smartiesJustBelow);
                        smartiesId = smartiesJustBelow;
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[NEXT] -> Next smarties ID"
                                     << m_nextSiblingSmarties;
                        }
                        smartiesData.clear();
                        m_smartiesTableModel.selectSmarties2QVL(
                                smartiesId, smartiesData);
                        m_lastRightClickedIndex =
                                indexFromSmartiesId(smartiesId);
                        if (sDebug) {
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[NEXT] -> Sibling Smarties loaded "
                                        "smartiesId."
                                     << smartiesId;
                            //                emit setBlockerOff("next");
                            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT "
                                        "SMARTIES] -> "
                                        "[NEXT] -> END Sibling Smarties loaded "
                                        "m_lastRightClickedIndex."
                                     << m_lastRightClickedIndex;
                        }
                        emit updateSmartiesData(smartiesData);
                    }
                });

        // Execute & close the dialog
        if (infoDialog.exec() == QDialog::Accepted) {
            m_smartiesTableModel.saveQVL2Smarties(smartiesId, smartiesData);
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [CLOSE "
                            "DIALOG] -> Smarties data saved from QVariantList "
                            "to database for "
                            "SmartiesId:"
                         << smartiesId;
            }
            activateSmarties(smartiesId);
            m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
            slotSmartiesTableChanged(smartiesId);
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [CLOSE "
                            "DIALOG] -> Smarties sidebar update for SmartiesId";
            }
        }
    }
    //    mutex.unlock();
}

void GroupedSmartiesFeature::slotToggleSmartiesLock() {
    Smarties smarties;
    if (readLastRightClickedSmarties(&smarties)) {
        smarties.setLocked(!smarties.isLocked());
        if (!m_pTrackCollection->updateSmarties(smarties)) {
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT TOGGLE LOCK] -> Failed to "
                            "toggle lock of smarties"
                         << smarties;
            }
        }
    } else {
        if (sDebug) {
            qDebug() << "[SMARTIESFEATURE] [SLOT TOGGLE LOCK] -> Failed to "
                        "toggle lock of selected smarties";
        }
    }
}

SmartiesId GroupedSmartiesFeature::smartiesIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return SmartiesId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return SmartiesId();
    }
    return SmartiesId(item->getData());
}

QModelIndex GroupedSmartiesFeature::indexFromSmartiesId(SmartiesId smartiesId) const {
    VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
        return QModelIndex();
    }
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                (SmartiesId(pTreeItem->getData()) == smartiesId)) {
            return index;
        }
    }
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [INDEX FROM SMARTIESID] -> Tree item "
                    "for smarties not found:"
                 << smartiesId;
    }
    return QModelIndex();
}

void GroupedSmartiesFeature::slotAnalyzeSmarties() {
    if (m_lastRightClickedIndex.isValid()) {
        SmartiesId smartiesId = smartiesIdFromIndex(m_lastRightClickedIndex);
        if (smartiesId.isValid()) {
            QList<AnalyzerScheduledTrack> tracks;
            tracks.reserve(
                    m_pTrackCollection->smarties().countSmartiesTracks(smartiesId));
            {
                SmartiesTrackSelectResult smartiesTracks(
                        m_pTrackCollection->smarties().selectSmartiesTracksSorted(
                                smartiesId));
                while (smartiesTracks.next()) {
                    tracks.append(smartiesTracks.trackId());
                }
            }
            emit analyzeTracks(tracks);
        }
    }
}

void GroupedSmartiesFeature::storePrevSiblingSmartiesId(SmartiesId smartiesId) {
    QModelIndex actIndex = indexFromSmartiesId(smartiesId);
    m_prevSiblingSmarties = SmartiesId();
    for (int i = (actIndex.row() + 1); i >= (actIndex.row() - 1); i -= 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_prevSiblingSmarties = smartiesIdFromIndex(newIndex);
            }
        }
    }
}

void GroupedSmartiesFeature::storeNextSiblingSmartiesId(SmartiesId smartiesId) {
    QModelIndex actIndex = indexFromSmartiesId(smartiesId);
    m_nextSiblingSmarties = SmartiesId();
    for (int i = (actIndex.row() - 1); i <= (actIndex.row() + 1); i += 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_nextSiblingSmarties = smartiesIdFromIndex(newIndex);
            }
        }
    }
}

void GroupedSmartiesFeature::slotSmartiesTableChanged(SmartiesId smartiesId) {
    Q_UNUSED(smartiesId);
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        // If the previously selected smarties was loaded to the tracks table and
        // selected in the sidebar try to activate that or a sibling
        rebuildChildModel();
        if (!activateSmarties(m_smartiesTableModel.selectedSmarties())) {
            // probably last clicked smarties was deleted, try to
            // select the stored sibling
            if (m_prevSiblingSmarties.isValid()) {
                activateSmarties(m_prevSiblingSmarties);
            }
        }
    } else {
        // No valid selection to restore
        rebuildChildModel();
    }
}

void GroupedSmartiesFeature::slotSmartiesContentChanged(SmartiesId smartiesId) {
    QSet<SmartiesId> updatedSmartiesIds;
    updatedSmartiesIds.insert(smartiesId);
    updateChildModel(updatedSmartiesIds);
}

void GroupedSmartiesFeature::slotUpdateSmartiesLabels(const QSet<SmartiesId>& updatedSmartiesIds) {
    updateChildModel(updatedSmartiesIds);
}

void GroupedSmartiesFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreateSmarties();
    } else {
        if (sDebug) {
            qDebug() << "[SMARTIESFEATURE] [HTML LINK CLICKED ] -> Unknown "
                        "smarties link clicked"
                     << link;
        }
    }
}

void GroupedSmartiesFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    std::vector<SmartiesId> sortedTrackSmarties;
    if (m_selectedTrackId.isValid()) {
        SmartiesTrackSelectResult trackSmartiesIter(
                m_pTrackCollection->smarties().selectTrackSmartiesSorted(m_selectedTrackId));
        while (trackSmartiesIter.next()) {
            sortedTrackSmarties.push_back(trackSmartiesIter.smartiesId());
        }
    }

    // Set all smarties the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem : pRootItem->children()) {
        DEBUG_ASSERT(pTreeItem != nullptr);
        bool smartiesContainsSelectedTrack =
                m_selectedTrackId.isValid() &&
                std::binary_search(
                        sortedTrackSmarties.begin(),
                        sortedTrackSmarties.end(),
                        SmartiesId(pTreeItem->getData()));
        pTreeItem->setBold(smartiesContainsSelectedTrack);
    }

    m_pSidebarModel->triggerRepaint();
}

void GroupedSmartiesFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}

QModelIndex GroupedSmartiesFeature::rebuildChildModel(SmartiesId selectedSmartiesId) {
    if (sDebug) {
        qDebug() << "[GROUPEDSMARTIESFEATURE] -> rebuildChildModel()" << selectedSmartiesId;
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
                    qDebug() << "[GROUPEDSMARTIESFEATURE] Saved open/close state "
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

    QList<QVariantMap> groupedSmarties = m_smartiesTableModel.getGroupedSmarties();
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedSmartiesVarLengthMask"));

    if (m_pConfig->getValue<int>(ConfigKey("[Library]", "GroupedSmartiesLength")) == 0) {
        // Fixed prefix length
        QMap<QString, int> groupCounts;
        for (int i = 0; i < groupedSmarties.size(); ++i) {
            const auto& smartiesData = groupedSmarties[i];
            const QString& groupName = smartiesData["group_name"].toString();
            groupCounts[groupName]++;
        }

        QMap<QString, TreeItem*> groupItems;
        std::vector<std::unique_ptr<TreeItem>> modelRows;

        for (int i = 0; i < groupedSmarties.size(); ++i) {
            const auto& smartiesData = groupedSmarties[i];
            const QString& groupName = smartiesData["group_name"].toString();
            SmartiesId smartiesId(smartiesData["smarties_id"]);

            SmartiesSummary smartiesSummary;
            if (!m_pTrackCollection->smarties().readSmartiesSummaryById(
                        smartiesId, &smartiesSummary)) {
                qWarning() << "[GROUPEDSMARTIESFEATURE] -> Failed to fetch summary "
                              "for smarties ID:"
                           << smartiesId;
                continue;
            }

            const QString& smartiesSummaryName = formatLabel(smartiesSummary);

            if (groupCounts[groupName] > 1) {
                TreeItem* pGroupItem = groupItems.value(groupName, nullptr);
                if (!pGroupItem) {
                    auto newGroup = std::make_unique<TreeItem>(groupName, kInvalidSmartiesId);
                    pGroupItem = newGroup.get();
                    groupItems.insert(groupName, pGroupItem);
                    modelRows.push_back(std::move(newGroup));
                }

                const QString& displaySmartiesName =
                        smartiesSummaryName.mid(groupName.length()).trimmed();
                if (sDebug) {
                    qDebug() << "[GROUPEDSMARTIESFEATURE] -> smartiesSummaryName - "
                                "displaySmartiesName = "
                             << smartiesSummaryName << " - " << displaySmartiesName;
                }

                TreeItem* pChildItem = pGroupItem->appendChild(
                        displaySmartiesName, smartiesId.toVariant().toInt());
                pChildItem->setFullPath(groupName + delimiter + displaySmartiesName);
                updateTreeItemForSmartiesSummary(pChildItem, smartiesSummary);
                if (sDebug) {
                    qDebug() << "[GROUPEDSMARTIESFEATURE] Added SmartiesId to group:"
                             << smartiesId << "Group:" << groupName;
                }
            } else {
                auto newSmarties = std::make_unique<TreeItem>(
                        smartiesSummaryName, smartiesId.toVariant().toInt());
                newSmarties->setFullPath(smartiesSummaryName);
                updateTreeItemForSmartiesSummary(newSmarties.get(), smartiesSummary);

                modelRows.push_back(std::move(newSmarties));
            }
        }

        m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);
        slotTrackSelected(m_selectedTrackId);

    } else {
        // variable group prefix length with mask
        QMap<QString, QList<QVariantMap>> topLevelGroups;
        for (int i = 0; i < groupedSmarties.size(); ++i) {
            const auto& smartiesData = groupedSmarties[i];
            const QString& groupName = smartiesData["group_name"].toString();
            const QString& topGroup = groupName.section(delimiter, 0, 0);
            topLevelGroups[topGroup].append(smartiesData);
        }

        if (sDebug) {
            qDebug() << "[GROUPEDSMARTIESFEATURE] Top-level groups:";
            for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
                qDebug() << "Group:" << it.key() << "-> Smarties:" << it.value().size();
            }
        }
        // lambda function to build tree
        std::function<void(
                const QString&, const QList<QVariantMap>&, TreeItem*)>
                buildTreeStructure;
        buildTreeStructure = [&](const QString& currentPath,
                                     const QList<QVariantMap>& smartiesGroups,
                                     TreeItem* pParentItem) {
            QMap<QString, QList<QVariantMap>> subgroupedSmarties;

            for (const QVariantMap& smartiesData : smartiesGroups) {
                const QString& groupName = smartiesData["group_name"].toString();

                if (sDebug) {
                    qDebug() << "[GROUPEDSMARTIESFEATURE] Processing smarties with "
                                "groupName:"
                             << groupName << "currentPath:" << currentPath;
                }

                if (!groupName.startsWith(currentPath)) {
                    if (sDebug) {
                        qDebug() << "[GROUPEDSMARTIESFEATURE] Skipping smarties. "
                                    "Group name does not match path:"
                                 << groupName << "Current path:" << currentPath;
                    }
                    continue;
                }

                const QString& remainingPath = groupName.mid(currentPath.length());
                int delimiterPos = remainingPath.indexOf(delimiter);

                if (delimiterPos >= 0) {
                    const QString& subgroupName = remainingPath.left(delimiterPos);
                    subgroupedSmarties[subgroupName].append(smartiesData);
                    if (sDebug) {
                        qDebug() << "[GROUPEDSMARTIESFEATURE] Added smarties to "
                                    "subgroup:"
                                 << subgroupName
                                 << "Remaining path:" << remainingPath;
                    }
                } else {
                    SmartiesId smartiesId(smartiesData["smarties_id"]);
                    SmartiesSummary smartiesSummary;
                    if (!m_pTrackCollection->smarties().readSmartiesSummaryById(
                                smartiesId, &smartiesSummary)) {
                        qWarning() << "[GROUPEDSMARTIESFEATURE] Failed to fetch "
                                      "summary for smarties ID:"
                                   << smartiesId;
                        continue;
                    }

                    const QString& displaySmartiesName =
                            formatLabel(smartiesSummary).mid(currentPath.length());

                    TreeItem* pChildItem = pParentItem->appendChild(
                            displaySmartiesName.trimmed(), smartiesId.toVariant());
                    pChildItem->setFullPath(currentPath + delimiter + displaySmartiesName);
                    updateTreeItemForSmartiesSummary(pChildItem, smartiesSummary);
                    if (sDebug) {
                        qDebug() << "[GROUPEDSMARTIESFEATURE] Added smarties "
                                    "displaySmartiesName: "
                                 << displaySmartiesName
                                 << "to Parent:" << pParentItem->getLabel();
                    }
                }
            }

            for (auto it = subgroupedSmarties.constBegin();
                    it != subgroupedSmarties.constEnd();
                    ++it) {
                const QString& subgroupName = it.key();
                const QList<QVariantMap>& subgroupSmarties = it.value();
                if (!subgroupSmarties.isEmpty()) {
                    if (subgroupSmarties.size() > 1) {
                        // subgroup has > 1 smarties -> create subgroup
                        auto pNewSubgroup = std::make_unique<TreeItem>(
                                subgroupName, kInvalidSmartiesId);
                        TreeItem* pSubgroupItem = pNewSubgroup.get();
                        pParentItem->insertChild(pParentItem->childCount(),
                                std::move(pNewSubgroup));
                        if (sDebug) {
                            qDebug() << "[GROUPEDSMARTIESFEATURE] Created subgroup:" << subgroupName
                                     << "Parent:" << pParentItem->getLabel();
                        }

                        // loop into the subgroup
                        buildTreeStructure(
                                currentPath + subgroupName + delimiter,
                                subgroupSmarties,
                                pSubgroupItem);
                    } else {
                        // only one smarties -> directly under the parent, NO subgroup
                        const QVariantMap& smartiesData = subgroupSmarties.first();
                        SmartiesId smartiesId(smartiesData["smarties_id"]);
                        SmartiesSummary smartiesSummary;
                        if (!m_pTrackCollection->smarties().readSmartiesSummaryById(
                                    smartiesId, &smartiesSummary)) {
                            qWarning() << "[GROUPEDSMARTIESFEATURE] Failed to "
                                          "fetch summary for smarties ID:"
                                       << smartiesId;
                            continue;
                        }

                        const QString& displaySmartiesName =
                                formatLabel(smartiesSummary)
                                        .mid(currentPath.length());

                        TreeItem* pChildItem = pParentItem->appendChild(
                                displaySmartiesName.trimmed(), smartiesId.toVariant());
                        pChildItem->setFullPath(currentPath + delimiter + displaySmartiesName);
                        updateTreeItemForSmartiesSummary(pChildItem, smartiesSummary);
                        if (sDebug) {
                            qDebug()
                                    << "[GROUPEDSMARTIESFEATURE] Added single "
                                       "smarties displaySmartiesName: "
                                    << displaySmartiesName
                                    << " to Parent:" << pParentItem->getLabel();
                        }
                    }
                }
            }
        };
        // building rootlevel groups
        for (auto it = topLevelGroups.constBegin(); it != topLevelGroups.constEnd(); ++it) {
            const QString& groupName = it.key();
            const QList<QVariantMap>& smartiesGroups = it.value();

            if (smartiesGroups.size() > 1) {
                auto pNewGroup = std::make_unique<TreeItem>(groupName, kInvalidSmartiesId);
                TreeItem* pGroupItem = pNewGroup.get();
                pRootItem->insertChild(pRootItem->childCount(), std::move(pNewGroup));
                if (sDebug) {
                    qDebug() << "[GROUPEDSMARTIESFEATURE] Created top-level group:" << groupName;
                }

                buildTreeStructure(groupName + delimiter, smartiesGroups, pGroupItem);
            } else {
                const QVariantMap& smartiesData = smartiesGroups.first();
                SmartiesId smartiesId(smartiesData["smarties_id"]);
                SmartiesSummary smartiesSummary;

                if (!m_pTrackCollection->smarties().readSmartiesSummaryById(
                            smartiesId, &smartiesSummary)) {
                    qWarning() << "[GROUPEDSMARTIESFEATURE] Failed to fetch "
                                  "summary for smarties ID:"
                               << smartiesId;
                    continue;
                }

                const QString& displaySmartiesName = formatLabel(smartiesSummary);
                TreeItem* pChildItem = pRootItem->appendChild(
                        displaySmartiesName.trimmed(), smartiesId.toVariant());
                updateTreeItemForSmartiesSummary(pChildItem, smartiesSummary);
                if (sDebug) {
                    qDebug() << "[GROUPEDSMARTIESFEATURE] Added smarties to "
                                "root:"
                             << displaySmartiesName;
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
                        qDebug() << "[GROUPEDSMARTIESFEATURE] Restored expanded "
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

void GroupedSmartiesFeature::updateChildModel(const QSet<SmartiesId>& updatedSmartiesIds) {
    if (sDebug) {
        qDebug() << "[GROUPEDSMARTIESFEATURE] -> updateChildModel() -> Updating "
                    "smarties"
                 << updatedSmartiesIds;
    }

    QModelIndex previouslySelectedIndex = m_lastRightClickedIndex;

    // Fetch grouped smarties using SmartiesTableModel
    QList<QVariantMap> groupedSmarties = m_smartiesTableModel.getGroupedSmarties();

    QMap<QString, QList<QVariantMap>> groupedSmartiesMap;
    for (int i = 0; i < groupedSmarties.size(); ++i) {
        const auto& smartiesData = groupedSmarties[i];
        groupedSmartiesMap[smartiesData["group_name"].toString()].append(smartiesData);
    }

    // Update full paths recursively for all items starting from the root
    updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());

    // Update or rebuild items
    for (const SmartiesId& smartiesId : updatedSmartiesIds) {
        // Find the updated smarties in groupedSmarties
        auto updatedGroup = std::find_if(
                groupedSmarties.begin(),
                groupedSmarties.end(),
                [&smartiesId](const QVariantMap& smartiesData) {
                    return smartiesData["smarties_id"].toInt() == smartiesId.toVariant().toInt();
                });

        if (updatedGroup != groupedSmarties.end()) {
            QModelIndex index = indexFromSmartiesId(smartiesId);
            if (index.isValid()) {
                // Update the existing item
                TreeItem* pItem = m_pSidebarModel->getItem(index);
                VERIFY_OR_DEBUG_ASSERT(pItem != nullptr) {
                    continue;
                }
                pItem->setData((*updatedGroup)["smarties_name"].toString());
                updateTreeItemForSmartiesSummary(pItem, SmartiesSummary(smartiesId));

                // Update fullPath for the entire tree under this item
                updateFullPathRecursive(m_pSidebarModel->getRootItem(), QString());

                m_pSidebarModel->triggerRepaint(index);
            } else {
                // Rebuild the group if the smarties is missing
                rebuildChildModel(smartiesId);
            }
        }
    }
    if (previouslySelectedIndex.isValid()) {
        m_lastRightClickedIndex = previouslySelectedIndex;
    }
}

void GroupedSmartiesFeature::activateChild(const QModelIndex& index) {
    if (sDebug) {
        qDebug() << "[GROUPEDSMARTIESFEATURE] -> activateChild() -> index" << index;
    }

    SmartiesId smartiesId(smartiesIdFromIndex(index));

    if (smartiesId.toString() == "-1") {
        // Group activated
        if (sDebug) {
            qDebug() << "[GROUPEDSMARTIESFEATURE] -> activateChild() -> Group activated";
        }
        const QString& fullPath = fullPathFromIndex(index);
        if (fullPath.isEmpty()) {
            qWarning() << "[GROUPEDSMARTIESFEATURE] -> activateChild() -> Group "
                          "activated: No valid full path for index: "
                       << index;
            return;
        }
        if (sDebug) {
            qDebug() << "[GROUPEDSMARTIESFEATURE] -> activateChild() -> Group "
                        "activated -> fullPath:"
                     << fullPath;
        }

        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        m_prevSiblingSmarties = SmartiesId();
        emit saveModelState();
        emit disableSearch();
        emit enableCoverArtDisplay(false);

        m_smartiesTableModel.selectSmartiesGroup(fullPath);
        emit featureSelect(this, m_lastClickedIndex);
        emit showTrackModel(&m_smartiesTableModel);
    } else {
        // Smarties activated
        if (sDebug) {
            qDebug() << "[GROUPEDSMARTIESFEATURE] -> activateChild() -> Child "
                        "smarties activated -> smartiesId: "
                     << smartiesId;
        }
        VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
            return;
        }
        m_lastClickedIndex = index;
        m_lastRightClickedIndex = QModelIndex();
        m_prevSiblingSmarties = SmartiesId();
        emit saveModelState();
        m_smartiesTableModel.selectSmarties(smartiesId);
        emit showTrackModel(&m_smartiesTableModel);
        emit enableCoverArtDisplay(true);
    }
}

QString GroupedSmartiesFeature::fullPathFromIndex(const QModelIndex& index) const {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedSmartiesVarLengthMask"));
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
    // if another root level smarties exists that is not in the group
    // (beginning of the name equal to root level groop name),
    // the member tracks would be added to the group,
    // with added delimiter only tracks in group member smarties are added
    fullPath = fullPath.mid(delimiter.length()).append(delimiter);
    return fullPath;
}

QString GroupedSmartiesFeature::groupNameFromIndex(const QModelIndex& index) const {
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

void GroupedSmartiesFeature::updateFullPathRecursive(TreeItem* pItem, const QString& parentPath) {
    const QString& delimiter = m_pConfig->getValue(
            ConfigKey("[Library]", "GroupedSmartiesVarLengthMask"));
    if (!pItem) {
        return;
    }

    QString currentFullPath = parentPath.isEmpty()
            ? pItem->getLabel()
            : parentPath + delimiter + pItem->getLabel();
    pItem->setFullPath(currentFullPath);

    if (sDebug) {
        qDebug() << "[GROUPEDSMARTIESFEATURE] -> Updated full path for item: " << pItem->getLabel()
                 << " FullPath: " << currentFullPath;
    }

    for (TreeItem* pChild : pItem->children()) {
        updateFullPathRecursive(pChild, currentFullPath);
    }
}
