#pragma once

#include <QList>
#include <QObject>
#include <QStack>
#include <QUrl>
#include <memory>

#include "audio/streaminfo.h"
#include "sources/metadatasource.h"
#include "track/beats.h"
#include "track/cue.h"
#include "track/cueinfoimporter.h"
#ifdef __STEM__
#include "track/steminfo.h"
#include "track/steminfoimporter.h"
#endif
#include "track/track_decl.h"
#include "track/trackrecord.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/compatibility/qmutex.h"
#include "util/fileaccess.h"
#include "util/performancetimer.h"
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
    Q_PROPERTY(QString genre READ getGenre STORED false NOTIFY genreChanged)
#if defined(__EXTRA_METADATA__)
    Q_PROPERTY(QString mood READ getMood STORED false NOTIFY moodChanged)
#endif // __EXTRA_METADATA__
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
    Q_PROPERTY(QString durationTextSeconds READ getDurationTextSeconds
                    STORED false NOTIFY durationChanged)
    Q_PROPERTY(QString durationTextCentiseconds READ getDurationTextCentiseconds
                    STORED false NOTIFY durationChanged)
    Q_PROPERTY(QString durationTextMilliseconds READ getDurationTextMilliseconds
                    STORED false NOTIFY durationChanged)
    Q_PROPERTY(QString info READ getInfo STORED false NOTIFY infoChanged)
    Q_PROPERTY(QString titleInfo READ getTitleInfo STORED false NOTIFY infoChanged)
    Q_PROPERTY(QDateTime sourceSynchronizedAt READ getSourceSynchronizedAt STORED false)

    mixxx::FileInfo getFileInfo() const {
        // Copying mixxx::FileInfo based on QFileInfo is thread-safe due to implicit sharing,
        // i.e. no locking needed.
        static_assert(mixxx::FileInfo::isQFileInfo());
        return m_fileAccess.info();
    }

    TrackId getId() const;

    // Returns absolute path to the file, including the filename.
    QString getLocation() const {
        const auto fileInfo = getFileInfo();
        if (!fileInfo.hasLocation()) {
            return {};
        }
        return fileInfo.location();
    }

    /// Set the file type
    ///
    /// Returns the old type to allow the caller to report if it has changed.
    QString setType(const QString& newType);

    /// Get the file type
    QString getType() const;

    // Get number of channels
    mixxx::audio::ChannelCount getChannels() const;

    mixxx::audio::SampleRate getSampleRate() const;

    void setBitrate(int);
    int getBitrate() const;
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
    bool trySetBpm(double bpmValue) {
        return trySetBpm(mixxx::Bpm(bpmValue));
    }
    bool trySetBpm(mixxx::Bpm bpm);

    double getBpm() const;
    QString getBpmText() const {
        return mixxx::Bpm::displayValueText(getBpm());
    }

    // A track with a locked BPM will not be re-analyzed by the beats or bpm
    // analyzer.
    void setBpmLocked(bool bpmLocked);
    bool isBpmLocked() const;

    void setReplayGain(const mixxx::ReplayGain&);
    // Adjust ReplayGain by multiplying the given gain amount.
    void adjustReplayGainFromPregain(double gain, const QString& requestingPlayerGroup);
    // Returns ReplayGain
    mixxx::ReplayGain getReplayGain() const;

    /// Checks if the internal metadata is in-sync with the
    /// metadata stored in file tags.
    bool checkSourceSynchronized() const;

    // The date/time of the last import or export of metadata
    void setSourceSynchronizedAt(const QDateTime& sourceSynchronizedAt);
    void resetSourceSynchronizedAt() {
        setSourceSynchronizedAt(QDateTime{});
    }
    QDateTime getSourceSynchronizedAt() const;

    void setDateAdded(const QDateTime& dateAdded);
    QDateTime getDateAdded() const;

    QString getTitle() const;
    void setTitle(const QString&);
    QString getArtist() const;
    void setArtist(const QString&);
    QString getAlbum() const;
    void setAlbum(const QString&);
    QString getAlbumArtist() const;
    void setAlbumArtist(const QString&);

    // Returns the content of the year library column.
    // This was original only the four digit (gregorian) calendar year of the release date
    // but allows to store any user string. Now it is alternatively used as
    // recording date/time in the ISO 8601 yyyy-MM-ddTHH:mm:ss format tunkated at any point,
    // following the TDRC ID3v2.4 frame or if not exists, TYER + TDAT.
    QString getYear() const;
    void setYear(const QString&);
    // Returns the track color
    mixxx::RgbColor::optional_t getColor() const;
    void setColor(const mixxx::RgbColor::optional_t&);
    QString getComment() const;
    void setComment(const QString&);
    void clearComment() {
        setComment(QString());
    }
    QString getComposer() const;
    void setComposer(const QString&);
    QString getGrouping() const;
    void setGrouping(const QString&);

    // Return track number/total
    QString getTrackNumber() const;
    QString getTrackTotal() const;
    // Set track number/total
    void setTrackNumber(const QString&);
    void setTrackTotal(const QString&);

    /// Return the genre as text
    QString getGenre() const;

    /// Update the genre text.
    ///
    /// Returns true if track metadata has been updated and false
    /// otherwise.
    ///
    /// TODO: Update the corresponding custom tags by splitting
    /// the text according to the given tag mapping configuration.
    /// All existing custom genre tags with their associated score
    /// will be replaced.
    bool updateGenre(
            /*TODO: const mixxx::TaggingConfig& config,*/
            const QString& genre);

