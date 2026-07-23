#pragma once

#include <QDateTime>
#include <QDir>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <memory>

#include "library/dao/dao.h"
#include "preferences/usersettings.h"
#include "track/trackid.h"

// DTO matching Phase 1 migration of fingerprint_metadata
struct FingerprintMetadata {
    TrackId trackId;
    quint32 fingerprintHash; // 32-bit SimHash
    QString chromaSha256;    // 64-char hex
    double fingerprintDuration{0.0};
    int fingerprintVersion{0};
    int cmrtGroupId{-1}; // -1 if not grouped
    double cmrtOffsetSeconds{0.0};
    bool isCanonical{false};
    bool fingerprintValid{true};
    bool fingerprintNeedsRegen{false};
    QDateTime computedAt;
};

// DTO matching Phase 1 migration of cmrt_groups
struct CmrtGroup {
    int groupId{-1};
    quint32 fingerprintHash{0};
    QString chromaSha256;
    TrackId canonicalTrackId;
    int trackCount{1};
    QDateTime createdAt;
    QDateTime lastUpdated;
    // MusicBrainz CMRT integration
    QString musicbrainzCmrtMbid;
    bool musicbrainzSynced{false};
    QDateTime musicbrainzLastSync;
    double musicbrainzCommunityScore{0.0};
    bool musicbrainzSubmitted{false};
    QString musicbrainzSubmissionMbid;
    bool localPreferred{true};
    QDateTime conflictResolvedAt;
    QString conflictResolution; // 'local_won' | 'remote_won'
};

struct CmrtMember {
    int memberId{-1};
    int groupId{-1};
    TrackId trackId;
    double offsetFromCanonical{0.0};
    double qualityScore{-1.0}; // -1.0 if not yet scored
    double matchScore{-1.0};
    bool isFakeLossless{false};
    QDateTime addedAt;
    int userQualityRating{-1}; // -1 if unrated
    // Whether this member loads its CMRT's beatgrid/cues (shifted by
    // offsetFromCanonical) instead of its own, at deck-load time. Purely
    // a display-time preference -- never touched by CmrtGroupingService,
    // only by the user via the library checkbox column.
    bool useCmrtData{false};
};

struct AcoustIdJob {
    int queueId{-1};
    TrackId trackId;
    int priority{5}; // lower = higher priority
    QString status;  // 'queued' | 'processing' | 'completed' | 'failed'
    int attempts{0};
    int maxAttempts{3};
    QDateTime lastAttempt; // nullable — isValid() == false if never attempted
    QString errorMessage;
    QDateTime queuedAt;
};

struct AcoustIdCacheEntry {
    QString chromaSha256;           // PK — SHA-256 of .chroma file
    QString acoustidId;             // AcoustID UUID
    QString musicbrainzRecordingId; // nullable
    QString musicbrainzReleaseId;   // nullable
    QString musicbrainzMetadata;    // full JSON response, nullable
    double confidence{-1.0};        // 0.0–1.0; -1.0 = not set (maps to NULL)
    QDateTime lookupTimestamp;
    QDateTime expiresAt; // isValid() == false if no TTL
};

struct UnmatchedTrackInfo {
    TrackId trackId;
    QString title;
    QString artist;
    double duration{0.0};
    // Empty if the track has not been enqueued yet
    QString queueStatus; // 'queued' | 'processing' | 'failed' | ''
    int attempts{0};
    bool fingerprintValid{false};
};

struct MbidGroupCandidate {
    int cmrtGroupId{-1};
    quint32 fingerprintHash{0}; // cmrt_groups.fingerprint_hash
    TrackId canonicalTrackId;
};

struct TrackQualityInfo {
    QString filetype;   // library.filetype
    int sampleRate{0};  // library.samplerate
    qint64 fileSize{0}; // track_locations.filesize
};

class TrackFingerprintDao : public DAO {
  public:
    explicit TrackFingerprintDao(UserSettingsPointer pConfig);
    ~TrackFingerprintDao() override = default;

    // fingerprint_metadata operations
    bool saveFingerprintMetadata(const FingerprintMetadata& metadata) const;
    std::unique_ptr<FingerprintMetadata> getFingerprintMetadata(TrackId trackId) const;
    bool markFingerprintNeedsRegen(TrackId trackId) const;
    bool deleteFingerprintMetadata(TrackId trackId) const;

    // cmrt_groups operations
    int createCmrtGroup(const CmrtGroup& group) const;
    std::unique_ptr<CmrtGroup> getCmrtGroup(int groupId) const;

