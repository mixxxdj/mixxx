#include "library/mixxxlibraryfeature.h"

#include <QtDebug>
#ifdef __ENGINEPRIME__
#include <QMenu>
#endif

#include "library/basetrackcache.h"
#include "library/dao/trackschema.h"
#include "library/library.h"
#include "library/librarytablemodel.h"
#include "library/missing_hidden/dlghidden.h"
#include "library/missing_hidden/dlgmissing.h"
#include "library/parser.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "library/trackcollectionmanager.h"
#include "library/treeitem.h"
#include "moc_mixxxlibraryfeature.cpp"
#include "sources/soundsourceproxy.h"
#include "util/dnd.h"
#include "widget/wlibrary.h"
#ifdef __ENGINEPRIME__
#include "widget/wlibrarysidebar.h"
#endif

MixxxLibraryFeature::MixxxLibraryFeature(Library* pLibrary,
        UserSettingsPointer pConfig)
        : LibraryFeature(pLibrary, pConfig, QStringLiteral("tracks")),
          kMissingTitle(tr("Missing Tracks")),
          kHiddenTitle(tr("Hidden Tracks")),
          m_pTrackCollection(pLibrary->trackCollectionManager()->internalCollection()),
          m_pLibraryTableModel(nullptr),
          m_pSidebarModel(make_parented<TreeItemModel>(this)),
          m_pMissingView(nullptr),
          m_pHiddenView(nullptr),
          m_trackCount{0} {
    QString idColumn = LIBRARYTABLE_ID;
    QStringList columns = {
            LIBRARYTABLE_ID,
            LIBRARYTABLE_PLAYED,
            LIBRARYTABLE_TIMESPLAYED,
            LIBRARYTABLE_LAST_PLAYED_AT,
            // has to be up here otherwise Played and TimesPlayed are not shown
            LIBRARYTABLE_ALBUMARTIST,
            LIBRARYTABLE_ALBUM,
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_TITLE,
            LIBRARYTABLE_YEAR,
            LIBRARYTABLE_RATING,
            LIBRARYTABLE_GENRE,
            LIBRARYTABLE_COMPOSER,
            LIBRARYTABLE_GROUPING,
            LIBRARYTABLE_TRACKNUMBER,
            LIBRARYTABLE_KEY,
            LIBRARYTABLE_KEY_ID,
            LIBRARYTABLE_BPM,
            LIBRARYTABLE_BPM_LOCK,
            LIBRARYTABLE_BEATS_VERSION,
            LIBRARYTABLE_DURATION,
            LIBRARYTABLE_BITRATE,
            LIBRARYTABLE_REPLAYGAIN,
            LIBRARYTABLE_FILETYPE,
            LIBRARYTABLE_DATETIMEADDED,
            TRACKLOCATIONSTABLE_LOCATION,
            TRACKLOCATIONSTABLE_FSDELETED,
            LIBRARYTABLE_COMMENT,
            LIBRARYTABLE_MIXXXDELETED,
            LIBRARYTABLE_COLOR,
            LIBRARYTABLE_COVERART_SOURCE,
            LIBRARYTABLE_COVERART_TYPE,
            LIBRARYTABLE_COVERART_LOCATION,
            LIBRARYTABLE_COVERART_COLOR,
            LIBRARYTABLE_COVERART_DIGEST,
            LIBRARYTABLE_COVERART_HASH,
            LIBRARYTABLE_WAVESUMMARYHEX};
    QStringList searchColumns = {
            LIBRARYTABLE_ARTIST,
            LIBRARYTABLE_ALBUM,
            LIBRARYTABLE_ALBUMARTIST,
            TRACKLOCATIONSTABLE_LOCATION,
            LIBRARYTABLE_GROUPING,
            LIBRARYTABLE_COMMENT,
            LIBRARYTABLE_TITLE,
            LIBRARYTABLE_GENRE,
            LIBRARYTABLE_CRATE};

    QStringList qualifiedTableColumns;
    for (const auto& col : columns) {
        qualifiedTableColumns.append(mixxx::trackschema::tableForColumn(col) +
                QLatin1Char('.') + col);
    }

    QSqlQuery query(m_pTrackCollection->database());
    QString tableName = "library_cache_view";
    QString queryString = QString(
            "CREATE TEMPORARY VIEW IF NOT EXISTS %1 AS "
            "SELECT %2 FROM library "
            "INNER JOIN track_locations ON library.location = track_locations.id")
                                  .arg(tableName, qualifiedTableColumns.join(","));
    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    BaseTrackCache* pBaseTrackCache = new BaseTrackCache(m_pTrackCollection,
            std::move(tableName),
            std::move(idColumn),
            std::move(columns),
            std::move(searchColumns),
            true);
    m_pBaseTrackCache = QSharedPointer<BaseTrackCache>(pBaseTrackCache);
    m_pTrackCollection->connectTrackSource(m_pBaseTrackCache);

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this,
            pLibrary->trackCollectionManager(),
            "mixxx.db.model.library");

    connect(m_pLibraryTableModel,
            &LibraryTableModel::updateTrackCount,
            this,
            &MixxxLibraryFeature::slotUpdateTrackCount);

    std::unique_ptr<TreeItem> pRootItem = TreeItem::newRoot(this);
    pRootItem->appendChild(kMissingTitle);
    pRootItem->appendChild(kHiddenTitle);

    m_pSidebarModel->setRootItem(std::move(pRootItem));

