#include <qstringlist.h>

#include <QUrl>

#include "wplaylistlistmodel.h"
#include "trackcollection.h"
#include "trackinfoobject.h"
#include "trackplaylist.h"
#include "trackplaylistlist.h"

WPlaylistListModel::WPlaylistListModel(QObject * parent) : QAbstractTableModel(parent)
{
    rowColors = false;
    //setHeaderData(0,Qt::Horizontal, tr("**"));
    setHeaderData(0,Qt::Horizontal, tr("Name"));
    setHeaderData(1,Qt::Horizontal, tr("Type"));
    setHeaderData(2,Qt::Horizontal, tr("Duration"));
    setHeaderData(3,Qt::Horizontal, tr("Comment"));
}

WPlaylistListModel::~WPlaylistListModel()
{
}
void WPlaylistListModel::setPlaylistList(TrackPlaylistList *pPlaylists)
{
    m_qPlaylists = pPlaylists;
}

int WPlaylistListModel::rowCount(const QModelIndex &parent) const
{
    return m_qPlaylists->count();
}

int WPlaylistListModel::columnCount(const QModelIndex &parent) const
{
    return 4;
}

QVariant WPlaylistListModel::data(const QModelIndex &index, int role) const
{
    //TrackInfoObject * m_pTrackInfo = m_pTrackPlaylist->getTrackAt(index.row());
    TrackPlaylist* m_pPlaylist = m_qPlaylists->at(index.row());

    if (!index.isValid()) 
        return QVariant();

    if (index.row() >= m_qPlaylists->count())
        return QVariant();

    /*
    if (role == Qt::ForegroundRole )
    {
        return foregroundColor;
    }*/
    else if (role == Qt::DisplayRole )
    {
        switch(index.column())
        {
        //case 0: return m_pTrackInfo->getScoreStr(); 
        
        case 0: return m_pPlaylist->getName();
        case 1: return "type";
        case 2: return "duration";
        case 3: return m_pPlaylist->getComment();
		default: return "Error: WPlaylistListModel::data invalid index";
        }
    }

    else
        return QVariant();
}

QVariant WPlaylistListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch(section)
        {
        //case 0:
        //    return QString("**");
        case 0:
            return QString("Name");
        case 1:
            return QString("Type");
        case 2:
            return QString("Duration");
        case 3:
            return QString("Comment"); 
		default:
			//this is a nasty error for the user to see, but its better than a crash and should help with debugging
			return QString("ERROR: WPlaylistListMode::headerData Invalid section parameter");	
        }
    }
    else
        return QString("%1").arg(section);
}

Qt::ItemFlags WPlaylistListModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (!index.isValid())
      return Qt::ItemIsEnabled;

    defaultFlags |= Qt::ItemIsDragEnabled;
    switch(index.column())
    {
      case 3: return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
      //case 6: return defaultFlags | QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    return defaultFlags;
}


bool WPlaylistListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        TrackPlaylist* m_pPlaylist = m_qPlaylists->at(index.row());

        switch(index.column())
        {
            case 3: m_pPlaylist->setComment(value.toString()); break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool WPlaylistListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    TrackPlaylist* m_pPlaylist = m_qPlaylists->at(index.row());
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        m_qPlaylists->remove(m_pPlaylist);

    endRemoveRows();
    return true;
}

Qt::DropActions WPlaylistListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction; //FIXME: Should we have dnd in this model?
}


QMimeData *WPlaylistListModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls;

/* This doesn't make any sense (yet?) for the WPlaylistListModel....

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            TrackPlaylist* m_pPlaylist = m_qPlaylists->at(index.row());
            QUrl url(QString(m_pTrackInfo->getLocation()));
            if (!url.isValid())
              qDebug("ERROR invalid url\n");
            else
              urls.append(url);
        }
    }
    mimeData->setUrls(urls);
    */
    
    
    return mimeData;
}

void WPlaylistListModel::setBackgroundColor(QColor bgColor)
{
    backgroundColor = bgColor;
}

void WPlaylistListModel::setForegroundColor(QColor fgColor)
{
    foregroundColor = fgColor;
}

void WPlaylistListModel::setRowColor(QColor evenColor, QColor unevenColor)
{
    backgroundColor = evenColor;
    rowUnevenColor = unevenColor;
}

void WPlaylistListModel::setBpmColor(QColor confirmColor, QColor noConfirmColor)
{
    bpmNoConfirmColor = noConfirmColor;
    bpmConfirmColor = confirmColor;
    rowColors = true;
}
