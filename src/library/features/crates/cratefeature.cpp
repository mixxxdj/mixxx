#include "library/features/crates/cratefeature.h"
#include "library/features/crates/cratemanager.h"
#include "library/features/crates/cratestorage.h"
#include "library/features/crates/cratetracks.h"
#include "library/features/crates/cratehierarchy.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QDesktopServices>

#include <algorithm>
#include <vector>

#include "controllers/keyboard/keyboardeventfilter.h"
#include "library/features/crates/cratefeaturehelper.h"
#include "library/features/crates/cratetablemodel.h"
#include "library/export/trackexportwizard.h"
#include "library/library.h"
#include "library/parser.h"
#include "library/parserm3u.h"
#include "library/parserpls.h"
#include "library/parsercsv.h"
#include "library/trackcollection.h"
#include "util/dnd.h"
#include "util/duration.h"
#include "util/time.h"
#include "widget/wlibrarystack.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

QString formatLabel(
        const CrateSummary& crateSummary) {
    return QString("%1 (%2) %3").arg(
        crateSummary.getName(),
        QString::number(crateSummary.getTrackCount()),
        crateSummary.getTrackDurationText());
}

} // anonymous namespace

CrateFeature::CrateFeature(UserSettingsPointer pConfig,
                           Library* pLibrary,
                           QObject* parent,
                           TrackCollection* pTrackCollection)
        : LibraryFeature(pConfig, pLibrary, pTrackCollection, parent),
          m_cratesIcon(":/images/library/ic_library_crates.png"),
          m_lockedCrateIcon(":/images/library/ic_library_locked.png"),
          m_pTrackCollection(pTrackCollection),
          m_pCrates(pTrackCollection->crates()),
          m_pCrateTableModel(nullptr) {


    initActions();

    // if closure does not have the same number of crates as the crates table
    // this means that the user just started mixxx with nested crates for the
    // first time, so we have to fill the closure table with (self,self,0)
    m_pCrates->checkClosure();

    m_pCrates->setRecursionEnabled();

    m_pChildModel = std::make_unique<CrateTreeModel>(this, m_pCrates);
    m_pChildModel->reloadTree();
    generateSummaries();

    connectLibrary(pLibrary);
    connectCrateManager();
}

CrateFeature::~CrateFeature() {
}

void CrateFeature::initActions() {
    m_pCreateCrateAction = std::make_unique<QAction>(tr("Create New Crate"), this);
    connect(m_pCreateCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotCreateCrate()));

    m_pCreateChildCrateAction = std::make_unique<QAction>(tr("Create Subcrate"), this);
    connect(m_pCreateChildCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotCreateChildCrate()));

    m_pDeleteCrateAction = std::make_unique<QAction>(tr("Remove"), this);
    connect(m_pDeleteCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotDeleteCrate()));

    m_pRenameCrateAction = std::make_unique<QAction>(tr("Rename"), this);
    connect(m_pRenameCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotRenameCrate()));

    m_pLockCrateAction = std::make_unique<QAction>(tr("Lock"), this);
    connect(m_pLockCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotToggleCrateLock()));

    m_pImportPlaylistAction = std::make_unique<QAction>(tr("Import Crate"), this);
    connect(m_pImportPlaylistAction.get(), SIGNAL(triggered()),
            this, SLOT(slotImportPlaylist()));

    m_pCreateImportPlaylistAction = std::make_unique<QAction>(tr("Import Crate"), this);
    connect(m_pCreateImportPlaylistAction.get(), SIGNAL(triggered()),
            this, SLOT(slotCreateImportCrate()));

    m_pExportPlaylistAction = std::make_unique<QAction>(tr("Export Crate"), this);
    connect(m_pExportPlaylistAction.get(), SIGNAL(triggered()),
            this, SLOT(slotExportPlaylist()));

    m_pExportTrackFilesAction = std::make_unique<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction.get(), SIGNAL(triggered()),
            this, SLOT(slotExportTrackFiles()));

    m_pDuplicateCrateAction = std::make_unique<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotDuplicateCrate()));

    m_pAnalyzeCrateAction = std::make_unique<QAction>(tr("Analyze entire Crate"), this);
    connect(m_pAnalyzeCrateAction.get(), SIGNAL(triggered()),
            this, SLOT(slotAnalyzeCrate()));

    m_pAutoDjTrackSourceAction = std::make_unique<QAction>(tr("Auto DJ Track Source"), this);
    m_pAutoDjTrackSourceAction->setCheckable(true);
    connect(m_pAutoDjTrackSourceAction.get(), SIGNAL(changed()),
            this, SLOT(slotAutoDjTrackSourceChanged()));

    m_pMoveCrateMenu = std::make_unique<QMenu>(nullptr);
    m_pMoveCrateMenu->setTitle(tr("Move subtree to"));
}

