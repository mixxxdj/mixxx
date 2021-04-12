#pragma once

#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QUrl>

#include "audio/streaminfo.h"
#include "sources/metadatasource.h"
#include "track/beats.h"
#include "track/cue.h"
#include "track/cueinfoimporter.h"
#include "track/track_decl.h"
#include "track/trackfile.h"
#include "track/trackrecord.h"
#include "util/sandbox.h"
#include "waveform/waveform.h"

class Track : public QObject {
    Q_OBJECT

  public:
    Track(TrackFile fileInfo,
            SecurityTokenPointer pSecurityToken,
            TrackId trackId = TrackId());
    Track(const Track&) = delete;
    ~Track() override;

    // Creates a new empty temporary instance for fake tracks or for
    // testing purposes. The resulting track will neither be stored
    // in the database nor will the metadata of the corresponding file
    // be updated.
    // Use SoundSourceProxy::importTemporaryTrack() for importing files
    // to ensure that the file will not be written while reading it!
    static TrackPointer newTemporary(
            TrackFile fileInfo = TrackFile(),
            SecurityTokenPointer pSecurityToken = SecurityTokenPointer());
    // Creates a dummy instance only for testing purposes.
    static TrackPointer newDummy(
            TrackFile fileInfo,
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
    Q_PROPERTY(double bpm READ getBpm)
    Q_PROPERTY(QString bpmFormatted READ getBpmText STORED false)
    Q_PROPERTY(QString key READ getKeyText WRITE setKeyText)
    Q_PROPERTY(double duration READ getDuration)
    Q_PROPERTY(QString durationFormatted READ getDurationTextSeconds STORED false)
    Q_PROPERTY(QString durationFormattedCentiseconds READ getDurationTextCentiseconds STORED false)
    Q_PROPERTY(QString durationFormattedMilliseconds READ getDurationTextMilliseconds STORED false)
    Q_PROPERTY(QString info READ getInfo STORED false)
    Q_PROPERTY(QString titleInfo READ getTitleInfo STORED false)

    TrackFile getFileInfo() const {
        // Copying TrackFile/QFileInfo is thread-safe (implicit sharing), no locking needed.
        return m_fileInfo;
    }
    SecurityTokenPointer getSecurityToken() const {
        // Copying a QSharedPointer is thread-safe, no locking needed.
        return m_pSecurityToken;
    }

    TrackId getId() const;

    // Returns absolute path to the file, including the filename.
    QString getLocation() const {
        return m_fileInfo.location();
    }
    // The (refreshed) canonical location
    QString getCanonicalLocation() const;
    // Checks if the file exists
    bool checkFileExists() const {
        return m_fileInfo.checkFileExists();
    }

    // File/format type
    void setType(const QString&);
    QString getType() const;

    // Get number of channels
    int getChannels() const;

    // Get sample rate
    mixxx::audio::SampleRate getSampleRate() const;

    // Sets the bitrate
    void setBitrate(int);
    // Returns the bitrate
    int getBitrate() const;
    // Returns the bitrate as a string
    QString getBitrateText() const;

    void setDuration(mixxx::Duration duration);
    void setDuration(double duration);
    double getDuration() const {
        return getDuration(DurationRounding::NONE);
    }
    // Returns the duration rounded to seconds
    int getDurationInt() const {
        return static_cast<int>(getDuration(DurationRounding::SECONDS));
    }
    // Returns the duration formatted as a string (H:MM:SS or H:MM:SS.cc or H:MM:SS.mmm)
    QString getDurationText(mixxx::Duration::Precision precision) const;

    // Helper functions for Q_PROPERTYs
    QString getDurationTextSeconds() const {
        return getDurationText(mixxx::Duration::Precision::SECONDS);
    }
    QString getDurationTextCentiseconds() const {
        return getDurationText(mixxx::Duration::Precision::CENTISECONDS);
    }
    QString getDurationTextMilliseconds() const {
        return getDurationText(mixxx::Duration::Precision::MILLISECONDS);
    }

    // Sets the BPM if not locked.
    bool trySetBpm(double bpm);

    // Returns BPM
    double getBpm() const {
        const QMutexLocker lock(&m_qMutex);
        return getBpmWhileLocked().getValue();
    }
    // Returns BPM as a string
    QString getBpmText() const;

    // A track with a locked BPM will not be re-analyzed by the beats or bpm
    // analyzer.
    void setBpmLocked(bool bpmLocked);
    bool isBpmLocked() const;

    // Set ReplayGain
    void setReplayGain(const mixxx::ReplayGain&);
    // Returns ReplayGain
    mixxx::ReplayGain getReplayGain() const;

    // Indicates if the metadata has been parsed from file tags.
    bool isMetadataSynchronized() const;
    // Only used by a free function in TrackDAO!
    void setMetadataSynchronized(bool metadataSynchronized);

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
    // Returns the track color
    mixxx::RgbColor::optional_t getColor() const;
    // Sets the track color
    void setColor(mixxx::RgbColor::optional_t);
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
    /// Resets the rating
    void resetRating() {
        setRating(mixxx::TrackRecord::kNoRating);
    }

    // Get URL for track
    QString getURL() const;
    // Set URL for track
    void setURL(const QString& url);

    /// Separator between artist and title string that is
    /// used for composing the track info.
    static const QString kArtistTitleSeparator;

    /// Formatted string with artist and title, separated by
    /// kArtistTitleSeparator.
    QString getInfo() const;

    /// The filename if BOTH artist AND title are empty, e.g. for tracks without
    /// any metadata in file tags. Otherwise just the title (even if it is empty).
    QString getTitleInfo() const;

    ConstWaveformPointer getWaveform() const;
    void setWaveform(ConstWaveformPointer pWaveform);

    ConstWaveformPointer getWaveformSummary() const;
    void setWaveformSummary(ConstWaveformPointer pWaveform);

    // Get the track's main cue point
    CuePosition getCuePoint() const;
    // Set the track's main cue point
    void setCuePoint(CuePosition cue);
    /// Shift all cues by a constant offset
    void shiftCuePositionsMillis(double milliseconds);
    // Call when analysis is done.
    void analysisFinished();

    // Calls for managing the track's cue points
    CuePointer createAndAddCue();
    CuePointer findCueByType(mixxx::CueType type) const; // NOTE: Cannot be used for hotcues.
    CuePointer findCueById(DbId id) const;
    void removeCue(const CuePointer& pCue);
    void removeCuesOfType(mixxx::CueType);
    QList<CuePointer> getCuePoints() const {
        // Copying implicitly shared collections is thread-safe
        return m_cuePoints;
    }

    void setCuePoints(const QList<CuePointer>& cuePoints);

    enum class ImportStatus {
        Pending,
        Complete,
    };
    /// Imports the given list of cue infos as cue points,
    /// thereby replacing all existing cue points!
    ///
    /// If the list is empty it tries to complete any pending
    /// import and returns the corresponding status.
    ImportStatus importCueInfos(
            mixxx::CueInfoImporterPointer pCueInfoImporter);
    ImportStatus getCueImportStatus() const;

    bool isDirty();

    // Get the track's Beats list
    mixxx::BeatsPointer getBeats() const;

    // Set the track's Beats if not locked
    bool trySetBeats(mixxx::BeatsPointer pBeats);
    bool trySetAndLockBeats(mixxx::BeatsPointer pBeats);

    /// Imports the given list of cue infos as cue points,
    /// thereby replacing all existing cue points!
    ///
    /// If the list is empty it tries to complete any pending
    /// import and returns the corresponding status.
    ImportStatus tryImportBeats(
            mixxx::BeatsImporterPointer pBeatsImporter,
            bool lockBpmAfterSet);
    ImportStatus getBeatsImportStatus() const;

    void resetKeys();
    void setKeys(const Keys& keys);
    Keys getKeys() const;
    void setKey(mixxx::track::io::key::ChromaticKey key,
            mixxx::track::io::key::Source keySource);
    void setKeyText(const QString& keyText,
            mixxx::track::io::key::Source keySource = mixxx::track::io::key::USER);
    mixxx::track::io::key::ChromaticKey getKey() const;
    QString getKeyText() const;

    void setCoverInfo(const CoverInfoRelative& coverInfo);
    CoverInfoRelative getCoverInfo() const;
    CoverInfo getCoverInfoWithLocation() const;
    // Verify the cover image hash and update it if necessary.
    // If the corresponding image has already been loaded it
    // could be provided as a parameter to avoid reloading
    // if actually needed.
    bool refreshCoverImageHash(
            const QImage& loadedImage = QImage());

    quint16 getCoverHash() const;

    // Set/get track metadata and cover art (optional) all at once.
    void importMetadata(
            mixxx::TrackMetadata importedMetadata,
            const QDateTime& metadataSynchronized = QDateTime());
    // Merge additional metadata that is not (yet) stored in the database
    // and only available from file tags.
    void mergeImportedMetadata(
            const mixxx::TrackMetadata& importedMetadata);

    void readTrackMetadata(
            mixxx::TrackMetadata* pTrackMetadata,
            bool* pMetadataSynchronized = nullptr) const;
    void readTrackRecord(
            mixxx::TrackRecord* pTrackRecord,
            bool* pDirty = nullptr) const;

    // Mark the track dirty if it isn't already.
    void markDirty();
    // Mark the track clean if it isn't already.
    void markClean();

    // Explicitly request to export the track's metadata. The actual
    // export is deferred to prevent race conditions when writing into
    // files that are still opened for reading.
    void markForMetadataExport();
    bool isMarkedForMetadataExport() const;

    void setAudioProperties(
            mixxx::audio::ChannelCount channelCount,
            mixxx::audio::SampleRate sampleRate,
            mixxx::audio::Bitrate bitrate,
            mixxx::Duration duration);
    void setAudioProperties(
            const mixxx::audio::StreamInfo& streamInfo);

  signals:
    void waveformUpdated();
    void waveformSummaryUpdated();
    void coverArtUpdated();
    void bpmUpdated(double bpm);
    void beatsUpdated();
    void keyUpdated(double key);
    void keysUpdated();
    void replayGainUpdated(mixxx::ReplayGain replayGain);
    void colorUpdated(mixxx::RgbColor::optional_t color);
    void cuesUpdated();
    void analyzed();

    void changed(TrackId trackId);
    void dirty(TrackId trackId);
    void clean(TrackId trackId);

  private slots:
    void slotCueUpdated();

  private:
    // Set a unique identifier for the track. Only used by
    // GlobalTrackCacheResolver!
    void initId(TrackId id);
    // Reset the unique identifier after purged from library
    // which undos a previous add. Only used by
    // GlobalTrackCacheResolver!
    void resetId();

    void relocate(
            TrackFile fileInfo,
            SecurityTokenPointer pSecurityToken = SecurityTokenPointer());

    // Set whether the TIO is dirty or not and unlock before emitting
    // any signals. This must only be called from member functions
    // while the TIO is locked.
    void markDirtyAndUnlock(QMutexLocker* pLock) {
        setDirtyAndUnlock(pLock, true);
    }
    void setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty);

