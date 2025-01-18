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
// #include "library/trackset/smarties/dlgsmartiesinfo.h"

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
            //            &SmartiesFeature::slotEditSmarties);
            &SmartiesFeature::slotCreateSmarties);
    m_pEditSmartiesAction = make_parented<QAction>(tr("Edit Smarties"), this);
    // m_pEditSmartiesAction->setShortcut(kEditSidebarItemShortcutKey);
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

    //    m_pAutoDjTrackSourceAction = make_parented<QAction>(tr("Auto DJ Track Source"), this);
    //    m_pAutoDjTrackSourceAction->setCheckable(true);
    //    connect(m_pAutoDjTrackSourceAction.get(),
    //            &QAction::changed,
    //            this,
    //            &SmartiesFeature::slotAutoDjTrackSourceChanged);

    m_pAnalyzeSmartiesAction = make_parented<QAction>(tr("Analyze entire Smarties"), this);
    connect(m_pAnalyzeSmartiesAction.get(),
            &QAction::triggered,
            this,
            &SmartiesFeature::slotAnalyzeSmarties);

    //    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Smarties"), this);
    //    connect(m_pImportPlaylistAction.get(),
    //            &QAction::triggered,
    //            this,
    //            &SmartiesFeature::slotImportPlaylist);
    //    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Smarties"), this);
    //    connect(m_pCreateImportPlaylistAction.get(),
    //            &QAction::triggered,
    //            this,
    //            &SmartiesFeature::slotCreateImportSmarties);
    //    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Smarties as Playlist"), this);
    //    connect(m_pExportPlaylistAction.get(),
    //            &QAction::triggered,
    //            this,
    //            &SmartiesFeature::slotExportPlaylist);
    //    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    //    connect(m_pExportTrackFilesAction.get(),
    //            &QAction::triggered,
    //            this,
    //            &SmartiesFeature::slotExportTrackFiles);
    // #ifdef __ENGINEPRIME__
    //    m_pExportAllSmartiesAction = make_parented<QAction>(tr("Export to Engine Prime"), this);
    //    connect(m_pExportAllSmartiesAction.get(),
    //            &QAction::triggered,
    //            this,
    //            &SmartiesFeature::exportAllSmarties);
    //    m_pExportSmartiesAction = make_parented<QAction>(tr("Export to Engine Prime"), this);
    //    connect(m_pExportSmartiesAction.get(),
    //            &QAction::triggered,
    //            this,
    //            [this]() {
    //                SmartiesId smartiesId = smartiesIdFromIndex(m_lastRightClickedIndex);
    //                if (smartiesId.isValid()) {
    //                    emit exportSmarties(smartiesId);
    //                }
    //            });
    // #endif
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
    // next line disabled to let the new treeitem creation format the displayname
    // pTreeItem->setLabel(formatLabel(smartiesSummary));
    pTreeItem->setIcon(smartiesSummary.isLocked() ? m_lockedSmartiesIcon : QIcon());
}

// bool SmartiesFeature::dropAcceptChild(
//         const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
//     SmartiesId smartiesId(smartiesIdFromIndex(index));
//     VERIFY_OR_DEBUG_ASSERT(smartiesId.isValid()) {
//         return false;
//     }
//  If a track is dropped onto a smarties's name, but the track isn't in the
//  library, then add the track to the library before adding it to the
//  playlist.
//  pSource != nullptr it is a drop from inside Mixxx and indicates all
//  tracks already in the DB
//    QList<TrackId> trackIds =
//            m_pLibrary->trackCollectionManager()->resolveTrackIdsFromUrls(urls, !pSource);
//    if (trackIds.isEmpty()) {
//        return false;
//    }

//    m_pTrackCollection->addSmartiesTracks(smartiesId, trackIds);
//    return true;
//}

