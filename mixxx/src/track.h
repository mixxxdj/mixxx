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

#include <qstring.h>
#include <qdom.h>
#include <qptrlist.h>
#include <qobject.h>
#include "trackplaylist.h"
#include "trackplaylistlist.h"

class TrackImporter;
class TrackInfoObject;
class TrackCollection;
class EngineBuffer;
class MixxxView;
class WTrackTable;
class WTreeList;
class ControlObjectThreadMain;

// This define sets the version of the tracklist. If any code is changed or
// bugfixed, this number should be increased. If TRACK_VERSION is larger
// than the version written in the current tracklist, the list will be
// re-parsed.
const int TRACK_VERSION = 10;

/**
@author Tue Haste Andersen
*/
class Track : public QObject
{
    Q_OBJECT
public:
    Track(QString location, MixxxView *pView, EngineBuffer *pBuffer1, EngineBuffer *pBuffer2);
    ~Track();
    /** Read xml file */
    void readXML(QString location);
    /** Write content to XML file */
    void writeXML(QString location);
    /** Get pointer to TrackCollection */
    TrackCollection *getTrackCollection();
    /** Force an update of menu and tree view */
    void updatePlaylistViews();
    
public slots:
    /** Decode playlist drops to WTrackTable, and loads corresponding playlist */
    void slotDrop(QDropEvent *e);
    /** Activate a playlist of the given name */
    void slotActivatePlaylist(QString name);
    /** Add a new playlist */
    void slotNewPlaylist();
    /** Delete a playlist */
    void slotDeletePlaylist(QString qName);
    /** Import a Playlist from a different format */
    void slotImportPlaylist();
    /** Bring up track popup menu */
    void slotTrackPopup(TrackInfoObject *pTrackInfoObject, int button);
    /** Load the given track in player 1 */
    void slotLoadPlayer1(TrackInfoObject *pTrackInfoObject);
    /** Load the given track in player 2 */
    void slotLoadPlayer2(TrackInfoObject *pTrackInfoObject);
    /** Load the given track in player 1 if it exists */
    void slotLoadPlayer1(QString filename);
    /** Load the given track in player 2 if it exists */
    void slotLoadPlayer2(QString filename);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackPlayer1(double);
    /** Slot used when playback reaches end of track */
    void slotEndOfTrackPlayer2(double);

private slots:
    /** Load the active popup track in player 1 */
    void slotLoadPlayer1();
    /** Load the active popup track in player 1 */
    void slotLoadPlayer2();
    /** Remove active popup track from active playlist */
    void slotRemoveFromPlaylist();
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

    /** Pointer to the Importer class */
    TrackImporter *m_pTrackImporter;
    /** List of pointers to TrackPlaylists */
    TrackPlaylistList m_qPlaylists;
    /** Current active playlist */
    TrackPlaylist *m_pActivePlaylist;
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
};

#endif
