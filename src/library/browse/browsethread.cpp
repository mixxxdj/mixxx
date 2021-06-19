#include "library/browse/browsethread.h"

#include <QDateTime>
#include <QDirIterator>
#include <QStringList>
#include <QtDebug>

#include "library/browse/browsetablemodel.h"
#include "moc_browsethread.cpp"
#include "sources/soundsourceproxy.h"
#include "track/track.h"
#include "util/datetime.h"
#include "util/trace.h"

QWeakPointer<BrowseThread> BrowseThread::m_weakInstanceRef;
static QMutex s_Mutex;

/*
 * This class is a singleton and represents a thread
 * that is used to read ID3 metadata
 * from a particular folder.
 *
 * The BrowseTableModel uses this class.
 * Note: Don't call getInstance() from places
 * other than the GUI thread. BrowseThreads emit
 * signals to BrowseModel objects. It does not
 * make sense to use this class in non-GUI threads
 */
BrowseThread::BrowseThread(QObject *parent)
        : QThread(parent) {
    m_bStopThread = false;
    m_model_observer = nullptr;
    //start Thread
    start(QThread::LowPriority);

}

BrowseThread::~BrowseThread() {
    qDebug() << "Wait to finish browser background thread";
    m_bStopThread = true;
    //wake up thread since it might wait for user input
    m_locationUpdated.wakeAll();
    //Wait until thread terminated
    //terminate();
    wait();
    qDebug() << "Browser background thread terminated!";
}

// static
BrowseThreadPointer BrowseThread::getInstanceRef() {
    BrowseThreadPointer strong = m_weakInstanceRef.toStrongRef();
    if (!strong) {
        s_Mutex.lock();
        strong = m_weakInstanceRef.toStrongRef();
        if (!strong) {
            strong = BrowseThreadPointer(new BrowseThread());
            m_weakInstanceRef = strong.toWeakRef();
        }
        s_Mutex.unlock();
    }
    return strong;
}

void BrowseThread::executePopulation(mixxx::FileAccess path, BrowseTableModel* client) {
    m_path_mutex.lock();
    m_path = std::move(path);
    m_model_observer = client;
    m_path_mutex.unlock();
    m_locationUpdated.wakeAll();
}

void BrowseThread::run() {
    QThread::currentThread()->setObjectName("BrowseThread");
    m_mutex.lock();

    while (!m_bStopThread) {
        //Wait until the user has selected a folder
        m_locationUpdated.wait(&m_mutex);
        Trace trace("BrowseThread");

        //Terminate thread if Mixxx closes
        if(m_bStopThread) {
            break;
        }
        // Populate the model
        populateModel();
    }
    m_mutex.unlock();
}

namespace {

class YearItem: public QStandardItem {
public:
  explicit YearItem(const QString& year)
          : QStandardItem(year) {
  }

  QVariant data(int role) const override {
      switch (role) {
      case Qt::DisplayRole: {
          const QString year(QStandardItem::data(role).toString());
          return mixxx::TrackMetadata::formatCalendarYear(year);
      }
      default:
          return QStandardItem::data(role);
      }
  }
};

} // namespace

