#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qobject.h>
#include <qptrlist.h>
#include "trackinfoobject.h"
#include <qtable.h>

class QString;
class QPopupMenu;
class QPoint;
class DlgPlaycontrol;
class EngineBuffer;
class WTrackTable;

// Defines for the rows in the table. Should be made as simple
// private consts, but it won't compile.
#define COL_SCORE 0
#define COL_TITLE 1
#define COL_ARTIST 2
#define COL_TYPE 3
#define COL_DURATION 4
#define COL_BITRATE 5
#define COL_INDEX 6

// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACKLIST_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
// - And yes, it should be a const member, but I don't know how to do that.
#define TRACKLIST_VERSION 1

class TrackList : public QObject
{
	Q_OBJECT
public:
	TrackList( const QString, WTrackTable *, DlgPlaycontrol *, DlgPlaycontrol *,
               EngineBuffer *, EngineBuffer * );
	~TrackList();
	void WriteXML( );

public slots:
    /** Can be called to update the whole tracklist with a new directory */
    void slotUpdateTracklist( QString );

private slots:
	void slotChangePlay_1(); // For recieving signals from the pulldown menu
	void slotChangePlay_2();
	void slotRightClick( int, int, const QPoint & );
    
private:
	TrackInfoObject *FileExistsInList( const QString );
	void ReadXML ();
	bool AddFiles(const char *);
	void UpdateScores();
    void UpdateTracklist();
    int ParseHeader( TrackInfoObject *Track );

	QString m_sDirectory; // the directory where the music files are stored
	QPtrList<TrackInfoObject> m_lTracks; // list of all tracks
	WTrackTable *m_ptableTracks;
	QPopupMenu *playSelectMenu;
    /** Pointers to the play controls */
    DlgPlaycontrol *m_pPlaycontrol1, *m_pPlaycontrol2;
    /** Points to the two play buffers */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;
    
	int m_iMaxTimesPlayed;
};

#endif
