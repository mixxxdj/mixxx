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

// Defines for the rows in the table. Should be made as simple
// private consts, but it won't compile.
#define ROW_SCORE 0
#define ROW_TITLE 1
#define ROW_ARTIST 2
#define ROW_TYPE 3
#define ROW_DURATION 4
#define ROW_BITRATE 5
#define ROW_INDEX 6

class TrackList : public QObject
{
	Q_OBJECT
public:
	TrackList( const QString, QTable *, DlgPlaycontrol *, DlgPlaycontrol *,
               EngineBuffer *, EngineBuffer * );
	~TrackList();
	void WriteXML( );

	TrackInfoObject *m_ptrackCurrent; // pointer to the currently selected track

private slots:
	void slotChangePlay_1(); // For recieving signals from the pulldown menu
	void slotChangePlay_2();
	void slotRightClick( int, int, const QPoint & );
    
private:
	TrackInfoObject *FileExistsInList( const QString );
	void ReadXML ();
	bool AddFiles(const char *);
	void UpdateScores();

	QString m_sDirectory; // the directory where the music files are stored
	QPtrList<TrackInfoObject> m_lTracks; // list of all tracks
	QTable *m_ptableTracks;
	QPopupMenu *playSelectMenu;
    /** Pointers to the play controls */
    DlgPlaycontrol *m_pPlaycontrol1, *m_pPlaycontrol2;
    /** Points to the two play buffers */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;
    
	int m_iMaxTimesPlayed;
};

#endif
