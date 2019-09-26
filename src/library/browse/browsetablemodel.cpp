#include <QtSql>
#include <QStringList>
#include <QtConcurrentRun>
#include <QMetaType>
#include <QMessageBox>
#include <QUrl>
#include <QTableView>

#include "library/browse/browsetablemodel.h"
#include "library/browse/browsethread.h"
#include "library/previewbuttondelegate.h"
#include "mixer/playerinfo.h"
#include "mixer/playermanager.h"
#include "control/controlobject.h"
#include "library/dao/trackdao.h"

BrowseTableModel::BrowseTableModel(QObject* parent,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : TrackModel(pTrackCollection->database(),
                     "mixxx.db.model.browse"),
          QStandardItemModel(parent),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager),
          m_previewDeckGroup(PlayerManager::groupForPreviewDeck(0)) {
    QStringList header_data;
    header_data.insert(COLUMN_PREVIEW, tr("Preview"));
    header_data.insert(COLUMN_FILENAME, tr("Filename"));
    header_data.insert(COLUMN_ARTIST, tr("Artist"));
    header_data.insert(COLUMN_TITLE, tr("Title"));
    header_data.insert(COLUMN_ALBUM, tr("Album"));
    header_data.insert(COLUMN_TRACK_NUMBER, tr("Track #"));
    header_data.insert(COLUMN_YEAR, tr("Year"));
    header_data.insert(COLUMN_GENRE, tr("Genre"));
    header_data.insert(COLUMN_COMPOSER, tr("Composer"));
    header_data.insert(COLUMN_COMMENT, tr("Comment"));
    header_data.insert(COLUMN_DURATION, tr("Duration"));
    header_data.insert(COLUMN_BPM, tr("BPM"));
    header_data.insert(COLUMN_KEY, tr("Key"));
    header_data.insert(COLUMN_TYPE, tr("Type"));
    header_data.insert(COLUMN_BITRATE, tr("Bitrate"));
    header_data.insert(COLUMN_REPLAYGAIN, tr("ReplayGain"));
    header_data.insert(COLUMN_NATIVELOCATION, tr("Location"));
    header_data.insert(COLUMN_ALBUMARTIST, tr("Album Artist"));
    header_data.insert(COLUMN_GROUPING, tr("Grouping"));
    header_data.insert(COLUMN_FILE_MODIFIED_TIME, tr("File Modified"));
    header_data.insert(COLUMN_FILE_CREATION_TIME, tr("File Created"));

    addSearchColumn(COLUMN_FILENAME);
    addSearchColumn(COLUMN_ARTIST);
    addSearchColumn(COLUMN_ALBUM);
    addSearchColumn(COLUMN_TITLE);
    addSearchColumn(COLUMN_GENRE);
    addSearchColumn(COLUMN_COMPOSER);
    addSearchColumn(COLUMN_KEY);
    addSearchColumn(COLUMN_COMMENT);
    addSearchColumn(COLUMN_ALBUMARTIST);
    addSearchColumn(COLUMN_GROUPING);
    addSearchColumn(COLUMN_FILE_MODIFIED_TIME);
    addSearchColumn(COLUMN_FILE_CREATION_TIME);

    setDefaultSort(COLUMN_FILENAME, Qt::AscendingOrder);

    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        m_columnIndexBySortColumnId[i] = -1;
    }
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_FILENAME] = COLUMN_FILENAME;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ARTIST] = COLUMN_ARTIST;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TITLE] = COLUMN_TITLE;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ALBUM] = COLUMN_ALBUM;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_ALBUMARTIST] = COLUMN_ALBUMARTIST;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_YEAR] = COLUMN_YEAR;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GENRE] = COLUMN_GENRE;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COMPOSER] = COLUMN_COMPOSER;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GROUPING] = COLUMN_GROUPING;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_TRACKNUMBER] = COLUMN_TRACK_NUMBER;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_FILETYPE] = COLUMN_TYPE;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_NATIVELOCATION] = COLUMN_NATIVELOCATION;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_COMMENT] = COLUMN_COMMENT;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_DURATION] = COLUMN_DURATION;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_BITRATE] = COLUMN_BITRATE;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_BPM] = COLUMN_BPM;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_REPLAYGAIN] = COLUMN_REPLAYGAIN;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_KEY] = COLUMN_KEY;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_PREVIEW] = COLUMN_PREVIEW;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_GROUPING] = COLUMN_GROUPING;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_FILE_MODIFIED_TIME] = COLUMN_FILE_MODIFIED_TIME;
    m_columnIndexBySortColumnId[TrackModel::SortColumnId::SORTCOLUMN_FILE_CREATION_TIME] = COLUMN_FILE_CREATION_TIME;

    m_sortColumnIdByColumnIndex.clear();
    for (int i = 0; i < TrackModel::SortColumnId::NUM_SORTCOLUMNIDS; ++i) {
        TrackModel::SortColumnId sortColumn = static_cast<TrackModel::SortColumnId>(i);
        int columnIndex = m_columnIndexBySortColumnId[sortColumn];
        if (columnIndex >= 0) {
            m_sortColumnIdByColumnIndex.insert(columnIndex, sortColumn);
        }
    }

    setHorizontalHeaderLabels(header_data);
    // register the QList<T> as a metatype since we use QueuedConnection below
    qRegisterMetaType< QList< QList<QStandardItem*> > >(
        "QList< QList<QStandardItem*> >");
    qRegisterMetaType<BrowseTableModel*>("BrowseTableModel*");

    m_pBrowseThread = BrowseThread::getInstanceRef();
    connect(m_pBrowseThread.data(), SIGNAL(clearModel(BrowseTableModel*)),
            this, SLOT(slotClear(BrowseTableModel*)),
            Qt::QueuedConnection);

    connect(m_pBrowseThread.data(),
            SIGNAL(rowsAppended(const QList< QList<QStandardItem*> >&, BrowseTableModel*)),
            this,
            SLOT(slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*)),
            Qt::QueuedConnection);

    connect(&PlayerInfo::instance(), SIGNAL(trackLoaded(QString, TrackPointer)),
            this, SLOT(trackLoaded(QString, TrackPointer)));
    trackLoaded(m_previewDeckGroup, PlayerInfo::instance().getTrackInfo(m_previewDeckGroup));
}

