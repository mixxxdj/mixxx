#ifndef TRACKINFOOBJECT_H
#define TRACKINFOOBJECT_H

#include <QAtomicInt>
#include <QDateTime>
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
#include "util/sandbox.h"
#include "waveform/waveform.h"

class SoundSourceProxy;

class TrackInfoObject;
typedef QSharedPointer<TrackInfoObject> TrackPointer;
typedef QWeakPointer<TrackInfoObject> TrackWeakPointer;

namespace Mixxx {
    class TrackMetadata;
}

class TrackInfoObject : public QObject {
    Q_OBJECT

  public:
    TrackInfoObject(const TrackInfoObject&) = delete;
    TrackInfoObject(TrackInfoObject&&) = delete;

    // Creates a new empty temporary instance for fake tracks or for
    // testing purposes. The resulting track will neither be stored
    // in the database nor will the metadata of the corresponding file
    // be updated.
    static TrackPointer newTemporary(
            const QFileInfo& fileInfo = QFileInfo(),
            const SecurityTokenPointer& pSecurityToken = SecurityTokenPointer());
    // Creates a new temporary instance from another track that references
    // the same file. Please remember that the corresponding file might
    // be modified after all references to the original track have been
    // dropped. To be safe just keep the pointer to the original track
    // for the lifetime of this temporary instance.
    static TrackPointer newTemporaryForSameFile(
            const TrackPointer& pTrack);
    // Creates a dummy instance for testing purposes.
    static TrackPointer newDummy(
            const QFileInfo& fileInfo,
            TrackId trackId);

    // Parse file metadata. If no file metadata is present, attempts to extract
    // artist and title information from the filename.
    void parse(bool parseCoverArt);

