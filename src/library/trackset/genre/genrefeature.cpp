#include "library/trackset/genre/genrefeature.h"

#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QStandardPaths>
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
#include "library/trackset/genre/genrefeaturehelper.h"
#include "library/trackset/genre/genresummary.h"
#include "library/treeitem.h"
#include "moc_genrefeature.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/defs.h"
#include "util/file.h"
#include "widget/wlibrary.h"
#include "widget/wlibrarysidebar.h"
#include "widget/wlibrarytextbrowser.h"

namespace {

QString formatLabel(
        const GenreSummary& genreSummary) {
    return QStringLiteral("%1 (%2) %3")
            .arg(
                    genreSummary.getName(),
                    QString::number(genreSummary.getTrackCount()),
                    genreSummary.getTrackDurationText());
}

const ConfigKey kConfigKeyLastImportExportGenreDirectoryKey(
        "[Library]", "LastImportExportGenreDirectory");

} // anonymous namespace

using namespace mixxx::library::prefs;

GenreFeature::GenreFeature(Library* pLibrary,
        UserSettingsPointer pConfig)
        : BaseTrackSetFeature(pLibrary, pConfig, "GENREHOME", QStringLiteral("genres")),
          m_lockedGenreIcon(":/images/library/ic_library_locked_tracklist.svg"),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_genreTableModel(this, pLibrary->trackCollectionManager()) {
    initActions();

    // construct child model
    m_pSidebarModel->setRootItem(TreeItem::newRoot(this));
    rebuildChildModel();

    connectLibrary(pLibrary);
    connectTrackCollection();
}

void GenreFeature::initActions() {
    // m_genreTableModel.rebuildCustomNames();
    m_pImportGenreModelFromCsvAction =
            make_parented<QAction>(tr("Import Genre-model from CSV"), this);
    connect(m_pImportGenreModelFromCsvAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotImportGenreModelFromCsv);
    m_pEditGenreAction =
            make_parented<QAction>(tr("Edit Genre"), this);
    connect(m_pEditGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotEditGenre);
    m_pEditGenreMultiAction =
            make_parented<QAction>(tr("Edit Multiple Genres"), this);
    connect(m_pEditGenreMultiAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotEditGenreMulti);
    m_pSetAllGenresVisibleAction =
            make_parented<QAction>(tr("Make all Genres Visible"), this);
    connect(m_pSetAllGenresVisibleAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotSetAllGenresVisible);
    m_pMakeGenreInVisible =
            make_parented<QAction>(tr("Make Genre InVisible"), this);
    connect(m_pMakeGenreInVisible.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotMakeGenreInVisible);
    m_pEditOrphanGenresAction =
            make_parented<QAction>(tr("Edit orphan track-genres"), this);
    connect(m_pEditOrphanGenresAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotEditOrphanTrackGenres);
    m_pExportGenresToCsv =
            make_parented<QAction>(tr("Export all genres to csv"), this);
    connect(m_pExportGenresToCsv.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotExportGenresToCsv);
    m_pImportGenresFromCsv =
            make_parented<QAction>(tr("Import genres from csv"), this);
    connect(m_pImportGenresFromCsv.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotImportGenresFromCsv);
    m_pCreateGenreAction = make_parented<QAction>(tr("Create New Genre"), this);
    connect(m_pCreateGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotCreateGenre);
    m_pRenameGenreAction = make_parented<QAction>(tr("Rename"), this);
    connect(m_pRenameGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotRenameGenre);
    m_pDuplicateGenreAction = make_parented<QAction>(tr("Duplicate"), this);
    connect(m_pDuplicateGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotDuplicateGenre);
    m_pDeleteGenreAction = make_parented<QAction>(tr("Remove"), this);
    const auto removeKeySequence =
            // TODO(XXX): Qt6 replace enum | with QKeyCombination
            QKeySequence(static_cast<int>(kHideRemoveShortcutModifier) |
                    kHideRemoveShortcutKey);
    m_pDeleteGenreAction->setShortcut(removeKeySequence);
    connect(m_pDeleteGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotDeleteGenre);
    m_pLockGenreAction = make_parented<QAction>(tr("Lock"), this);
    connect(m_pLockGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotToggleGenreLock);

    m_pAutoDjTrackSourceAction = make_parented<QAction>(tr("Auto DJ Track Source"), this);
    m_pAutoDjTrackSourceAction->setCheckable(true);
    connect(m_pAutoDjTrackSourceAction.get(),
            &QAction::changed,
            this,
            &GenreFeature::slotAutoDjTrackSourceChanged);

    m_pAnalyzeGenreAction = make_parented<QAction>(tr("Analyze entire Genre"), this);
    connect(m_pAnalyzeGenreAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotAnalyzeGenre);

    m_pImportPlaylistAction = make_parented<QAction>(tr("Import Genre"), this);
    connect(m_pImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotImportPlaylist);
    m_pCreateImportPlaylistAction = make_parented<QAction>(tr("Import Genre"), this);
    connect(m_pCreateImportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotCreateImportGenre);
    m_pExportPlaylistAction = make_parented<QAction>(tr("Export Genre as Playlist"), this);
    connect(m_pExportPlaylistAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotExportPlaylist);
    m_pExportTrackFilesAction = make_parented<QAction>(tr("Export Track Files"), this);
    connect(m_pExportTrackFilesAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::slotExportTrackFiles);
#ifdef __ENGINEPRIME__
    m_pExportAllGenresAction = make_parented<QAction>(tr("Export to Engine DJ"), this);
    connect(m_pExportAllGenresAction.get(),
            &QAction::triggered,
            this,
            &GenreFeature::exportAllGenres);
    m_pExportGenreAction = make_parented<QAction>(tr("Export to Engine DJ"), this);
    connect(m_pExportGenreAction.get(),
            &QAction::triggered,
            this,
            [this]() {
                GenreId genreId = genreIdFromIndex(m_lastRightClickedIndex);
                if (genreId.isValid()) {
                    emit exportGenre(genreId);
                }
            });
#endif
}

