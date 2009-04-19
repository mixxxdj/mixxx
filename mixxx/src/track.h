//
// C++ Interface: track
//
// Description:
//
//
// Author: Tue Haste Andersen <haste@diku.dk>, (C) 2003
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef TRACK_H
#define TRACK_H

#include <QtCore>
#include <qdom.h>
#include <QMenu>
#include <QObject>
#include <QDropEvent>
#include "trackplaylist.h"
#include "trackplaylistlist.h"
#include "configobject.h"

#include <QEvent>

class TrackImporter;
class TrackInfoObject;
class TrackCollection;
class EngineBuffer;
class MixxxView;
class WTrackTable;
class WTrackTableModel;
class WPromoTracksModel;
class AnalyserQueue;

#ifdef __IPOD__
class WIPodTracksModel;
#endif
class WPlaylistListModel;
class WTreeList;
class ControlObjectThreadMain;
class QSortFilterProxyModel;
class QDirModel;
class LibraryScanner;
class LibraryScannerDialog;

// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACK_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
const int TRACK_VERSION = 10;

const QString PLAYLIST_NAME_IPOD  = "ipod";
const QString PLAYLIST_NAME_PROMO = "promo";

/**
@author Tue Haste Andersen
*/
class Track : public QObject
{
    Q_OBJECT
public:
    Track(QString location, MixxxView *pView, ConfigObject<ConfigValue> *config, EngineBuffer *pBuffer1, EngineBuffer *pBuffer2, AnalyserQueue* eaq);
    ~Track();
    /** Read xml file */
    void readXML(QString location);
    /** Write content to XML file */
    void writeXML(QString location);
    /** Get pointer to TrackCollection */
    TrackCollection *getTrackCollection();
    /** Get pointer to a playlist by it's index */
    TrackPlaylist* getPlaylistByIndex(int index);

#ifdef __IPOD__
    /** The ipod tracks playlist */
    TrackPlaylist m_qIPodPlaylist;
#endif

    /** Force an update of playlist menu and anything that sees the playlists*/
    void updatePlaylistViews();
    /** Get the list of playlists*/
    TrackPlaylistList* getPlaylists();
    /** Checks if the library directory's "last modified" timestamp has been changed */
    bool checkLibraryLastModified();
	
    /** Current active playlist */
    TrackPlaylist *m_pActivePlaylist; //FIXME wtf public variable?

    /** eventFilter to catch m_pView->m_pLineEdit focus events and selectAll for the text TODO: It has been suggested QEditLine should be subclassed with a custom FocusInEvent() method */
    bool eventFilter(QObject *obj, QEvent *e);

    /** Implement a timer which will update the filter on the tracktableview a maximum of 4 times a second, if m_pView->m_pLineEdit changes */
    void timerEvent(QTimerEvent *event);

    void resizeColumnsForLibraryMode();

	void appShuttingDown();

public slots:
    /** Decode playlist drops to WTrackTable, and loads corresponding playlist */
    void slotDrop(QDropEvent *e);
    /** Scan the library (starts up in a different thread) */
    void slotScanLibrary();
    /** Activate a playlist of the given name */
    void slotActivatePlaylist(QString name);
    /**Activate a Playlist from ComboBox*/
    void slotActivatePlaylist(int index);
    /** Add a new playlist */
    void slotNewPlaylist();
    /** Delete a playlist */
    void slotDeletePlaylist(QString qName);
    /** Import a Playlist from a different format */
    void slotImportPlaylist();
    /** Load the given track in player 1 */
    void slotLoadPlayer1(TrackInfoObject *pTrackInfoObject, bool bStartFromEndPos = false);
    /** Load the given track in player 2 */
    void slotLoadPlayer2(TrackInfoObject *pTrackInfoObject, bool bStartFromEndPos = false);
    /** Load the given track in player 1 if it exists */
    void slotLoadPlayer1(QString filename, bool bStartFromEndPos = false);
    /** Load the given track in player 2 if it exists */
    void slotLoadPlayer2(QString filename, bool bStartFromEndPos = false);
    /** Finish loading a track into Player1 after Reader has loaded the song into memory */
    void slotFinishLoadingPlayer1(TrackInfoObject* pTrackInfoObject, bool bStartFromEndPos);
    /** Finish loading a track into Player2 after Reader has loaded the song into memory */
    void slotFinishLoadingPlayer2(TrackInfoObject* pTrackInfoObject, bool bStartFromEndPos);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackPlayer1(double);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackPlayer2(double);
    /** Slot for loading next track in player 1 */
    void slotNextTrackPlayer1(double);
    /** Slot for loading previous track in player 1 */
    void slotPrevTrackPlayer1(double);
    /** Slot for loading next track in player 1 */
    void slotNextTrackPlayer2(double);
    /** Slot for loading previous track in player 1 */
    void slotPrevTrackPlayer2(double);

    /** Slots for loading the selected track into players */
    void slotLoadSelectedTrackCh1(double);
    void slotLoadSelectedTrackCh2(double);
    void slotLoadSelectedIntoFirstStopped(double);
    /** Slots for moving the selection cursor in the track list */
    void slotSelectNextTrack(double);
    void slotSelectPrevTrack(double);
    void slotSelectTrackKnob(double);

    /** Slots for moving between playlist options */
    void slotSelectNextPlaylist(double);
    void slotSelectPrevPlaylist(double);

