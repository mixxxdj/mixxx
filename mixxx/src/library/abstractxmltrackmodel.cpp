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


AbstractXmlTrackModel::AbstractXmlTrackModel(QString settingsNamespace)
        : TrackModel(QSqlDatabase::database("QSQLITE"),
                     settingsNamespace) {
}

AbstractXmlTrackModel::~AbstractXmlTrackModel()
{
}

Qt::ItemFlags AbstractXmlTrackModel::flags ( const QModelIndex & index ) const
{
    Qt::ItemFlags defaultFlags = QAbstractTableModel::flags(index);

    if (!index.isValid())
        return Qt::ItemIsEnabled;

    defaultFlags |= Qt::ItemIsDragEnabled;

    return defaultFlags;
}

QMimeData* AbstractXmlTrackModel::mimeData(const QModelIndexList &indexes) const {
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
                else
                    urls.append(url);
            }
        }
    }
    mimeData->setUrls(urls);
    return mimeData;
}


QVariant AbstractXmlTrackModel::data ( const QModelIndex & index, int role ) const
{
    if (!index.isValid())
        return QVariant();

    if (m_trackNodes.size() < index.row())
        return QVariant();

    QDomNode songNode = m_trackNodes.at(index.row());

    // tooltips and the display role should be the same thing.
    if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
        if ( index.column() > m_ColumnNames.size())
            return QVariant();

        return getTrackColumnData(songNode, index);
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

bool AbstractXmlTrackModel::addTrack(const QModelIndex& index, QString location)
{
    //Should do nothing... hmmm
    return false;
}

/** Removes a track from the library track collection. */
void AbstractXmlTrackModel::removeTrack(const QModelIndex& index)
{
    //Should do nothing... hmmm
}

void AbstractXmlTrackModel::removeTracks(const QModelIndexList& indices)
{
}

void AbstractXmlTrackModel::moveTrack(const QModelIndex& sourceIndex,
                                      const QModelIndex& destIndex)
{
    //Should do nothing... hmmm
}

QString AbstractXmlTrackModel::getTrackLocation(const QModelIndex& index) const
{
    TrackPointer track = getTrack(index);
    QString location = track->getLocation();
    // track is auto-deleted
    return location;
}

TrackPointer AbstractXmlTrackModel::getTrack(const QModelIndex& index) const
{
    QDomNode songNode = m_trackNodes.at(index.row());
    return parseTrackNode(songNode);
}

TrackPointer AbstractXmlTrackModel::getTrackByLocation(const QString& location) const
{
    if ( !m_mTracksByLocation.contains(location))
        return TrackPointer();

    QDomNode songNode = m_mTracksByLocation[location];
    return parseTrackNode(songNode);
}

/*
TrackPointer AbstractXmlTrackModel::parseTrackNode(QDomNode node)
{
    qDebug() << "Child class has not implemented parseTrackNode";
    return NULL;
}
*/

void AbstractXmlTrackModel::search(const QString& searchText)
{
    m_currentSearch = searchText;
    // TODO(XXX) Implement searching
}

const QList<int>& AbstractXmlTrackModel::searchColumns() const {
    return m_searchColumns;
}

const QString AbstractXmlTrackModel::currentSearch()
{
    return m_currentSearch;
}

void AbstractXmlTrackModel::addColumnName(int index, QString name)
{
    m_ColumnNames.insert(index, name);
}

void AbstractXmlTrackModel::addSearchColumn(int index) {
    m_searchColumns.push_back(index);
}

TrackModel::CapabilitiesFlags AbstractXmlTrackModel::getCapabilities() const {
    return TRACKMODELCAPS_ADDTOPLAYLIST | TRACKMODELCAPS_ADDTOCRATE |
            TRACKMODELCAPS_ADDTOAUTODJ;
}

