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
        : QThread{parent},
          m_pModelObserver{},
          m_runState{} {
    qDebug() << "Starting browser background thread";
    start(QThread::LowPriority);
    qDebug() << "Wait for browser background thread start";
    waitUntilThreadIsRunning();
    qDebug() << "Browser background thread has started";
}

BrowseThread::~BrowseThread() {
    qDebug() << "Request to terminate browser background thread";
    requestThreadToStop();
    qDebug() << "Wait for browser background thread to finish";
    wait();
    qDebug() << "Browser background thread terminated";
}

void BrowseThread::waitUntilThreadIsRunning() {
    m_requestMutex.lock();
    while (!m_runState) {
        m_requestCondition.wait(&m_requestMutex);
    }
    m_requestMutex.unlock();
}

void BrowseThread::requestThreadToStop() {
    m_requestMutex.lock();
    m_runState = false;
    m_requestMutex.unlock();
    m_requestCondition.wakeOne();
}

// static
BrowseThreadPointer BrowseThread::getInstanceRef() {
    // We create a single BrowseThread instead which is used by multiple
    // BrowseTableModel instances. We store a weakpointer so when all
    // BrowseTableModel have been destroyed, so is the BrowseThread.

    static QWeakPointer<BrowseThread> s_weakInstanceRef;
    static QMutex s_mutex;

    BrowseThreadPointer strong = s_weakInstanceRef.toStrongRef();

    if (!strong) {
        s_mutex.lock();
        strong = s_weakInstanceRef.toStrongRef();
        if (!strong) {
            strong = BrowseThreadPointer(new BrowseThread());
            s_weakInstanceRef = strong.toWeakRef();
        }
        s_mutex.unlock();
    }

    return strong;
}

void BrowseThread::requestPopulateModel(mixxx::FileAccess path, BrowseTableModel* pModelObserver) {
    // Inform the thread to populate a new path.
    // Note: if another request is currently being processed, this will replace it.

    qDebug() << "Request populate model" << path.info() << pModelObserver;
    m_requestMutex.lock();
    m_requestedPath = std::move(path);
    m_pModelObserver = pModelObserver;
    m_requestMutex.unlock();
    m_requestCondition.wakeOne();
}

void BrowseThread::run() {
    QThread::currentThread()->setObjectName("BrowseThread");

    // Inform the constructor the thread started
    m_requestMutex.lock();
    m_runState = true;
    m_requestMutex.unlock();
    m_requestCondition.wakeOne();

    while (true) {
        // Wait for a new population request, or for a termination request
        qDebug() << "Wait for a new population request";
        m_requestMutex.lock();
        while (!m_requestedPath.isSet() && m_runState) {
            m_requestCondition.wait(&m_requestMutex);
        }
        auto path = std::move(m_requestedPath);
        auto pModelObserver = m_pModelObserver;
        auto bRun = m_runState;
        m_requestedPath = mixxx::FileAccess();
        m_requestMutex.unlock();

        Trace trace("BrowseThread");

        if (!bRun) {
            qDebug() << "Termination request";
            // Terminate thread if Mixxx closes
            return;
        } else {
            qDebug() << "New population request";
            // Populate the model
            populateModel(path, pModelObserver);
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

void BrowseThread::populateModel(const mixxx::FileAccess& path, BrowseTableModel* pModelObserver) {
    // Executed by the thread run for incoming populate model requests

    if (!path.info().hasLocation()) {
        // Abort if the location is inaccessible or does not exist
        qWarning() << "Skipping" << path.info();
        return;
    }
    qDebug() << "populateModel" << path.info() << pModelObserver;

    // Refresh the name filters in case we loaded new SoundSource plugins.
    const QStringList nameFilters = SoundSourceProxy::getSupportedFileNamePatterns();

    QDirIterator fileIt(path.info().location(),
            nameFilters,
            QDir::Files | QDir::NoDotAndDotDot);

    // remove all rows
    // This is a blocking operation
    // see signal/slot connection in BrowseTableModel
    emit clearModel(pModelObserver);

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
        m_requestMutex.lock();
        const bool abortProcess = !m_runState ||
                (m_requestedPath.isSet() &&
                        (m_requestedPath.info() != path.info() ||
                                m_pModelObserver != pModelObserver));
        m_requestMutex.unlock();
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
            emit rowsAppended(rows, pModelObserver);
            qDebug() << "Append" << rows.count() << "tracks from "
                     << path.info().locationPath();
            rows.clear();
        }
        // Sleep additionally for 20ms which prevents us from GUI freezes
        msleep(20);
    }
    emit rowsAppended(rows, pModelObserver);
    qDebug() << "Append last" << rows.count() << "tracks from" << path.info().locationPath();
}
