#ifndef TRACKINFOOBJECT_H
#define TRACKINFOOBJECT_H

#include <QAtomicInt>
#include <QFileInfo>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QSharedPointer>

#include "library/dao/cue.h"
#include "library/coverart.h"
#include "proto/keys.pb.h"
#include "track/beats.h"
#include "track/keys.h"
#include "track/trackid.h"
#include "track/playcounter.h"
#include "track/trackmetadata.h"
#include "util/sandbox.h"
#include "waveform/waveform.h"

class TrackInfoObject;
typedef QSharedPointer<TrackInfoObject> TrackPointer;
typedef QWeakPointer<TrackInfoObject> TrackWeakPointer;

class TrackInfoObject : public QObject {
    Q_OBJECT

  public:
    TrackInfoObject(const TrackInfoObject&) = delete;

    // Creates a new empty temporary instance for fake tracks or for
    // testing purposes. The resulting track will neither be stored
    // in the database nor will the metadata of the corresponding file
    // be updated.
    // NOTE(uklotzde): Temporary track objects do not provide any guarantees
    // regarding safe file access, i.e. tags might be written back into the
    // file whenever the corresponding track is evicted from TrackCache!
    static TrackPointer newTemporary(
            const QFileInfo& fileInfo = QFileInfo(),
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer());
    // Creates a dummy instance for testing purposes.
    static TrackPointer newDummy(
            const QFileInfo& fileInfo,
            TrackId trackId);