BrowseTableModel::~BrowseTableModel() {
}

int BrowseTableModel::columnIndexFromSortColumnId(TrackModel::SortColumnId column) {
    if (column == TrackModel::SortColumnId::SORTCOLUMN_INVALID ||
        column >= TrackModel::SortColumnId::NUM_SORTCOLUMNIDS) {
        return -1;
    }

    return m_columnIndexBySortColumnId[column];
}

TrackModel::SortColumnId BrowseTableModel::sortColumnIdFromColumnIndex(int index) {
    return m_sortColumnIdByColumnIndex.value(index, TrackModel::SortColumnId::SORTCOLUMN_INVALID);
}

const QList<int>& BrowseTableModel::searchColumns() const {
    return m_searchColumns;
}

void BrowseTableModel::addSearchColumn(int index) {
    m_searchColumns.push_back(index);
}

void BrowseTableModel::setPath(const MDir& path) {
    m_current_directory = path;
    m_pBrowseThread->executePopulation(m_current_directory, this);
}

TrackPointer BrowseTableModel::getTrack(const QModelIndex& index) const {
    QString track_location = getTrackLocation(index);
    if (m_pRecordingManager->getRecordingLocation() == track_location) {
        QMessageBox::critical(
            0, tr("Mixxx Library"),
            tr("Could not load the following file because"
               " it is in use by Mixxx or another application.")
            + "\n" +track_location);
        return TrackPointer();
    }
    // NOTE(uklotzde, 2015-12-08): Accessing tracks from the browse view
    // will implicitly add them to the library. Is this really what we
    // want here??
    // NOTE(rryan, 2015-12-27): This was intentional at the time since
    // some people use Browse instead of the library and we want to let
    // them edit the tracks in a way that persists across sessions
    // and we didn't want to edit the files on disk by default
    // unless the user opts in to that.
    return m_pTrackCollection->getTrackDAO()
            .getOrAddTrack(track_location, true, NULL);
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const {
    int row = index.row();

    QModelIndex index2 = this->index(row, COLUMN_NATIVELOCATION);
    QString nativeLocation = data(index2).toString();
    QString location = QDir::fromNativeSeparators(nativeLocation);
    return location;
}

TrackId BrowseTableModel::getTrackId(const QModelIndex& index) const {
    TrackPointer pTrack = getTrack(index);
    if (pTrack) {
        return pTrack->getId();
    } else {
        qWarning()
                << "Track is not available in library"
                << getTrackLocation(index);
        return TrackId();
    }
}

const QLinkedList<int> BrowseTableModel::getTrackRows(TrackId trackId) const {
    Q_UNUSED(trackId);
    // We can't implement this as it stands.
    return QLinkedList<int>();
}

void BrowseTableModel::search(const QString& searchText, const QString& extraFilter) {
    Q_UNUSED(extraFilter);
    Q_UNUSED(searchText);
}

const QString BrowseTableModel::currentSearch() const {
    return QString("");
}

bool BrowseTableModel::isColumnInternal(int) {
    return false;
}

bool BrowseTableModel::isColumnHiddenByDefault(int column) {
    if (column == COLUMN_COMPOSER ||
            column == COLUMN_TRACK_NUMBER ||
            column == COLUMN_YEAR ||
            column == COLUMN_GROUPING ||
            column == COLUMN_NATIVELOCATION ||
            column == COLUMN_ALBUMARTIST ||
            column == COLUMN_FILE_CREATION_TIME ||
            column == COLUMN_REPLAYGAIN) {
        return true;
    }
    return false;
}

void BrowseTableModel::moveTrack(const QModelIndex&, const QModelIndex&) {
}

void BrowseTableModel::removeTracks(const QModelIndexList&) {
}

QMimeData* BrowseTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    // Ok, so the list of indexes we're given contains separates indexes for
    // each column, so even if only one row is selected, we'll have like 7
    // indexes.  We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url = TrackFile(getTrackLocation(index)).toUrl();
                if (!url.isValid()) {
                    qDebug() << "ERROR invalid url" << url;
                    continue;
                }
                urls.append(url);
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}

