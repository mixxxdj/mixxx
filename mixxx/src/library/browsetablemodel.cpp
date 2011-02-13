
#include <QtCore>
#include <QtSql>
#include <QStringList>
#include <QtConcurrentRun>

#include "browsetablemodel.h"
#include "soundsourceproxy.h"
#include "mixxxutils.cpp"

//constants
const int COLUMN_FILENAME = 0;
const int COLUMN_ARTIST = 1;
const int COLUMN_TITLE = 2;
const int COLUMN_ALBUM = 3;
const int COLUMN_TRACK_NUMBER = 4;
const int COLUMN_YEAR = 5;
const int COLUMN_GENRE = 6;
const int COLUMN_COMMENT = 7;
const int COLUMN_DURATION = 8;
const int COLUMN_BPM = 9;
const int COLUMN_KEY = 10;
const int COLUMN_TYPE = 11;
const int COLUMN_BITRATE = 12;
const int COLUMN_LOCATION = 13;

BrowseTableModel::BrowseTableModel(QObject* parent)
        : QStandardItemModel(parent),
          TrackModel(QSqlDatabase::database("QSQLITE"),
                     "mixxx.db.model.browse")
{
    QStringList header_data;

    header_data.insert(COLUMN_FILENAME, tr("Filename"));
    header_data.insert(COLUMN_ARTIST, tr("Artist"));
    header_data.insert(COLUMN_TITLE, tr("Title"));
    header_data.insert(COLUMN_ALBUM, tr("Album"));
    header_data.insert(COLUMN_TRACK_NUMBER, tr("Track #"));
    header_data.insert(COLUMN_YEAR, tr("Year"));
    header_data.insert(COLUMN_GENRE, tr("Genre"));
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
    addSearchColumn(COLUMN_KEY);
    addSearchColumn(COLUMN_COMMENT);
     
    setHorizontalHeaderLabels(header_data); 

    /*
     * Start background thread.
     * Used to read the ID3 tags
     */
    m_bStopThread = false;
    m_future = QtConcurrent::run(this, &BrowseTableModel::browserThread);
    m_path = "";
}

BrowseTableModel::~BrowseTableModel()
{
    qDebug() << "Wait to finish browser background thread";
    m_bStopThread = true;
    //wake up thread since it might wait for user input
    m_locationUpdated.wakeAll();
    m_future.waitForFinished();
    qDebug() << "Browser background thread terminated!";
}

const QList<int>& BrowseTableModel::searchColumns() const {
    return m_searchColumns;
}
void BrowseTableModel::addSearchColumn(int index) {
    m_searchColumns.push_back(index);
}
void BrowseTableModel::setPath(QString absPath)
{
    m_path = absPath;
    m_locationUpdated.wakeAll();

}

TrackPointer BrowseTableModel::getTrack(const QModelIndex& index) const
{
    TrackInfoObject* tio = new TrackInfoObject(getTrackLocation(index));
    return TrackPointer(tio, &QObject::deleteLater);
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const
{
    int row = index.row();
            
    QModelIndex index2 = this->index(row, COLUMN_LOCATION);
    return data(index2).toString();
    
}

void BrowseTableModel::search(const QString& searchText)
{

}

const QString BrowseTableModel::currentSearch()
{
    return QString();
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

void BrowseTableModel::removeTrack(const QModelIndex& index)
{

}

void BrowseTableModel::removeTracks(const QModelIndexList& indices)
{

}

bool BrowseTableModel::addTrack(const QModelIndex& index, QString location)
{
    return false;
}

QMimeData* BrowseTableModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

    //Ok, so the list of indexes we're given contains separates indexes for
    //each column, so even if only one row is selected, we'll have like 7 indexes.
    //We need to only count each row once:
    QList<int> rows;

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            if (!rows.contains(index.row())) {
                rows.push_back(index.row());
                QUrl url = QUrl::fromLocalFile(getTrackLocation(index));
                if (!url.isValid())
                    qDebug() << "ERROR invalid url\n";
                else {
                    urls.append(url);
                    qDebug() << "Appending URL:" << url;
                }
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}
void BrowseTableModel::populateModel()
{
    //Refresh the name filters in case we loaded new
    //SoundSource plugins.
    QStringList nameFilters(SoundSourceProxy::supportedFileExtensionsString().split(" "));
    QDirIterator fileIt(m_path, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    QString thisPath(m_path);
    //remove all rows
    removeRows(0, rowCount());
    
    int row = 0;
    //Iterate over the files
    while (fileIt.hasNext())
    {
        /*
         * If a user quickly jumps through the folders
         * the current task becomes "dirty"
         */
        if(thisPath != m_path){
            //qDebug() << "Exit populateModel()";
            return;
        }
        QString filepath = fileIt.next();
        TrackInfoObject tio(filepath);

        QStandardItem* item = new QStandardItem(tio.getFilename());
        setItem(row, COLUMN_FILENAME, item);
           
        item = new QStandardItem(tio.getArtist());
        setItem(row, COLUMN_ARTIST, item);
          
        item = new QStandardItem(tio.getTitle());
        setItem(row, COLUMN_TITLE, item);
           
        item = new QStandardItem(tio.getAlbum());
        setItem(row, COLUMN_ALBUM, item);

        QString duration = MixxxUtils::secondsToMinutes(qVariantValue<int>(tio.getDuration()));
        item = new QStandardItem(duration);
        setItem(row, COLUMN_DURATION, item);

        item = new QStandardItem(tio.getBpmStr());
        setItem(row, COLUMN_BPM, item);
           
        item = new QStandardItem(tio.getKey());
        setItem(row, COLUMN_KEY, item);
            
        item = new QStandardItem(tio.getYear());
        setItem(row, COLUMN_YEAR, item);

        item = new QStandardItem(tio.getBitrateStr());
        setItem(row, COLUMN_BITRATE, item);
            
        item = new QStandardItem(tio.getType());
        setItem(row, COLUMN_TYPE, item);
            
        item = new QStandardItem(tio.getGenre());
        setItem(row, COLUMN_GENRE, item);

        item = new QStandardItem(tio.getTrackNumber());
        setItem(row, COLUMN_TRACK_NUMBER, item);

        item = new QStandardItem(tio.getComment());
        setItem(row, COLUMN_COMMENT, item);

        item = new QStandardItem(filepath);
        setItem(row, COLUMN_LOCATION, item);
          
        ++row;
   
    }
}
void BrowseTableModel::browserThread()
{
    //Give the thread low priority to prevent GUI freezing
    QThread* thisThread = QThread::currentThread();
    thisThread->setPriority(QThread::LowestPriority);

    while(1){
        m_mutex.lock();
        //Wait until the user has selected a folder
        m_locationUpdated.wait(&m_mutex);

        //Terminate thread if Mixxx closes
        if(m_bStopThread)
            return;

        /*
         * Populate the model
         */
        populateModel();

        m_mutex.unlock();

    }
}