void CrateFeature::connectLibrary(Library* pLibrary) {
    connect(pLibrary, SIGNAL(trackSelected(TrackPointer)),
            this, SLOT(slotTrackSelected(TrackPointer)));
}

void CrateFeature::connectCrateManager() {
    connect(m_pCrates, SIGNAL(crateInserted(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pCrates, SIGNAL(crateUpdated(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pCrates, SIGNAL(crateDeleted(CrateId)),
            this, SLOT(slotCrateTableChanged(CrateId)));
    connect(m_pCrates, SIGNAL(crateTracksChanged(CrateId, QList<TrackId>, QList<TrackId>)),
            this, SLOT(slotCrateContentChanged(CrateId)));
    connect(m_pCrates, SIGNAL(crateSummaryChanged(QSet<CrateId>)),
            this, SLOT(slotUpdateCrateLabels(QSet<CrateId>)));
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

} // anonymus namespace

bool CrateFeature::dragMoveAccept(QUrl url) {
    return SoundSourceProxy::isUrlSupported(url) ||
                Parser::isPlaylistFilenameSupported(url.toLocalFile());
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
    m_pCrates->addTracksToCrate(crateId, trackIds);
    return true;
}

bool CrateFeature::dragMoveAcceptChild(const QModelIndex& index, QUrl url) {
    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return false;
    }
    Crate crate;
    if (!m_pCrates->storage().readCrateById(crateId, &crate) || crate.isLocked()) {
        return false;
    }
    return SoundSourceProxy::isUrlSupported(url) ||
        Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

parented_ptr<QWidget> CrateFeature::createPaneWidget(KeyboardEventFilter* pKeyboard,
            int paneId, QWidget* parent) {
    auto pContainer = make_parented<WLibraryStack>(parent);
    m_panes[paneId] = pContainer.toWeakRef();

    auto pEdit = make_parented<WLibraryTextBrowser>(pContainer.get());
    pEdit->setHtml(formatRootViewHtml());
    pEdit->setOpenLinks(false);
    pEdit->installEventFilter(pKeyboard);
    connect(pEdit.get(), SIGNAL(anchorClicked(const QUrl)),
            this, SLOT(htmlLinkClicked(const QUrl)));

    m_idBrowse[paneId] = pContainer->addWidget(pEdit.get());

    auto pTable = LibraryFeature::createPaneWidget(pKeyboard, paneId,
                                                   pContainer.get());
    m_idTable[paneId] = pContainer->addWidget(pTable.get());

    return pContainer;
}

QPointer<TreeItemModel> CrateFeature::getChildModel() {
    return m_pChildModel.get();
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
    if (index.data(AbstractRole::RoleData).toString() == CrateTreeModel::RECURSION_DATA) {
        toggleRecursion();
        return;
    }
    CrateId crateId(crateIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        return;
    }

    m_pCrateTableModel = getTableModel(m_featurePane);
    Crate crate;
    m_pCrates->storage().readCrateById(crateId, &crate);
    m_pCrateTableModel->selectCrate(crate);
    showTable(m_featurePane);
    //    restoreSearch(QString("crate: \"%1\"").arg(m_pCrates->hierarchy().getNamePathFromId(crate.getId())));
    restoreSearch(QString(""));
    showBreadCrumb(index);
    showTrackModel(m_pCrateTableModel);
}

void CrateFeature::invalidateChild() {
    m_lastClickedIndex.clear();
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
    // TODO(XXX): How to select the newly created crate without
    // a corresponding table model? m_pCrateTableModel = nullptr
    // when creating crates by clicking the link on the HTML view.
    // if (m_pCrateTableModel) {
    //     Crate crate;
    //     m_pCrates->storage().readCrateById(crateId, &crate);
    //     m_pCrateTableModel->selectCrate(crate);
    //     emit(showTrackModel(m_pCrateTableModel));
    // }
    // emit(enableCoverArtDisplay(true));
    // // Update selection
    // emit(featureSelect(this, m_lastRightClickedIndex));
    activateChild(m_lastRightClickedIndex);
    return true;
}

bool CrateFeature::readLastRightClickedCrate(Crate* pCrate) const {
    CrateId crateId(crateIdFromIndex(m_lastRightClickedIndex));
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        qWarning() << "Failed to determine id of selected crate";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(m_pCrates->storage().readCrateById(crateId, pCrate)) {
        qWarning() << "Failed to read selected crate with id" << crateId;
        return false;
    }
    return true;
}

void CrateFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(nullptr);
    menu.addAction(m_pCreateCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction.get());
    menu.exec(globalPos);
}

void CrateFeature::onRightClickChild(const QPoint& globalPos, const QModelIndex& index) {
    //Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;

    CrateId crateId(crateIdFromIndex(index));
    if (!crateId.isValid()) {
        return;
    }

    Crate crate;
    if (!m_pCrates->storage().readCrateById(crateId, &crate)) {
        return;
    }

    m_pMoveCrateMenu->clear();
    QSignalMapper* signalMapper = new QSignalMapper(this);
    // add option to move to root if the crate isn't there already
    if (m_pCrates->hierarchy().getParentId(crate.getId()).isValid()) {
        auto pAction = std::make_unique<QAction>("Root", this);
        signalMapper->setMapping(pAction.get(), CrateId().value());
        connect(pAction.get(), SIGNAL(triggered()),
                signalMapper, SLOT(map()));
        m_pMoveCrateMenu->addAction(pAction.get());
        pAction.release();
    }

    CrateSelectResult allCrates(m_pCrates->storage().selectCrates());
    Crate moveCrate;
    while (allCrates.populateNext(&moveCrate)) {
        if (moveCrate.getId() == crate.getId() ||
            m_pCrates->hierarchy().collectChildCrateIds(crate.getId()).contains(moveCrate.getId().toString()) ||
            moveCrate.getId() == m_pCrates->hierarchy().getParentId(crate.getId())) {
            continue; // skip adding the selected crate, it's parent and it's children
        }
        QString namePath = m_pCrates->hierarchy().getNamePathFromId(moveCrate.getId());
        auto pAction = std::make_unique<QAction>(namePath.right(namePath.size()-1), this);
        pAction->setEnabled(!moveCrate.isLocked());
        signalMapper->setMapping(pAction.get(), moveCrate.getId().value());
        connect(pAction.get(), SIGNAL(triggered()),
                signalMapper, SLOT(map()));
        m_pMoveCrateMenu->addAction(pAction.get());
        pAction.release();
    }
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(slotMoveSubtreeToCrate(int)));

    m_pDeleteCrateAction->setEnabled(!crate.isLocked());
    m_pRenameCrateAction->setEnabled(!crate.isLocked());

    m_pAutoDjTrackSourceAction->setChecked(crate.isAutoDjSource());

    m_pLockCrateAction->setText(crate.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(NULL);
    menu.addAction(m_pCreateCrateAction.get());
    menu.addAction(m_pCreateChildCrateAction.get());
    menu.addSeparator();
    menu.addAction(m_pRenameCrateAction.get());
    menu.addAction(m_pDuplicateCrateAction.get());
    menu.addAction(m_pDeleteCrateAction.get());
    menu.addAction(m_pLockCrateAction.get());
    menu.addMenu(m_pMoveCrateMenu.get());
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
    CrateId newCrateId = CrateFeatureHelper(
      m_pCrates, m_pConfig).createEmptyCrate();
    if (newCrateId.isValid()) {
        rebuildChildModel(newCrateId);
    }
}

void CrateFeature::slotCreateChildCrate() {
    Crate parent;
    if (readLastRightClickedCrate(&parent)) {
        CrateId newCrateId = CrateFeatureHelper(
          m_pCrates, m_pConfig).createEmptyCrate(parent);
        if (newCrateId.isValid()) {
            rebuildChildModel(newCrateId);
        }
    }
}

void CrateFeature::slotDeleteCrate() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        if (crate.isLocked()) {
            qWarning() << "Refusing to delete locked crate" << crate;
            return;
        }
        // better deletion requeried
        if (m_pCrates->hierarchy().hasChildren(crate.getId())) {
            if (QMessageBox::question(
                  nullptr,
                  tr("Deleting multiple crates"),
                  tr("Deleting this crate will delete all its subcrates too. Are you sure you want to delete this crate?"),
                  QMessageBox::Ok,
                  QMessageBox::Cancel) == QMessageBox::Cancel) {
                return;
            }
        }

        CrateId parentId = m_pCrates->hierarchy().getParentId(crate.getId());
        if (m_pCrates->deleteCrate(crate)) {
            qDebug() << "Deleted crate" << crate;
            rebuildChildModel(parentId);
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
            crate.setName(crate.getName().simplified());
            if (!crate.hasName()) {
                QMessageBox::warning(
                        nullptr,
                        tr("Renaming Crate Failed"),
                        tr("A crate cannot have a blank name."));
                continue;
            }
            CrateId parentId(m_pCrates->hierarchy().getParentId(crate.getId()));
            Crate parent;
            m_pCrates->storage().readCrateById(parentId, &parent);

            if (!m_pCrates->hierarchy().isNameValidForHierarchy(crate.getName(), crate, parent)) {
                QMessageBox::warning(
                        nullptr,
                        tr("Renaming Crate Failed"),
                        tr("A crate by that name already exists."));
                crate.resetName();
                continue;
            }
        }

        if (!m_pCrates->updateCrate(crate)) {
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
                m_pCrates, m_pConfig).duplicateCrate(crate);
        if (crateId.isValid()) {
            rebuildChildModel(crateId);
        }
    } else {
        qDebug() << "Failed to duplicate selected crate";
    }
}

void CrateFeature::slotMoveSubtreeToCrate(int iCrateId) {
    Crate selectedCrate;
    CrateId destinationCrateId(iCrateId);

    if (readLastRightClickedCrate(&selectedCrate)) {
        if (selectedCrate.getId() == destinationCrateId) {
            return;
        }

        QStringList childIds = m_pCrates->hierarchy().collectChildCrateIds(selectedCrate.getId());
        if (childIds.contains(destinationCrateId.toString())) {
            QMessageBox::warning(
              nullptr,
              tr("Moving Subtree Failed"),
              tr("Cannot move a subree into itself."));
            return;
        }

        Crate destinationCrate;
        m_pCrates->storage().readCrateById(destinationCrateId, &destinationCrate);

        m_pCrates->moveCrate(selectedCrate, destinationCrate);

        rebuildChildModel();
    }
}


void CrateFeature::slotToggleCrateLock() {
    Crate crate;
    if (readLastRightClickedCrate(&crate)) {
        crate.setLocked(!crate.isLocked());
        if (!m_pCrates->updateCrate(crate)) {
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
            m_pCrates->updateCrate(crate);
        }
    }
}

void CrateFeature::toggleRecursion() {
    if (m_pCrates->isRecursionEnabled()) {
        m_pCrates->setRecursionEnabled(false);
    } else {
        m_pCrates->setRecursionEnabled();
    }
    rebuildChildModel();
}

//TODO(gramanas): don't reset the tree in every change
void CrateFeature::rebuildChildModel(CrateId selectedCrateId) {
    qDebug() << "CrateFeature::rebuildChildModel()" << selectedCrateId;
    m_pChildModel->reloadTree();
    generateSummaries();

    if (selectedCrateId.isValid()) {
        QModelIndex index = indexFromCrateId(selectedCrateId);
        if (index.isValid()) {
            activateChild(index);
            return;
        }
    }
    activate();

    // TODO(XXX): This is not optimal at all.
    // The tree item model needs to get revamped in order to allow
    // the addition of rows one by one. QAbstactItemModel allows this
    // but our implementation of it so far did not need that feature
    // so it was not implemented
}

void CrateFeature::generateSummaries() {
    CrateSelectResult crates(m_pCrates->storage().selectCrates());
    Crate crate;
    QSet<CrateId> ids;
    while (crates.populateNext(&crate)) {
        ids.insert(crate.getId());
    }
    updateChildModel(ids);
}

void CrateFeature::updateChildModel(const QSet<CrateId>& updatedCrateIds) {
    for (const CrateId& crateId: updatedCrateIds) {
        QModelIndex index = indexFromCrateId(crateId);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        CrateSummary crateSummary;
        VERIFY_OR_DEBUG_ASSERT(
          m_pCrates->storage().readCrateSummaryById(crateId, &crateSummary)) {
            continue;
        }
        updateTreeItemForCrateSummary(m_pChildModel->getItem(index), crateSummary);
        m_pChildModel->triggerRepaint(index);
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
    bool ok = false;
    int id = index.data(AbstractRole::RoleData).toInt(&ok);
    if (ok) {
        return CrateId(id);
    }
    return CrateId();
}

QModelIndex CrateFeature::indexFromCrateId(CrateId crateId) const {
    VERIFY_OR_DEBUG_ASSERT(crateId.isValid()) {
        qDebug() << "CrateId is invalid:" << crateId;
        return QModelIndex();
    }

    QModelIndexList indexList = m_pChildModel->match(m_pChildModel->index(0, 0),
                                                     AbstractRole::RoleData,
                                                     crateId.toVariant(), 1,
                                                     Qt::MatchRecursive);
    if (!indexList.isEmpty()) {
        return indexList.back();
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

void CrateFeature::slotImportPlaylistFile(const QString &playlist_file) {
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
    m_pCrateTableModel->addTracks(QModelIndex(), entries);
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
                if (!m_pCrates->storage().readCrateByName(crate.getName())) {
                    // unused crate name found
                    break; // terminate loop
                }
            }
        }

        if (!m_pCrates->insertCrate(crate, &lastCrateId)) {
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
                    m_pCrates->tracks().countCrateTracks(crateId));
            {
                CrateTrackSelectResult crateTracks(
                        m_pCrates->tracks().selectCrateTracksSorted(crateId));
                while (crateTracks.next()) {
                    trackIds.append(crateTracks.trackId());
                }
            }
            emit(analyzeTracks(trackIds));
        }
    }
}

