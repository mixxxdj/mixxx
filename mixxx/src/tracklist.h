#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qobject.h>
#include <qptrlist.h>
#include "trackinfoobject.h"
#include <qtable.h>
#include <qlabel.h>

class QString;
class QPopupMenu;
class QPoint;
class EngineBuffer;
class WTrackTable;

// Defines for the rows in the table.
const int COL_SCORE = 0;
const int COL_TITLE = 1;
const int COL_ARTIST = 2;
const int COL_TYPE = 3;
const int COL_DURATION = 4;
const int COL_BITRATE = 5;
const int COL_INDEX = 6;

// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACKLIST_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
const int TRACKLIST_VERSION = 5;

class TrackList : public QObject
{
    Q_OBJECT
public:
    TrackList( const QString, WTrackTable *, QLabel *, QLabel *,
               EngineBuffer *, EngineBuffer * );
    ~TrackList();
    void WriteXML( );

public slots:
    /** Can be called to update the whole tracklist with a new directory */
    void slotUpdateTracklist( QString );

private slots:
    void slotChangePlay_1(); // For recieving signals from the pulldown menu
    void slotChangePlay_2();
    void slotClick( int, int, int, const QPoint & );
    
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
    QLabel *m_pText1, *m_pText2;
    /** Points to the two play buffers */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;
    
    int m_iMaxTimesPlayed;
};

#endif
