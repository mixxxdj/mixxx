
#include <QtCore>
#include <QtSql>
#include <QStringList>

#include "browsetablemodel.h"
#include "soundsourceproxy.h"


BrowseTableModel::BrowseTableModel(QObject* parent)
        : QStandardItemModel(parent),
          TrackModel(QSqlDatabase::database("QSQLITE"),
                     "mixxx.db.model.browse")
{
    QStringList header_data;
    header_data << "Filename" << "Artist" << "Title" << "Album" << "BPM" << "Key" << "Year";
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
    
    
    //Iterate over the files
    while (fileIt.hasNext())
    {
        QString filepath = fileIt.next();
        
        TrackInfoObject tio(filepath);
        QList<QStandardItem*> row;
        QStandardItem* filename_item = new QStandardItem(filepath);
        QStandardItem* artist_item = new QStandardItem(tio.getArtist());
        
        row << filename_item << artist_item;
        appendRow(row);
        
    }
    
}

TrackPointer BrowseTableModel::getTrack(const QModelIndex& index) const
{
	

    return TrackPointer();
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