#if defined(__EXTRA_METADATA__)
    /// Return the mood as text
    QString getMood() const;

    /// Update the mood text.
    ///
    /// Returns true if track metadata has been updated and false
    /// otherwise.
    ///
    /// TODO: Update the corresponding custom tags by splitting
    /// the text according to the given tag mapping configuration.
    /// All existing custom mood tags with their associated score
    /// will be replaced.
    bool updateMood(
            /*TODO: const mixxx::TaggingConfig& config,*/
            const QString& mood);
#endif // __EXTRA_METADATA__

    PlayCounter getPlayCounter() const;
    void setPlayCounter(const PlayCounter& playCounter);
    void resetPlayCounter(int iTimesPlayed = 0) {
        setPlayCounter(PlayCounter(iTimesPlayed));
    }
    // Sets played status and increments or decrements the play count
    void updatePlayCounter(bool bPlayed = true);
    // Sets played status but leaves play count untouched
    void updatePlayedStatusKeepPlayCount(bool bPlayed);

    // Only required for the times_played property
    int getTimesPlayed() const {
        return getPlayCounter().getTimesPlayed();
    }

    QDateTime getLastPlayedAt() const {
        return getPlayCounter().getLastPlayedAt();
    }

    int getRating() const;
    void setRating(int);
    void resetRating() {
        setRating(mixxx::TrackRecord::kNoRating);
    }

    QString getURL() const;
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

    const ConstWaveformPointer& getWaveform() const;
    void setWaveform(ConstWaveformPointer pWaveform);

    ConstWaveformPointer getWaveformSummary() const;
    void setWaveformSummary(ConstWaveformPointer pWaveform);

    /// Get the track's main cue point
    mixxx::audio::FramePos getMainCuePosition() const;
    // Set the track's main cue point
    void setMainCuePosition(mixxx::audio::FramePos position);
    /// Shift all cues by a constant offset
    void shiftCuePositionsMillis(mixxx::audio::FrameDiff_t milliseconds);
    /// Set hoctues' indices sorted by their frame position.
    /// If compress is true, indices are consecutive and start at 0.
    /// Set false to sort only, ie. keep empty hotcues before and in between.
    void setHotcueIndicesSortedByPosition(HotcueSortMode sortMode);

    // Call when analysis is done.
    void analysisFinished();

    // Calls for managing the track's cue points
    CuePointer createAndAddCue(
            mixxx::CueType type,
            int hotCueIndex,
            mixxx::audio::FramePos startPosition,
            mixxx::audio::FramePos endPosition,
            mixxx::RgbColor color = mixxx::PredefinedColorPalettes::kDefaultCueColor,
            double stem1vol = 1.0,
            double stem2vol = 1.0,
            double stem3vol = 1.0,
            double stem4vol = 1.0);
    CuePointer createAndAddCue(
            mixxx::CueType type,
            int hotCueIndex,
            double startPositionSamples,
            double endPositionSamples,
            mixxx::RgbColor color = mixxx::PredefinedColorPalettes::kDefaultCueColor,
            double stem1vol = 1.0,
            double stem2vol = 1.0,
            double stem3vol = 1.0,
            double stem4vol = 1.0) {
        return createAndAddCue(type,
                hotCueIndex,
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        startPositionSamples),
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        endPositionSamples),
                color,
                stem1vol,
                stem2vol,
                stem3vol,
                stem4vol);
    }
    CuePointer findCueByType(mixxx::CueType type) const; // NOTE: Cannot be used for hotcues.
    CuePointer findCueById(DbId id) const;
    CuePointer findHotcueByIndex(int idx) const;
    void removeCue(const CuePointer& pCue);
    void removeCuesOfType(mixxx::CueType);
    QList<CuePointer> getCuePoints() const {
        const QMutexLocker lock(&m_qMutex);
        // lock thread-unsafe copy constructors of QList
        return m_cuePoints;
    }
    void swapHotcues(int a, int b);
    void setCuePoints(const QList<CuePointer>& cuePoints);

