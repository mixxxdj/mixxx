//
// C++ Interface: trackcollection
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRACKCOLLECTION_H
#define TRACKCOLLECTION_H

#include <qdom.h>
#include "trackinfoobject.h"

class TrackInfoObject;
class BpmDetector;

/**
@author Tue Haste Andersen
*/
class TrackCollection
{
public:
    TrackCollection();
    ~TrackCollection();
    /** Read database content from XML file */
    void readXML(QDomNode node);
    /** Write database content to XML file */
    void writeXML(QDomDocument &domXML, QDomElement &root);
    /** Add a track to the database */
    void addTrack(TrackInfoObject *pTrack);
    /** Removes a track from the library track collection. */
    void removeTrack(TrackInfoObject* pTrack);

    /** clears the track collection. */
    void clear(); // FIXME: check that this does not cause a memory leak...

    /** Get a track from the database, identified by id. Returns 0 if the track was
      * not found */
    TrackInfoObject *getTrack(int id);
    /** Get a track from the database, identified by pathname. Returns 0 if
      * the track was not found. If the track is not in the database, a TIO is
       * created and added to the database */
    TrackInfoObject *getTrack(QString location);
	int getSize();
protected:
    /** Used in binary search for an ID in the list */
    TrackInfoObject *getTrack(int id, int min, int mid, int max);
    /** List of TrackInfoObjects. This is the actual database */
    QList <TrackInfoObject*> m_qTrackList;
    /** Counter used to assign unique id's to tracks in database */
    long int m_iCounter;
};

#endif