    /** Returns pointer to active playlist */
    TrackPlaylist *getActivePlaylist();
    /** Slot for sending track to Play Queue */
    void slotSendToPlayqueue(TrackInfoObject *pTrackInfoObject);
    /** Slot for sending track to Play Queue */
    void slotSendToPlayqueue(QString filename);
    /** Slot for sending a playlist to Play Queue */
    void slotSendToPlayqueue(TrackPlaylist* playlist);
    /** Slot for sending a track to a playlist */
    void slotSendToPlaylist(TrackPlaylist* playlist, TrackInfoObject* pTrackInfoObject);
    /** Slot for sending a track to a playlist */
    void slotSendToPlaylist(TrackPlaylist* playlist, QString filename);

    /** Slot for showing a particular playlist in the track table view */
    void slotShowPlaylist(TrackPlaylist* playlist);
    /** Runs the BPM detection on every track in the TrackCollection */
    void slotBatchBPMDetection();

signals:
    /** A new track has been loaded in player 1 */
    void newTrackPlayer1(TrackInfoObject *);
    /** A new track has been loaded in player 2 */
    void newTrackPlayer2(TrackInfoObject *);
    /** Signal to update playlists menu */
    void updateMenu(TrackPlaylistList *);
    /** Signal to set active playlist */
    void activePlaylist(TrackPlaylist *);
private:
    /** Returns pointer to playlist by the given name */
    TrackPlaylist *getPlaylist(QString qName);

    /** Creates the promo tracks playlist dynamically */
    void initPromoTracks();
    /** Load the special promo track metadata from an XML file */
    void loadPromoTrackXMLData(QString xmlPath, QString promoDirPath);
    /** Check if the promo track directory exists */
    bool checkPromoDirExists();

    AnalyserQueue* m_analyserQueue;

    /** Pointer to the Importer class */
    TrackImporter *m_pTrackImporter;
    /** List of pointers to TrackPlaylists */
    TrackPlaylistList m_qPlaylists;
    /** The "library" playlist */
    TrackPlaylist m_qLibraryPlaylist;
    /** The play queue playlist */
    TrackPlaylist m_qPlayqueuePlaylist;
    /** The promo tracks playlist */
    TrackPlaylist m_qPromoPlaylist;

    /** Data Model for Library */
    WTrackTableModel *m_pLibraryModel;
    /** Data Model for Playqueue*/
    WTrackTableModel *m_pPlayQueueModel;
    /** Data Model for Promo Tracks */
    WPromoTracksModel *m_pPromoModel;
#ifdef __IPOD__
    /** Data Model for Promo Tracks */
    WIPodTracksModel *m_pIPodModel;
#endif
    /** Data Model containing a single playlist */
    WTrackTableModel *m_pPlaylistModel;
    /** Data Model containing the list of playlists */
    WPlaylistListModel *m_pPlaylistListModel;

    /** Pointer to playlist for which a popup menu is currently displayed */
    TrackPlaylist *m_pActivePopupPlaylist;
    /** Pointer to TrackInfoObject for which a popup menu is currently displayed */
    TrackInfoObject *m_pActivePopupTrack;
    /** Pointer to TrackCollection */
    TrackCollection *m_pTrackCollection;
    /** Pointer to MixxxView */
    MixxxView *m_pView;
    /** Pointer to EngineBuffer for channel 1 and 2 */
    EngineBuffer *m_pBuffer1, *m_pBuffer2;
    /** Pointer to TrackInfoObject's of current loaded tracks */
    TrackInfoObject *m_pTrackPlayer1, *m_pTrackPlayer2;
    /** Pointer to ControlObject signalling end of track */
    ControlObjectThreadMain *m_pEndOfTrackCh1, *m_pEndOfTrackCh2;
    /** Pointer to ControlObject dertermining end of track mode */
    ControlObjectThreadMain *m_pEndOfTrackModeCh1, *m_pEndOfTrackModeCh2;
    /** Pointer to ControlObject for play buttons */
    ControlObjectThreadMain *m_pPlayButtonCh1, *m_pPlayButtonCh2;
    /** Pointer to ControlObject for next/prev buttons */
    ControlObjectThreadMain *m_pNextTrackCh1, *m_pNextTrackCh2, *m_pPrevTrackCh1, *m_pPrevTrackCh2;
    /** Pointer to ControlObject for cue points (NOT the cue buttons - the cue points are the indices of the cue point samples.)*/
    ControlObjectThreadMain *m_pCuePointCh1, *m_pCuePointCh2;
    /** Pointer to ControlObject for playlist navigation/loading into Players */
     ControlObjectThreadMain *m_pLoadSelectedTrackCh1, *m_pLoadSelectedTrackCh2, *m_pLoadSelectedIntoFirstStopped, *m_pSelectNextTrack, *m_pSelectPrevTrack, *m_pSelectTrackKnob, *m_pSelectNextPlaylist, *m_pSelectPrevPlaylist;
    /** Pointer to ControlObject for play position */
    ControlObjectThreadMain *m_pPlayPositionCh1, *m_pPlayPositionCh2;

    /** Pointer to ControlObject for Ch1 and Ch2 visual resample rate */
    ControlObjectThreadMain *m_pVisualResampleCh1, *m_pVisualResampleCh2;

    /** Pointer to the library scanner */
    LibraryScanner *m_pScanner;
    /** Pointer to the library scanning dialog */
    LibraryScannerDialog* m_pLibScannerDlg;
    /** Index of the library playlist */
    unsigned int m_iLibraryIdx;
    /** Index of the play queue playlist */
    unsigned int m_iPlayqueueIdx;
    /** Provides access to the config keys */
    ConfigObject<ConfigValue> *m_pConfig;

    int savedRowPosition;

	int m_timerID;
};

#endif
