
#include <QtCore>
#include <QtSql>
#include <QStringList>

#include "browsetablemodel.h"
#include "soundsourceproxy.h"
#include "mixxxutils.cpp"


BrowseTableModel::BrowseTableModel(QObject* parent)
        : QStandardItemModel(parent),
          TrackModel(QSqlDatabase::database("QSQLITE"),
                     "mixxx.db.model.browse")
{
    QStringList header_data;
    header_data << "Filename" << "Artist" << "Title" << "Album" 
        << "Year" << "BPM" << "Key" << "Duration" << "Type"
        << "Bitrate" << "Genre" << "Track #" << "Comment"
        << "Location";
    setHorizontalHeaderLabels(header_data); 
}

BrowseTableModel::~BrowseTableModel()
{

}

void BrowseTableModel::setPath(QString absPath)
{
    //Refresh the name filters in case we loaded new
    //SoundSource plugins.
    QStringList nameFilters(SoundSourceProxy::supportedFileExtensionsString().split(" "));
    QDirIterator fileIt(absPath, nameFilters, QDir::Files | QDir::NoDotAndDotDot);
    
    //remove all rows
    removeRows(0, rowCount());
    
    int row = 0;
    //Iterate over the files
    while (fileIt.hasNext())
    {
        QString filepath = fileIt.next();
        TrackInfoObject tio(filepath);
        
        int columns = columnCount();
        //Note that headers can change via context menus
        //Also note that the order can change
        
        for(int column=0; column < columns; ++column){
        
            QString header = horizontalHeaderItem(column)->text();
            if(header == "Filename"){
                QStandardItem* item = new QStandardItem(tio.getFilename());
                setItem(row, column, item);
            }
            if(header == "Artist"){
                QStandardItem* item = new QStandardItem(tio.getArtist());
                setItem(row, column, item);
            }
            if(header == "Title"){
                QStandardItem* item = new QStandardItem(tio.getTitle());
                setItem(row, column, item);
            }
            if(header == "Album"){
                QStandardItem* item = new QStandardItem(tio.getAlbum());
                setItem(row, column, item);
            }
            if(header == "BPM"){
                QStandardItem* item = new QStandardItem(tio.getBpmStr());
                setItem(row, column, item);
            }
            if(header == "Key"){
                QStandardItem* item = new QStandardItem(tio.getKey());
                setItem(row, column, item);
            }
            if(header == "Year"){
                QStandardItem* item = new QStandardItem(tio.getYear());
                setItem(row, column, item);
            }
            if(header == "Duration"){
                QString duration = MixxxUtils::secondsToMinutes(qVariantValue<int>(tio.getDuration()));
                QStandardItem* item = new QStandardItem(duration);
                setItem(row, column, item);
            }
            if(header == "Bitrate"){
                QStandardItem* item = new QStandardItem(tio.getBitrateStr());
                setItem(row, column, item);
            }
            if(header == "Type"){
                QStandardItem* item = new QStandardItem(tio.getType());
                setItem(row, column, item);
            }
            if(header == "Genre"){
                QStandardItem* item = new QStandardItem(tio.getGenre());
                setItem(row, column, item);
            }
            if(header == "Track #"){
                QStandardItem* item = new QStandardItem(tio.getTrackNumber());
                setItem(row, column, item);
            }
            if(header == "Comment"){
                QStandardItem* item = new QStandardItem(tio.getComment());
                setItem(row, column, item);
            }
            if(header == "Location"){
                QStandardItem* item = new QStandardItem(filepath);
                setItem(row, column, item);
            }
        }
        ++row;    
    }   
}

TrackPointer BrowseTableModel::getTrack(const QModelIndex& index) const
{
    TrackInfoObject* tio = new TrackInfoObject();
    int row = index.row();
    int columns = columnCount();
    //Note that headers can change via context menus
    //Also note that the order can change
        
    for(int column=0; column < columns; ++column){
        QString header = horizontalHeaderItem(column)->text();
        if(header == "Location"){
            tio->setLocation(item(row, column)->text());
        }
    }
        
	
	
	
    

    return TrackPointer(tio, &QObject::deleteLater);
}

QString BrowseTableModel::getTrackLocation(const QModelIndex& index) const
{
    return 0;
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
	//TODO
}

void BrowseTableModel::removeTracks(const QModelIndexList& indices)
{
	//TODO
}

bool BrowseTableModel::addTrack(const QModelIndex& index, QString location)
{
	//TODO
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
