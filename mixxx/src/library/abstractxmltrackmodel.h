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
 * This is the class definition for a base utility class used for parsing  *
 * external XML track and playlist databases. The abstract implementation  *
 * includes helper methods and default implementations to help reduce code *
 * rewriting.                                                              *
 *                                                                         *
 * This class also assumes that the constructor loads the track nodes as a *
 * QDomeNodesList. The Class must implement parseTrackNode to convert the  *
 * QDomNode into a TrackInfoObject.                                        *
 ***************************************************************************/


#ifndef TRACKMODELXML_H
#define TRACKMODELXML_H

#include <QtSql>
#include <QtXml>
#include "trackmodel.h"

class AbstractXmlTrackModel : public QAbstractTableModel, public TrackModel
{
    Q_OBJECT
    public:
    AbstractXmlTrackModel(QString settingsNamespace);
    virtual ~AbstractXmlTrackModel();

    //QAbstractTableModel stuff
    virtual Qt::ItemFlags flags ( const QModelIndex & index ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const;
    virtual QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex& parent) const;

    //Track Model stuff
    virtual TrackPointer getTrack(const QModelIndex& index) const;
    virtual QString getTrackLocation(const QModelIndex& index) const;
    virtual TrackPointer getTrackByLocation(const QString& location) const;
    virtual void search(const QString& searchText);
    virtual const QList<int>& searchColumns() const;
    virtual const QString currentSearch();
    virtual bool isColumnInternal(int column) = 0;
    virtual void removeTrack(const QModelIndex& index);
    virtual void removeTracks(const QModelIndexList& indices);
    virtual bool addTrack(const QModelIndex& index, QString location);
    virtual void moveTrack(const QModelIndex& sourceIndex, const QModelIndex& destIndex);
    virtual TrackModel::CapabilitiesFlags getCapabilities() const;

public slots:


signals:
 	void startedLoading();
 	void progressLoading(QString path);
 	void finishedLoading();

private:
    QList<QString> m_ColumnNames;

protected:
    /* Implemented by AbstractXmlTrackModel implementations to parse the DOM Node into a TrackInfoObject */
    virtual TrackPointer parseTrackNode(QDomNode node) const = 0;
    /* Implemented by AbstractXmlTrackModel implementations to return the data for song columns */
    virtual QVariant getTrackColumnData(QDomNode node, const QModelIndex& index) const = 0;
    /* Called by AbstractXmlTrackModel implementations to enumerate their columns */
    void addColumnName(int index, QString name);

    // Add a column to be searched when searching occurs
    void addSearchColumn(int index);

    /* The list of song nodes */
    QDomNodeList m_trackNodes;
    /* A map of the song DOM nodes by track location */
    QMap <QString, QDomNode> m_mTracksByLocation;
    // A list of columns that the implementation wants searched
    QList<int> m_searchColumns;
    // The most recent search executed.
    QString m_currentSearch;
};

#endif
