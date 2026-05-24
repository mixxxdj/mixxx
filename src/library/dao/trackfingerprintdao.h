#pragma once

#include <QDateTime>
#include <QSqlDatabase>
#include <QString>
#include <memory>

#include "library/dao/dao.h"
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

class TrackFingerprintDao : public DAO {
  public:
    TrackFingerprintDao() = default;
    ~TrackFingerprintDao() override = default;

    // fingerprint_metadata operations
    bool saveFingerprintMetadata(const FingerprintMetadata& metadata) const;
    std::unique_ptr<FingerprintMetadata> getFingerprintMetadata(TrackId trackId) const;
    bool markFingerprintNeedsRegen(TrackId trackId) const;
    bool deleteFingerprintMetadata(TrackId trackId) const;

    // cmrt_groups operations
    int createCmrtGroup(const CmrtGroup& group) const;
    std::unique_ptr<CmrtGroup> getCmrtGroup(int groupId) const;
};
