/***************************************************************************
                          trackinfoobject.h  -  description
                             -------------------
    begin                : 10 02 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRACKINFOOBJECT_H
#define TRACKINFOOBJECT_H

#include <QList>
#include <QDateTime>
#include <QObject>
#include <QFileInfo>
#include <QMutex>
#include <QVector>
#include <QSharedPointer>
#include <QWeakPointer>

#include "defs.h"
#include "track/beats.h"
#include "library/dao/cue.h"

class QString;
class QDomElement;
class QDomDocument;
class QDomNode;
class ControlObject;
class TrackPlaylist;
class Cue;
class Waveform;

class TrackInfoObject;

typedef QSharedPointer<TrackInfoObject> TrackPointer;
typedef QWeakPointer<TrackInfoObject> TrackWeakPointer;

#include "segmentation.h"

class TrackInfoObject : public QObject
{
    Q_OBJECT
public:
    /** Initialize a new track with the filename. */
    TrackInfoObject(const QString sLocation="", bool parseHeader=true);
    // Initialize track with a QFileInfo class
    TrackInfoObject(const QFileInfo& fileInfo, bool parseHeader=true);
    // Creates a new track given information from the xml file. 
    TrackInfoObject(const QDomNode &);
    virtual ~TrackInfoObject();

    /** Returns true if the object contains valid information */
    bool isValid() const;
    int parse();
    void writeToXML( QDomDocument &, QDomElement & );

    /** Returns the duration in seconds */
    int getDuration() const;
    /** Set duration in seconds */
    void setDuration(int);
    /** Returns the duration as a string: H:MM:SS */
    QString getDurationStr() const;

    // Accessors for various stats of the file on disk. These are auto-populated
    // when the TIO is constructed, or when setLocation() is called.

    Q_PROPERTY(QString artist READ getArtist WRITE setArtist)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum)
    Q_PROPERTY(QString genre READ getGenre WRITE setGenre)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer)
    Q_PROPERTY(QString year READ getYear WRITE setYear)
    Q_PROPERTY(QString track_number READ getTrackNumber WRITE setTrackNumber)
    Q_PROPERTY(int times_played READ getTimesPlayed)
    Q_PROPERTY(QString comment READ getComment WRITE setComment)
    Q_PROPERTY(float bpm READ getBpm WRITE setBpm)
    Q_PROPERTY(QString bpmFormatted READ getBpmStr STORED false)
    Q_PROPERTY(int duration READ getDuration WRITE setDuration)
    Q_PROPERTY(QString durationFormatted READ getDurationStr STORED false)


    // Returns absolute path to the file, including the filename.
    QString getLocation() const;
    // Returns the absolute path to the directory containing the file
    QString getDirectory() const;
    // Returns the filename of the file.
    QString getFilename() const;
    // Returns file creation date
    QDateTime getCreateDate() const;
    // Returns the length of the file in bytes
    int getLength() const;
    // Returns whether the file exists on disk or not. Updated as of the time
    // the TrackInfoObject is created, or when setLocation() is called.
    bool exists() const;



    /** Returns ReplayGain*/
    float getReplayGain() const;
    /** Set ReplayGain*/
    void setReplayGain(float);
    /** Returns BPM */
    float getBpm() const;
    /** Set BPM */
    void setBpm(float);
    /** Returns BPM as a string */
    QString getBpmStr() const;
    // A track with a locked BPM will not be re-analyzed by the beats or bpm
    // analyzer.
    void setBpmLock(bool hasLock);
    bool hasBpmLock() const;
    bool getHeaderParsed() const;
    void setHeaderParsed(bool parsed = true);
    /** Returns the user comment */
    QString getComment() const;
    /** Sets the user commnet */
    void setComment(QString);
    /** Returns the file type */
    QString getType() const;
    /** Sets the type of the string */
    void setType(QString);
    /** Returns the bitrate */
    int getBitrate() const;
    /** Returns the bitrate as a string */
    QString getBitrateStr() const;
    /** Sets the bitrate */
    void setBitrate(int);
    /** Set sample rate */
    void setSampleRate(int iSampleRate);
    /** Get sample rate */
    int getSampleRate() const;
    /** Set number of channels */
    void setChannels(int iChannels);
    /** Get number of channels */
    int getChannels() const;
    /** Output a formatted string with all the info */
    QString getInfo() const;

    QDateTime getDateAdded() const;
    void setDateAdded(QDateTime dateAdded);

    /** Getter/Setter methods for metadata */
    /** Return title */
    QString getTitle() const;
    /** Set title */
    void setTitle(QString);
    /** Return artist */
    QString getArtist() const;
    /** Set artist */
    void setArtist(QString);
    /** Return album */
    QString getAlbum() const;
    /** Set album */
    void setAlbum(QString);
    /** Return Year */
    QString getYear() const;
    /** Set year */
    void setYear(QString);
    /** Return genre */
    QString getGenre() const;
    /** Set genre */
    void setGenre(QString);
    /** Return composer */
    QString getComposer() const;
    /** Set composer */
    void setComposer(QString);
    /** Return Track Number */
    QString getTrackNumber() const;
    /** Set Track Number */
    void setTrackNumber(QString);
    /** Return number of times the track has been played */
    int getTimesPlayed() const;
    /** Set number of times the track has been played */
    void setTimesPlayed(int t);
    /** Increment times played with one */
    void incTimesPlayed();
    /** Returns true if track has been played this instance*/
    bool getPlayed() const;
    /** Set played status and increment or decrement playcount. */
    void setPlayedAndUpdatePlaycount(bool);
    /** Set played status without affecting the playcount */
    void setPlayed(bool bPlayed);

    int getId() const;

    /** Returns rating */
    int getRating() const;
    /** Sets rating */
    void setRating(int);

    /** Returns KEY_CODE */
    QString getKey() const;
    /** Set KEY_CODE */
    void setKey(QString);

    /** Get URL for track */
    QString getURL();
    /** Set URL for track */
    void setURL(QString url);

    Waveform* getWaveform();
    const Waveform* getWaveform() const;
    void setWaveform(Waveform* pWaveform);

    Waveform* getWaveformSummary();
    const Waveform* getWaveformSummary() const;
    void setWaveformSummary(Waveform* pWaveformSummary);

    /** Save the cue point (in samples... I think) */
    void setCuePoint(float cue);
    /** Get saved the cue point */
    float getCuePoint();

    // Calls for managing the track's cue points
    Cue* addCue();
    void removeCue(Cue* cue);
    const QList<Cue*>& getCuePoints();
    void setCuePoints(QList<Cue*> cuePoints);

    bool isDirty();

    // Signals to the creator of this TrackInfoObject to save the Track as it
    // may be deleted.
    void doSave();

    // Returns true if the track location has changed
    bool locationChanged();

    /** Set the track's full file path */
    void setLocation(QString location);

    // Get the track's Beats list
    BeatsPointer getBeats() const;

    // Set the track's Beats
    void setBeats(BeatsPointer beats);

    const Segmentation<QString>* getChordData();
    void setChordData(Segmentation<QString> cd);

  public slots:
    void slotCueUpdated();

