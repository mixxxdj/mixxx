//
// C++ Implementation: trackcollection
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "trackcollection.h"
#include "xmlparse.h"
#include "trackinfoobject.h"
#include "bpm/bpmdetector.h"
#include <qfileinfo.h>
#include <QtDebug>
#include "defs.h"


TrackCollection::TrackCollection()
{
    m_iCounter = -1;
}

TrackCollection::~TrackCollection()
{
}

void TrackCollection::readXML(QDomNode node)
{
    // For each track...
    QDomNode tracknode = node.firstChild();
    while (!tracknode.isNull())
    {
        if (tracknode.isElement() && tracknode.nodeName()=="Track")
        {
            TrackInfoObject * pTrack = new TrackInfoObject(tracknode);
            addTrack(pTrack);

            //qDebug() << "Trying to add" << pTrack->getTitle() << "to TrackCollection";

            // Update counter
            if (pTrack->getId()>m_iCounter)
                m_iCounter = pTrack->getId();

        }
        tracknode = tracknode.nextSibling();
    }
}

void TrackCollection::writeXML(QDomDocument &domXML, QDomElement &root)
{
    //qDebug() << "ELEMENTS " << m_qTrackList.count();
    QDomElement trackroot = domXML.createElement("TrackList");

    QListIterator<TrackInfoObject *> it(m_qTrackList);
    TrackInfoObject *cur_track;
    while (it.hasNext())
    {
        cur_track = it.next();
        QDomElement elementNew = domXML.createElement("Track");
        cur_track->writeToXML(domXML, elementNew);
        trackroot.appendChild(elementNew);
    }
    root.appendChild(trackroot);
}

void TrackCollection::addTrack(TrackInfoObject * pTrack)
{
    // If id is not already set in the TrackInfoObject, assign it an ID,
    // and increase the ID counter.
    if (pTrack->getId() <= -1)
    {
        ++m_iCounter;
        pTrack->setId(m_iCounter);


    }
    m_qTrackList.append(pTrack);
}

/** Removes a track from the library track collection. */
void TrackCollection::removeTrack(TrackInfoObject* pTrack)
{
    m_qTrackList.removeAll(pTrack);
}

/** clear the track collection. */
void TrackCollection::clear()
{
    m_qTrackList.clear();
}

TrackInfoObject *TrackCollection::getTrack(int id)
{
    //Q_ASSERT(id < m_qTrackList.size());

    if (id < m_qTrackList.size())
        return m_qTrackList.at(id);
    else
    {
        qDebug() << "Warning - track ID > trackcollection size in" << __FILE__ << "on line:" << __LINE__;
        return NULL;
    }

    // Binary search
    //return getTrack(id, -1, m_qTrackList.count()/2, m_qTrackList.count());

/*
    // Linear search through list to find the track of the given id
    QPtrList<TrackInfoObject>::iterator it = m_qTrackList.begin();
    while (it!=m_qTrackList.end())
    {
        if ((*it)->getId()==id)
            break;
 ++it;
    }
    if (it && (*it)->getId()==id)
    {
        qDebug() << "found " << id;
        return (*it);
    }
    else
    {
        qDebug() << "not found " << id;
        return 0;
    }
 */
}


TrackInfoObject * TrackCollection::getTrack(int id, int min, int mid, int max)
{
    //qDebug() << "id " << id << ", min " << min << ", mid " << mid << ", max " << max;
    int midId = 0;

    if (!m_qTrackList.at(mid))
        return 0;
    m_qTrackList.at(mid)->getId();
    if (midId==id)
        return m_qTrackList.at(mid);
    else if (midId<id && !(mid==min && mid+(max-mid)/2==mid))
        return getTrack(id, mid, mid+(max-mid)/2, max);
    else if (midId>id && !((mid-min/2)==mid && mid==max))
        return getTrack(id, min, (mid-min)/2, mid);

    return 0;
}

TrackInfoObject * TrackCollection::getTrack(QString location)
{
    // Search through list to find the track of the given filename
    QListIterator<TrackInfoObject *> it(m_qTrackList);
    TrackInfoObject * cur_track;
    while (it.hasNext())
    {
        cur_track = it.next();
        if (cur_track->getLocation()==location)
            return cur_track;
    }
    //if (cur_track && cur_track->getLocation()==location)
    //    return cur_track;
    //else
    {
        // We didn't find the track in the collection, so add a new entry
        QFileInfo file(location);
        if (file.exists())
        {
            TrackInfoObject * pTrack = new TrackInfoObject(file.absolutePath(), file.fileName());
            // Add track to the collection
            if (pTrack->isValid())
            {
                addTrack(pTrack);
                //qDebug() << "Found new track:" << pTrack->getFilename();
                return pTrack;
            }
            else
            {
                qDebug() << "Could not parse" << file.fileName();
                delete pTrack;
            }
        }
    }
    return 0;
}

int TrackCollection::getSize()
{
    return m_qTrackList.count();
    //return m_iCounter + 1;
}
