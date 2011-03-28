
#include <QtCore>
#include <QtSql>
#include <QStringList>
#include <QtConcurrentRun>
#include <QMetaType>

#include "library/browse/browsetablemodel.h"
#include "soundsourceproxy.h"
#include "mixxxutils.cpp"

BrowseTableModel::BrowseTableModel(QObject* parent)
        : QStandardItemModel(parent),
          TrackModel(QSqlDatabase::database("QSQLITE"),
                     "mixxx.db.model.browse") {
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
    //register the QList<T> as a metatype since we use QueuedConnection below
    qRegisterMetaType< QList< QList<QStandardItem*> > >("QList< QList<QStandardItem*> >");
    qRegisterMetaType<BrowseTableModel*>("BrowseTableModel*");

    QObject::connect(BrowseThread::getInstance(), SIGNAL(clearModel(BrowseTableModel*)),
                     this, SLOT(slotClear(BrowseTableModel*)), Qt::QueuedConnection);

    QObject::connect(BrowseThread::getInstance(), SIGNAL(rowsAppended(const QList< QList<QStandardItem*> >&, BrowseTableModel*)),
            this, SLOT(slotInsert(const QList< QList<QStandardItem*> >&, BrowseTableModel*)), Qt::QueuedConnection);

}

BrowseTableModel::~BrowseTableModel()
{

}

const QList<int>& BrowseTableModel::searchColumns() const {
    return m_searchColumns;
}
void BrowseTableModel::addSearchColumn(int index) {
    m_searchColumns.push_back(index);
}
void BrowseTableModel::setPath(QString absPath)
{
   BrowseThread::getInstance()->executePopulation(absPath, this);

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
int BrowseTableModel::getTrackId(const QModelIndex& index) const {
    // We can't implement this as it stands.
    return -1;
}

int BrowseTableModel::getTrackRow(int trackId) const {
    // We can't implement this as it stands.
    return -1;
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

void BrowseTableModel::slotClear(BrowseTableModel* caller_object)
{
    if(caller_object == this)
        removeRows(0, rowCount());
}


void BrowseTableModel::slotInsert(const QList< QList<QStandardItem*> >& rows, BrowseTableModel* caller_object){
    //There exists more than one BrowseTableModel in Mixxx
    //We only want to receive items here, this object has 'ordered' by the BrowserThread (singleton)
    if(caller_object == this){
        //qDebug() << "BrowseTableModel::slotInsert";
        for(int i=0; i < rows.size(); ++i){
            appendRow(rows.at(i));
        }
    }

}