#ifdef __ENGINEPRIME__
    m_pExportLibraryAction = make_parented<QAction>(tr("Export to Engine DJ"), this);
    connect(m_pExportLibraryAction.get(),
            &QAction::triggered,
            this,
            &MixxxLibraryFeature::exportLibrary);
#endif
}

void MixxxLibraryFeature::bindLibraryWidget(WLibrary* pLibraryWidget,
                                     KeyboardEventFilter* pKeyboard) {
    m_pHiddenView = new DlgHidden(pLibraryWidget, m_pConfig, m_pLibrary,
                                  pKeyboard);
    pLibraryWidget->registerView(kHiddenTitle, m_pHiddenView);
    connect(m_pHiddenView,
            &DlgHidden::trackSelected,
            this,
            &MixxxLibraryFeature::trackSelected);

    m_pMissingView = new DlgMissing(pLibraryWidget, m_pConfig, m_pLibrary,
                                    pKeyboard);
    pLibraryWidget->registerView(kMissingTitle, m_pMissingView);
    connect(m_pMissingView,
            &DlgMissing::trackSelected,
            this,
            &MixxxLibraryFeature::trackSelected);
}

QVariant MixxxLibraryFeature::title() {
    const QString title = tr("Tracks") + QStringLiteral(" (%1)").arg(m_trackCount);
    return title;
}

TreeItemModel* MixxxLibraryFeature::sidebarModel() const {
    return m_pSidebarModel;
}

void MixxxLibraryFeature::refreshLibraryModels() {
    if (m_pLibraryTableModel) {
        m_pLibraryTableModel->select();
    }
    if (m_pMissingView) {
        m_pMissingView->onShow();
    }
    if (m_pHiddenView) {
        m_pHiddenView->onShow();
    }
}

void MixxxLibraryFeature::searchAndActivate(const QString& query) {
    VERIFY_OR_DEBUG_ASSERT(m_pLibraryTableModel) {
        return;
    }
    m_pLibraryTableModel->search(query);
    selectAndActivate();
}

#ifdef __ENGINEPRIME__
void MixxxLibraryFeature::bindSidebarWidget(WLibrarySidebar* pSidebarWidget) {
    // store the sidebar widget pointer for later use in onRightClick
    m_pSidebarWidget = pSidebarWidget;
}
#endif

void MixxxLibraryFeature::slotUpdateTrackCount() {
    m_trackCount = m_pLibraryTableModel->rowCount();

    // Force updating the Tracks sidebar item.
    // `select` must be false as we don't want to select again
    emit featureIsLoading(this, false);
}

void MixxxLibraryFeature::activate() {
    //qDebug() << "MixxxLibraryFeature::activate()";
    emit saveModelState();
    emit showTrackModel(m_pLibraryTableModel);
    emit enableCoverArtDisplay(true);
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data().toString();
    emit saveModelState();
    emit switchToView(itemName);
    if (m_pMissingView && itemName == kMissingTitle) {
        emit restoreSearch(m_pMissingView->currentSearch());
    } else if (m_pHiddenView && itemName == kHiddenTitle) {
        emit restoreSearch(m_pHiddenView->currentSearch());
    }
    emit enableCoverArtDisplay(true);
}

bool MixxxLibraryFeature::dropAccept(const QList<QUrl>& urls, QObject* pSource) {
    if (pSource) {
        // We don't accept internal drags onto Tracks as all tracks with a
        // source are already in the library.
        return false;
    }

    const QList<mixxx::FileInfo> fileInfos =
            // collect all tracks, accept playlist files
            DragAndDropHelper::supportedTracksFromUrls(urls, false, true);
    const QList<TrackId> trackIds =
            m_pLibrary->trackCollectionManager()->resolveTrackIds(fileInfos, nullptr);
    return trackIds.size() > 0;
}

bool MixxxLibraryFeature::dragMoveAccept(const QList<QUrl>& urls) {
    return DragAndDropHelper::urlsContainSupportedTrackFiles(urls, true);
}

#ifdef __ENGINEPRIME__
void MixxxLibraryFeature::onRightClick(const QPoint& globalPos) {
    QMenu menu(m_pSidebarWidget);
    menu.addAction(m_pExportLibraryAction.get());
    menu.exec(globalPos);
}
#endif
