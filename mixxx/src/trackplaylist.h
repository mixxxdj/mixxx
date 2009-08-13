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
#include <q3ptrlist.h>
#include <qevent.h>
#include <QLayout>
#include <QLabel>
#include <qprogressbar.h>
//Added by qt3to4:
#include <QDropEvent>
#include "trackinfoobject.h"
class TrackCollection;
class WTrackTable;
class LibraryScanner;


/** Sort Comparison Functions */
bool ScoreLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool TitleLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool ArtistLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool TypeLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool DurationLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool BitrateLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool BpmLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool CommentLesser(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool ScoreGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool TitleGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool ArtistGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool TypeGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool DurationGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool BitrateGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool BpmGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);
bool CommentGreater(const TrackInfoObject *tio1, const TrackInfoObject *tio2);


/**
@author Tue Haste Andersen
*/
class TrackPlaylist : public QObject, public QList<TrackInfoObject*>
{
    Q_OBJECT
public:
    /** Construct an empty playlist */
    TrackPlaylist();
    /** Construct an empty playlist */
    TrackPlaylist(TrackCollection *pTrackCollection, QString qName="Default");
    /** Construct a playlist from the definition at the dom node */
    TrackPlaylist(TrackCollection *pTrackCollectionm, QDomNode node);
    /** Destruct the playlist */
    ~TrackPlaylist();
    /** Write database content to XML file */
    void writeXML(QDomDocument &doc, QDomElement &header);
    /** Add a track to the playlist */
    void addTrack(TrackInfoObject *pTrack);
    /** Add a track to the playlist */
    void addTrack(QString qLocation);
    /** Get name of playlist */
    QString getListName();
    /** Get comment for playlist */
    QString getComment();
    /** Set comment for playlist */
    void setComment(QString comment);
    /** Loads the playlist from an XML node */
    void loadFromXMLNode(QDomNode node);
    /** Sets the TrackCollection pointer */
    void setTrackCollection(TrackCollection * pTrackCollection);
    /** Set name of list */
    void setListName(QString name);
    /** Returns name of playlist */
    QString getName();

    /** Sort routines */
    void sortByScore(bool ascending);
    void sortByTitle(bool ascending);
    void sortByArtist(bool ascending);
    void sortByType(bool ascending);
    void sortByDuration(bool ascending);
    void sortByBitrate(bool ascending);
    void sortByBpm(bool ascending);
    void sortByComment(bool ascending);

    void dumpInfo();

signals:
    void startedLoading();         //Started loading a playlist/library
    void finishedLoading();        //Finished loading a playlist/library
    void progressLoading(QString trackName);

public slots:
    /** Decode drop event and calls addPath */
    void slotDrop(QDropEvent *e);

protected:
    /** Sorting algorithm... */
    int operator<(TrackPlaylist * p2);

private:
    /** Pointer to TrackCollection */
    TrackCollection *m_pTrackCollection;
    /** Name of list */
    QString m_qName;
    /** Comment for playlist */
    QString m_qComment;
};

#endif