signals:
    void waveformUpdated();
    void waveformSummaryUpdated();
    void bpmUpdated(double bpm);
    void beatsUpdated();
    void ReplayGainUpdated(double replaygain);
    void cuesUpdated();
    void changed(TrackInfoObject* pTrack);
    void dirty(TrackInfoObject* pTrack);
    void clean(TrackInfoObject* pTrack);
    void save(TrackInfoObject* pTrack);

private slots:
    void slotBeatsUpdated();

private:

    // Common initialization function between all TIO constructors.
    void initialize(bool parseHeader);

    // Initialize all the location variables.
    void populateLocation(const QFileInfo& fileInfo);

    // Method for parsing information from knowing only the file name.  It
    // assumes that the filename is written like: "artist - trackname.xxx"
    void parseFilename();

    // Set whether the TIO is dirty not. This should never be called except by
    // TIO local methods or the TrackDAO.
    void setDirty(bool bDirty);

    // Set a unique identifier for the track. Only used by services like
    // TrackDAO
    void setId(int iId);

    // Flag that indicates whether or not the TIO has changed. This is used by
    // TrackDAO to determine whether or not to write the Track back.
    bool m_bDirty;

    // Special flag for telling if the track location was changed.
    bool m_bLocationChanged;

    // The filename
    QString m_sFilename;
    // The full path to the file, including the filename.
    QString m_sLocation;
    // The full path to the directory containing the file.
    QString m_sDirectory;
    // Length of track in bytes
    int m_iLength;

    /** Metadata */
    /** Album */
    QString m_sAlbum;
    /** Artist */
    QString m_sArtist;
    /** Title */
    QString m_sTitle;
    /** Genre */
    QString m_sGenre;
    /** Composer */
    QString m_sComposer;
    /** Year */
    QString m_sYear;
    /** Track Number */
    QString m_sTrackNumber;

    /** File type */
    QString m_sType;
    /** User comment */
    QString m_sComment;
    /** URL (used in promo track) */
    QString m_sURL;
    /** Duration of track in seconds */
    int m_iDuration;
    /** Sample rate */
    int m_iSampleRate;
    /** Number of channels */
    int m_iChannels;
    /**Track rating */
    int m_Rating;
    /** Bitrate, number of kilobits per second of audio in the track*/
    int m_iBitrate;
    /** Number of times the track has been played */
    int m_iTimesPlayed;
    /** Replay Gain volume */
    float m_fReplayGain;
    /** Has this track been played this sessions? */
    bool m_bPlayed;
    /** True if header was parsed */
    bool m_bHeaderParsed;
    /** Id. Unique ID of track */
    int m_iId;
    /** Cue point in samples or something */
    float m_fCuePoint;
    /** Date. creation date of file */
    QDateTime m_dCreateDate;
    // Date the track was added to the library
    QDateTime m_dateAdded;

    QString m_key;

    /** BPM lock **/
    bool m_bBpmLock;

    // The list of cue points for the track
    QList<Cue*> m_cuePoints;


    /** Mutex protecting access to object */
    mutable QMutex m_qMutex;

    /** True if object contains valid information */
    bool m_bIsValid;

    Segmentation<QString> m_chordData;

    // Storage for the track's beats
    BeatsPointer m_pBeats;

    //Visual waveform data
    Waveform* m_waveform;
    Waveform* m_waveformSummary;

    friend class TrackDAO;
};

#endif
