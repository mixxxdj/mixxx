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
class ControlObject;

// Defines for the rows in the table.
const int COL_SCORE = 0;
const int COL_TITLE = 1;
const int COL_ARTIST = 2;
const int COL_TYPE = 3;
const int COL_DURATION = 4;
const int COL_BITRATE = 5;
const int COL_INDEX = 6;

// End of track mode constants
const int END_OF_TRACK_MODE_STOP = 0;
const int END_OF_TRACK_MODE_NEXT = 1;
const int END_OF_TRACK_MODE_LOOP = 2;
const int END_OF_TRACK_MODE_PING = 3;


// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACKLIST_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
const int TRACKLIST_VERSION = 5;

class TrackList : public QObject
{
    Q_OBJECT
public:
    TrackList(const QString, WTrackTable *, QLabel *, QLabel *,
              EngineBuffer *, EngineBuffer *);
    ~TrackList();
    void WriteXML( );

public slots:
    /** Can be called to update the whole tracklist with a new directory */
    void slotUpdateTracklist(QString);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackCh1(double);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackCh2(double);
    
private slots:
    /** Loads new track for channel 1. Idx refers to index in m_lTracks. If not given it loads the
      * currently marked track in m_pTableTracks. */
    void slotChangePlay_1(int idx=-1); 
    /** Loads new track for channel 2. Idx refers to index in m_lTracks. If not given it loads the
      * currently marked track in m_pTableTracks. */
    void slotChangePlay_2(int idx=-1);
    void slotClick(int, int, int, const QPoint &);
    
private:
    TrackInfoObject *FileExistsInList(const QString);
    void ReadXML();
    bool AddFiles(const char *);
    void UpdateScores();
    void UpdateTracklist();
    int ParseHeader(TrackInfoObject *Track);

    /** Index of current track in channel 1 and 2 */
    int m_iCurTrackIdxCh1, m_iCurTrackIdxCh2;
    
    /** The directory where the music files are stored */
    QString m_sDirectory; 
    /** list of all tracks */
    QPtrList<TrackInfoObject> m_lTracks; 
    WTrackTable *m_ptableTracks;
    QPopupMenu *playSelectMenu;
    /** Pointers to the play controls */
    QLabel *m_pText1, *m_pText2;
    /** Points to the two play buffers */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;
    
    int m_iMaxTimesPlayed;

    /** Pointer to ControlObject signalling end of track */
    ControlObject *m_pEndOfTrackCh1, *m_pEndOfTrackCh2;
    /** Pointer to ControlObject dertermining end of track mode */
    ControlObject *m_pEndOfTrackModeCh1, *m_pEndOfTrackModeCh2;
    /** Pointer to ControlObjects for play buttons */
    ControlObject *m_pPlayCh1, *m_pPlayCh2;
    
};

#endif