void GenreFeature::connectLibrary(Library* pLibrary) {
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
            &GenreFeature::slotResetSelectedTrack);
}

void GenreFeature::connectTrackCollection() {
    connect(m_pTrackCollection, // created new, duplicated or imported playlist to new genre
            &TrackCollection::genreInserted,
            this,
            &GenreFeature::slotGenreTableChanged);
    connect(m_pTrackCollection, // renamed, un/locked, toggled AutoDJ source
            &TrackCollection::genreUpdated,
            this,
            &GenreFeature::slotGenreTableChanged);
    connect(m_pTrackCollection,
            &TrackCollection::genreDeleted,
            this,
            &GenreFeature::slotGenreTableChanged);
    connect(m_pTrackCollection, // genre tracks hidden, unhidden or purged
            &TrackCollection::genreTracksChanged,
            this,
            &GenreFeature::slotGenreContentChanged);
    connect(m_pTrackCollection,
            &TrackCollection::genreSummaryChanged,
            this,
            &GenreFeature::slotUpdateGenreLabels);
}

QVariant GenreFeature::title() {
    return tr("Genres");
}

QString GenreFeature::formatRootViewHtml() const {
    QString genresTitle = tr("Genres");
    QString genresSummary =
            tr("Genres are a great way to help organize the music you want to "
               "DJ with.");
    QString genresSummary2 =
            tr("Make a genre for your next gig, for your favorite electrohouse "
               "tracks, or for your most requested tracks.");
    QString genresSummary3 =
            tr("Genres let you organize your music however you'd like!");

    QString html;
    QString createGenreLink = tr("Create New Genre");
    html.append(QStringLiteral("<h2>%1</h2>").arg(genresTitle));
    html.append(QStringLiteral("<p>%1</p>").arg(genresSummary));
    html.append(QStringLiteral("<p>%1</p>").arg(genresSummary2));
    html.append(QStringLiteral("<p>%1</p>").arg(genresSummary3));
    // Colorize links in lighter blue, instead of QT default dark blue.
    // Links are still different from regular text, but readable on dark/light backgrounds.
    // https://github.com/mixxxdj/mixxx/issues/9103
    html.append(
            QStringLiteral("<a style=\"color:#0496FF;\" href=\"create\">%1</a>")
                    .arg(createGenreLink));
    return html;
}

std::unique_ptr<TreeItem> GenreFeature::newTreeItemForGenreSummary(
        const GenreSummary& genreSummary) {
    auto pTreeItem = TreeItem::newRoot(this);
    updateTreeItemForGenreSummary(pTreeItem.get(), genreSummary);
    return pTreeItem;
}

void GenreFeature::updateTreeItemForGenreSummary(
        TreeItem* pTreeItem, const GenreSummary& genreSummary) const {
    DEBUG_ASSERT(pTreeItem != nullptr);
    if (pTreeItem->getData().isNull()) {
        // Initialize a newly created tree item
        pTreeItem->setData(genreSummary.getId().toVariant());
    } else {
        // The data (= GenreId) is immutable once it has been set
        DEBUG_ASSERT(GenreId(pTreeItem->getData()) == genreSummary.getId());
    }
    // Update mutable properties
    pTreeItem->setLabel(formatLabel(genreSummary));
    pTreeItem->setIcon(genreSummary.isLocked() ? m_lockedGenreIcon : QIcon());
}

