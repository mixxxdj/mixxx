#ifndef TRACKLIST_H
#define TRACKLIST_H

#include <qobject.h>
#include <qptrlist.h>
#include "trackinfoobject.h"
#include "wtreelist.h"
#include <qtable.h>
#include <qlabel.h>


class QString;
class QPopupMenu;
class QPoint;
class EngineBuffer;
class WTrackTable;
class ControlObject;
class WNumberPos;
class QDragLeaveEvent;

// Defines for the rows in the table.
const int COL_SCORE = 0;
const int COL_TITLE = 1;
const int COL_ARTIST = 2;
const int COL_COMMENT = 3;
const int COL_TYPE = 4;
const int COL_DURATION = 5;
const int COL_BPM = 6;
const int COL_BITRATE = 7;
const int COL_INDEX = 8;

// Number of rows in the table
const int ROW_NO = 9;



// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACKLIST_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
const int TRACKLIST_VERSION = 9;

class TrackList : public QObject
{
    Q_OBJECT
public:
    TrackList(const QString, const QString,WTrackTable *,WTreeList *, QLabel *, QLabel *,
              WNumberPos *, WNumberPos *, EngineBuffer *, EngineBuffer *);
    ~TrackList();
    /** Loads the given track in player 1 */
    void loadTrack1(QString name);
    /** Loads the given track in player 2 */
    void loadTrack2(QString name);

    WTreeList *wTree;


public slots:
    void writeXML();
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
    void slotBrowseDir();
    void refreshPlaylist();
    void slotClearPlaylist();
    void slotFindTrack();

private:
    TrackInfoObject *fileExistsInList(const QString);
    void readXML();
    /** Adds the files given in <path> to the list of files.
        Returns true if any new files were in fact added. */
    bool addFiles(const char *);
    void updateScores();
    void updateTracklist();
    void loadPlaylist( QString );
    int ParseHeader(TrackInfoObject *Track);

    /** Index of current track in channel 1 and 2 */
    int m_iCurTrackIdxCh1, m_iCurTrackIdxCh2;
    /** Pointer to TrackInfoObject's of current loaded tracks */
    TrackInfoObject *m_pTrack1, *m_pTrack2;

    /** The directory where the music files are stored */
    QString m_sDirectory;
    /** list of all tracks */
    QPtrList<TrackInfoObject> m_lTracks;

    QString currentPlaylist;

    WTrackTable *m_pTableTracks;
    QPopupMenu *playSelectMenu;
    QPopupMenu *treeSelectMenu;
    /** Pointers to the play controls */
    QLabel *m_pText1, *m_pText2;
    /** Pointer to absolute position widgets */
    WNumberPos *m_pNumberPos1, *m_pNumberPos2;
    /** Points to the two play buffers */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;

    /** The number of times most often played track has been played */
    int m_iMaxTimesPlayed;
    /** Pointer to ControlObject signalling end of track */
    ControlObject *m_pEndOfTrackCh1, *m_pEndOfTrackCh2;
    /** Pointer to ControlObject dertermining end of track mode */
    ControlObject *m_pEndOfTrackModeCh1, *m_pEndOfTrackModeCh2;
    /** Pointer to ControlObjects for play buttons */
    ControlObject *m_pPlayCh1, *m_pPlayCh2;

};

#endif
