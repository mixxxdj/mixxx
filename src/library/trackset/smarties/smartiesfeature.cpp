#include "library/trackset/smarties/smartiesfeature.h"

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
#include "moc_smartiesfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {
// constexpr int kInvalidSmartiesId = -1;
const bool sDebug = true;

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

SmartiesFeature::SmartiesFeature(Library* pLibrary, UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary,
                  pConfig,
                  "SMARTIESHOME",
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

void SmartiesFeature::initActions() {
    m_pCreateSmartiesAction = make_parented<QAction>(tr("Create New Smarties"), this);
    connect(m_pCreateSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotCreateSmarties);
    m_pEditSmartiesAction = make_parented<QAction>(tr("Edit Smarties"), this);
    connect(m_pEditSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotEditSmarties);
    m_pRenameSmartiesAction = make_parented<QAction>(tr("Rename"), this);
    m_pRenameSmartiesAction->setShortcut(kRenameSidebarItemShortcutKey);
    connect(m_pRenameSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotRenameSmarties);
    m_pDuplicateSmartiesAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotDuplicateSmarties);
    m_pDeleteSmartiesAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteSmartiesAction->setShortcut(removeKeySequence);
    connect(m_pDeleteSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotDeleteSmarties);
    m_pLockSmartiesAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotToggleSmartiesLock);
    m_pAnalyzeSmartiesAction = make_parented<QAction>(tr("Analyze entire Smarties"), this);
    connect(m_pAnalyzeSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotAnalyzeSmarties);
}

void SmartiesFeature::connectLibrary(Library* pLibrary) {
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
            &SmartiesFeature::slotResetSelectedTrack);
}

void SmartiesFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new or duplicate smarties
            &TrackCollection::smartiesInserted,
            this,
            &SmartiesFeature::slotSmartiesTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::smartiesUpdated,
            this,
            &SmartiesFeature::slotSmartiesTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::smartiesDeleted,
            this,
            &SmartiesFeature::slotSmartiesTableChanged);
    connect(m_pTrackCollection, // smarties tracks hidden, unhidden or purged
            &TrackCollection::smartiesTracksChanged,
            this,
            &SmartiesFeature::slotSmartiesContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::smartiesSummaryChanged,
            this,
            &SmartiesFeature::slotUpdateSmartiesLabels);
}

QVariant SmartiesFeature::title() {
    return tr("Smarties");
}