bool GenreFeature::dropAcceptChild(
        const QModelIndex& index, const QList<QUrl>& urls, QObject* pSource) {
    GenreId genreId(genreIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(genreId.isValid()) {
        return false;
    }
    // If a track is dropped onto a genre's name, but the track isn't in the
    // library, then add the track to the library before adding it to the
    // playlist.
    // pSource != nullptr it is a drop from inside Mixxx and indicates all
    // tracks already in the DB
    QList<TrackId> trackIds =
            m_pLibrary->trackCollectionManager()->resolveTrackIdsFromUrls(urls, !pSource);
    if (trackIds.isEmpty()) {
        return false;
    }

    m_pTrackCollection->addGenreTracks(genreId, trackIds);
    return true;
}

bool GenreFeature::dragMoveAcceptChild(const QModelIndex& index, const QUrl& url) {
    GenreId genreId(genreIdFromIndex(index));
    if (!genreId.isValid()) {
        return false;
    }
    Genre genre;
    if (!m_pTrackCollection->genres().readGenreById(genreId, &genre) ||
            genre.isLocked()) {
        return false;
    }
    return SoundSourceProxy::isUrlSupported(url) ||
            Parser::isPlaylistFilenameSupported(url.toLocalFile());
}

void GenreFeature::bindLibraryWidget(
        WLibrary* libraryWidget, KeyboardEventFilter* keyboard) {
    Q_UNUSED(keyboard);
    WLibraryTextBrowser* edit = new WLibraryTextBrowser(libraryWidget);
    edit->setHtml(formatRootViewHtml());
    edit->setOpenLinks(false);
    connect(edit,
            &WLibraryTextBrowser::anchorClicked,
            this,
            &GenreFeature::htmlLinkClicked);
    libraryWidget->registerView(m_rootViewName, edit);
}

void GenreFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClickChild
    m_pSidebarWidget = pSidebarWidget;
}

TreeItemModel* GenreFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void GenreFeature::activate() {
    m_lastClickedIndex = QModelIndex();
    BaseTrackSetFeature::activate();
}

void GenreFeature::activateChild(const QModelIndex& index) {
    qDebug() << "   GenreFeature::activateChild()" << index;
    GenreId genreId(genreIdFromIndex(index));
    VERIFY_OR_DEBUG_ASSERT(genreId.isValid()) {
        return;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    m_prevSiblingGenre = GenreId();
    emit saveModelState();
    m_genreTableModel.selectGenre(genreId);
    emit showTrackModel(&m_genreTableModel);
    emit enableCoverArtDisplay(true);
}

bool GenreFeature::activateGenre(GenreId genreId) {
    qDebug() << "GenreFeature::activateGenre()" << genreId;
    VERIFY_OR_DEBUG_ASSERT(genreId.isValid()) {
        return false;
    }
    if (!m_pTrackCollection->genres().readGenreSummaryById(genreId)) {
        // this may happen if called by slotGenreTableChanged()
        // and the genre has just been deleted
        return false;
    }
    QModelIndex index = indexFromGenreId(genreId);
    VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
        return false;
    }
    m_lastClickedIndex = index;
    m_lastRightClickedIndex = QModelIndex();
    m_prevSiblingGenre = GenreId();
    emit saveModelState();
    m_genreTableModel.selectGenre(genreId);
    emit showTrackModel(&m_genreTableModel);
    emit enableCoverArtDisplay(true);
    // Update selection
    emit featureSelect(this, m_lastClickedIndex);
    return true;
}

bool GenreFeature::readLastRightClickedGenre(Genre* pGenre) const {
    GenreId genreId(genreIdFromIndex(m_lastRightClickedIndex));
    VERIFY_OR_DEBUG_ASSERT(genreId.isValid()) {
        qWarning() << "Failed to determine id of selected genre";
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(
            m_pTrackCollection->genres().readGenreById(genreId, pGenre)) {
        qWarning() << "Failed to read selected genre with id" << genreId;
        return false;
    }
    return true;
}

bool GenreFeature::isChildIndexSelectedInSidebar(const QModelIndex& index) {
    return m_pSidebarWidget && m_pSidebarWidget->isChildIndexSelected(index);
}

void GenreFeature::onRightClick(const QPoint& globalPos) {
    m_lastRightClickedIndex = QModelIndex();
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateGenreAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditGenreMultiAction.get());
    menu.addAction(m_pSetAllGenresVisibleAction.get());
    menu.addAction(m_pEditOrphanGenresAction.get());
    menu.addSeparator();
    menu.addAction(m_pExportGenresToCsv.get());
    menu.addAction(m_pImportGenresFromCsv.get());
    menu.addAction(m_pImportGenreModelFromCsvAction.get());
    menu.addSeparator();
    menu.addAction(m_pCreateImportPlaylistAction.get());
#ifdef __ENGINEPRIME__
    menu.addSeparator();
    menu.addAction(m_pExportAllGenresAction.get());
#endif
    menu.exec(globalPos);
}

