//
// C++ Interface: trackplaylist
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRACKPLAYLIST_H
#define TRACKPLAYLIST_H

#include <qobject.h>
#include <qstring.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qevent.h>
#include "trackinfoobject.h"
class TrackCollection;
class WTrackTable;
class Track;

/**
@author Tue Haste Andersen
*/
class TrackPlaylist : public QObject
{
    Q_OBJECT
public:
    /** Construct an empty playlist */
    TrackPlaylist(TrackCollection *pTrackCollection, QString qName="Default");
    /** Construct a playlist from the definition at the dom node */
    TrackPlaylist(TrackCollection *pTrackCollectionm, QDomNode node);
    /** Destruct the playlist */
    ~TrackPlaylist();
    /** Set pointer to Track object */
    static void setTrack(Track *pTrack);
    /** Write database content to XML file */
    void writeXML(QDomDocument &doc, QDomElement &header);
    /** Add a track to the playlist */
    void addTrack(TrackInfoObject *pTrack);
    /** Add a track to the playlist */
    void addTrack(QString qLocation);
    /** Add all tracks from the playlist to the WTrackTable */
    void activate(WTrackTable *pTable);
    /** Remove all tracks from the WTrackTable */
    void deactivate();
    /** Get name of list */
    QString getListName();
    /** Set name of list */
    void setListName(QString name);
    /** Add recursively the tracks in path to the collection, and then to this list */
    void addPath(QString qPath);
    /** Updates the score field in the WTrackTable */
    void updateScores();

public slots:
    /** Decode drop event and calls addPath */
    void slotDrop(QDropEvent *e);
    /** Remove a track from the playlist */
    void slotRemoveTrack(TrackInfoObject *pTrack);

private:
    /** List of pointers to TrackInfoObjects */
    QPtrList<TrackInfoObject> m_qList;
    /** Pointer to TrackCollection */
    TrackCollection *m_pTrackCollection;
    /** Name of list */
    QString m_qName;
    /** Pointer to WTrackTable. This is 0 if the playlist is not displayed in a table */
    WTrackTable *m_pTable;
    /** Static pointer to Track */
    static Track *spTrack;
};

#endif