QString SmartiesFeature::formatRootViewHtml() const {
    QString smartiesTitle = tr("Smarties");
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

std::unique_ptr<TreeItem> SmartiesFeature::newTreeItemForSmartiesSummary(
        const SmartiesSummary& smartiesSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForSmartiesSummary(pTreeItem.get(), smartiesSummary);
    return pTreeItem;
}

void SmartiesFeature::updateTreeItemForSmartiesSummary(
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
    // next line is disabled to let the new treeitem creation format the
    // displayname in grouped-logic
    pTreeItem->setLabel(formatLabel(smartiesSummary));
    pTreeItem->setIcon(smartiesSummary.isLocked() ? m_lockedSmartiesIcon : QIcon());
}

void SmartiesFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &SmartiesFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void SmartiesFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* SmartiesFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void SmartiesFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    m_lastRightClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

void SmartiesFeature::activateChild(const QModelIndex& index) {
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [ACTIVATE CHILD] -> index " << index;
    }
    SmartiesId smartiesId(smartiesIdFromIndex(index));
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

bool SmartiesFeature::activateSmarties(SmartiesId smartiesId) {
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

bool SmartiesFeature::readLastRightClickedSmarties(Smarties* pSmarties) const {
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

bool SmartiesFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void SmartiesFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateSmartiesAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditSmartiesAction.get());
    menu.addSeparator();
    menu.exec(globalPos);
}

void SmartiesFeature::onRightClickChild(
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

void SmartiesFeature::slotCreateSmartiesFromSearch(const QString& text) {
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

void SmartiesFeature::slotCreateSmartiesFromUI() {
    SmartiesId smartiesId =
            SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySmartiesFromUI();
    if (smartiesId.isValid()) {
        // expand Smarties and scroll to new smarties
        m_pSidebarWidget->selectChildIndex(indexFromSmartiesId(smartiesId), false);
    }
}

void SmartiesFeature::slotCreateSmarties() {
    SmartiesId smartiesId =
            SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptySmarties();
    if (smartiesId.isValid()) {
        // expand Smarties and scroll to new smarties
        m_pSidebarWidget->selectChildIndex(indexFromSmartiesId(smartiesId), false);
    }
}

void SmartiesFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteSmarties();
}

void SmartiesFeature::slotDeleteSmarties() {
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

void SmartiesFeature::slotFindPreviousSmarties() {
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

void SmartiesFeature::slotFindNextSmarties() {
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

void SmartiesFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameSmarties();
}

void SmartiesFeature::slotRenameSmarties() {
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

void SmartiesFeature::slotDuplicateSmarties() {
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

void SmartiesFeature::SetActiveSmartiesToLastRightClicked(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
}

void SmartiesFeature::selectSmartiesForEdit(SmartiesId selectedSmartiesId) {
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [REBUILD CHILD MODEL] -> "
                    "selectedSmartiesId "
                 << selectedSmartiesId;
    }
    //    return selectedSmartiesId;
}

void SmartiesFeature::slotEditSmarties() {
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
        dlgSmartiesInfo infoDialog(
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

void SmartiesFeature::slotToggleSmartiesLock() {
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

QModelIndex SmartiesFeature::rebuildChildModel(SmartiesId selectedSmartiesId) {
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [REBUILDCHILDMODEL] -> "
                    "selectedSmartiesId "
                 << selectedSmartiesId;
    }

    m_lastRightClickedIndex = QModelIndex();

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    std::vector<std::unique_ptr<TreeItem>> modelRows;
    modelRows.reserve(m_pTrackCollection->smarties().countSmarties());

    int selectedRow = -1;
    SmartiesSummarySelectResult smartiesSummaries(
            m_pTrackCollection->smarties().selectSmartiesSummaries());
    SmartiesSummary smartiesSummary;
    while (smartiesSummaries.populateNext(&smartiesSummary)) {
        modelRows.push_back(newTreeItemForSmartiesSummary(smartiesSummary));
        if (selectedSmartiesId == smartiesSummary.getId()) {
            // save index for selection
            selectedRow = static_cast<int>(modelRows.size()) - 1;
        }
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);

    // Update rendering of smarties depending on the currently selected track
    slotTrackSelected(m_selectedTrackId);

    if (selectedRow >= 0) {
        return m_pSidebarModel->index(selectedRow, 0);
    } else {
        return QModelIndex();
    }
}

void SmartiesFeature::updateChildModel(const QSet<SmartiesId>& updatedSmartiesIds) {
    const SmartiesStorage& smartiesStorage = m_pTrackCollection->smarties();
    for (const SmartiesId& smartiesId : updatedSmartiesIds) {
        QModelIndex index = indexFromSmartiesId(smartiesId);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        SmartiesSummary smartiesSummary;
        VERIFY_OR_DEBUG_ASSERT(
                smartiesStorage.readSmartiesSummaryById(smartiesId, &smartiesSummary)) {
            continue;
        }
        updateTreeItemForSmartiesSummary(
                m_pSidebarModel->getItem(index), smartiesSummary);
        m_pSidebarModel->triggerRepaint(index);
    }

    if (m_selectedTrackId.isValid()) {
        // Smarties containing the currently selected track might
        // have been modified.
        slotTrackSelected(m_selectedTrackId);
    }
}

SmartiesId SmartiesFeature::smartiesIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return SmartiesId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return SmartiesId();
    }
    return SmartiesId(item->getData());
}

QModelIndex SmartiesFeature::indexFromSmartiesId(SmartiesId smartiesId) const {
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

void SmartiesFeature::slotAnalyzeSmarties() {
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

void SmartiesFeature::storePrevSiblingSmartiesId(SmartiesId smartiesId) {
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

void SmartiesFeature::storeNextSiblingSmartiesId(SmartiesId smartiesId) {
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

void SmartiesFeature::slotSmartiesTableChanged(SmartiesId smartiesId) {
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

void SmartiesFeature::slotSmartiesContentChanged(SmartiesId smartiesId) {
    QSet<SmartiesId> updatedSmartiesIds;
    updatedSmartiesIds.insert(smartiesId);
    updateChildModel(updatedSmartiesIds);
}

void SmartiesFeature::slotUpdateSmartiesLabels(const QSet<SmartiesId>& updatedSmartiesIds) {
    updateChildModel(updatedSmartiesIds);
}

void SmartiesFeature::htmlLinkClicked(const QUrl& link) {
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

void SmartiesFeature::slotTrackSelected(TrackId trackId) {
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

void SmartiesFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}