void GenreFeature::onRightClickChild(
        const QPoint& globalPos, const QModelIndex& index) {
    // Save the model index so we can get it in the action slots...
    m_lastRightClickedIndex = index;
    GenreId genreId(genreIdFromIndex(index));
    if (!genreId.isValid()) {
        return;
    }

    Genre genre;
    if (!m_pTrackCollection->genres().readGenreById(genreId, &genre)) {
        return;
    }

    m_pDeleteGenreAction->setEnabled(!genre.isLocked());
    m_pRenameGenreAction->setEnabled(!genre.isLocked());

    m_pAutoDjTrackSourceAction->setChecked(genre.isAutoDjSource());

    m_pLockGenreAction->setText(genre.isLocked() ? tr("Unlock") : tr("Lock"));

    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pCreateGenreAction.get());
    menu.addSeparator();
    menu.addAction(m_pEditGenreAction.get());
    menu.addSeparator();
    menu.addAction(m_pMakeGenreInVisible.get());
    menu.addSeparator();
    menu.addAction(m_pRenameGenreAction.get());
    menu.addAction(m_pDuplicateGenreAction.get());
    menu.addAction(m_pDeleteGenreAction.get());
    menu.addAction(m_pLockGenreAction.get());
    menu.addSeparator();
    menu.addAction(m_pAutoDjTrackSourceAction.get());
    menu.addSeparator();
    menu.addAction(m_pAnalyzeGenreAction.get());
    menu.addSeparator();
    if (!genre.isLocked()) {
        menu.addAction(m_pImportPlaylistAction.get());
    }
    menu.addAction(m_pExportPlaylistAction.get());
    menu.addAction(m_pExportTrackFilesAction.get());
#ifdef __ENGINEPRIME__
    menu.addAction(m_pExportGenreAction.get());
#endif
    menu.exec(globalPos);
}

void GenreFeature::slotCreateGenre() {
    GenreId genreId =
            GenreFeatureHelper(m_pTrackCollection, m_pConfig)
                    .createEmptyGenre();
    if (genreId.isValid()) {
        // expand Genres and scroll to new genre
        m_pSidebarWidget->selectChildIndex(indexFromGenreId(genreId), false);
    }
}

void GenreFeature::slotEditGenre() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        qDebug() << "[GenreFeature] -> slotEditGenre() -> genre" << genre;
        qDebug() << "[GenreFeature] -> slotEditGenre() -> genre.getId() " << genre.getId();
        m_genreTableModel.editGenre(genre.getId());
        rebuildChildModel();
    }
}

void GenreFeature::slotMakeGenreInVisible() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        qDebug() << "[GenreFeature] -> slotMakeGenreInVisible() -> genre" << genre;
        qDebug() << "[GenreFeature] -> slotMakeGenreInVisible() -> genre.getId() " << genre.getId();
        m_genreTableModel.setGenreInvisible(genre.getId());
        rebuildChildModel();
    }
}

void GenreFeature::slotEditOrphanTrackGenres() {
    qDebug() << "[GenreFeature] -> slotEditOrphanTrackGenres()";
    m_genreTableModel.EditOrphanTrackGenres();
    rebuildChildModel();
}

void GenreFeature::slotEditGenreMulti() {
    qDebug() << "[GenreFeature] -> slotEditGenreMulti()";
    m_genreTableModel.EditGenresMulti();
    rebuildChildModel();
}

void GenreFeature::slotSetAllGenresVisible() {
    qDebug() << "[GenreFeature] -> slotAllGenresVisible()";
    m_genreTableModel.setAllGenresVisible();
    rebuildChildModel();
}

void GenreFeature::slotImportGenreModelFromCsv() {
    qDebug() << "[GenreFeature] -> slotImportGenreModelFromCsv()";
    m_genreTableModel.importModelFromCsv();
    rebuildChildModel();
}

