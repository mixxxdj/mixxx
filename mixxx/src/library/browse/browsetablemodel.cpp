
#include <QtCore>
#include <QtSql>
#include <QStringList>
#include <QtConcurrentRun>
#include <QMetaType>
#include <QMessageBox>

#include "library/browse/browsetablemodel.h"
#include "library/browse/browsethread.h"
#include "soundsourceproxy.h"
#include "mixxxutils.cpp"
#include "playerinfo.h"
#include "controlobject.h"
#include "library/dao/trackdao.h"
#include "audiotagger.h"


BrowseTableModel::BrowseTableModel(QObject* parent,
                                   TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : TrackModel(pTrackCollection->getDatabase(),
                     "mixxx.db.model.browse"),
          QStandardItemModel(parent),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager) {
    QStringList header_data;
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
    header_data.insert(COLUMN_LOCATION, tr("Location"));

    addSearchColumn(COLUMN_FILENAME);
    addSearchColumn(COLUMN_ARTIST);
    addSearchColumn(COLUMN_ALBUM);
    addSearchColumn(COLUMN_TITLE);
    addSearchColumn(COLUMN_GENRE);
    addSearchColumn(COLUMN_COMPOSER);
    addSearchColumn(COLUMN_KEY);
    addSearchColumn(COLUMN_COMMENT);

    setHorizontalHeaderLabels(header_data);
    // register the QList<T> as a metatype since we use QueuedConnection below
    qRegisterMetaType< QList< QList<QStandardItem*> > >(
        "QList< QList<QStandardItem*> >");
    qRegisterMetaType<BrowseTableModel*>("BrowseTableModel*");

    connect(BrowseThread::getInstance(), SIGNAL(clearModel(BrowseTableModel*)),
            this, SLOT(slotClear(BrowseTableModel*)),
            Qt::QueuedConnection);

    connect(
        BrowseThread::getInstance(),
        SIGNAL(rowsAppended(const QList< QList<QStandardItem*> >&, BrowseTableModel*)),
        this,
        SLOT(slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*)),
        Qt::QueuedConnection);
}

BrowseTableModel::~BrowseTableModel() {
}

const QList<int>& BrowseTableModel::searchColumns() const {
    return m_searchColumns;
}

void BrowseTableModel::addSearchColumn(int index) {
    m_searchColumns.push_back(index);
}

void BrowseTableModel::setPath(QString absPath) {
    m_current_path = absPath;
    BrowseThread::getInstance()->executePopulation(m_current_path, this);
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

    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
    int track_id = track_dao.getTrackId(track_location);
    if (track_id < 0) {
        // Add Track to library
        track_id = track_dao.addTrack(track_location, true);
    }

    return track_dao.getTrack(track_id);
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const {
    int row = index.row();

    QModelIndex index2 = this->index(row, COLUMN_LOCATION);
    return data(index2).toString();
}

int BrowseTableModel::getTrackId(const QModelIndex& index) const {
    Q_UNUSED(index);
    // We can't implement this as it stands.
    return -1;
}

const QLinkedList<int> BrowseTableModel::getTrackRows(int trackId) const {
    Q_UNUSED(trackId);
    // We can't implement this as it stands.
    return QLinkedList<int>();
}

void BrowseTableModel::search(const QString& searchText) {
    Q_UNUSED(searchText);
}

const QString BrowseTableModel::currentSearch() const {
    return QString(""); 
}

bool BrowseTableModel::isColumnInternal(int) {
    return false;
}

bool BrowseTableModel::isColumnHiddenByDefault(int) {
    return false;
}

void BrowseTableModel::moveTrack(const QModelIndex&, const QModelIndex&) {
}

QItemDelegate* BrowseTableModel::delegateForColumn(const int) {
    return NULL;
}

void BrowseTableModel::removeTrack(const QModelIndex& index) {
    if (!index.isValid()) {
        return;
    }
    QStringList trackLocations;
    trackLocations.append(getTrackLocation(index));
    removeTracks(trackLocations);
}

void BrowseTableModel::removeTracks(const QModelIndexList& indices) {
    QStringList trackLocations;
    foreach (QModelIndex index, indices) {
        if (!index.isValid()) {
            continue;
        }
        trackLocations.append(getTrackLocation(index));
    }
    removeTracks(trackLocations);
}

void BrowseTableModel::removeTracks(QStringList trackLocations) {
    if (trackLocations.size() == 0)
        return;

    // Ask user if s/he is sure
    if (QMessageBox::question(
        NULL, tr("Mixxx Library"),
        tr("Warning: This will permanently delete the following files:")
        + "\n" + trackLocations.join("\n") + "\n" +
        tr("Are you sure you want to delete these files from your computer?"),
        QMessageBox::Yes, QMessageBox::Abort) == QMessageBox::Abort) {
        return;
    }


    bool any_deleted = false;
    TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();

    foreach (QString track_location, trackLocations) {
        // If track is in use or deletion fails, show an error message.
        if (isTrackInUse(track_location) || !QFile::remove(track_location)) {
            QMessageBox::critical(
                0, tr("Mixxx Library"),
                tr("Could not delete the following file because"
                   " it is in use by Mixxx or another application:") + "\n" +track_location);
            continue;
        }

        qDebug() << "BrowseFeature: User deleted track " << track_location;
        any_deleted = true;

        // If the track was contained in the Mixxx library, delete it
        if (track_dao.trackExistsInDatabase(track_location)) {
            int id = track_dao.getTrackId(track_location);
            qDebug() << "BrowseFeature: Deletion affected database";
            track_dao.removeTrack(id);
        }
    }

    // Repopulate model if any tracks were actually deleted
    if (any_deleted) {
        BrowseThread::getInstance()->executePopulation(m_current_path, this);
    }
}

bool BrowseTableModel::addTrack(const QModelIndex& index, QString location) {
    Q_UNUSED(index);
    Q_UNUSED(location);
    return false;
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
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid()) {
                    qDebug() << "ERROR invalid url\n";
                } else {
                    urls.append(url);
                    qDebug() << "Appending URL:" << url;
                }
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
       column == COLUMN_TYPE) {
        return defaultFlags;
    } else {
        return defaultFlags | Qt::ItemIsEditable;
    }
}

