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
#include "track/trackrecord.h"
#include "util/fileaccess.h"
#include "util/memory.h"
#include "waveform/waveform.h"

class Track : public QObject {
    Q_OBJECT

  public:
    Track(mixxx::FileAccess fileAccess,
            TrackId trackId = TrackId());
    Track(const Track&) = delete;
    ~Track() override;

    // Creates a new empty temporary instance for fake tracks or for
    // testing purposes. The resulting track will neither be stored
    // in the database nor will the metadata of the corresponding file
    // be updated.
    static TrackPointer newTemporary(
            mixxx::FileAccess fileAccess = mixxx::FileAccess());
    static TrackPointer newTemporary(
            const QString& filePath) {
        return newTemporary(mixxx::FileAccess(mixxx::FileInfo(filePath)));
    }
    static TrackPointer newTemporary(
            const QDir& dir,
            const QString& file) {
        return newTemporary(mixxx::FileAccess(mixxx::FileInfo(dir, file)));
    }

    // Creates a dummy instance only for testing purposes.
    static TrackPointer newDummy(
            const QString& filePath,
            TrackId trackId);

    Q_PROPERTY(QString artist READ getArtist WRITE setArtist NOTIFY artistChanged)
    Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString album READ getAlbum WRITE setAlbum NOTIFY albumChanged)
    Q_PROPERTY(QString albumArtist READ getAlbumArtist WRITE setAlbumArtist
                    NOTIFY albumArtistChanged)
    Q_PROPERTY(QString genre READ getGenre WRITE setGenre NOTIFY genreChanged)
    Q_PROPERTY(QString composer READ getComposer WRITE setComposer NOTIFY composerChanged)
    Q_PROPERTY(QString grouping READ getGrouping WRITE setGrouping NOTIFY groupingChanged)
    Q_PROPERTY(QString year READ getYear WRITE setYear NOTIFY yearChanged)
    Q_PROPERTY(QString trackNumber READ getTrackNumber WRITE setTrackNumber
                    NOTIFY trackNumberChanged)
    Q_PROPERTY(QString trackTotal READ getTrackTotal WRITE setTrackTotal NOTIFY trackTotalChanged)
    Q_PROPERTY(int timesPlayed READ getTimesPlayed NOTIFY timesPlayedChanged)
    Q_PROPERTY(QString comment READ getComment WRITE setComment NOTIFY commentChanged)
    Q_PROPERTY(double bpm READ getBpm NOTIFY bpmChanged)
    Q_PROPERTY(QString bpmText READ getBpmText STORED false NOTIFY bpmChanged)
    Q_PROPERTY(QString keyText READ getKeyText WRITE setKeyText NOTIFY keyChanged)
    Q_PROPERTY(double duration READ getDuration NOTIFY durationChanged)
    Q_PROPERTY(QString durationText READ getDurationTextSeconds STORED false NOTIFY durationChanged)
    Q_PROPERTY(QString durationTextCentiseconds READ getDurationTextCentiseconds
                    STORED false NOTIFY durationChanged)
    Q_PROPERTY(QString durationTextMilliseconds READ getDurationTextMilliseconds
                    STORED false NOTIFY durationChanged)
    Q_PROPERTY(QString info READ getInfo STORED false NOTIFY infoChanged)
    Q_PROPERTY(QString titleInfo READ getTitleInfo STORED false NOTIFY infoChanged)

    mixxx::FileAccess getFileAccess() const {
        // Copying QFileInfo is thread-safe due to implicit sharing,
        // i.e. no locking needed.
        return m_fileAccess;
    }
    mixxx::FileInfo getFileInfo() const {
        // Copying QFileInfo is thread-safe due to implicit sharing,
        // i.e. no locking needed.
        return m_fileAccess.info();
    }

    TrackId getId() const;

    // Returns absolute path to the file, including the filename.
    QString getLocation() const {
        return m_fileAccess.info().location();
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
    double getDuration() const;
    // Returns the duration rounded to seconds
    int getDurationSecondsInt() const;
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
    double getBpm() const;
    // Returns BPM as a string
    QString getBpmText() const {
        return mixxx::Bpm::displayValueText(getBpm());
    }

    // A track with a locked BPM will not be re-analyzed by the beats or bpm
    // analyzer.
    void setBpmLocked(bool bpmLocked);
    bool isBpmLocked() const;

    // Updates the ReplayGain ratio value, preserving the other properties of the
    // existing ReplayGain.
    void setReplayGainRatio(double ratio);
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
    void setColor(const mixxx::RgbColor::optional_t&);
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

    QDateTime getLastPlayedAt() const {
        return getPlayCounter().getLastPlayedAt();
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
    CuePointer createAndAddCue(
            mixxx::CueType type,
            int hotCueIndex,
            double sampleStartPosition,
            double sampleEndPosition);
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
    bool refreshCoverImageDigest(
            const QImage& loadedImage = QImage());

    /// Set track metadata after importing from the source.
    ///
    /// The timestamp tracks when metadata has last been synchronized
    /// with file tags, either by importing or exporting the metadata.
    void replaceMetadataFromSource(
            mixxx::TrackMetadata importedMetadata,
            const QDateTime& metadataSynchronized);

    mixxx::TrackMetadata getMetadata(
            bool* pMetadataSynchronized = nullptr) const;

    mixxx::TrackRecord getRecord(
            bool* pDirty = nullptr) const;
    bool replaceRecord(
            mixxx::TrackRecord newRecord,
            mixxx::BeatsPointer pOptionalBeats = nullptr);

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
    void artistChanged(const QString&);
    void titleChanged(const QString&);
    void albumChanged(const QString&);
    void albumArtistChanged(const QString&);
    void genreChanged(const QString&);
    void composerChanged(const QString&);
    void groupingChanged(const QString&);
    void yearChanged(const QString&);
    void trackNumberChanged(const QString&);
    void trackTotalChanged(const QString&);
    void commentChanged(const QString&);
    void bpmChanged();
    void keyChanged();
    void timesPlayedChanged();
    void durationChanged();
    void infoChanged();

    void waveformUpdated();
    void waveformSummaryUpdated();
    void coverArtUpdated();
    void beatsUpdated();
    void replayGainUpdated(mixxx::ReplayGain replayGain);
    void colorUpdated(const mixxx::RgbColor::optional_t& color);
    void cuesUpdated();
    void analyzed();

    void changed(TrackId trackId);
    void dirty(TrackId trackId);
    void clean(TrackId trackId);

  private slots:
    void slotCueUpdated();

  private:
    /// Set a unique identifier for the track.
    /// Only used by GlobalTrackCacheResolver when the track is saved to db for the first time
    void initId(TrackId id);
    /// Remove the TrackId.
    /// Only used by GlobalTrackCacheResolver when the track is purged from the library
    void resetId();

    void relocate(mixxx::FileAccess fileAccess);

    // Set whether the TIO is dirty or not and unlock before emitting
    // any signals. This must only be called from member functions
    // while the TIO is locked.
    void markDirtyAndUnlock(QMutexLocker* pLock) {
        setDirtyAndUnlock(pLock, true);
    }
    void setDirtyAndUnlock(QMutexLocker* pLock, bool bDirty);

    void afterKeysUpdated(QMutexLocker* pLock);

    void afterBeatsAndBpmUpdated(QMutexLocker* pLock);
    void emitBeatsAndBpmUpdated();

    /// Emits a changed signal for each Q_PROPERTY
    void emitChangedSignalsForAllMetadata();

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

    /// Merge additional metadata that is not (yet) stored in the database
    /// and only available from file tags.
    ///
    /// Returns true if the track has been modified and false otherwise.
    bool mergeExtraMetadataFromSource(
            const mixxx::TrackMetadata& importedMetadata);

    ExportTrackMetadataResult exportMetadata(
            const mixxx::MetadataSource& metadataSource,
            const UserSettingsPointer& pConfig);

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
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    mutable QRecursiveMutex m_qMutex;
#else
    mutable QMutex m_qMutex;
#endif

    // The file
    mixxx::FileAccess m_fileAccess;

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