// bool SmartiesFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
//     SmartiesId smartiesId(smartiesIdFromIndex(index));
//     if (!smartiesId.isValid()) {
//         return false;
//     }
//     Smarties smarties;
//     if (!m_pTrackCollection->smarties().readSmartiesById(smartiesId, &smarties) ||
//             smarties.isLocked()) {
//         return false;
//     }
//     return SoundSourceProxy::isUrlSupported(url) ||
//             Parser::isPlaylistFilenameSupported(url.toLocalFile());
// }

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

void SmartiesFeature::oldactivateChild(const QModelIndex& index) {
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
    //    menu.addAction(m_pCreateImportPlaylistAction.get());
    // #ifdef __ENGINEPRIME__
    //    menu.addSeparator();
    //    menu.addAction(m_pExportAllSmartiesAction.get());
    // #endif
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

    //    m_pAutoDjTrackSourceAction->setChecked(smarties.isAutoDjSource());

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
    //    menu.addAction(m_pAutoDjTrackSourceAction.get());
    menu.addSeparator();
    menu.addAction(m_pAnalyzeSmartiesAction.get());
    menu.addSeparator();
    //    if (!smarties.isLocked()) {
    //        menu.addAction(m_pImportPlaylistAction.get());
    //    }
    //    menu.addAction(m_pExportPlaylistAction.get());
    //    menu.addAction(m_pExportTrackFilesAction.get());
    // #ifdef __ENGINEPRIME__
    //    menu.addAction(m_pExportSmartiesAction.get());
    // #endif
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
        //        m_pUpdateDuplicateSmarties
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

// bool SmartiesFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
//     SmartiesId smartiesId(smartiesIdFromIndex(index));
//     if (!smartiesId.isValid()) {
//         return false;
//     }
//     Smarties smarties;
//     if (!m_pTrackCollection->smarties().readSmartiesById(smartiesId, &smarties) ||
//             smarties.isLocked()) {
//         return false;
//     }

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
        dlgSmartiesInfo infoDialog(this); // Pass this to provide the SmartiesFeature instance
        if (sDebug) {
            qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [START] -> "
                        "[INIT DIALOG] -> INIT DIALOG ";
        }
        infoDialog.init(smartiesData, playlistsCratesData);

        // Connect the dataUpdated signal to update smartiesData when Apply is clicked
        connect(&infoDialog,
                &dlgSmartiesInfo::dataUpdated,
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
                    emit updateSmartiesData(smartiesData);
                });

        connect(&infoDialog, &dlgSmartiesInfo::requestDeleteSmarties, this, [this, &smartiesId]() {
            //            emit setBlockerOff("delete");
            if (sDebug) {
                qDebug()
                        << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [DELETE] "
                           "-> START Request DELETE Smarties smartiesId "
                        << smartiesId;
            }
            m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
            if (!smartiesId.isValid()) {
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[DELETE] -> Invalid SmartiesId. : "
                             << smartiesId;
                }
                return;
            } else {
                slotFindPreviousSmarties();
                auto smartiesJustAbove = m_prevSiblingSmarties;
                slotDeleteSmarties();
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[DELETE] -> smarties is deleted -> new active "
                                "smarties.";
                }
                activateSmarties(smartiesJustAbove);
                smartiesId = smartiesJustAbove;
                smartiesData.clear();
                m_smartiesTableModel.selectSmarties2QVL(smartiesId, smartiesData);
                m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                activateSmarties(smartiesId);
                //                emit setBlockerOff("delete");
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[DELETE] -> END DELETE Smarties smartiesId "
                             << smartiesId;
                }
                emit updateSmartiesData(smartiesData);
            }
        });

        connect(&infoDialog, &dlgSmartiesInfo::requestNewSmarties, this, [this, &smartiesId]() {
            //            emit setBlockerOff("new");
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [NEW] "
                            "-> START Request NEW Smarties smartiesId "
                         << smartiesId;
            }
            SmartiesId smartiesId =
                    SmartiesFeatureHelper(m_pTrackCollection, m_pConfig)
                            .createEmptySmartiesFromUI();
            if (!smartiesId.isValid()) {
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[NEW] -> Creation failed.";
                }
                return;
            } else {
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[NEW] -> New smarties created. smartiesId "
                             << smartiesId;
                }
            }
            activateSmarties(smartiesId);
            smartiesData.clear();
            m_smartiesTableModel.selectSmarties2QVL(smartiesId, smartiesData);
            slotSmartiesTableChanged(smartiesId);
            m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [NEW] "
                            "-> END Smarties created smartiesId "
                         << smartiesId;
            }
            emit updateSmartiesData(smartiesData);
            //            emit setBlockerOff("new");
        });

        connect(&infoDialog,
                &dlgSmartiesInfo::requestPreviousSmarties,
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
        connect(&infoDialog, &dlgSmartiesInfo::requestNextSmarties, this, [this, &smartiesId]() {
            //            emit setBlockerOff("next");
            if (sDebug) {
                qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> [NEXT] "
                            "-> START Request NEXT Smarties smartiesId "
                         << smartiesId;
            }
            m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
            if (!smartiesId.isValid()) {
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
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
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[NEXT] -> Next smarties ID"
                             << m_nextSiblingSmarties;
                }
                smartiesData.clear();
                m_smartiesTableModel.selectSmarties2QVL(smartiesId, smartiesData);
                m_lastRightClickedIndex = indexFromSmartiesId(smartiesId);
                if (sDebug) {
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
                                "[NEXT] -> Sibling Smarties loaded smartiesId."
                             << smartiesId;
                    //                emit setBlockerOff("next");
                    qDebug() << "[SMARTIESFEATURE] [SLOT EDIT SMARTIES] -> "
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

// void SmartiesFeature::slotAutoDjTrackSourceChanged() {
//     Smarties smarties;
//     if (readLastRightClickedSmarties(&smarties)) {
//         if (smarties.isAutoDjSource() != m_pAutoDjTrackSourceAction->isChecked()) {
//             smarties.setAutoDjSource(m_pAutoDjTrackSourceAction->isChecked());
//             m_pTrackCollection->updateSmarties(smarties);
//         }
//     }
// }

QModelIndex SmartiesFeature::oldrebuildChildModel(SmartiesId selectedSmartiesId) {
    if (sDebug) {
        qDebug() << "[SMARTIESFEATURE] [REBUILD CHILD MODEL] -> "
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

void SmartiesFeature::oldupdateChildModel(const QSet<SmartiesId>& updatedSmartiesIds) {
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

// void SmartiesFeature::slotImportPlaylist() {
//     // qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();
//
//     QString playlistFile = getPlaylistFile();
//     if (playlistFile.isEmpty()) {
//         return;
//     }

//    // Update the import/export smarties directory
//    QString fileDirectory(playlistFile);
//    fileDirectory.truncate(playlistFile.lastIndexOf("/"));
//    m_pConfig->set(kConfigKeyLastImportExportSmartiesDirectoryKey,
//            ConfigValue(fileDirectory));

//    SmartiesId smartiesId = smartiesIdFromIndex(m_lastRightClickedIndex);
//    Smarties smarties;
//    if (m_pTrackCollection->smarties().readSmartiesById(smartiesId, &smarties)) {
//        qDebug() << "Importing playlist file" << playlistFile << "into smarties"
//                 << smartiesId << smarties;
//    } else {
//        qDebug() << "Importing playlist file" << playlistFile << "into smarties"
//                 << smartiesId << smarties << "failed!";
//        return;
//    }

//    slotImportPlaylistFile(playlistFile, smartiesId);
//    activateChild(m_lastRightClickedIndex);
//}

// void SmartiesFeature::slotImportPlaylistFile(const QString& playlistFile,
// SmartiesId smartiesId) {
//  The user has picked a new directory via a file dialog. This means the
//  system sandboxer (if we are sandboxed) has granted us permission to this
//  folder. We don't need access to this file on a regular basis so we do not
//  register a security bookmark.
//  TODO(XXX): Parsing a list of track locations from a playlist file
//  is a general task and should be implemented separately.
//    QList<QString> locations = Parser().parse(playlistFile);
//    if (locations.empty()) {
//        return;
//    }

//    if (smartiesId == m_smartiesTableModel.selectedSmarties()) {
// Add tracks directly to the model
//        m_smartiesTableModel.addTracks(QModelIndex(), locations);
//    } else {
// Create a temporary table model since the main one might have another
// smarties selected which is not the smarties that received the right-click.
//        std::unique_ptr<SmartiesTableModel> pSmartiesTableModel =
//                std::make_unique<SmartiesTableModel>(this, m_pLibrary->trackCollectionManager());
//        pSmartiesTableModel->selectSmarties(smartiesId);
//        pSmartiesTableModel->select();
//        pSmartiesTableModel->addTracks(QModelIndex(), locations);
//    }
//}

// void SmartiesFeature::slotCreateImportSmarties() {
//  Get file to read
//    QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
//    if (playlistFiles.isEmpty()) {
//        return;
//    }

// Set last import directory
//    QString fileDirectory(playlistFiles.first());
//    fileDirectory.truncate(playlistFiles.first().lastIndexOf("/"));
//    m_pConfig->set(kConfigKeyLastImportExportSmartiesDirectoryKey,
//            ConfigValue(fileDirectory));

//    SmartiesId lastSmartiesId;

// For each selected file create a new smarties
//    for (const QString& playlistFile : playlistFiles) {
//        const QFileInfo fileInfo(playlistFile);

//        Smarties smarties;

// Get a valid name
//        const QString baseName = fileInfo.baseName();
//        for (int i = 0;; ++i) {
//            auto name = baseName;
//            if (i > 0) {
//                name += QStringLiteral(" %1").arg(i);
//            }
//            name = name.trimmed();
//            if (!name.isEmpty()) {
//                if (!m_pTrackCollection->smarties().readSmartiesByName(name)) {
// unused smarties name found
//                    smarties.setName(std::move(name));
//                    DEBUG_ASSERT(smarties.hasName());
//                    break; // terminate loop
//                }
//            }
//        }

//        if (!m_pTrackCollection->insertSmarties(smarties, &lastSmartiesId)) {
//            QMessageBox::warning(nullptr,
//                    tr("Smarties Creation Failed"),
//                    tr("An unknown error occurred while creating smarties: ") +
//                            smarties.getName());
//            return;
//        }

//        slotImportPlaylistFile(playlistFile, lastSmartiesId);
//    }
//    activateSmarties(lastSmartiesId);
//}

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

// void SmartiesFeature::slotExportPlaylist() {
//     SmartiesId smartiesId = smartiesIdFromIndex(m_lastRightClickedIndex);
//     Smarties smarties;
//     if (m_pTrackCollection->smarties().readSmartiesById(smartiesId, &smarties)) {
//         qDebug() << "Exporting smarties" << smartiesId << smarties;
//     } else {
//         qDebug() << "Failed to export smarties" << smartiesId;
//         return;
//     }

//    QString lastSmartiesDirectory = m_pConfig->getValue(
//            kConfigKeyLastImportExportSmartiesDirectoryKey,
//            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

// Open a dialog to let the user choose the file location for smarties export.
// The location is set to the last used directory for import/export and the file
// name to the playlist name.
//    const QString fileLocation = getFilePathWithVerifiedExtensionFromFileDialog(
//            tr("Export Smarties"),
//            lastSmartiesDirectory.append("/").append(smarties.getName()),
//            tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;PLS Playlist "
//               "(*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"),
//            tr("M3U Playlist (*.m3u)"));
// Exit method if user cancelled the open dialog.
//    if (fileLocation.isEmpty()) {
//        return;
//    }
// Update the import/export smarties directory
//    QString fileDirectory(fileLocation);
//    fileDirectory.truncate(fileLocation.lastIndexOf("/"));
//    m_pConfig->set(kConfigKeyLastImportExportSmartiesDirectoryKey,
//            ConfigValue(fileDirectory));

// The user has picked a new directory via a file dialog. This means the
// system sandboxer (if we are sandboxed) has granted us permission to this
// folder. We don't need access to this file on a regular basis so we do not
// register a security bookmark.

// check config if relative paths are desired
//   bool useRelativePath =
//           m_pConfig->getValue<bool>(
//                    kUseRelativePathOnExportConfigKey);

// Create list of files of the smarties
// Create a new table model since the main one might have an active search.
//    std::unique_ptr<SmartiesTableModel> pSmartiesTableModel =
//            std::make_unique<SmartiesTableModel>(this, m_pLibrary->trackCollectionManager());
//    pSmartiesTableModel->selectSmarties(smartiesId);
//    pSmartiesTableModel->select();

//    if (fileLocation.endsWith(".csv", Qt::CaseInsensitive)) {
//        ParserCsv::writeCSVFile(fileLocation, pSmartiesTableModel.get(), useRelativePath);
//    } else if (fileLocation.endsWith(".txt", Qt::CaseInsensitive)) {
//        ParserCsv::writeReadableTextFile(fileLocation, pSmartiesTableModel.get(), false);
//    } else {
// populate a list of files of the smarties
//        QList<QString> playlistItems;
//        int rows = pSmartiesTableModel->rowCount();
//        for (int i = 0; i < rows; ++i) {
//            QModelIndex index = pSmartiesTableModel->index(i, 0);
//            playlistItems << pSmartiesTableModel->getTrackLocation(index);
//        }
//        exportPlaylistItemsIntoFile(
//                fileLocation,
//                playlistItems,
//                useRelativePath);
//    }
//}

// void SmartiesFeature::slotExportTrackFiles() {
//     SmartiesId smartiesId(smartiesIdFromIndex(m_lastRightClickedIndex));
//     if (!smartiesId.isValid()) {
//         return;
//     }
//  Create a new table model since the main one might have an active search.
//    std::unique_ptr<SmartiesTableModel> pSmartiesTableModel =
//            std::make_unique<SmartiesTableModel>(this, m_pLibrary->trackCollectionManager());
//    pSmartiesTableModel->selectSmarties(smartiesId);
//    pSmartiesTableModel->select();

//    int rows = pSmartiesTableModel->rowCount();
//    TrackPointerList trackpointers;
//    for (int i = 0; i < rows; ++i) {
//        QModelIndex index = pSmartiesTableModel->index(i, 0);
//        auto pTrack = pSmartiesTableModel->getTrack(index);
//        VERIFY_OR_DEBUG_ASSERT(pTrack != nullptr) {
//            continue;
//        }
//        trackpointers.push_back(pTrack);
//    }

//    if (trackpointers.isEmpty()) {
//        return;
//    }

//    TrackExportWizard track_export(nullptr, m_pConfig, trackpointers);
//    track_export.exportTracks();
//}

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

QModelIndex SmartiesFeature::rebuildChildModel(SmartiesId selectedSmartiesId) {
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

void SmartiesFeature::updateChildModel(const QSet<SmartiesId>& updatedSmartiesIds) {
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

    //    const QString& delimiter = m_pConfig->getValue(ConfigKey("[Library]",
    //    "GroupedSmartiesVarLengthMask"));

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
            /////////            // const QString& groupName =
            ///(*updatedGroup)["group_name"].toString();
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

void SmartiesFeature::activateChild(const QModelIndex& index) {
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

QString SmartiesFeature::fullPathFromIndex(const QModelIndex& index) const {
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

QString SmartiesFeature::groupNameFromIndex(const QModelIndex& index) const {
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

void SmartiesFeature::updateFullPathRecursive(TreeItem* pItem, const QString& parentPath) {
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