void GenreFeature::slotExportGenresToCsv() {
    qDebug() << "[GenreFeature] -> slotExportGenresToCsv()";
    m_genreTableModel.exportGenresToCsv();
    rebuildChildModel();
}

void GenreFeature::slotImportGenresFromCsv() {
    qDebug() << "[GenreFeature] -> slotImportGenresFromCsv()";
    m_genreTableModel.importGenresFromCsv();
    rebuildChildModel();
}

void GenreFeature::deleteItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotDeleteGenre();
}

void GenreFeature::slotDeleteGenre() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        if (genre.isLocked()) {
            qWarning() << "Refusing to delete locked genre" << genre;
            return;
        }
        GenreId genreId = genre.getId();
        // Store sibling id to restore selection after genre was deleted
        // to avoid the scroll position being reset to Genre root item.
        m_prevSiblingGenre = GenreId();
        if (isChildIndexSelectedInSidebar(m_lastRightClickedIndex)) {
            storePrevSiblingGenreId(genreId);
        }

        QMessageBox::StandardButton btn = QMessageBox::question(nullptr,
                tr("Confirm Deletion"),
                tr("Do you really want to delete genre <b>%1</b>?")
                        .arg(genre.getName()),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);
        if (btn == QMessageBox::Yes) {
            if (m_pTrackCollection->deleteGenre(genreId)) {
                qDebug() << "Deleted genre" << genre;
                return;
            }
        } else {
            return;
        }
    }
    qWarning() << "Failed to delete selected genre";
}

void GenreFeature::renameItem(const QModelIndex& index) {
    m_lastRightClickedIndex = index;
    slotRenameGenre();
}

void GenreFeature::slotRenameGenre() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        const QString oldName = genre.getName();
        genre.resetName();
        for (;;) {
            bool ok = false;
            auto newName =
                    QInputDialog::getText(nullptr,
                            tr("Rename Genre"),
                            tr("Enter new name for genre:"),
                            QLineEdit::Normal,
                            oldName,
                            &ok)
                            .trimmed();
            if (!ok || newName.isEmpty()) {
                return;
            }
            if (newName.isEmpty()) {
                QMessageBox::warning(nullptr,
                        tr("Renaming Genre Failed"),
                        tr("A genre cannot have a blank name."));
                continue;
            }
            if (m_pTrackCollection->genres().readGenreByName(newName)) {
                QMessageBox::warning(nullptr,
                        tr("Renaming Genre Failed"),
                        tr("A genre by that name already exists."));
                continue;
            }
            genre.setName(std::move(newName));
            DEBUG_ASSERT(genre.hasName());
            break;
        }

        if (!m_pTrackCollection->updateGenre(genre)) {
            qDebug() << "Failed to rename genre" << genre;
        }
    } else {
        qDebug() << "Failed to rename selected genre";
    }
}

void GenreFeature::slotDuplicateGenre() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        GenreId newGenreId =
                GenreFeatureHelper(m_pTrackCollection, m_pConfig)
                        .duplicateGenre(genre);
        if (newGenreId.isValid()) {
            qDebug() << "Duplicate genre" << genre << ", new genre:" << newGenreId;
            return;
        }
    }
    qDebug() << "Failed to duplicate selected genre";
}

void GenreFeature::slotToggleGenreLock() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        genre.setLocked(!genre.isLocked());
        if (!m_pTrackCollection->updateGenre(genre)) {
            qDebug() << "Failed to toggle lock of genre" << genre;
        }
    } else {
        qDebug() << "Failed to toggle lock of selected genre";
    }
}

void GenreFeature::slotAutoDjTrackSourceChanged() {
    Genre genre;
    if (readLastRightClickedGenre(&genre)) {
        if (genre.isAutoDjSource() != m_pAutoDjTrackSourceAction->isChecked()) {
            genre.setAutoDjSource(m_pAutoDjTrackSourceAction->isChecked());
            m_pTrackCollection->updateGenre(genre);
        }
    }
}

