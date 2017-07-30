#include <QtSql>
#include <QStringList>
#include <QtConcurrentRun>
#include <QMetaType>
#include <QMessageBox>
#include <QUrl>

#include "library/browse/browsetablemodel.h"
#include "library/browse/browsethread.h"
#include "library/previewbuttondelegate.h"
#include "mixer/playerinfo.h"
#include "control/controlobject.h"
#include "library/dao/trackdao.h"
#include "track/trackmetadatataglib.h"
#include "util/dnd.h"

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
    header_data.insert(COLUMN_LOCATION, tr("Location"));
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
    return m_pTrackCollection->getTrackDAO()
            .getOrAddTrack(track_location, true, NULL);
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const {
    int row = index.row();

    QModelIndex index2 = this->index(row, COLUMN_LOCATION);
    return data(index2).toString();
}

TrackId BrowseTableModel::getTrackId(const QModelIndex& index) const {
    Q_UNUSED(index);
    // We can't implement this as it stands.
    return TrackId();
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
            column == COLUMN_LOCATION ||
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
                QUrl url = DragAndDropHelper::urlFromLocation(getTrackLocation(index));
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

    if (isTrackInUse(track_location) ||
            column == COLUMN_FILENAME ||
            column == COLUMN_BITRATE ||
            column == COLUMN_DURATION ||
            column == COLUMN_TYPE ||
            column == COLUMN_FILE_MODIFIED_TIME ||
            column == COLUMN_FILE_CREATION_TIME ||
            column == COLUMN_REPLAYGAIN) {
        return defaultFlags;
    } else {
        return defaultFlags | Qt::ItemIsEditable;
    }
}

bool BrowseTableModel::isTrackInUse(const QString& track_location) const {
    if (PlayerInfo::instance().isFileLoaded(track_location)) {
        return true;
    }

    if (m_pRecordingManager->getRecordingLocation() == track_location) {
        return true;
    }

    return false;
}

bool BrowseTableModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) {
    Q_UNUSED(role);

    if (!index.isValid()) {
        return false;
    }
    qDebug() << "BrowseTableModel::setData(" << index.data() << ")";
    int row = index.row();
    int col = index.column();

    mixxx::TrackMetadata trackMetadata;

    // set tagger information
    trackMetadata.setArtist(this->index(row, COLUMN_ARTIST).data().toString());
    trackMetadata.setTitle(this->index(row, COLUMN_TITLE).data().toString());
    trackMetadata.setAlbum(this->index(row, COLUMN_ALBUM).data().toString());
    trackMetadata.setKey(this->index(row, COLUMN_KEY).data().toString());
    trackMetadata.setBpm(mixxx::Bpm(this->index(row, COLUMN_BPM).data().toDouble()));
    trackMetadata.setComment(this->index(row, COLUMN_COMMENT).data().toString());
    trackMetadata.setTrackNumber(this->index(row, COLUMN_TRACK_NUMBER).data().toString());
    trackMetadata.setYear(this->index(row, COLUMN_YEAR).data().toString());
    trackMetadata.setGenre(this->index(row, COLUMN_GENRE).data().toString());
    trackMetadata.setComposer(this->index(row, COLUMN_COMPOSER).data().toString());
    trackMetadata.setAlbumArtist(this->index(row, COLUMN_ALBUMARTIST).data().toString());
    trackMetadata.setGrouping(this->index(row, COLUMN_GROUPING).data().toString());

    // check if one the item were edited
    if (col == COLUMN_ARTIST) {
        trackMetadata.setArtist(value.toString());
    } else if (col == COLUMN_TITLE) {
        trackMetadata.setTitle(value.toString());
    } else if (col == COLUMN_ALBUM) {
        trackMetadata.setAlbum(value.toString());
    } else if (col == COLUMN_BPM) {
        trackMetadata.setBpm(mixxx::Bpm(value.toDouble()));
    } else if (col == COLUMN_KEY) {
        trackMetadata.setKey(value.toString());
    } else if (col == COLUMN_TRACK_NUMBER) {
        trackMetadata.setTrackNumber(value.toString());
    } else if (col == COLUMN_COMMENT) {
        trackMetadata.setComment(value.toString());
    } else if (col == COLUMN_GENRE) {
        trackMetadata.setGenre(value.toString());
    } else if (col == COLUMN_COMPOSER) {
        trackMetadata.setComposer(value.toString());
    } else if (col == COLUMN_YEAR) {
        trackMetadata.setYear(value.toString());
    } else if (col == COLUMN_ALBUMARTIST) {
        trackMetadata.setAlbumArtist(value.toString());
    } else if (col == COLUMN_GROUPING) {
        trackMetadata.setGrouping(value.toString());
    } else {
        qWarning() << "BrowseTableModel::setData(): no tagger column";
        return false;
    }

    QStandardItem* item = itemFromIndex(index);
    QString track_location(getTrackLocation(index));
    if (OK == mixxx::taglib::writeTrackMetadataIntoFile(trackMetadata, track_location)) {
        // Modify underlying interalPointer object
        item->setText(value.toString());
        item->setToolTip(item->text());
        return true;
    } else {
        // reset to old value in error
        item->setText(index.data().toString());
        item->setToolTip(item->text());
        QMessageBox::critical(
            0, tr("Mixxx Library"),
            tr("Could not update file metadata.")
            + "\n" +track_location);
        return false;
    }
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
            for (int row = 0; row < rowCount(); ++row) {
                QModelIndex i = index(row, COLUMN_PREVIEW);
                QString location = index(row, COLUMN_LOCATION).data().toString();
                if (location == pTrack->getLocation()) {
                    QStandardItem* item = itemFromIndex(i);
                    item->setText("1");
                    break;
                }
            }
        }
    }
}

bool BrowseTableModel::isColumnSortable(int column) {
    if (COLUMN_PREVIEW == column) {
        return false;
    }
    return true;
}

QAbstractItemDelegate* BrowseTableModel::delegateForColumn(const int i, QObject* pParent) {
    if (PlayerManager::numPreviewDecks() > 0 && i == COLUMN_PREVIEW) {
        return new PreviewButtonDelegate(pParent, i);
    }
    return NULL;
}