void CrateFeature::slotExportPlaylist() {
    CrateId crateId = m_pCrateTableModel->selectedCrate();
    Crate crate;
    if (m_pCrates->storage().readCrateById(crateId, &crate)) {
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
    Crate selectedCrate;
    m_pCrates->storage().readCrateById(m_pCrateTableModel->selectedCrate(), &selectedCrate);
    pCrateTableModel->selectCrate(selectedCrate);
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
    Crate crate;
    m_pCrates->storage().readCrateById(m_pCrateTableModel->selectedCrate(), &crate);
    pCrateTableModel->selectCrate(crate);
    pCrateTableModel->select();

    int rows = pCrateTableModel->rowCount();
    QList<TrackPointer> trackpointers;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = m_pCrateTableModel->index(i, 0);
        trackpointers.push_back(m_pCrateTableModel->getTrack(index));
    }

    TrackExportWizard track_export(nullptr, m_pConfig, trackpointers);
    track_export.exportTracks();
}

void CrateFeature::slotCrateTableChanged(CrateId crateId) {
    Q_UNUSED(crateId);
    rebuildChildModel(crateId);
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

    TreeItem* pRootItem = m_pChildModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    TrackId selectedTrackId;
    std::vector<CrateId> sortedTrackCrates;
    if (m_pSelectedTrack) {
        selectedTrackId = m_pSelectedTrack->getId();
        CrateTrackSelectResult trackCratesIter(
                m_pCrates->tracks().selectTrackCratesSorted(selectedTrackId));
        while (trackCratesIter.next()) {
            sortedTrackCrates.push_back(trackCratesIter.crateId());
        }
    }

    // Set all crates the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem: pRootItem->children()) {
        updateTreeItemForTrackSelection(pTreeItem, selectedTrackId, sortedTrackCrates);
    }

    m_pChildModel->triggerRepaint();
}

void CrateFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackPointer());
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
