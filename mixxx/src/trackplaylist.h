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
class Track;


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
    /** Returns name of playlist */
    QString getName();
    /** Get TrackInfoObject of first track in playlist */
    TrackInfoObject *getFirstTrack();
	int getSongNum();
    int getIndexOf(int id);
	TrackInfoObject *getTrackAt(int index);
	TrackCollection *getCollection();

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
    
public slots:
    /** Decode drop event and calls addPath */
    void slotDrop(QDropEvent *e);
    /** Remove a track from the playlist */
    void slotRemoveTrack(TrackInfoObject *pTrack);

private:
    /** List of pointers to TrackInfoObjects */
    QList<TrackInfoObject*> m_qList;
    /** Pointer to TrackCollection */
    TrackCollection *m_pTrackCollection;
	TrackCollection *personalTrackCollection;
    /** Name of list */
    QString m_qName;
    /** Pointer to WTrackTable. This is 0 if the playlist is not displayed in a table */
    //WTrackTable *m_pTable;
    /** Static pointer to Track */
    static Track *spTrack;
	int iCounter;

	// These functions allow the displaying of a useful message of some kind if it all takes too long
	static void startTiming();
	static void stopTiming();
	static void checkTiming(QString path);
	static void setupTiming();

	static QTime m_timer;
	static int m_timeruses;
	static QWidget* m_progress;
	static QLabel* m_current;
	static bool m_timersetup;

};

#endif