QModelIndex GenreFeature::rebuildChildModel(GenreId selectedGenreId) {
    qDebug() << "GenreFeature::rebuildChildModel()" << selectedGenreId;

    m_lastRightClickedIndex = QModelIndex();

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return QModelIndex();
    }
    m_pSidebarModel->removeRows(0, pRootItem->childRows());

    std::vector<std::unique_ptr<TreeItem>> modelRows;
    modelRows.reserve(m_pTrackCollection->genres().countGenres());

    int selectedRow = -1;
    GenreSummarySelectResult genreSummaries(
            m_pTrackCollection->genres().selectGenreSummaries());
    GenreSummary genreSummary;
    while (genreSummaries.populateNext(&genreSummary)) {
        modelRows.push_back(newTreeItemForGenreSummary(genreSummary));
        if (selectedGenreId == genreSummary.getId()) {
            // save index for selection
            selectedRow = static_cast<int>(modelRows.size()) - 1;
        }
    }

    // Append all the newly created TreeItems in a dynamic way to the childmodel
    m_pSidebarModel->insertTreeItemRows(std::move(modelRows), 0);

    // Update rendering of genres depending on the currently selected track
    slotTrackSelected(m_selectedTrackId);

    if (selectedRow >= 0) {
        return m_pSidebarModel->index(selectedRow, 0);
    } else {
        return QModelIndex();
    }
}

void GenreFeature::updateChildModel(const QSet<GenreId>& updatedGenreIds) {
    const GenreStorage& genreStorage = m_pTrackCollection->genres();
    for (const GenreId& genreId : updatedGenreIds) {
        QModelIndex index = indexFromGenreId(genreId);
        VERIFY_OR_DEBUG_ASSERT(index.isValid()) {
            continue;
        }
        GenreSummary genreSummary;
        VERIFY_OR_DEBUG_ASSERT(
                genreStorage.readGenreSummaryById(genreId, &genreSummary)) {
            continue;
        }
        updateTreeItemForGenreSummary(
                m_pSidebarModel->getItem(index), genreSummary);
        m_pSidebarModel->triggerRepaint(index);
    }

    if (m_selectedTrackId.isValid()) {
        // Genres containing the currently selected track might
        // have been modified.
        slotTrackSelected(m_selectedTrackId);
    }
}

GenreId GenreFeature::genreIdFromIndex(const QModelIndex& index) const {
    if (!index.isValid()) {
        return GenreId();
    }
    TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
    if (item == nullptr) {
        return GenreId();
    }
    return GenreId(item->getData());
}

QModelIndex GenreFeature::indexFromGenreId(GenreId genreId) const {
    VERIFY_OR_DEBUG_ASSERT(genreId.isValid()) {
        return QModelIndex();
    }
    for (int row = 0; row < m_pSidebarModel->rowCount(); ++row) {
        QModelIndex index = m_pSidebarModel->index(row, 0);
        TreeItem* pTreeItem = m_pSidebarModel->getItem(index);
        DEBUG_ASSERT(pTreeItem != nullptr);
        if (!pTreeItem->hasChildren() && // leaf node
                (GenreId(pTreeItem->getData()) == genreId)) {
            return index;
        }
    }
    qDebug() << "Tree item for genre not found:" << genreId;
    return QModelIndex();
}