    Q_PROPERTY(QString artist READ getArtist WRITE setArtist)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist)
    Q_PROPERTY(QString genre READ getGenre WRITE setGenre)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping)
    Q_PROPERTY(QString year READ getYear WRITE setYear)
    Q_PROPERTY(QString track_number READ getTrackNumber WRITE setTrackNumber)
    Q_PROPERTY(QString track_total READ getTrackTotal WRITE setTrackTotal)
    Q_PROPERTY(int times_played READ getTimesPlayed)
    Q_PROPERTY(QString comment READ getComment WRITE setComment)
    Q_PROPERTY(double bpm READ getBpm WRITE setBpm)
    Q_PROPERTY(QString bpmFormatted READ getBpmText STORED false)
    Q_PROPERTY(QString key READ getKeyText WRITE setKeyText)
    Q_PROPERTY(int duration READ getDuration WRITE setDuration)
    Q_PROPERTY(QString durationFormatted READ getDurationText STORED false)

    TrackId getId() const;

    QFileInfo getFileInfo() const {
        // Copying a QFileInfo is thread-safe (implicit sharing), no locking needed.
        return m_fileInfo;
    }
    SecurityTokenPointer getSecurityToken() const {
        // Copying a QSharedPointer is thread-safe, no locking needed.
        return m_pSecurityToken;
    }

    // Accessors for various stats of the file on disk.
    // Returns absolute path to the file, including the filename.
    QString getLocation() const;
    QString getCanonicalLocation() const;
    // Returns the absolute path to the directory containing the file
    QString getDirectory() const;
    // Returns the name of the file.
    QString getFileName() const;
    // Returns the size of the file in bytes
    int getFileSize() const;
    // Returns file modified datetime. Limited by the accuracy of what Qt
    // QFileInfo gives us.
    QDateTime getFileModifiedTime() const;
    // Returns file creation datetime. Limited by the accuracy of what Qt
    // QFileInfo gives us.
    QDateTime getFileCreationTime() const;
    // Returns whether the file exists on disk or not.
    bool exists() const;

    // Returns the file type
    QString getType() const;
    // Sets the file type. Only used by TrackDAO and SoundSourceProxy!
    void setType(const QString&);

    void setChannels(int iChannels);
    // Get number of channels
    int getChannels() const;

    // Set sample rate
    void setSampleRate(int iSampleRate);
    // Get sample rate
    int getSampleRate() const;
    // Set number of channels

    // Sets the bitrate
    void setBitrate(int);
    // Returns the bitrate
    int getBitrate() const;
    // Returns the bitrate as a string
    QString getBitrateText() const;

    // Set duration in seconds
    void setDuration(int);
    // Returns the duration in seconds
    int getDuration() const;
    // Returns the duration as a string: H:MM:SS
    QString getDurationText() const;

    // Set BPM
    double setBpm(double);
    // Returns BPM
    double getBpm() const;
    // Returns BPM as a string
    QString getBpmText() const;

    // A track with a locked BPM will not be re-analyzed by the beats or bpm
    // analyzer.
    void setBpmLocked(bool bpmLocked = true);
    bool isBpmLocked() const;

    // Set ReplayGain
    void setReplayGain(const Mixxx::ReplayGain&);
    // Returns ReplayGain
    Mixxx::ReplayGain getReplayGain() const;

    // Indicates if the metadata has been parsed from file tags.
    bool isHeaderParsed() const;
    // Only used by TrackDAO!
    void setHeaderParsed(bool headerParsed);

    void setDateAdded(const QDateTime& dateAdded);
    QDateTime getDateAdded() const;

    // Getter/Setter methods for metadata
    // Return title
    QString getTitle() const;
    // Set title
    void setTitle(const QString&);
    // Return artist
    QString getArtist() const;
    // Set artist
    void setArtist(const QString&);
    // Return album
    QString getAlbum() const;
    // Set album
    void setAlbum(const QString&);
    // Return album artist
    QString getAlbumArtist() const;
    // Set album artist
    void setAlbumArtist(const QString&);
    // Return Year
    QString getYear() const;
    // Set year
    void setYear(const QString&);
    // Return genre
    QString getGenre() const;
    // Set genre
    void setGenre(const QString&);
    // Returns the user comment
    QString getComment() const;
    // Sets the user commnet
    void setComment(const QString&);
    // Return composer
    QString getComposer() const;
    // Set composer
    void setComposer(const QString&);
    // Return grouping
    QString getGrouping() const;
    // Set grouping
    void setGrouping(const QString&);
    // Return track number/total
    QString getTrackNumber() const;
    QString getTrackTotal() const;
    // Set track number/total
    void setTrackNumber(const QString&);
    void setTrackTotal(const QString&);

    PlayCounter getPlayCounter() const;
    void setPlayCounter(const PlayCounter& playCounter);
    void resetPlayCounter(int iTimesPlayed = 0) {
        setPlayCounter(PlayCounter(iTimesPlayed));
    }
    // Sets played status and increments or decrements the play count
    void updatePlayCounter(bool bPlayed = true);

    // Only required for the times_played property
    int getTimesPlayed() const {
        return getPlayCounter().getTimesPlayed();
    }

    // Returns rating
    int getRating() const;
    // Sets rating
    void setRating(int);

    // Get URL for track
    QString getURL() const;
    // Set URL for track
    void setURL(const QString& url);

    // Output a formatted string with artist and title.
    QString getInfo() const;

    ConstWaveformPointer getWaveform();
    void setWaveform(ConstWaveformPointer pWaveform);

    ConstWaveformPointer getWaveformSummary() const;
    void setWaveformSummary(ConstWaveformPointer pWaveform);

    void setAnalyzerProgress(int progress);
    int getAnalyzerProgress() const;

    /** Save the cue point (in samples... I think) */
    void setCuePoint(float cue);
    // Get saved the cue point
    float getCuePoint() const;

    // Calls for managing the track's cue points
    CuePointer addCue();
    void removeCue(const CuePointer& pCue);
    QList<CuePointer> getCuePoints() const;
    void setCuePoints(const QList<CuePointer>& cuePoints);

    bool isDirty();

    // Get the track's Beats list
    BeatsPointer getBeats() const;

    // Set the track's Beats
    void setBeats(BeatsPointer beats);

    void resetKeys() {
        setKeys(Keys());
    }
    void setKeys(const Keys& keys);
    Keys getKeys() const;
    void setKey(mixxx::track::io::key::ChromaticKey key,
                mixxx::track::io::key::Source keySource);
    void setKeyText(const QString& keyText,
                    mixxx::track::io::key::Source keySource = mixxx::track::io::key::USER);
    mixxx::track::io::key::ChromaticKey getKey() const;
    QString getKeyText() const;

    void setCoverInfo(const CoverInfo& cover);
    CoverInfo getCoverInfo() const;

    void setCoverArt(const CoverArt& coverArt);
    CoverArt getCoverArt() const;

    // Set/get track metadata and cover art (optional) all at once.
    void setTrackMetadata(
            const Mixxx::TrackMetadata& trackMetadata,
            bool parsedFromFile);
    void getTrackMetadata(
            Mixxx::TrackMetadata* pTrackMetadata,
            bool* pHeaderParsed) const;

    // Mark the track dirty if it isn't already.
    void markDirty();
    // Mark the track clean if it isn't already.
    void markClean();

    // Called when the shared pointer reference count for a library TrackPointer
    // drops to zero.
    static void onTrackReferenceExpired(TrackInfoObject* pTrack);

    // Set whether the track should delete itself when its reference count drops
    // to zero. This happens during shutdown when TrackDAO has already been
    // destroyed.
    void setDeleteOnReferenceExpiration(bool deleteOnReferenceExpiration);

  public slots:
    void slotCueUpdated();

  signals:
    void waveformUpdated();
    void waveformSummaryUpdated();
    void coverArtUpdated();
    void analyzerProgress(int progress);
    void bpmUpdated(double bpm);
    void beatsUpdated();
    void keyUpdated(double key);
    void keysUpdated();
    void ReplayGainUpdated(Mixxx::ReplayGain replayGain);
    void cuesUpdated();
    void changed(TrackInfoObject* pTrack);
    void dirty(TrackInfoObject* pTrack);
    void clean(TrackInfoObject* pTrack);
    void referenceExpired(TrackInfoObject* pTrack);

  private slots:
    void slotBeatsUpdated();

  private:
    TrackInfoObject(
            const QFileInfo& fileInfo,
            const SecurityTokenPointer& pSecurityToken,
            TrackId trackId);

    // Set whether the TIO is dirty or not and unlock before emitting
    // any signals. This must only be called from member functions
    // while the TIO is locked.
    void markDirtyAndUnlock(QMutexLocker* pLock, bool bDirty = true);
    void setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty);

    void setBeatsAndUnlock(QMutexLocker* pLock, BeatsPointer pBeats);
    void setKeysAndUnlock(QMutexLocker* pLock, const Keys& keys);

    // Set a unique identifier for the track.
    // Only used by TrackDAO!
    void setId(TrackId id);

    // The file
    const QFileInfo m_fileInfo;

    const SecurityTokenPointer m_pSecurityToken;

    // Whether the track should delete itself when its reference count drops to
    // zero. Used for cleaning up after shutdown.
    volatile bool m_bDeleteOnReferenceExpiration;

    // Mutex protecting access to object
    mutable QMutex m_qMutex;

    // The unique ID of track. This value is only set once after the track
    // has been inserted or is loaded from the library DB.
    TrackId m_id;

    // Flag that indicates whether or not the TIO has changed. This is used by
    // TrackDAO to determine whether or not to write the Track back.
    bool m_bDirty;

    // File type
    QString m_sType;

    // Track metadata
    Mixxx::TrackMetadata m_metadata;

    // URL (used in promo track)
    QString m_sURL;

    // Track rating
    int m_iRating;

    // Cue point in samples or something
    float m_fCuePoint;
    // Date the track was added to the library
    QDateTime m_dateAdded;

    PlayCounter m_playCounter;

    Keys m_keys;

    // Various boolean flags. Please refer to the corresponding
    // setter/getter functions for detailed information about
    // their usage.
    bool m_bHeaderParsed;
    bool m_bBpmLocked;

    // The list of cue points for the track
    QList<CuePointer> m_cuePoints;

    // Storage for the track's beats
    BeatsPointer m_pBeats;

    //Visual waveform data
    ConstWaveformPointer m_waveform;
    ConstWaveformPointer m_waveformSummary;

    QAtomicInt m_analyzerProgress; // in 0.1%

    CoverArt m_coverArt;

    friend class TrackDAO;
};

#endif // TRACKINFOOBJECT_H
