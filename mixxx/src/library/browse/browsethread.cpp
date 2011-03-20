#include <QStringList>
#include <QDirIterator>
#include <QtCore>

#include "library/browse/browsethread.h"
#include "library/browse/browsetablemodel.h"
#include "soundsourceproxy.h"
#include "mixxxutils.cpp"

BrowseThread::BrowseThread(QObject *parent)
        : QThread(parent),
          m_bStopThread(false) {
    //QObject::moveToThread(this);
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

void BrowseThread::setPath(QString& path) {
    m_path = path;
    m_locationUpdated.wakeAll();
}

void BrowseThread::run() {
    while(!m_bStopThread) {
        m_mutex.lock();
        //Wait until the user has selected a folder
        m_locationUpdated.wait(&m_mutex);

        //Terminate thread if Mixxx closes
        if(m_bStopThread)
            return;
        // Populate the model
        populateModel();
        m_mutex.unlock();
    }
}

void BrowseThread::populateModel() {
    //Refresh the name filters in case we loaded new
    //SoundSource plugins.
    QStringList nameFilters(SoundSourceProxy::supportedFileExtensionsString().split(" "));
    QDirIterator fileIt(m_path, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    QString thisPath(m_path);

    /*
     * remove all rows
     * This is a blocking operation
     * see signal/slot connection in BrowseTableModel
     */
    emit(clearModel());


    int row = 0;
    //Iterate over the files
    while (fileIt.hasNext())
    {
        /*
         * If a user quickly jumps through the folders
         * the current task becomes "dirty"
         */
        if(thisPath != m_path){
            qDebug() << "Exit populateModel()";
            return populateModel();
        }

        QString filepath = fileIt.next();
        TrackInfoObject tio(filepath);
        QList<QStandardItem*> column_data;

        QStandardItem* item = new QStandardItem(tio.getFilename());
        column_data.insert(COLUMN_FILENAME, item);

        item = new QStandardItem(tio.getArtist());
        column_data.insert(COLUMN_ARTIST, item);

        item = new QStandardItem(tio.getTitle());
        column_data.insert(COLUMN_TITLE, item);

        item = new QStandardItem(tio.getAlbum());
        column_data.insert(COLUMN_ALBUM, item);

        item = new QStandardItem(tio.getTrackNumber());
        column_data.insert(COLUMN_TRACK_NUMBER, item);

        item = new QStandardItem(tio.getYear());
        column_data.insert(COLUMN_YEAR, item);

        item = new QStandardItem(tio.getGenre());
        column_data.insert(COLUMN_GENRE, item);

        item = new QStandardItem(tio.getComment());
        column_data.insert(COLUMN_COMMENT, item);

        QString duration = MixxxUtils::secondsToMinutes(qVariantValue<int>(tio.getDuration()));
        item = new QStandardItem(duration);
        column_data.insert(COLUMN_DURATION, item);

        item = new QStandardItem(tio.getBpmStr());
        column_data.insert(COLUMN_BPM, item);

        item = new QStandardItem(tio.getKey());
        column_data.insert(COLUMN_KEY, item);

        item = new QStandardItem(tio.getType());
        column_data.insert(COLUMN_TYPE, item);

        item = new QStandardItem(tio.getBitrateStr());
        column_data.insert(COLUMN_BITRATE, item);

        item = new QStandardItem(filepath);
        column_data.insert(COLUMN_LOCATION, item);

        // this is a blocking operation
        // see signal/slot connection in BrowseTableModel
        emit(rowDataAppended(column_data));

        //QCoreApplication::processEvents();
        ++row;

        //Sleep for 50ms which prevents us from GUI freezes
        msleep(50);
    }
}