    Q_PROPERTY(QString artist READ getArtist WRITE setArtist)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist)
    Q_PROPERTY(QString genre READ getGenre WRITE setGenre)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping)
    Q_PROPERTY(QString year READ getYear WRITE setYear)
    Q_PROPERTY(QString track_number READ getTrackNumber WRITE setTrackNumber)
    Q_PROPERTY(int times_played READ getTimesPlayed)
    Q_PROPERTY(QString comment READ getComment WRITE setComment)
    Q_PROPERTY(double bpm READ getBpm WRITE setBpm)
    Q_PROPERTY(QString bpmFormatted READ getBpmStr STORED false)
    Q_PROPERTY(QString key READ getKeyText WRITE setKeyText)
    Q_PROPERTY(int duration READ getDuration WRITE setDuration)
    Q_PROPERTY(QString durationFormatted READ getDurationStr STORED false)

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

    // Sets the type of the string
    void setType(const QString&);
    // Returns the file type
    QString getType() const;

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
    QString getBitrateStr() const;

    // Set duration in seconds
    void setDuration(int);
    // Returns the duration in seconds
    int getDuration() const;
    // Returns the duration as a string: H:MM:SS
    QString getDurationStr() const;

    // Set BPM
    void setBpm(double);
    // Returns BPM
    double getBpm() const;
    // Returns BPM as a string
    QString getBpmStr() const;

    // A track with a locked BPM will not be re-analyzed by the beats or bpm
    // analyzer.
    void setBpmLock(bool hasLock);
    bool hasBpmLock() const;

    // Set ReplayGain
    void setReplayGain(float);
    // Returns ReplayGain
    float getReplayGain() const;

    void setHeaderParsed(bool parsed = true);
    bool getHeaderParsed() const;

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
    // Return Track Number
    QString getTrackNumber() const;
    // Set Track Number
    void setTrackNumber(const QString&);
    // Return number of times the track has been played
    int getTimesPlayed() const;
    // Set number of times the track has been played
    void setTimesPlayed(int t);
    // Increment times played with one
    void incTimesPlayed();
    // Returns true if track has been played this instance
    bool getPlayed() const;
    // Set played status and increment or decrement playcount.
    void setPlayedAndUpdatePlaycount(bool);
    // Set played status without affecting the playcount
    void setPlayed(bool bPlayed);

    // Returns rating
    int getRating() const;
    // Sets rating
    void setRating(int);

    // Get URL for track
    QString getURL();
    // Set URL for track
    void setURL(const QString& url);

    // Output a formatted string with artist and title.
    QString getInfo() const;

    ConstWaveformPointer getWaveform();
    void setWaveform(ConstWaveformPointer pWaveform);

    ConstWaveformPointer getWaveformSummary() const;
    void setWaveformSummary(ConstWaveformPointer pWaveform);

    void setAnalyserProgress(int progress);
    int getAnalyserProgress() const;

    /** Save the cue point (in samples... I think) */
    void setCuePoint(float cue);
    // Get saved the cue point
    float getCuePoint();

    // Calls for managing the track's cue points
    Cue* addCue();
    void removeCue(Cue* cue);
    const QList<Cue*>& getCuePoints();
    void setCuePoints(QList<Cue*> cuePoints);

    bool isDirty();

    // Get the track's Beats list
    BeatsPointer getBeats() const;

    // Set the track's Beats
    void setBeats(BeatsPointer beats);

    void setKeys(Keys keys);
    const Keys& getKeys() const;
    double getNumericKey() const;
    mixxx::track::io::key::ChromaticKey getKey() const;
    QString getKeyText() const;
    void setKey(mixxx::track::io::key::ChromaticKey key,
                mixxx::track::io::key::Source source);
    void setKeyText(QString key,
                    mixxx::track::io::key::Source source=mixxx::track::io::key::USER);

    void setCoverInfo(const CoverInfo& cover);
    CoverInfo getCoverInfo() const;

    void setCoverArt(const CoverArt& cover);
    CoverArt getCoverArt() const;

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
    void analyserProgress(int progress);
    void bpmUpdated(double bpm);
    void beatsUpdated();
    void keyUpdated(double key);
    void keysUpdated();
    void ReplayGainUpdated(double replaygain);
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
            const SecurityTokenPointer& pToken,
            TrackId trackId);

    void setMetadata(const Mixxx::TrackMetadata& trackMetadata);
    void getMetadata(Mixxx::TrackMetadata* pTrackMetadata);

    friend class SoundSourceProxy;
    void parseTrackMetadata(
            const SoundSourceProxy& proxy,
            bool parseCoverArt,
            bool reloadFromFile);

    // Set whether the TIO is dirty not. This should never be called except by
    // TIO local methods or the TrackDAO.
    void setDirty(bool bDirty);

    // Set a unique identifier for the track. Only used by services like
    // TrackDAO
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

    // Metadata
    // Album
    QString m_sAlbum;
    // Artist
    QString m_sArtist;
    // Album Artist
    QString m_sAlbumArtist;
    // Title
    QString m_sTitle;
    // Genre
    QString m_sGenre;
    // Composer
    QString m_sComposer;
    // Grouping
    QString m_sGrouping;
    // Year
    QString m_sYear;
    // Track Number
    QString m_sTrackNumber;

    // File type
    QString m_sType;
    // User comment
    QString m_sComment;
    // URL (used in promo track)
    QString m_sURL;
    // Duration of track in seconds
    int m_iDuration;
    // Sample rate
    int m_iSampleRate;
    // Number of channels
    int m_iChannels;
    // Track rating
    int m_Rating;
    // Bitrate, number of kilobits per second of audio in the track
    int m_iBitrate;
    // Number of times the track has been played
    int m_iTimesPlayed;
    // Replay Gain volume
    float m_fReplayGain;
    // Has this track been played this sessions?
    bool m_bPlayed;
    // True if header was parsed
    bool m_bHeaderParsed;
    // Cue point in samples or something
    float m_fCuePoint;
    // Date the track was added to the library
    QDateTime m_dateAdded;

    Keys m_keys;

    // BPM lock
    bool m_bBpmLock;

    // The list of cue points for the track
    QList<Cue*> m_cuePoints;

    // Storage for the track's beats
    BeatsPointer m_pBeats;

    //Visual waveform data
    ConstWaveformPointer m_waveform;
    ConstWaveformPointer m_waveformSummary;

    QAtomicInt m_analyserProgress; // in 0.1%

    CoverArt m_coverArt;

    friend class TrackDAO;
    friend class LegacyLibraryImporter;
};

#endif // TRACKINFOOBJECT_H
