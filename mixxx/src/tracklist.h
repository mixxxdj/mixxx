/***************************************************************************
                          tracklist.h  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Ingo Kossyk & Tue Haste Andersen
    email                : kossyki@cs.tu-berlin.de & haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qobject.h>
#include <qptrlist.h>
#include "trackinfoobject.h"
#include "wtreelist.h"
#include "wnumberpos.h"
#include <qtable.h>
#include <qlabel.h>
#include <qdom.h>

class QString;
class QPopupMenu;
class QPoint;
class EngineBuffer;
class WTrackTable;
class ControlObject;
class QDragLeaveEvent;
	
// Defines for the rows in the table.
const int COL_SCORE = 0;
const int COL_TITLE = 1;
const int COL_ARTIST = 2;
const int COL_COMMENT = 3;
const int COL_TYPE = 4;
const int COL_DURATION = 5;
const int COL_BITRATE = 6;
const int COL_BPM = 7;
const int COL_INDEX = 8;

const int ROW_NO = 9;

// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACKLIST_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
const int TRACKLIST_VERSION = 7;

class TrackList : public QObject
{
    Q_OBJECT
public:
    TrackList(const QString, const QString,WTrackTable *,WTreeList *, QLabel *, QLabel *,
              WNumberPos *, WNumberPos *, EngineBuffer *, EngineBuffer *);
    ~TrackList();
    WTreeList *wTree;
    QString currentPlaylist;
	 /** Loads the given track in player 1 */
    void loadTrack1(QString name);
    /** Loads the given track in player 2 */
    void loadTrack2(QString name);
    /** Returns pointer to current TrackInfoObject used in player 1 */
    TrackInfoObject *getTrackInfo1();
    /** Returns pointer to current TrackInfoObject used in player 2 */
    TrackInfoObject *getTrackInfo2();

public slots:
    /** Can be called to update the Directory the Root Item of Treelist points to */
    void slotUpdateRoot(QString);
    /** Can be called to update the whole tracklist with a new directory */
    void slotUpdateTracklist(QString);
    //Fake slot for the second signal
    void slotUpdateTracklistFake(QString);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackCh1(double);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackCh2(double);
    bool AddFiles(const char *, QDomDocument * docXML);
	void WriteXML(QPtrList<TrackInfoObject> * tempTracks);
    int getTrackCount(QDomDocument * docXML);
private slots:
    /** Loads new track for channel 1. Idx refers to index in m_lTracks. If not given it loads the
      * currently marked track in m_pTableTracks. */
    void slotChangePlay_1(int idx=-1); 
    /** Loads new track for channel 2. Idx refers to index in m_lTracks. If not given it loads the
      * currently marked track in m_pTableTracks. */
    void slotChangePlay_2(int idx=-1);
    void slotClick(int, int, int, const QPoint &);
	void slotTreeClick(QListViewItem *, const QPoint &, int);
    void slotDeleteTrack(int idx=-1);
    void slotDeletePlaylist();
    void slotSavePlsAs();
    void slotSavePls();
	void refreshPlaylist();
	void slotClearPlaylist();
	void slotFindTrack();
    
private:
    TrackInfoObject *FileExistsInList(const QString, QDomDocument * docXML);
    void ReadXML();
    void UpdateScores();
    void UpdateTracklist(QDomDocument * domXML);
    void loadPlaylist( QString , QDomDocument * domXML);
	int ParseHeader(TrackInfoObject *Track);

    /** Index of current track in channel 1 and 2 */
    int m_iCurTrackIdxCh1, m_iCurTrackIdxCh2;
    /** Pointer to TrackInfoObject's of current loaded tracks */
    TrackInfoObject *m_pTrack1, *m_pTrack2;

    WTrackTable *m_pTableTracks;
    QPopupMenu *playSelectMenu;
    QPopupMenu *treeSelectMenu;
    /** Pointers to the play controls */
    QLabel *m_pText1, *m_pText2;
    /** Points to the two play buffers */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;
    QPtrList<TrackInfoObject> m_lTracks;
	/** Pointer to absolute position widgets */
    WNumberPos *m_pNumberPos1;
	WNumberPos *m_pNumberPos2;
	
    QString m_sDirectory; 
	int m_iMaxTimesPlayed;
     
    /** Pointer to ControlObject signalling end of track */
    ControlObject *m_pEndOfTrackCh1, *m_pEndOfTrackCh2;
    /** Pointer to ControlObject dertermining end of track mode */
    ControlObject *m_pEndOfTrackModeCh1, *m_pEndOfTrackModeCh2;
    /** Pointer to ControlObjects for play buttons */
    ControlObject *m_pPlayCh1, *m_pPlayCh2;
    
};

#endif
