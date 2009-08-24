/***************************************************************************
                           abstractxmltrackmodel.h
                              -------------------
     begin                : 8/23/2009
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

/***************************************************************************
 *                                                                         *
 * This is the class implementation for the base utility class used for    *
 * parsing external XML track and playlist databases.                      *
 *                                                                         *
 ***************************************************************************/


#include <QtCore>
#include <QtGui>
#include <QtSql>
#include <QtDebug>
#include <QtXmlPatterns/QXmlQuery>


#include "abstractxmltrackmodel.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "defs.h"
#include "defs_audiofiles.h"


AbstractXmlTrackModel::AbstractXmlTrackModel()
{
    /* We need to throw some horrible error or exception telling
     * developers to ACTUALLY implement this, since this is only a
     * base abstract class.
     */
    qDebug() << "Unimplemented AbstractXmlTrackModel constructor";
}

AbstractXmlTrackModel::~AbstractXmlTrackModel()
{
}

Qt::ItemFlags AbstractXmlTrackModel::flags ( const QModelIndex & index ) const
{
    return QAbstractTableModel::flags(index);
}

QVariant AbstractXmlTrackModel::data ( const QModelIndex & index, int role ) const
{
    if (!index.isValid())
        return QVariant();
    
    if (m_trackNodes.size() < index.row())
        return QVariant();
    
    QDomNode songNode = m_trackNodes.at(index.row());
    
    
    if (role == Qt::DisplayRole) {
        if ( index.column() > m_ColumnNames.size())
            return QVariant();
        
        return getTrackColumnData(songNode, index);
        
        /* This is what the getTrackColumn implementation should look like
         QString getTrackColumn(QDomNode songNode, const QModelIndex & index)
         {
            switch (index.column()) {
                case RhythmboxTrackModel::COLUMN_ARTIST: 
                    return songNode.firstChildElement("artist").text();
                case RhythmboxTrackModel::COLUMN_TITLE:
                    return songNode.firstChildElement("title").text();
                case RhythmboxTrackModel::COLUMN_ALBUM:
                    return songNode.firstChildElement("album").text();
                case RhythmboxTrackModel::COLUMN_DATE:
                    return songNode.firstChildElement("date").text();
                case RhythmboxTrackModel::COLUMN_GENRE:
                    return songNode.firstChildElement("genre").text();
                case RhythmboxTrackModel::COLUMN_LOCATION:
                    return songNode.firstChildElement("location").text();
                case RhythmboxTrackModel::COLUMN_DURATION:
                    return songNode.firstChildElement("duration").text();
                
                default:
                    return QVariant();
            }
         }
        */
        
    }
    
    return QVariant();
}

QVariant AbstractXmlTrackModel::headerData ( int section, Qt::Orientation orientation, int role ) const
{
    /* Only respond to requests for column header display names */
    if ( role != Qt::DisplayRole )
        return QVariant();
    
    if (orientation == Qt::Horizontal)
    {
        if ( section > m_ColumnNames.size())
            return QVariant();
        
        return m_ColumnNames[section];
    }
    
    return QVariant();
}

int AbstractXmlTrackModel::rowCount ( const QModelIndex & parent ) const
{
    return m_trackNodes.count();
}

int AbstractXmlTrackModel::columnCount(const QModelIndex& parent) const
{
    if (parent != QModelIndex()) //Some weird thing for table-based models.
        return 0;
    
    return m_ColumnNames.size();
}

void AbstractXmlTrackModel::addTrack(const QModelIndex& index, QString location)
{
    //Should do nothing... hmmm
}

/** Removes a track from the library track collection. */
void AbstractXmlTrackModel::removeTrack(const QModelIndex& index)
{
    //Should do nothing... hmmm
}

QString AbstractXmlTrackModel::getTrackLocation(const QModelIndex& index) const
{
    TrackInfoObject *track;
    QString location;
    
    track = getTrack(index);
    location = track->getLocation();
    
    delete track;
    
    return location;
}

TrackInfoObject * AbstractXmlTrackModel::getTrack(const QModelIndex& index) const
{
    QDomNode songNode = m_trackNodes.at(index.row());
    return parseTrackNode(songNode);
}

TrackInfoObject * AbstractXmlTrackModel::getTrackByLocation(const QString& location) const
{
    if ( !m_mTracksByLocation.contains(location))
        return NULL;
    
    QDomNode songNode = m_mTracksByLocation[location];
    return parseTrackNode(songNode);
}

/*
TrackInfoObject *AbstractXmlTrackModel::parseTrackNode(QDomNode node)
{
    qDebug() << "Child class has not implemented parseTrackNode";
    return NULL;
}
*/

void AbstractXmlTrackModel::search(const QString& searchText)
{
    //FIXME
}

void AbstractXmlTrackModel::addColumnName(int index, QString name)
{
    m_ColumnNames[index] = name;
}