    QList<MbidGroupCandidate> getGroupsForMbid(const QString& musicbrainzRecordingId) const;
    QList<MbidGroupCandidate> getAllGroupCandidates() const;

    std::optional<TrackQualityInfo> getTrackQualityInfo(TrackId trackId) const;
    bool updateCanonicalTrack(int groupId, TrackId newCanonicalTrackId) const;

    // cmrt_members operations
    bool addCmrtMember(const CmrtMember& member) const;
    QList<CmrtMember> getCmrtMembersForGroup(int groupId) const;
    bool deleteCmrtMember(TrackId trackId) const;
    bool updateMemberOffset(TrackId trackId, double offsetFromCanonical) const;

    std::unique_ptr<CmrtMember> getCmrtMemberByTrackId(TrackId trackId) const;

    bool updateMemberUseCmrtData(TrackId trackId, bool useCmrtData) const;

    bool updateMemberMatchScore(TrackId trackId, double matchScore) const;

    double getMemberQualityScore(TrackId trackId) const;

    // Updates track_count on cmrt_groups when membership changes.
    // Pass +1 when adding a member, -1 when removing.
    bool updateCmrtGroupTrackCount(int groupId, int delta) const;

    // .chroma file I/O
    // Fingerprints live in ~/.mixxx/fingerprints/track_{id}.chroma
    QByteArray loadChromaFile(TrackId trackId) const;
    bool saveChromaFile(TrackId trackId, const QByteArray& data) const;
    bool deleteChromaFile(TrackId trackId) const;

    // acoustid_queue operations
    // Enqueues a track for AcoustID lookup. Silently no-ops if track_id
    // is already in the queue (UNIQUE constraint on track_id).
    bool enqueueAcoustId(TrackId trackId, int priority = 5) const;

    // Updates the status of a queued job and increments the attempt counter.
    // Pass a non-empty errorMessage only on failure.
    bool updateQueueStatus(
            int queueId,
            const QString& status,
            const QString& errorMessage = QString()) const;

    // Returns up to `limit` jobs that are ready to run, ordered by
    // priority ASC then queued_at ASC. Used by the background worker
    // and by the startup load (Q7 in Final_Database.md).
    QList<AcoustIdJob> getPendingJobs(int limit = 10) const;

    // Removes a track's queue entry entirely — called from onPurgingTracks.
    bool deleteQueueEntry(TrackId trackId) const;

    // acoustid_cache operations
    // Stores or refreshes an AcoustID API response keyed on SHA-256.
    // Safe to call multiple times for the same chromaSha256 — updates in place.
    bool cacheAcoustIdResult(const AcoustIdCacheEntry& entry) const;

    // Returns the cached entry for a given SHA-256,
    // or nullptr if not found or expired.
    std::unique_ptr<AcoustIdCacheEntry> lookupAcoustIdCache(
            const QString& chromaSha256) const;

    // Removes entries whose expires_at has passed.
    // Call periodically to prevent unbounded cache growth.
    bool deleteExpiredCacheEntries() const;

    // MusicBrainzQueue view support

    // Returns all tracks that have a valid fingerprint but no AcoustID
    // yet, along with their current queue status. Used to populate the
    // MusicBrainzQueue sidebar.
    QList<UnmatchedTrackInfo> getUnmatchedTracks() const;

    // Resets a failed job back to queued state so the worker retries it.
    // Sets status='queued', attempts=0, error_message=NULL, last_attempt=NULL.
    bool reQueueJob(TrackId trackId) const;

    // Clears all fingerprint data for a single track:
    // deletes the .chroma file, handles canonical reassignment or group
    // deletion in cmrt_groups, removes cmrt_members, fingerprint_metadata,
    // and acoustid_queue rows for this track.
    // Returns true if the cleanup completed without errors.
    bool clearFingerprintData(TrackId trackId) const;

    // Calls clearFingerprintData() for every track that has a
    // fingerprint_metadata row. Returns the number of tracks cleared.
    // Used by the "Clear All Fingerprints" button in Preferences.
    int clearAllFingerprintData() const;

  private:
    // Returns ~/.mixxx/fingerprints/ — creates the directory on first call.
    QDir getFingerprintStoragePath() const;
    // Returns the absolute path to track_{trackId}.chroma
    QString getChromaFilePath(TrackId trackId) const;

    const UserSettingsPointer m_pConfig;
};