bool BrowseTableModel::isTrackInUse(const QString &track_location) const {
    int decks = ControlObject::getControl(
        ConfigKey("[Master]", "num_decks"))->get();
    // check if file is loaded to a deck
    for (int i = 1; i <= decks; ++i) {
        TrackPointer loaded_track = PlayerInfo::Instance().getTrackInfo(
            QString("[Channel%1]").arg(i));
        if (loaded_track && (loaded_track->getLocation() == track_location)) {
            return true;
        }
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
    QString track_location = getTrackLocation(index);
    AudioTagger tagger(track_location);

    // set tagger information
    tagger.setArtist(this->index(row, COLUMN_ARTIST).data().toString());
    tagger.setTitle(this->index(row, COLUMN_TITLE).data().toString());
    tagger.setAlbum(this->index(row, COLUMN_ALBUM).data().toString());
    tagger.setKey(this->index(row, COLUMN_KEY).data().toString());
    tagger.setBpm(this->index(row, COLUMN_BPM).data().toString());
    tagger.setComment(this->index(row, COLUMN_COMMENT).data().toString());
    tagger.setTracknumber(
        this->index(row, COLUMN_TRACK_NUMBER).data().toString());
    tagger.setYear(this->index(row, COLUMN_YEAR).data().toString());
    tagger.setGenre(this->index(row, COLUMN_GENRE).data().toString());
    tagger.setComposer(this->index(row, COLUMN_COMPOSER).data().toString());

    // check if one the item were edited
    if (col == COLUMN_ARTIST) {
        tagger.setArtist(value.toString());
    } else if (col == COLUMN_TITLE) {
        tagger.setTitle(value.toString());
    } else if (col == COLUMN_ALBUM) {
        tagger.setAlbum(value.toString());
    } else if (col == COLUMN_BPM) {
        tagger.setBpm(value.toString());
    } else if (col == COLUMN_KEY) {
        tagger.setKey(value.toString());
    } else if (col == COLUMN_TRACK_NUMBER) {
        tagger.setTracknumber(value.toString());
    } else if (col == COLUMN_COMMENT) {
        tagger.setComment(value.toString());
    } else if (col == COLUMN_GENRE) {
        tagger.setGenre(value.toString());
    } else if (col == COLUMN_COMPOSER) {
        tagger.setComposer(value.toString());
    } else if (col == COLUMN_YEAR) {
        tagger.setYear(value.toString());
    }


    QStandardItem* item = itemFromIndex(index);
    if (tagger.save()) {
        // Modify underlying interalPointer object
        item->setText(value.toString());
        return true;
    } else {
        // reset to old value in error
        item->setText(index.data().toString());
        QMessageBox::critical(
            0, tr("Mixxx Library"),
            tr("Could not update file metadata.")
            + "\n" +track_location);
        return false;
    }
}