#ifdef __STEM__
    QList<StemInfo> getStemInfo() const {
        const QMutexLocker lock(&m_qMutex);
        // lock thread-unsafe copy constructors of QList
        return m_stemInfo;
    }
    // Setter is only available internally. See setStemPointsWhileLocked

    bool hasStem() const {
        const QMutexLocker lock(&m_qMutex);
        // lock thread-unsafe copy constructors of QList
        return !m_stemInfo.isEmpty();
    }
#endif

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
            std::unique_ptr<mixxx::CueInfoImporter> pCueInfoImporter);
    ImportStatus getCueImportStatus() const;

    bool isDirty() const;

    // Get the track's Beats list
    mixxx::BeatsPointer getBeats() const;

    // Set the track's Beats if not locked
    bool trySetBeats(mixxx::BeatsPointer pBeats);
    bool trySetAndLockBeats(mixxx::BeatsPointer pBeats);

    void undoBeatsChange();
    bool canUndoBeatsChange() const {
        return !m_pBeatsUndoStack.isEmpty();
    }

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

    /// Set track metadata after importing from the source.
    ///
    /// The timestamp tracks when metadata has last been synchronized
    /// with file tags, either by importing or exporting the metadata.
    void replaceMetadataFromSource(
            mixxx::TrackMetadata importedMetadata,
            const QDateTime& sourceSynchronizedAt);

    mixxx::TrackMetadata getMetadata(
            mixxx::TrackRecord::SourceSyncStatus*
                    pSourceSyncStatus = nullptr) const;

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

    // Information about the actual properties of the
    // audio stream is only available after opening the
    // source at least once. On this occasion the metadata
    // stream info of the track need to be updated to reflect
    // these values.
    bool hasStreamInfoFromSource() const {
        const auto locked = lockMutex(&m_qMutex);
        return m_record.hasStreamInfoFromSource();
    }

  signals:
    void artistChanged(const QString&);
    void titleChanged(const QString&);
    void albumChanged(const QString&);
    void albumArtistChanged(const QString&);
    void genreChanged(const QString&);
#if defined(__EXTRA_METADATA__)
    void moodChanged(const QString&);
#endif // __EXTRA_METADATA__
    void composerChanged(const QString&);
    void groupingChanged(const QString&);
    void yearChanged(const QString&);
    void trackNumberChanged(const QString&);
    void trackTotalChanged(const QString&);
    void commentChanged(const QString&);
    void bpmChanged();
    void bpmLockChanged(bool locked);
    void keyChanged();
    void timesPlayedChanged();
    void durationChanged();
    void infoChanged();

    void waveformUpdated();
    void waveformSummaryUpdated();
    void coverArtUpdated();
    void beatsUpdated();
    void replayGainUpdated(mixxx::ReplayGain replayGain);
    // This signal indicates that ReplayGain is being adjusted, and pregains should be
    // adjusted in the opposite direction to compensate (no audible change).
    void replayGainAdjusted(const mixxx::ReplayGain&, const QString& requestingPlayerGroup);
    void colorUpdated(const mixxx::RgbColor::optional_t& color);
    void ratingUpdated(int rating);
    void cuesUpdated();
