/***************************************************************************
                           rhythmboxPlaylistsource.cpp
                              -------------------
     begin                : 8/17/2009
     copyright            : (C) 2009 Phillip Whelan
     email                : pwhelan@mixxx.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QtXmlPatterns/QXmlQuery>

#include "library/rhythmboxtrackmodel.h"
#include "library/rhythmboxplaylistmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"

RhythmboxPlaylistModel::RhythmboxPlaylistModel(RhythmboxTrackModel *Rhythhmbox) :
    m_pRhythmbox(Rhythhmbox),
    m_sCurrentPlaylist("")
{
    int idx = 0;
    QDomDocument rhythmplaylistdb;
    QXmlQuery query;
    QString res;

    QFile db(QDir::homePath() + "/.gnome2/rhythmbox/playlists.xml");
    if ( ! db.exists()) {
        db.setFileName(QDir::homePath() + "/.local/share/rhythmbox/playlists.xml");
        if ( ! db.exists())
            return;
    }

    if (!db.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    /*
     * Use QXmlQuery to execute an XPath query. We add the version to
     * the XPath query to make sure it is the schema we expect.
     */
    query.setFocus(&db);
    query.setQuery("rhythmdb-playlists/playlist[@type='static']");
    if ( ! query.isValid())
        return;

    query.evaluateTo(&res);
    db.close();

    rhythmplaylistdb.setContent("<rhythmdb-playlists>" + res + "</rhythmdb-playlists>");
    m_playlistNodes = rhythmplaylistdb.elementsByTagName("playlist");


    /* Add Playlists to the internal playlist map */
    while (( idx < m_playlistNodes.count())) {
        QDomNode n = m_playlistNodes.item(idx);
        QDomElement e = n.toElement();

        qDebug() << "Adding Rhythmbox Playlist" << e.attribute("name");
        QString playlist = e.attribute("name");

        m_mPlaylists[playlist] = n.childNodes();
        m_sCurrentPlaylist = playlist;

        idx++;
    }


    qDebug() << rhythmplaylistdb.doctype().name();
    qDebug() << "RhythmboxPlaylistModel: m_playlistNodes size is" << m_playlistNodes.size();

    QMapIterator<QString, QDomNodeList>iter (m_mPlaylists);
    while (iter.hasNext())
    {
        iter.next();
        qDebug() << "RB Playlist: " << iter.key();
    }
}

RhythmboxPlaylistModel::~RhythmboxPlaylistModel()
{
}

Qt::ItemFlags RhythmboxPlaylistModel::flags ( const QModelIndex & index ) const
{
    return QAbstractTableModel::flags(index);
}

QVariant RhythmboxPlaylistModel::data ( const QModelIndex & index, int role ) const
{
    if ( m_sCurrentPlaylist == "" )
        return QVariant();

    if (!index.isValid())
        return QVariant();

    TrackInfoObject *pTrack = getTrack(index);
    if ( pTrack == NULL )
        return QVariant();

    int totalSeconds, seconds, mins;
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case RhythmboxPlaylistModel::COLUMN_ARTIST:
                return pTrack->getArtist();
            case RhythmboxPlaylistModel::COLUMN_TITLE:
                return pTrack->getTitle();
            case RhythmboxPlaylistModel::COLUMN_ALBUM:
                return pTrack->getAlbum();
            case RhythmboxPlaylistModel::COLUMN_DATE:
                return pTrack->getYear();
            case RhythmboxPlaylistModel::COLUMN_GENRE:
                return pTrack->getGenre();
            case RhythmboxPlaylistModel::COLUMN_LOCATION:
                return pTrack->getLocation();
            case RhythmboxPlaylistModel::COLUMN_DURATION:
                // TODO(XXX) Pull this out into a MixxxUtil or something.

                //Let's reformat this song length into a human readable MM:SS format.
                totalSeconds = pTrack->getDuration();
                seconds = totalSeconds % 60;
                mins = totalSeconds / 60;
                //int hours = mins / 60; //Not going to worry about this for now. :)

                //Construct a nicely formatted duration string now.
                return QString("%1:%2").arg(mins).arg(seconds, 2, 10, QChar('0'));
            default:
                return QVariant();
        }
    }

    return QVariant();
}

bool RhythmboxPlaylistModel::isColumnInternal(int column) {
    return false;
}

QVariant RhythmboxPlaylistModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    /* Only respond to requests for column header display names */
    if ( role != Qt::DisplayRole )
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case RhythmboxPlaylistModel::COLUMN_ARTIST:
                return QString(tr("Artist"));

            case RhythmboxPlaylistModel::COLUMN_TITLE:
                return QString(tr("Title"));

            case RhythmboxPlaylistModel::COLUMN_ALBUM:
                return QString(tr("Album"));

            case RhythmboxPlaylistModel::COLUMN_DATE:
                return QString(tr("Date"));

            case RhythmboxPlaylistModel::COLUMN_GENRE:
                return QString(tr("Genre"));

            case RhythmboxPlaylistModel::COLUMN_LOCATION:
                return QString(tr("Location"));

            case RhythmboxPlaylistModel::COLUMN_DURATION:
                return QString(tr("Duration"));

            default:
                return QString(tr("Unknown"));
        }
    }

    return QVariant();
}

int RhythmboxPlaylistModel::rowCount ( const QModelIndex & parent ) const
{
    // FIXME
    //if ( !m_mPlaylists.containts(m_sCurrentPlaylist))
    //    return 0;

    if ( m_sCurrentPlaylist == "" )
        return 0;

    return m_mPlaylists[m_sCurrentPlaylist].size();
}

int RhythmboxPlaylistModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    return RhythmboxPlaylistModel::NUM_COLUMNS;
}

void RhythmboxPlaylistModel::addTrack(const QModelIndex& index, QString location)
{
    //Should do nothing... hmmm
}

/** Removes a track from the library track collection. */
void RhythmboxPlaylistModel::removeTrack(const QModelIndex& index)
{
    //Should do nothing... hmmm
}

void RhythmboxPlaylistModel::moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex)
{
    //Should do nothing... hmmm
}

QString RhythmboxPlaylistModel::getTrackLocation(const QModelIndex& index) const
{
    //FIXME
    return QString();
}

TrackInfoObject * RhythmboxPlaylistModel::getTrack(const QModelIndex& index) const
{
    QDomNodeList playlistTrackList = m_mPlaylists[m_sCurrentPlaylist];
    QDomNode pnode = playlistTrackList.at(index.row());
    QString location = pnode.toElement().text();

 	return m_pRhythmbox->getTrackByLocation(location);
}

QItemDelegate* RhythmboxPlaylistModel::delegateForColumn(const int i) {
    return NULL;
}

QList<QString> RhythmboxPlaylistModel::getPlaylists()
{
    return m_mPlaylists.keys();
}

int RhythmboxPlaylistModel::numPlaylists() {
    return m_mPlaylists.size();
}

QString RhythmboxPlaylistModel::playlistTitle(int n) {
    return m_mPlaylists.keys().at(n);
}

void RhythmboxPlaylistModel::setPlaylist(QString playlist)
{
    if ( m_mPlaylists.contains(playlist))
        m_sCurrentPlaylist = playlist;
    else
        m_sCurrentPlaylist = "";

    // force the layout to update
    emit(layoutChanged());
}

void RhythmboxPlaylistModel::search(const QString& searchText)
{
    m_currentSearch = searchText;
}

const QString RhythmboxPlaylistModel::currentSearch() {
    return m_currentSearch;
}

const QList<int>& RhythmboxPlaylistModel::searchColumns() const {
    return m_pRhythmbox->searchColumns();
}