    void afterKeysUpdated(QMutexLocker* pLock);
    void emitKeysUpdated(mixxx::track::io::key::ChromaticKey newKey);

    void afterBeatsAndBpmUpdated(QMutexLocker* pLock);
    void emitBeatsAndBpmUpdated(mixxx::Bpm newBpm);

    /// Sets beats and returns a boolean to indicate if BPM/Beats were updated.
    /// Only supposed to be called while the caller guards this a lock.
    bool setBeatsWhileLocked(mixxx::BeatsPointer pBeats);

    /// Imports pending beats from a BeatImporter and returns a boolean to
    /// indicate if BPM/beats were updated. Only supposed to be called while
    /// the caller guards this a lock.
    bool importPendingBeatsWhileLocked();

    /// Sets cue points and returns a boolean to indicate if cues were updated.
    /// Only supposed to be called while the caller guards this a lock.
    bool setCuePointsWhileLocked(const QList<CuePointer>& cuePoints);

    /// Imports pending cues from a CueInfoImporter and returns a boolean to
    /// indicate if cues were updated. Only supposed to be called while the
    /// caller guards this a lock.
    bool importPendingCueInfosWhileLocked();

    mixxx::Bpm getBpmWhileLocked() const;
    bool trySetBpmWhileLocked(double bpmValue);
    bool trySetBeatsWhileLocked(
            mixxx::BeatsPointer pBeats,
            bool lockBpmAfterSet = false);

