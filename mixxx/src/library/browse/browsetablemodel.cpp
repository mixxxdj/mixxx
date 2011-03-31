
#include <QtCore>
#include <QtSql>
#include <QStringList>
#include <QtConcurrentRun>
#include <QMetaType>
#include <QMessageBox>

#include "library/browse/browsetablemodel.h"
#include "soundsourceproxy.h"
#include "mixxxutils.cpp"
#include "playerinfo.h"
#include "controlobject.h"
#include "library/dao/trackdao.h"
#include <QMessageBox>

BrowseTableModel::BrowseTableModel(QObject* parent, TrackCollection* pTrackCollection,
                                   RecordingManager* pRecordingManager)
        : QStandardItemModel(parent),
          m_pTrackCollection(pTrackCollection),
          m_pRecordingManager(pRecordingManager),
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
    m_current_path = absPath;
    BrowseThread::getInstance()->executePopulation(m_current_path = absPath, this);

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
    if(!index.isValid()) return;

    QString track_location = getTrackLocation(index);
    int decks = ControlObject::getControl(ConfigKey("[Master]","num_decks"))->get();
    //check if file is loaded to a deck
    for(int i=1; i <= decks; ++i){
        TrackPointer loaded_track = PlayerInfo::Instance().getTrackInfo(QString("[Channel%1]").arg(i));
        if(loaded_track && (loaded_track->getLocation() == track_location)){
            QMessageBox::critical(0, tr("Mixxx Library"), tr("Could not delete ")+track_location+
                                  tr(" because it is currently in use by Mixxx.") );
            return;
        }
    }

    //qDebug() << "Recordingmamanger: " << m_pRecordingManager;
    //Check if file is subject to a current recording operation
    if(m_pRecordingManager->getRecordingLocation() == track_location){
        QMessageBox::critical(0, tr("Mixxx Library"), tr("Could not delete ")+track_location+
                              tr(" because it is currently subject to a recording operation.") );
        return;
    }

    // try to delete track
    if(!QFile::remove(track_location)){
        QMessageBox::critical(0, tr("Mixxx Library"),"Could not delete "+track_location+
                              tr(" because it in use by Mixxx or another application") );
    }
    else
    {
        if(QMessageBox::question(NULL,tr("Mixxx Library"),
                                 tr("Do you really want to delete the following track from the filesystem?\n ")
                                 +track_location,
                                 QMessageBox::Yes, QMessageBox::Abort
                                 ) == QMessageBox::Abort)
        {
            return;
        }

        qDebug() << "BrowseFeature: User deleted track " << track_location;
        //repopulate model
        BrowseThread::getInstance()->executePopulation(m_current_path, this);

        //If the track was contained in the Mixxx library, delete it
        TrackDAO& track_dao = m_pTrackCollection->getTrackDAO();
        if(track_dao.trackExistsInDatabase(track_location))
        {
            int id = track_dao.getTrackId(track_location);
            qDebug() << "BrowseFeature: Deletion affected database";
            track_dao.removeTrack(id);
        }
    }
}

void BrowseTableModel::removeTracks(const QModelIndexList& indices)
{
    foreach (QModelIndex index, indices) {
       removeTrack(index);
    }
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
TrackModel::CapabilitiesFlags BrowseTableModel::getCapabilities() const
{
    return TRACKMODELCAPS_NONE;
}
Qt::ItemFlags BrowseTableModel::flags(const QModelIndex &index) const{

    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    //Enable dragging songs from this data model to elsewhere (like the waveform
    //widget to load a track into a Player).
    defaultFlags |= Qt::ItemIsDragEnabled;

    int row = index.row();
    int column = index.column();

    if(column == COLUMN_FILENAME ||
            column == COLUMN_BITRATE ||
            column == COLUMN_DURATION ||
            column == COLUMN_TYPE){
        return defaultFlags;
    }
    else{
        return defaultFlags | Qt::ItemIsEditable;
    }


}
