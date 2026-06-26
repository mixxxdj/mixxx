#include "musicbrainz/cmrtgroupingservice.h"

#include <QtDebug>
#include <cstring>

#include "analyzer/qualityscorer.h"
#include "library/library_prefs.h"

namespace mixxx {

namespace {

const bool sDebugCmrtGroupingService = true;

// Converts a .chroma file's raw bytes back into the uint32 array
// FingerprintMatcher operates on.
QVector<quint32> chromaBytesToVector(const QByteArray& rawChromaData) {
    const int numValues = rawChromaData.size() / static_cast<int>(sizeof(quint32));
    if (numValues == 0) {
        return {};
    }
    QVector<quint32> result(numValues);
    std::memcpy(result.data(), rawChromaData.constData(), numValues * sizeof(quint32));
    return result;
}

double scoreTrackQuality(const std::optional<TrackQualityInfo>& info) {
    if (!info) {
        return 0.0;
    }
    return QualityScorer::calculateScore(
            {info->filetype, info->sampleRate, info->fileSize});
}

} // namespace

CmrtGroupingService::CmrtGroupingService(
        TrackFingerprintDao& fingerprintDao, UserSettingsPointer pConfig)
        : m_fingerprintDao(fingerprintDao),
          m_pConfig(pConfig) {
}

void CmrtGroupingService::processTrack(
        TrackId trackId, const QString& musicbrainzRecordingId) {
    if (sDebugCmrtGroupingService) {
        qDebug() << "CmrtGroupingService -> [processTrack] -> entry"
                 << "trackId:" << trackId
                 << "mbid:" << musicbrainzRecordingId;
    }

    const auto pFpRow = m_fingerprintDao.getFingerprintMetadata(trackId);
    if (!pFpRow || !pFpRow->fingerprintValid) {
        if (sDebugCmrtGroupingService) {
            qDebug() << "CmrtGroupingService -> [processTrack] -> "
                        "no valid fingerprint_metadata row, skipping"
                     << "trackId:" << trackId;
        }
        return;
    }

    const double qualityScore =
            scoreTrackQuality(m_fingerprintDao.getTrackQualityInfo(trackId));

    // With an MBID, only groups whose canonical track shares it are worth
    // comparing against -- getGroupsForMbid() does that filtering in SQL.
    // Without one (AcoustID found no match for this track), there's nothing
    // to filter on, so fall back to scanning every group. More expensive,
    // but only reached for tracks AcoustID couldn't identify.
    const QList<MbidGroupCandidate> candidates = musicbrainzRecordingId.isEmpty()
            ? m_fingerprintDao.getAllGroupCandidates()
            : m_fingerprintDao.getGroupsForMbid(musicbrainzRecordingId);

    const QVector<quint32> trackFp =
            chromaBytesToVector(m_fingerprintDao.loadChromaFile(trackId));

    const float matchThreshold = static_cast<float>(m_pConfig->getValue(
            mixxx::library::prefs::kCmrtMatchThresholdConfigKey,
            mixxx::library::prefs::kCmrtMatchThresholdDefault));

    // Check every candidate and keep the best-scoring match rather than
    // stopping at the first one that passes -- with an MBID filter this is
    // usually one candidate anyway, but the no-MBID fallback can hand back
    // several SimHash-plausible groups, and the first one to clear the
    // threshold isn't necessarily the closest match.
    bool haveBestMatch = false;
    MbidGroupCandidate bestCandidate;
    FingerprintMatcher::MatchResult bestMatchResult;

    for (const MbidGroupCandidate& candidate : candidates) {
        // SimHash pre-filter first -- O(1) integer comparison versus the
        // disk read + full comparison below.
        if (!FingerprintMatcher::simHashCandidatesMatch(
                    pFpRow->fingerprintHash,
                    candidate.fingerprintHash,
                    FingerprintMatcher::kSimHashMaxHammingBits)) {
            continue;
        }

        const QVector<quint32> candidateFp = chromaBytesToVector(
                m_fingerprintDao.loadChromaFile(candidate.canonicalTrackId));
        const auto matchResult = FingerprintMatcher::compare(
                trackFp, candidateFp, matchThreshold);
        if (matchResult.isMatch &&
                (!haveBestMatch || matchResult.score > bestMatchResult.score)) {
            haveBestMatch = true;
            bestCandidate = candidate;
            bestMatchResult = matchResult;
        }
    }

    if (haveBestMatch) {
        if (sDebugCmrtGroupingService) {
            qDebug() << "CmrtGroupingService -> [processTrack] -> "
                        "best match across"
                     << candidates.size() << "candidate(s):"
                     << "group:" << bestCandidate.cmrtGroupId
                     << "score:" << bestMatchResult.score;
        }
        handleMatchedCandidate(trackId, bestCandidate, bestMatchResult, qualityScore);
        return;
    }

    // No existing group matched -- this track is the first one seen for
    // this mastering.
    createNewGroup(trackId, *pFpRow, qualityScore);
}

void CmrtGroupingService::assignToExistingGroup(
        TrackId trackId,
        int groupId,
        double offsetSeconds,
        double qualityScore,
        double matchScore) {
    CmrtMember member;
    member.groupId = groupId;
    member.trackId = trackId;
    member.offsetFromCanonical = offsetSeconds;
    member.qualityScore = qualityScore;
    member.matchScore = matchScore;
    member.addedAt = QDateTime::currentDateTimeUtc();
    m_fingerprintDao.addCmrtMember(member);
    m_fingerprintDao.updateCmrtGroupTrackCount(groupId, +1);

    if (auto pMeta = m_fingerprintDao.getFingerprintMetadata(trackId)) {
        pMeta->cmrtGroupId = groupId;
        pMeta->cmrtOffsetSeconds = offsetSeconds;
        pMeta->isCanonical = false;
        m_fingerprintDao.saveFingerprintMetadata(*pMeta);
    }

    if (sDebugCmrtGroupingService) {
        qDebug() << "CmrtGroupingService -> [assignToExistingGroup] -> added"
                 << trackId << "to group" << groupId
                 << "offsetSeconds:" << offsetSeconds;
    }
}

void CmrtGroupingService::createNewGroup(
        TrackId trackId, const FingerprintMetadata& fpRow, double qualityScore) {
    CmrtGroup group;
    group.fingerprintHash = fpRow.fingerprintHash;
    group.chromaSha256 = fpRow.chromaSha256;
    group.canonicalTrackId = trackId;
    group.trackCount = 1;
    group.createdAt = QDateTime::currentDateTimeUtc();

    const int groupId = m_fingerprintDao.createCmrtGroup(group);
    if (groupId < 0) {
        qWarning() << "CmrtGroupingService -> [createNewGroup] -> "
                      "failed to create cmrt_group for track"
                   << trackId;
        return;
    }

    CmrtMember member;
    member.groupId = groupId;
    member.trackId = trackId;
    member.offsetFromCanonical = 0.0;
    member.qualityScore = qualityScore;
    member.addedAt = QDateTime::currentDateTimeUtc();
    m_fingerprintDao.addCmrtMember(member);

    FingerprintMetadata updatedFpRow = fpRow;
    updatedFpRow.cmrtGroupId = groupId;
    updatedFpRow.cmrtOffsetSeconds = 0.0;
    updatedFpRow.isCanonical = true;
    m_fingerprintDao.saveFingerprintMetadata(updatedFpRow);

    if (sDebugCmrtGroupingService) {
        qDebug() << "CmrtGroupingService -> [createNewGroup] -> created group"
                 << groupId << "with canonical track" << trackId;
    }
}

void CmrtGroupingService::handleMatchedCandidate(TrackId newTrackId,
        const MbidGroupCandidate& candidate,
        const FingerprintMatcher::MatchResult& matchResult,
        double newTrackQualityScore) {
    const double offsetSeconds =
            matchResult.offsetItems * FingerprintMatcher::kItemDurationSeconds;
    const double canonicalQualityScore = scoreTrackQuality(
            m_fingerprintDao.getTrackQualityInfo(candidate.canonicalTrackId));

    if (sDebugCmrtGroupingService) {
        qDebug() << "CmrtGroupingService -> [handleMatchedCandidate] ->"
                 << "newTrack:" << newTrackId << "score:" << newTrackQualityScore
                 << "vs canonical:" << candidate.canonicalTrackId
                 << "score:" << canonicalQualityScore
                 << "offsetSeconds:" << offsetSeconds;
    }

    if (newTrackQualityScore > canonicalQualityScore) {
        replaceCanonical(candidate.cmrtGroupId,
                candidate.canonicalTrackId,
                newTrackId,
                offsetSeconds,
                newTrackQualityScore,
                matchResult.score);
    } else {
        assignToExistingGroup(
                newTrackId,
                candidate.cmrtGroupId,
                offsetSeconds,
                newTrackQualityScore,
                matchResult.score);
    }
}

void CmrtGroupingService::replaceCanonical(int groupId,
        TrackId oldCanonicalId,
        TrackId newCanonicalId,
        double offsetOfNewFromOld,
        double newCanonicalQualityScore,
        double matchScoreOfNewFromOld) {
    if (sDebugCmrtGroupingService) {
        qDebug() << "CmrtGroupingService -> [replaceCanonical] ->"
                 << "group:" << groupId << "old:" << oldCanonicalId
                 << "new:" << newCanonicalId
                 << "offsetOfNewFromOld:" << offsetOfNewFromOld;
    }

    // Snapshot the membership list before changing anything below -- the
    // "everyone else" loop in step 4 needs the group as it stood before
    // newCanonicalId joined.
    const QList<CmrtMember> existingMembers =
            m_fingerprintDao.getCmrtMembersForGroup(groupId);

    // 1. Point the group at the new canonical track.
    m_fingerprintDao.updateCanonicalTrack(groupId, newCanonicalId);

    // 2. Add the new track as a member at offset 0 and mark it canonical.
    CmrtMember newMember;
    newMember.groupId = groupId;
    newMember.trackId = newCanonicalId;
    newMember.offsetFromCanonical = 0.0;
    newMember.qualityScore =
            scoreTrackQuality(m_fingerprintDao.getTrackQualityInfo(newCanonicalId));
    newMember.addedAt = QDateTime::currentDateTimeUtc();
    m_fingerprintDao.addCmrtMember(newMember);
    m_fingerprintDao.updateCmrtGroupTrackCount(groupId, +1);

    if (auto pNewMeta = m_fingerprintDao.getFingerprintMetadata(newCanonicalId)) {
        pNewMeta->cmrtGroupId = groupId;
        pNewMeta->cmrtOffsetSeconds = 0.0;
        pNewMeta->isCanonical = true;
        m_fingerprintDao.saveFingerprintMetadata(*pNewMeta);
    }

    // 3. Demote the old canonical to a regular member. Its offset from the
    //    new canonical is the negative of the offset already calculated
    //    going the other direction in handleMatchedCandidate() -- no second
    //    FingerprintMatcher pass needed for this one.
    m_fingerprintDao.updateMemberOffset(oldCanonicalId, -offsetOfNewFromOld);
    m_fingerprintDao.updateMemberMatchScore(oldCanonicalId, matchScoreOfNewFromOld);
    if (auto pOldMeta = m_fingerprintDao.getFingerprintMetadata(oldCanonicalId)) {
        pOldMeta->isCanonical = false;
        pOldMeta->cmrtOffsetSeconds = -offsetOfNewFromOld;
        m_fingerprintDao.saveFingerprintMetadata(*pOldMeta);
    }

    // 4. Recalculate offsets for every other existing member against the
    //    new canonical. O(member count) .chroma reads + comparisons --
    //    fine for the small group sizes expected in practice (a handful of
    //    masterings per recording), not something that needs batching yet.
    const QVector<quint32> newCanonicalFp =
            chromaBytesToVector(m_fingerprintDao.loadChromaFile(newCanonicalId));

    for (const CmrtMember& member : existingMembers) {
        if (member.trackId == oldCanonicalId || member.trackId == newCanonicalId) {
            continue;
        }
        const QVector<quint32> memberFp =
                chromaBytesToVector(m_fingerprintDao.loadChromaFile(member.trackId));
        const auto matchResult = FingerprintMatcher::compare(memberFp, newCanonicalFp);
        const double offsetSeconds =
                matchResult.offsetItems * FingerprintMatcher::kItemDurationSeconds;

        m_fingerprintDao.updateMemberOffset(member.trackId, offsetSeconds);
        m_fingerprintDao.updateMemberMatchScore(member.trackId, matchResult.score);
        if (auto pMemberMeta = m_fingerprintDao.getFingerprintMetadata(member.trackId)) {
            pMemberMeta->cmrtOffsetSeconds = offsetSeconds;
            m_fingerprintDao.saveFingerprintMetadata(*pMemberMeta);
        }
    }

    if (sDebugCmrtGroupingService) {
        qDebug() << "CmrtGroupingService -> [replaceCanonical] -> done, recalculated"
                 << existingMembers.size() << "other member offset(s)";
    }
}

} // namespace mixxx
