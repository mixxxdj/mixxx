// mixxxlibraryfeature.cpp
// Created 8/23/2009 by RJ Ryan (rryan@mit.edu)

#include <QtDebug>

#include "library/mixxxlibraryfeature.h"

#include "library/librarytablemodel.h"
#include "library/missingtablemodel.h"
#include "library/queryutil.h"
#include "library/trackcollection.h"
#include "treeitem.h"

#define CHILD_MISSING "Missing Songs"

MixxxLibraryFeature::MixxxLibraryFeature(QObject* parent,
                                         TrackCollection* pTrackCollection)
        : LibraryFeature(parent) {
    QStringList columns;
    columns << "library." + LIBRARYTABLE_ID
            << "library." + LIBRARYTABLE_PLAYED
            << "library." + LIBRARYTABLE_TIMESPLAYED
            << "library." + LIBRARYTABLE_ARTIST
            << "library." + LIBRARYTABLE_TITLE
            << "library." + LIBRARYTABLE_ALBUM
            << "library." + LIBRARYTABLE_YEAR
            << "library." + LIBRARYTABLE_DURATION
            << "library." + LIBRARYTABLE_RATING
            << "library." + LIBRARYTABLE_GENRE
            << "library." + LIBRARYTABLE_FILETYPE
            << "library." + LIBRARYTABLE_TRACKNUMBER
            << "library." + LIBRARYTABLE_KEY
            << "library." + LIBRARYTABLE_DATETIMEADDED
            << "library." + LIBRARYTABLE_BPM
            << "library." + LIBRARYTABLE_BITRATE
            << "track_locations.location"
            << "track_locations.fs_deleted"
            << "library." + LIBRARYTABLE_COMMENT
            << "library." + LIBRARYTABLE_MIXXXDELETED;

    QSqlQuery query(pTrackCollection->getDatabase());
    QString queryString = "CREATE TEMPORARY VIEW IF NOT EXISTS library_cache_view AS SELECT "
            + columns.join(",") +
            " FROM library INNER JOIN track_locations "
            "ON library.location = track_locations.id "
            "WHERE (" + LibraryTableModel::DEFAULT_LIBRARYFILTER + ")";

    query.prepare(queryString);
    if (!query.exec()) {
        LOG_FAILED_QUERY(query);
    }

    // Strip out library. and track_locations.
    for (QStringList::iterator it = columns.begin();
         it != columns.end(); ++it) {
        if (it->startsWith("library.")) {
            *it = it->replace("library.", "");
        } else if (it->startsWith("track_locations.")) {
            *it = it->replace("track_locations.", "");
        }
    }

    BaseTrackCache* pBaseTrackCache = new BaseTrackCache(
        pTrackCollection, "library_cache_view", LIBRARYTABLE_ID, columns, true);
    connect(&pTrackCollection->getTrackDAO(), SIGNAL(trackDirty(int)),
            pBaseTrackCache, SLOT(slotTrackDirty(int)));
    connect(&pTrackCollection->getTrackDAO(), SIGNAL(trackClean(int)),
            pBaseTrackCache, SLOT(slotTrackClean(int)));

    pTrackCollection->addTrackSource(
        QString("default"), QSharedPointer<BaseTrackCache>(pBaseTrackCache));

    // These rely on the 'default' track source being present.
    m_pLibraryTableModel = new LibraryTableModel(this, pTrackCollection);
    m_pMissingTableModel = new MissingTableModel(this, pTrackCollection);

    TreeItem *rootItem = new TreeItem();
    TreeItem *childItem = new TreeItem(CHILD_MISSING, CHILD_MISSING,
                                       this, rootItem);
    rootItem->appendChild(childItem);
    m_childModel.setRootItem(rootItem);
}

MixxxLibraryFeature::~MixxxLibraryFeature() {
    delete m_pLibraryTableModel;
    delete m_pMissingTableModel;
}

QVariant MixxxLibraryFeature::title() {
    return tr("Library");
}

QIcon MixxxLibraryFeature::getIcon() {
    return QIcon(":/images/library/ic_library_library.png");
}

TreeItemModel* MixxxLibraryFeature::getChildModel() {
    return &m_childModel;
}

void MixxxLibraryFeature::refreshLibraryModels() {
    m_pLibraryTableModel->select();
    m_pMissingTableModel->select();
}

void MixxxLibraryFeature::activate() {
    qDebug() << "MixxxLibraryFeature::activate()";
    emit(showTrackModel(m_pLibraryTableModel));
}

void MixxxLibraryFeature::activateChild(const QModelIndex& index) {
    QString itemName = index.data().toString();

    /*if (itemName == m_childModel.stringList().at(0))
        emit(showTrackModel(m_pMissingTableModel));
     */
    if (itemName == CHILD_MISSING)
        emit(showTrackModel(m_pMissingTableModel));
}

void MixxxLibraryFeature::onRightClick(const QPoint& globalPos) {
}

void MixxxLibraryFeature::onRightClickChild(const QPoint& globalPos,
                                            QModelIndex index) {
}

bool MixxxLibraryFeature::dropAccept(QUrl url) {
    return false;
}

bool MixxxLibraryFeature::dropAcceptChild(const QModelIndex& index, QUrl url) {
    return false;
}

bool MixxxLibraryFeature::dragMoveAccept(QUrl url) {
    return false;
}

bool MixxxLibraryFeature::dragMoveAcceptChild(const QModelIndex& index,
                                              QUrl url) {
    return false;
}void MixxxLibraryFeature::onLazyChildExpandation(const QModelIndex &index){
//Nothing to do because the childmodel is not of lazy nature.
}