    bool trySetBeatsMarkDirtyAndUnlock(
            QMutexLocker* pLock,
            mixxx::BeatsPointer pBeats,
            bool lockBpmAfterSet);
    bool tryImportPendingBeatsMarkDirtyAndUnlock(
            QMutexLocker* pLock,
            bool lockBpmAfterSet);

    void setCuePointsMarkDirtyAndUnlock(
            QMutexLocker* pLock,
            const QList<CuePointer>& cuePoints);
    void importPendingCueInfosMarkDirtyAndUnlock(
            QMutexLocker* pLock);

    enum class DurationRounding {
        SECONDS, // rounded to full seconds
        NONE     // unmodified
    };
    double getDuration(DurationRounding rounding) const;

    ExportTrackMetadataResult exportMetadata(
            mixxx::MetadataSourcePointer pMetadataSource,
            UserSettingsPointer pConfig);

    // Information about the actual properties of the
    // audio stream is only available after opening the
    // source at least once. On this occasion the metadata
    // stream info of the track need to be updated to reflect
    // these values.
    bool hasStreamInfoFromSource() const {
        QMutexLocker lock(&m_qMutex);
        return m_record.hasStreamInfoFromSource();
    }
    void updateStreamInfoFromSource(
            mixxx::audio::StreamInfo&& streamInfo);

    // Mutex protecting access to object
    mutable QMutex m_qMutex;

    // The file
    mutable TrackFile m_fileInfo;

    SecurityTokenPointer m_pSecurityToken;

    mixxx::TrackRecord m_record;

    // Flag that indicates whether or not the TIO has changed. This is used by
    // TrackDAO to determine whether or not to write the Track back.
    bool m_bDirty;

    // Flag indicating that the user has explicitly requested to save
    // the metadata.
    bool m_bMarkedForMetadataExport;

    // The list of cue points for the track
    QList<CuePointer> m_cuePoints;

    // Storage for the track's beats
    mixxx::BeatsPointer m_pBeats;

    //Visual waveform data
    ConstWaveformPointer m_waveform;
    ConstWaveformPointer m_waveformSummary;

    mixxx::BeatsImporterPointer m_pBeatsImporterPending;
    mixxx::CueInfoImporterPointer m_pCueInfoImporterPending;

    friend class TrackDAO;
    friend class GlobalTrackCache;
    friend class GlobalTrackCacheResolver;
    friend class SoundSourceProxy;
};
