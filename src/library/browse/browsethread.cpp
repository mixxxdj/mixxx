#include "library/browse/browsethread.h"

#include <QDirIterator>
#include <QStringList>
#include <QtDebug>

#include "library/browse/browsetablemodel.h"
#include "moc_browsethread.cpp"
#include "sources/soundsourceproxy.h"
#include "util/datetime.h"
#include "util/trace.h"

namespace {
constexpr int kRowBatchSize = 10;
} // namespace

/*
 * This class is a singleton and represents a thread
 * that is used to read ID3 metadata
 * from a particular folder.
 *
 * The BrowseTableModel uses this class.
 * Note: Don't call getInstanceRef() from places
 * other than the GUI thread. BrowseThreads emit
 * signals to BrowseModel objects. It does not
 * make sense to use this class in non-GUI threads
 */
BrowseThread::BrowseThread(QObject* parent)
        : QThread{parent}, m_modelObserver{}, m_bRun{} {
    // Start thread
    start(QThread::LowPriority);
    qDebug() << "Wait to start browser background thread";
    // Wait until the thread is running
    m_mutex.lock();
    while (!m_bRun) {
        m_condition.wait(&m_mutex);
    }
    m_mutex.unlock();
    qDebug() << "Browser background thread started";
}

BrowseThread::~BrowseThread() {
    qDebug() << "Wait to terminate browser background thread";
    // Inform the thread we want it to terminate
    m_mutex.lock();
    m_bRun = false;
    m_condition.wakeOne();
    m_mutex.unlock();
    // Wait until thread terminated
    wait();
    qDebug() << "Browser background thread terminated";
}

// static
BrowseThreadPointer BrowseThread::getInstanceRef() {
    // We create a single BrowseThread instead which is used by multiple
    // BrowseTableModel instances. We store a weakpointer so when all
    // BrowseTableModel have been destroyed, so it the BrowseThread.

    static QWeakPointer<BrowseThread> s_weakInstanceRef;
    static QMutex s_mutex;

    s_mutex.lock();
    BrowseThreadPointer strong = s_weakInstanceRef.toStrongRef();
    if (!strong) {
        strong = BrowseThreadPointer(new BrowseThread());
        s_weakInstanceRef = strong.toWeakRef();
    }
    s_mutex.unlock();

    return strong;
}

void BrowseThread::requestPopulateModel(mixxx::FileAccess path, BrowseTableModel* modelObserver) {
    // Inform the thread to populate a new path.
    // Note: if another request is currently being processed, this will replace it.

    qDebug() << "Request populate model" << path.info() << modelObserver;
    m_mutex.lock();
    m_path = std::move(path);
    m_modelObserver = modelObserver;
    m_condition.wakeOne();
    m_mutex.unlock();
}

void BrowseThread::run() {
    QThread::currentThread()->setObjectName("BrowseThread");

    // Inform the constructor the thread started
    m_mutex.lock();
    m_bRun = true;
    m_condition.wakeOne();
    m_mutex.unlock();

    while (true) {
        // Wait for a new population request, or for a termination request
        qDebug() << "Wait for a new population request";
        m_mutex.lock();
        while (!m_path.isSet() && m_bRun) {
            m_condition.wait(&m_mutex);
        }
        auto path = std::move(m_path);
        auto modelObserver = m_modelObserver;
        auto bRun = m_bRun;
        m_path = mixxx::FileAccess();
        m_mutex.unlock();

        Trace trace("BrowseThread");

        if (!bRun) {
            qDebug() << "Termination request";
            // Terminate thread if Mixxx closes
            return;
        } else {
            qDebug() << "New population request";
            // Populate the model
            populateModel(path, modelObserver);
        }
    }
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

void BrowseThread::populateModel(mixxx::FileAccess path, BrowseTableModel* modelObserver) {
    // Executed by the thread run for incoming populate model requests

    if (!path.info().hasLocation()) {
        // Abort if the location is inaccessible or does not exist
        qWarning() << "Skipping" << path.info();
        return;
    }
    qDebug() << "populateModel" << path.info() << modelObserver;

    // Refresh the name filters in case we loaded new SoundSource plugins.
    const QStringList nameFilters = SoundSourceProxy::getSupportedFileNamePatterns();

    QDirIterator fileIt(path.info().location(),
            nameFilters,
            QDir::Files | QDir::NoDotAndDotDot);

    // remove all rows
    // This is a blocking operation
    // see signal/slot connection in BrowseTableModel
    emit clearModel(modelObserver);

    QList<QList<QStandardItem*>> rows;
    rows.reserve(kRowBatchSize);
    QList<QStandardItem*> row_data;
    row_data.reserve(NUM_COLUMNS);

    int row = 0;
    // Iterate over the files
    while (fileIt.hasNext()) {
        // If while processing a new population request or a termination request
        // was received, we abort the current request. This happens if a user
        // quickly jumps through the folders and the current task becomes
        // "dirty".
        m_mutex.lock();
        const bool abortProcess = !m_bRun ||
                (m_path.isSet() &&
                        (m_path.info() != path.info() ||
                                modelObserver != m_modelObserver));
        m_mutex.unlock();
        if (abortProcess) {
            qDebug() << "Abort populateModel";
            // The run function will take care of the new request.
            return;
        }

        QStandardItem* item = new QStandardItem("0");
        item->setData("0", Qt::UserRole);
        row_data.insert(COLUMN_PREVIEW, item);

        const auto fileAccess = mixxx::FileAccess(
                mixxx::FileInfo(fileIt.next()),
                path.token());
        {
            mixxx::TrackMetadata trackMetadata;
            // Both resetMissingTagMetadata = false/true have the same effect
            constexpr auto resetMissingTagMetadata = false;
            SoundSourceProxy::importTrackMetadataAndCoverImageFromFile(
                    fileAccess,
                    &trackMetadata,
                    nullptr,
                    resetMissingTagMetadata);

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
            const mixxx::Bpm bpm = trackMetadata.getTrackInfo().getBpm();
            item->setData(bpm.isValid() ? bpm.value() : mixxx::Bpm::kValueUndefined, Qt::UserRole);
            row_data.insert(COLUMN_BPM, item);

            item = new QStandardItem(trackMetadata.getTrackInfo().getKeyText());
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
        row_data.clear();
        ++row;
        // If 10 tracks have been analyzed, send it to GUI
        // Will limit GUI freezing
        if (row % kRowBatchSize == 0) {
            // this is a blocking operation
            emit rowsAppended(rows, modelObserver);
            qDebug() << "Append" << rows.count() << "tracks from "
                     << path.info().locationPath();
            rows.clear();
        }
        // Sleep additionally for 20ms which prevents us from GUI freezes
        msleep(20);
    }
    emit rowsAppended(rows, modelObserver);
    qDebug() << "Append last" << rows.count() << "tracks from" << path.info().locationPath();
}