void GenreFeature::slotImportPlaylist() {
    // qDebug() << "slotImportPlaylist() row:" ; //<< m_lastRightClickedIndex.data();

    QString playlistFile = getPlaylistFile();
    if (playlistFile.isEmpty()) {
        return;
    }

    // Update the import/export genre directory
    QFileInfo fileDirectory(playlistFile);
    m_pConfig->set(kConfigKeyLastImportExportGenreDirectoryKey,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    GenreId genreId = genreIdFromIndex(m_lastRightClickedIndex);
    Genre genre;
    if (m_pTrackCollection->genres().readGenreById(genreId, &genre)) {
        qDebug() << "Importing playlist file" << playlistFile << "into genre"
                 << genreId << genre;
    } else {
        qDebug() << "Importing playlist file" << playlistFile << "into genre"
                 << genreId << genre << "failed!";
        return;
    }

    slotImportPlaylistFile(playlistFile, genreId);
    activateChild(m_lastRightClickedIndex);
}

void GenreFeature::slotImportPlaylistFile(const QString& playlistFile, GenreId genreId) {
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

    if (genreId == m_genreTableModel.selectedGenre()) {
        // Add tracks directly to the model
        m_genreTableModel.addTracks(QModelIndex(), locations);
    } else {
        // Create a temporary table model since the main one might have another
        // genre selected which is not the genre that received the right-click.
        std::unique_ptr<GenreTableModel> pGenreTableModel =
                std::make_unique<GenreTableModel>(this, m_pLibrary->trackCollectionManager());
        pGenreTableModel->selectGenre(genreId);
        pGenreTableModel->select();
        pGenreTableModel->addTracks(QModelIndex(), locations);
    }
}

void GenreFeature::slotCreateImportGenre() {
    // Get file to read
    const QStringList playlistFiles = LibraryFeature::getPlaylistFiles();
    if (playlistFiles.isEmpty()) {
        return;
    }

    // Set last import directory
    QFileInfo fileDirectory(playlistFiles.first());
    m_pConfig->set(kConfigKeyLastImportExportGenreDirectoryKey,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    GenreId lastGenreId;

    // For each selected file create a new genre
    for (const QString& playlistFile : playlistFiles) {
        const QFileInfo fileInfo(playlistFile);

        Genre genre;

        // Get a valid name
        const QString baseName = fileInfo.completeBaseName();
        for (int i = 0;; ++i) {
            auto name = baseName;
            if (i > 0) {
                name += QStringLiteral(" %1").arg(i);
            }
            name = name.trimmed();
            if (!name.isEmpty()) {
                if (!m_pTrackCollection->genres().readGenreByName(name)) {
                    // unused genre name found
                    genre.setName(std::move(name));
                    DEBUG_ASSERT(genre.hasName());
                    break; // terminate loop
                }
            }
        }

        if (!m_pTrackCollection->insertGenre(genre, &lastGenreId)) {
            QMessageBox::warning(nullptr,
                    tr("Genre Creation Failed"),
                    tr("An unknown error occurred while creating genre: ") +
                            genre.getName());
            return;
        }

        slotImportPlaylistFile(playlistFile, lastGenreId);
    }
    activateGenre(lastGenreId);
}

void GenreFeature::slotAnalyzeGenre() {
    if (m_lastRightClickedIndex.isValid()) {
        GenreId genreId = genreIdFromIndex(m_lastRightClickedIndex);
        if (genreId.isValid()) {
            QList<AnalyzerScheduledTrack> tracks;
            tracks.reserve(
                    m_pTrackCollection->genres().countGenreTracks(genreId));
            {
                GenreTrackSelectResult genreTracks(
                        m_pTrackCollection->genres().selectGenreTracksSorted(
                                genreId));
                while (genreTracks.next()) {
                    tracks.append(genreTracks.trackId());
                }
            }
            emit analyzeTracks(tracks);
        }
    }
}

void GenreFeature::slotExportPlaylist() {
    GenreId genreId = genreIdFromIndex(m_lastRightClickedIndex);
    Genre genre;
    if (m_pTrackCollection->genres().readGenreById(genreId, &genre)) {
        qDebug() << "Exporting genre" << genreId << genre;
    } else {
        qDebug() << "Failed to export genre" << genreId;
        return;
    }

    QString lastGenreDirectory = m_pConfig->getValue(
            kConfigKeyLastImportExportGenreDirectoryKey,
            QStandardPaths::writableLocation(QStandardPaths::MusicLocation));

    // Open a dialog to let the user choose the file location for genre export.
    // The location is set to the last used directory for import/export and the file
    // name to the playlist name.
    const QString fileLocation = getFilePathWithVerifiedExtensionFromFileDialog(
            tr("Export Genre"),
            lastGenreDirectory.append("/").append(genre.getName()),
            tr("M3U Playlist (*.m3u);;M3U8 Playlist (*.m3u8);;PLS Playlist "
               "(*.pls);;Text CSV (*.csv);;Readable Text (*.txt)"),
            tr("M3U Playlist (*.m3u)"));
    // Exit method if user cancelled the open dialog.
    if (fileLocation.isEmpty()) {
        return;
    }
    // Update the import/export genre directory
    QFileInfo fileDirectory(fileLocation);
    m_pConfig->set(kConfigKeyLastImportExportGenreDirectoryKey,
            ConfigValue(fileDirectory.absoluteDir().canonicalPath()));

    // The user has picked a new directory via a file dialog. This means the
    // system sandboxer (if we are sandboxed) has granted us permission to this
    // folder. We don't need access to this file on a regular basis so we do not
    // register a security bookmark.

    // check config if relative paths are desired
    bool useRelativePath =
            m_pConfig->getValue<bool>(
                    kUseRelativePathOnExportConfigKey);

    // Create list of files of the genre
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<GenreTableModel> pGenreTableModel =
            std::make_unique<GenreTableModel>(this, m_pLibrary->trackCollectionManager());
    pGenreTableModel->selectGenre(genreId);
    pGenreTableModel->select();

    if (fileLocation.endsWith(".csv", Qt::CaseInsensitive)) {
        ParserCsv::writeCSVFile(fileLocation, pGenreTableModel.get(), useRelativePath);
    } else if (fileLocation.endsWith(".txt", Qt::CaseInsensitive)) {
        ParserCsv::writeReadableTextFile(fileLocation, pGenreTableModel.get(), false);
    } else {
        // populate a list of files of the genre
        QList<QString> playlistItems;
        int rows = pGenreTableModel->rowCount();
        for (int i = 0; i < rows; ++i) {
            QModelIndex index = pGenreTableModel->index(i, 0);
            playlistItems << pGenreTableModel->getTrackLocation(index);
        }
        exportPlaylistItemsIntoFile(
                fileLocation,
                playlistItems,
                useRelativePath);
    }
}

void GenreFeature::slotExportTrackFiles() {
    GenreId genreId(genreIdFromIndex(m_lastRightClickedIndex));
    if (!genreId.isValid()) {
        return;
    }
    // Create a new table model since the main one might have an active search.
    std::unique_ptr<GenreTableModel> pGenreTableModel =
            std::make_unique<GenreTableModel>(this, m_pLibrary->trackCollectionManager());
    pGenreTableModel->selectGenre(genreId);
    pGenreTableModel->select();

    int rows = pGenreTableModel->rowCount();
    TrackPointerList trackpointers;
    for (int i = 0; i < rows; ++i) {
        QModelIndex index = pGenreTableModel->index(i, 0);
        auto pTrack = pGenreTableModel->getTrack(index);
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

void GenreFeature::storePrevSiblingGenreId(GenreId genreId) {
    QModelIndex actIndex = indexFromGenreId(genreId);
    m_prevSiblingGenre = GenreId();
    for (int i = (actIndex.row() + 1); i >= (actIndex.row() - 1); i -= 2) {
        QModelIndex newIndex = actIndex.sibling(i, actIndex.column());
        if (newIndex.isValid()) {
            TreeItem* pTreeItem = m_pSidebarModel->getItem(newIndex);
            DEBUG_ASSERT(pTreeItem != nullptr);
            if (!pTreeItem->hasChildren()) {
                m_prevSiblingGenre = genreIdFromIndex(newIndex);
            }
        }
    }
}

void GenreFeature::slotGenreTableChanged(GenreId genreId) {
    Q_UNUSED(genreId);
    if (isChildIndexSelectedInSidebar(m_lastClickedIndex)) {
        // If the previously selected genre was loaded to the tracks table and
        // selected in the sidebar try to activate that or a sibling
        rebuildChildModel();
        if (!activateGenre(m_genreTableModel.selectedGenre())) {
            // probably last clicked genre was deleted, try to
            // select the stored sibling
            if (m_prevSiblingGenre.isValid()) {
                activateGenre(m_prevSiblingGenre);
            }
        }
    } else {
        // No valid selection to restore
        rebuildChildModel();
    }
}

void GenreFeature::slotGenreContentChanged(GenreId genreId) {
    QSet<GenreId> updatedGenreIds;
    updatedGenreIds.insert(genreId);
    updateChildModel(updatedGenreIds);
}

void GenreFeature::slotUpdateGenreLabels(const QSet<GenreId>& updatedGenreIds) {
    updateChildModel(updatedGenreIds);
}

void GenreFeature::htmlLinkClicked(const QUrl& link) {
    if (QString(link.path()) == "create") {
        slotCreateGenre();
    } else {
        qDebug() << "Unknown genre link clicked" << link;
    }
}

void GenreFeature::slotTrackSelected(TrackId trackId) {
    m_selectedTrackId = trackId;

    TreeItem* pRootItem = m_pSidebarModel->getRootItem();
    VERIFY_OR_DEBUG_ASSERT(pRootItem != nullptr) {
        return;
    }

    std::vector<GenreId> sortedTrackGenres;
    if (m_selectedTrackId.isValid()) {
        GenreTrackSelectResult trackGenresIter(
                m_pTrackCollection->genres().selectTrackGenresSorted(m_selectedTrackId));
        while (trackGenresIter.next()) {
            sortedTrackGenres.push_back(trackGenresIter.genreId());
        }
    }

    // Set all genres the track is in bold (or if there is no track selected,
    // clear all the bolding).
    for (TreeItem* pTreeItem : pRootItem->children()) {
        DEBUG_ASSERT(pTreeItem != nullptr);
        bool genreContainsSelectedTrack =
                m_selectedTrackId.isValid() &&
                std::binary_search(
                        sortedTrackGenres.begin(),
                        sortedTrackGenres.end(),
                        GenreId(pTreeItem->getData()));
        pTreeItem->setBold(genreContainsSelectedTrack);
    }

    m_pSidebarModel->triggerRepaint();
}

void GenreFeature::slotResetSelectedTrack() {
    slotTrackSelected(TrackId{});
}
