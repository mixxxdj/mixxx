/*
 * browsethread.cpp         (C) 2011 Tobias Rafreider
 */

#include <QtDebug>
#include <QStringList>
#include <QDateTime>
#include <QDirIterator>

#include "library/browse/browsethread.h"
#include "library/browse/browsetablemodel.h"
#include "soundsourceproxy.h"
#include "util/time.h"
#include "util/trace.h"

BrowseThread* BrowseThread::m_instance = NULL;
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
    m_model_observer = NULL;
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
BrowseThread* BrowseThread::getInstance() {
    if (!m_instance)
    {
        s_Mutex.lock();

         if (!m_instance)
               m_instance = new BrowseThread();

         s_Mutex.unlock();
    }
    return m_instance;
}
void BrowseThread::destroyInstance() {
    s_Mutex.lock();
    if(m_instance){
        delete m_instance;
        m_instance = 0;
    }
    s_Mutex.unlock();
}

void BrowseThread::executePopulation(const MDir& path, BrowseTableModel* client) {
    m_path_mutex.lock();
    m_path = path;
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

void BrowseThread::populateModel() {
    m_path_mutex.lock();
    MDir thisPath = m_path;
    BrowseTableModel* thisModelObserver = m_model_observer;
    m_path_mutex.unlock();

    // Refresh the name filters in case we loaded new SoundSource plugins.
    QStringList nameFilters(SoundSourceProxy::supportedFileExtensionsString().split(" "));

    QDirIterator fileIt(thisPath.dir().canonicalPath(), nameFilters,
                        QDir::Files | QDir::NoDotAndDotDot);

    // remove all rows
    // This is a blocking operation
    // see signal/slot connection in BrowseTableModel
    emit(clearModel(thisModelObserver));

    QList< QList<QStandardItem*> > rows;

    int row = 0;
    // Iterate over the files
    while (fileIt.hasNext()) {
        // If a user quickly jumps through the folders
        // the current task becomes "dirty"
        m_path_mutex.lock();
        MDir newPath = m_path;
        m_path_mutex.unlock();

        if (thisPath.dir() != newPath.dir()) {
            qDebug() << "Abort populateModel()";
            return populateModel();
        }

        QString filepath = fileIt.next();
        TrackInfoObject tio(filepath, thisPath.token());
        QList<QStandardItem*> row_data;

        QStandardItem* item = new QStandardItem(tio.getFilename());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_FILENAME, item);

        item = new QStandardItem(tio.getArtist());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_ARTIST, item);

        item = new QStandardItem(tio.getTitle());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_TITLE, item);

        item = new QStandardItem(tio.getAlbum());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_ALBUM, item);

        item = new QStandardItem(tio.getAlbumArtist());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_ALBUMARTIST, item);

        item = new QStandardItem(tio.getTrackNumber());
        item->setToolTip(item->text());
        item->setData(item->text().toInt(), Qt::UserRole);
        row_data.insert(COLUMN_TRACK_NUMBER, item);

        item = new QStandardItem(tio.getYear());
        item->setToolTip(item->text());
        item->setData(item->text().toInt(), Qt::UserRole);
        row_data.insert(COLUMN_YEAR, item);

        item = new QStandardItem(tio.getGenre());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_GENRE, item);

        item = new QStandardItem(tio.getComposer());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_COMPOSER, item);

        item = new QStandardItem(tio.getGrouping());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_GROUPING, item);

        item = new QStandardItem(tio.getComment());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_COMMENT, item);

        QString duration = Time::formatSeconds(qVariantValue<int>(
                tio.getDuration()), false);
        item = new QStandardItem(duration);
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_DURATION, item);

        item = new QStandardItem(tio.getBpmStr());
        item->setToolTip(item->text());
        item->setData(tio.getBpm(), Qt::UserRole);
        row_data.insert(COLUMN_BPM, item);

        item = new QStandardItem(tio.getKeyText());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_KEY, item);

        item = new QStandardItem(tio.getType());
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_TYPE, item);

        item = new QStandardItem(tio.getBitrateStr());
        item->setToolTip(item->text());
        item->setData(tio.getBitrate(), Qt::UserRole);
        row_data.insert(COLUMN_BITRATE, item);

        item = new QStandardItem(filepath);
        item->setToolTip(item->text());
        item->setData(item->text(), Qt::UserRole);
        row_data.insert(COLUMN_LOCATION, item);

        QDateTime modifiedTime = tio.getFileModifiedTime().toLocalTime();
        item = new QStandardItem(modifiedTime.toString(Qt::DefaultLocaleShortDate));
        item->setToolTip(item->text());
        item->setData(modifiedTime, Qt::UserRole);
        row_data.insert(COLUMN_FILE_MODIFIED_TIME, item);

        QDateTime creationTime = tio.getFileCreationTime().toLocalTime();
        item = new QStandardItem(creationTime.toString(Qt::DefaultLocaleShortDate));
        item->setToolTip(item->text());
        item->setData(creationTime, Qt::UserRole);
        row_data.insert(COLUMN_FILE_CREATION_TIME, item);

        rows.append(row_data);
        ++row;
        // If 10 tracks have been analyzed, send it to GUI
        // Will limit GUI freezing
        if (row % 10 == 0) {
            // this is a blocking operation
            emit(rowsAppended(rows, thisModelObserver));
            //qDebug() << "Append " << rows.count() << " from " << filepath;
            rows.clear();
        }
        // Sleep additionally for 10ms which prevents us from GUI freezes
        msleep(20);
    }
    emit(rowsAppended(rows, thisModelObserver));
    //qDebug() << "Append last " << rows.count() << " from " << thisPath;
}