void BrowseThread::populateModel() {
    m_path_mutex.lock();
    auto thisPath = m_path;
    BrowseTableModel* thisModelObserver = m_model_observer;
    m_path_mutex.unlock();

    if (!thisPath.info().hasLocation()) {
        // Abort if the location is inaccessible or does not exist
        qWarning() << "Skipping" << thisPath.info();
        return;
    }

    // Refresh the name filters in case we loaded new SoundSource plugins.
    const QStringList nameFilters = SoundSourceProxy::getSupportedFileNamePatterns();

    QDirIterator fileIt(thisPath.info().location(),
            nameFilters,
            QDir::Files | QDir::NoDotAndDotDot);

    // remove all rows
    // This is a blocking operation
    // see signal/slot connection in BrowseTableModel
    emit clearModel(thisModelObserver);

    QList< QList<QStandardItem*> > rows;

    int row = 0;
    // Iterate over the files
    while (fileIt.hasNext()) {
        // If a user quickly jumps through the folders
        // the current task becomes "dirty"
        m_path_mutex.lock();
        auto newPath = m_path;
        m_path_mutex.unlock();

        if (thisPath.info() != newPath.info()) {
            qDebug() << "Abort populateModel()";
            populateModel();
            return;
        }

        QList<QStandardItem*> row_data;

        QStandardItem* item = new QStandardItem("0");
        item->setData("0", Qt::UserRole);
        row_data.insert(COLUMN_PREVIEW, item);

        const auto fileAccess = mixxx::FileAccess(
                mixxx::FileInfo(fileIt.next()),
                thisPath.token());
        {
            mixxx::TrackMetadata trackMetadata;
            SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
                    fileAccess,
                    &trackMetadata,
                    nullptr);

            item = new QStandardItem(fileAccess.info().fileName());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_FILENAME, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getArtist());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_ARTIST, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getTitle());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_TITLE, item);

            item = new QStandardItem(trackMetadata.getAlbumInfo().getTitle());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_ALBUM, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getTrackNumber());
            item->setToolTip(item->text());
            item->setData(item->text().toInt(), Qt::UserRole);
            row_data.insert(COLUMN_TRACK_NUMBER, item);

            const QString year(trackMetadata.getTrackInfo().getYear());
            item = new YearItem(year);
            item->setToolTip(year);
            // The year column is sorted according to the numeric calendar year
            item->setData(mixxx::TrackMetadata::parseCalendarYear(year), Qt::UserRole);
            row_data.insert(COLUMN_YEAR, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getGenre());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_GENRE, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getComposer());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_COMPOSER, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getComment());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_COMMENT, item);

            QString duration = trackMetadata.getDurationText(
                    mixxx::Duration::Precision::SECONDS);
            item = new QStandardItem(duration);
            item->setToolTip(item->text());
            item->setData(trackMetadata.getStreamInfo()
                                  .getDuration()
                                  .toDoubleSeconds(),
                    Qt::UserRole);
            row_data.insert(COLUMN_DURATION, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getBpmText());
            item->setToolTip(item->text());
            item->setData(trackMetadata.getTrackInfo().getBpm().getValue(), Qt::UserRole);
            row_data.insert(COLUMN_BPM, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getKey());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_KEY, item);

            item = new QStandardItem(fileAccess.info().suffix());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_TYPE, item);

            item = new QStandardItem(trackMetadata.getBitrateText());
            item->setToolTip(item->text());
            item->setData(
                    static_cast<qlonglong>(
                            trackMetadata.getStreamInfo().getBitrate().value()),
                    Qt::UserRole);
            row_data.insert(COLUMN_BITRATE, item);

            QString location = fileAccess.info().location();
            QString nativeLocation = QDir::toNativeSeparators(location);
            item = new QStandardItem(nativeLocation);
            item->setToolTip(nativeLocation);
            item->setData(location, Qt::UserRole);
            row_data.insert(COLUMN_NATIVELOCATION, item);

            item = new QStandardItem(trackMetadata.getAlbumInfo().getArtist());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_ALBUMARTIST, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getGrouping());
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_GROUPING, item);

            const auto fileLastModified =
                    fileAccess.info().lastModified();
            item = new QStandardItem(
                    mixxx::displayLocalDateTime(fileLastModified));
            item->setToolTip(item->text());
            item->setData(fileLastModified, Qt::UserRole);
            row_data.insert(COLUMN_FILE_MODIFIED_TIME, item);

            const auto fileCreated =
                    fileAccess.info().birthTime();
            item = new QStandardItem(
                    mixxx::displayLocalDateTime(fileCreated));
            item->setToolTip(item->text());
            item->setData(fileCreated, Qt::UserRole);
            row_data.insert(COLUMN_FILE_CREATION_TIME, item);

            const mixxx::ReplayGain replayGain(trackMetadata.getTrackInfo().getReplayGain());
            item = new QStandardItem(
                    mixxx::ReplayGain::ratioToString(replayGain.getRatio()));
            item->setToolTip(item->text());
            item->setData(item->text(), Qt::UserRole);
            row_data.insert(COLUMN_REPLAYGAIN, item);
        }

        rows.append(row_data);
        ++row;
        // If 10 tracks have been analyzed, send it to GUI
        // Will limit GUI freezing
        if (row % 10 == 0) {
            // this is a blocking operation
            emit rowsAppended(rows, thisModelObserver);
            qDebug() << "Append " << rows.count() << " from " << fileAccess.info();
            rows.clear();
        }
        // Sleep additionally for 10ms which prevents us from GUI freezes
        msleep(20);
    }
    emit rowsAppended(rows, thisModelObserver);
    qDebug() << "Append last " << rows.count();
}