void BrowseTableModel::slotClear(BrowseTableModel* caller_object) {
    if (caller_object == this) {
        removeRows(0, rowCount());
    }
}

void BrowseTableModel::slotInsert(const QList< QList<QStandardItem*> >& rows,
                                  BrowseTableModel* caller_object) {
    // There exists more than one BrowseTableModel in Mixxx We only want to
    // receive items here, this object has 'ordered' by the BrowserThread
    // (singleton)
    if (caller_object == this) {
        //qDebug() << "BrowseTableModel::slotInsert";
        for (int i = 0; i < rows.size(); ++i) {
            appendRow(rows.at(i));
        }
    }
}

TrackModel::CapabilitiesFlags BrowseTableModel::getCapabilities() const {
    // See src/library/trackmodel.h for the list of TRACKMODELCAPS
    return TRACKMODELCAPS_NONE
            | TRACKMODELCAPS_ADDTOPLAYLIST
            | TRACKMODELCAPS_ADDTOCRATE
            | TRACKMODELCAPS_ADDTOAUTODJ
            | TRACKMODELCAPS_LOADTODECK
            | TRACKMODELCAPS_LOADTOPREVIEWDECK
            | TRACKMODELCAPS_LOADTOSAMPLER;
}

Qt::ItemFlags BrowseTableModel::flags(const QModelIndex &index) const {
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    // Enable dragging songs from this data model to elsewhere (like the
    // waveform widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    QString track_location = getTrackLocation(index);
    int column = index.column();

    switch (column) {
    case COLUMN_FILENAME:
    case COLUMN_BITRATE:
    case COLUMN_DURATION:
    case COLUMN_TYPE:
    case COLUMN_FILE_MODIFIED_TIME:
    case COLUMN_FILE_CREATION_TIME:
    case COLUMN_REPLAYGAIN:
        // read-only
        return defaultFlags;
    default:
        // editable
        return defaultFlags | Qt::ItemIsEditable;
    }
}

