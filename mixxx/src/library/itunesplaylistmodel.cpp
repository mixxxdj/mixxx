// itunesplaylistmodel.cpp
// Created 12/19/2009 by RJ Ryan (rryan@mit.edu)
// Adapted from Philip Whelan's RhythmboxPlaylistModel

#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QtXmlPatterns/QXmlQuery>

#include "library/itunesplaylistmodel.h"
#include "library/itunestrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"

#include "mixxxutils.cpp"

ITunesPlaylistModel::ITunesPlaylistModel(ITunesTrackModel *pTrackModel) :
        TrackModel(QSqlDatabase::database("QSQLITE"), "mixxx.db.model.itunes_playlist"),
        m_pTrackModel(pTrackModel),
        m_sCurrentPlaylist("")
{

}

ITunesPlaylistModel::~ITunesPlaylistModel()
{
}

Qt::ItemFlags ITunesPlaylistModel::flags ( const QModelIndex & index ) const
{
    return QAbstractTableModel::flags(index);
}

QVariant ITunesPlaylistModel::data ( const QModelIndex & index, int role ) const
{
    if ( m_sCurrentPlaylist == "" )
        return QVariant();

    if (!index.isValid())
        return QVariant();

    TrackInfoObject *pTrack = getTrack(index);
    if ( pTrack == NULL )
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        switch (index.column()) {
            case ITunesPlaylistModel::COLUMN_ARTIST:
                return pTrack->getArtist();
            case ITunesPlaylistModel::COLUMN_TITLE:
                return pTrack->getTitle();
            case ITunesPlaylistModel::COLUMN_ALBUM:
                return pTrack->getAlbum();
            case ITunesPlaylistModel::COLUMN_DATE:
                return pTrack->getYear();
            case ITunesPlaylistModel::COLUMN_BPM:
                return pTrack->getBpm();
            case ITunesPlaylistModel::COLUMN_GENRE:
                return pTrack->getGenre();
            case ITunesPlaylistModel::COLUMN_LOCATION:
                return pTrack->getLocation();
            case ITunesPlaylistModel::COLUMN_DURATION:
                return MixxxUtils::secondsToMinutes(pTrack->getDuration());
            default:
                return QVariant();
        }
    }

    return QVariant();
}

bool ITunesPlaylistModel::isColumnInternal(int column) {
    return false;
}

QVariant ITunesPlaylistModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    /* Only respond to requests for column header display names */
    if ( role != Qt::DisplayRole )
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case ITunesPlaylistModel::COLUMN_ARTIST:
                return QString(tr("Artist"));

            case ITunesPlaylistModel::COLUMN_TITLE:
                return QString(tr("Title"));

            case ITunesPlaylistModel::COLUMN_ALBUM:
                return QString(tr("Album"));

            case ITunesPlaylistModel::COLUMN_DATE:
                return QString(tr("Date"));

            case ITunesPlaylistModel::COLUMN_BPM:
                return QString(tr("BPM"));

            case ITunesPlaylistModel::COLUMN_GENRE:
                return QString(tr("Genre"));

            case ITunesPlaylistModel::COLUMN_LOCATION:
                return QString(tr("Location"));

            case ITunesPlaylistModel::COLUMN_DURATION:
                return QString(tr("Duration"));

            default:
                return QString(tr("Unknown"));
        }
    }

    return QVariant();
}

int ITunesPlaylistModel::rowCount ( const QModelIndex & parent ) const
{
    // FIXME
    //if ( !m_mPlaylists.containts(m_sCurrentPlaylist))
    //    return 0;

    if (!m_pTrackModel || m_sCurrentPlaylist == "" )
        return 0;

    return m_pTrackModel->m_mPlaylists[m_sCurrentPlaylist].size();
}

int ITunesPlaylistModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return ITunesPlaylistModel::NUM_COLUMNS;
}

bool ITunesPlaylistModel::addTrack(const QModelIndex& index, QString location)
{
    //Should do nothing... hmmm
    return false;
}

/** Removes a track from the library track collection. */
void ITunesPlaylistModel::removeTrack(const QModelIndex& index)
{
    //Should do nothing... hmmm
}

void ITunesPlaylistModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
    //Should do nothing... hmmm
}

QString ITunesPlaylistModel::getTrackLocation(const QModelIndex& index) const
{
    //FIXME
    return QString();
}

TrackInfoObject * ITunesPlaylistModel::getTrack(const QModelIndex& index) const
{
    int row = index.row();

    if (!m_pTrackModel ||
        !m_pTrackModel->m_mPlaylists.contains(m_sCurrentPlaylist)) {
        return NULL;
    }

    // Qt should do this by reference for us so we aren't actually making a copy
    // of the list.
    QList<QString> songIds = m_pTrackModel->m_mPlaylists[m_sCurrentPlaylist];

    if (row < 0 || row >= songIds.length()) {
        return NULL;
    }

    return m_pTrackModel->getTrackById(songIds.at(row));
}

QItemDelegate* ITunesPlaylistModel::delegateForColumn(const int i) {
    return NULL;
}

QList<QString> ITunesPlaylistModel::getPlaylists()
{
    if (!m_pTrackModel) {
        return QList<QString>();
    }
    return m_pTrackModel->m_mPlaylists.keys();
}

int ITunesPlaylistModel::numPlaylists() {
    if (!m_pTrackModel) {
            return 0;
    }
    return m_pTrackModel->m_mPlaylists.size();
}

QString ITunesPlaylistModel::playlistTitle(int n) {
    if (!m_pTrackModel) {
        return "";
    }
    return m_pTrackModel->m_mPlaylists.keys().at(n);
}

void ITunesPlaylistModel::setPlaylist(QString playlist)
{
    if (m_pTrackModel && m_pTrackModel->m_mPlaylists.contains(playlist))
        m_sCurrentPlaylist = playlist;
    else
        m_sCurrentPlaylist = "";

    // force the layout to update
    emit(layoutChanged());
}

void ITunesPlaylistModel::search(const QString& searchText)
{
    m_currentSearch = searchText;
}

const QString ITunesPlaylistModel::currentSearch() {
    return m_currentSearch;
}

const QList<int>& ITunesPlaylistModel::searchColumns() const {
    return m_pTrackModel->searchColumns();
}