#ifdef __STEM__
    void stemsUpdated();
#endif
    void loopRemove();
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
    void markDirtyAndUnlock(QT_RECURSIVE_MUTEX_LOCKER* pLock) {
        setDirtyAndUnlock(pLock, true);
    }
    void setDirtyAndUnlock(QT_RECURSIVE_MUTEX_LOCKER* pLock, bool bDirty);

    void afterKeysUpdated(QT_RECURSIVE_MUTEX_LOCKER* pLock);

    void afterBeatsAndBpmUpdated(QT_RECURSIVE_MUTEX_LOCKER* pLock);
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

#ifdef __STEM__
    /// Sets stem info and returns a boolean to indicate if stems were updated.
    /// Only supposed to be called while the caller guards this a lock.
    bool setStemInfosWhileLocked(QList<StemInfo> stemInfo);

    /// Imports pending stem info from a stemInfoImporter and returns a boolean to
    /// indicate if stems were updated. Only supposed to be called while the
    /// caller guards this a lock.
    bool importPendingStemInfosWhileLocked();
#endif

    mixxx::Bpm getBpmWhileLocked() const;
    bool trySetBpmWhileLocked(mixxx::Bpm bpm);
    bool trySetBeatsWhileLocked(
            mixxx::BeatsPointer pBeats,
            bool lockBpmAfterSet = false);

    bool trySetBeatsMarkDirtyAndUnlock(
            QT_RECURSIVE_MUTEX_LOCKER* pLock,
            mixxx::BeatsPointer pBeats,
            bool lockBpmAfterSet);
    bool tryImportPendingBeatsMarkDirtyAndUnlock(
            QT_RECURSIVE_MUTEX_LOCKER* pLock,
            bool lockBpmAfterSet);

    void setCuePointsMarkDirtyAndUnlock(
            QT_RECURSIVE_MUTEX_LOCKER* pLock,
            const QList<CuePointer>& cuePoints);
    void importPendingCueInfosMarkDirtyAndUnlock(
            QT_RECURSIVE_MUTEX_LOCKER* pLock);

    /// Merge additional metadata that is not (yet) stored in the database
    /// and only available from file tags.
    ///
    /// Returns true if the track has been modified and false otherwise.
    bool mergeExtraMetadataFromSource(
            const mixxx::TrackMetadata& importedMetadata);

    bool exportSeratoMetadata();

    ExportTrackMetadataResult exportMetadata(
            const mixxx::MetadataSource& metadataSource,
            const SyncTrackMetadataParams& syncParams);
    void updateStreamInfoFromSource(
            mixxx::audio::StreamInfo&& streamInfo);

    // Mutex protecting access to object
    mutable QT_RECURSIVE_MUTEX m_qMutex;

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

#ifdef __STEM__
    // The list of stem info
    QList<StemInfo> m_stemInfo;
#endif

    // Storage for the track's beats
    mixxx::BeatsPointer m_pBeats;
    QStack<mixxx::BeatsPointer> m_pBeatsUndoStack;
    bool m_undoingBeatsChange;
    PerformanceTimer m_beatChangeTimer;

    // Visual waveform data
    ConstWaveformPointer m_waveform;
    ConstWaveformPointer m_waveformSummary;

    mixxx::BeatsImporterPointer m_pBeatsImporterPending;
    std::unique_ptr<mixxx::CueInfoImporter> m_pCueInfoImporterPending;

    friend class TrackDAO;
    void setHeaderParsedFromTrackDAO(bool headerParsed) {
        // Always operating on a newly created, exclusive instance! No need
        // to lock the mutex.
        DEBUG_ASSERT(!m_record.m_headerParsed);
        m_record.m_headerParsed = headerParsed;
    }
    /// Set the genre text WITHOUT updating the corresponding custom tags.
    ///
    /// TODO: Remove and populate TrackRecord from the database instead.
    void setGenreFromTrackDAO(
            const QString& genre);

    friend class GlobalTrackCache;
    friend class GlobalTrackCacheResolver;
    friend class SoundSourceProxy;
};