bool BrowseTableModel::setData(
        const QModelIndex& index,
        const QVariant& value,
        int role) {
    Q_UNUSED(role);

    QStandardItem* item = itemFromIndex(index);
    DEBUG_ASSERT(nullptr != item);

    TrackPointer pTrack(getTrack(index));
    if (!pTrack) {
        qWarning() << "BrowseTableModel::setData():"
                << "Failed to resolve track"
                << getTrackLocation(index);
        // restore previous item content
        item->setText(index.data().toString());
        item->setToolTip(item->text());
        return false;
    }

    // check if one the item were edited
    int col = index.column();
    switch (col) {
    case COLUMN_ARTIST:
        pTrack->setArtist(value.toString());
        break;
    case COLUMN_TITLE:
        pTrack->setTitle(value.toString());
        break;
    case COLUMN_ALBUM:
        pTrack->setAlbum(value.toString());
        break;
    case COLUMN_BPM:
        pTrack->setBpm(value.toDouble());
        break;
    case COLUMN_KEY:
        pTrack->setKeyText(value.toString());
        break;
    case COLUMN_TRACK_NUMBER:
        pTrack->setTrackNumber(value.toString());
        break;
    case COLUMN_COMMENT:
        pTrack->setComment(value.toString());
        break;
    case COLUMN_GENRE:
        pTrack->setGenre(value.toString());
        break;
    case COLUMN_COMPOSER:
        pTrack->setComposer(value.toString());
        break;
    case COLUMN_YEAR:
        pTrack->setYear(value.toString());
        break;
    case COLUMN_ALBUMARTIST:
        pTrack->setAlbumArtist(value.toString());
        break;
    case COLUMN_GROUPING:
        pTrack->setGrouping(value.toString());
        break;
    default:
        qWarning() << "BrowseTableModel::setData():"
            << "No tagger column";
        // restore previous item context
        item->setText(index.data().toString());
        item->setToolTip(item->text());
        return false;
    }

    item->setText(value.toString());
    item->setToolTip(item->text());
    return true;
}

void BrowseTableModel::trackLoaded(QString group, TrackPointer pTrack) {
    if (group == m_previewDeckGroup) {
        for (int row = 0; row < rowCount(); ++row) {
            QModelIndex i = index(row, COLUMN_PREVIEW);
            if (i.data().toBool()) {
                QStandardItem* item = itemFromIndex(i);
                item->setText("0");
            }
        }
        if (pTrack) {
            QString trackLocation = pTrack->getLocation();
            for (int row = 0; row < rowCount(); ++row) {
                QModelIndex i = index(row, COLUMN_PREVIEW);
                QString location = getTrackLocation(i);
                if (location == trackLocation) {
                    QStandardItem* item = itemFromIndex(i);
                    item->setText("1");
                    break;
                }
            }
        }
    }
}

bool BrowseTableModel::isColumnSortable(int column) {
    return COLUMN_PREVIEW != column;
}

QAbstractItemDelegate* BrowseTableModel::delegateForColumn(const int i, QObject* pParent) {
    QTableView* pTableView = qobject_cast<QTableView*>(pParent);
    DEBUG_ASSERT(pTableView);
    if (PlayerManager::numPreviewDecks() > 0 && i == COLUMN_PREVIEW) {
        return new PreviewButtonDelegate(pTableView, i);
    }
    return NULL;
}
