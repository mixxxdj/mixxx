#pragma once

#include "library/dao/dao.h"
#include "track/bpmsegments.h"
#include "track/keysegments.h"
#include "track/trackid.h"

#define BPM_SEGMENTS_TABLE "track_bpm_segments"
#define KEY_SEGMENTS_TABLE "track_key_segments"

class SegmentsDAO : public DAO {
  public:
    ~SegmentsDAO() override = default;

    QList<BpmSegmentsPointer> getBpmSegmentsForTrack(TrackId trackId) const;
    void saveTrackBpmSegments(TrackId trackId, const QList<BpmSegmentsPointer>& segmentList) const;
    bool deleteBpmSegmentsForTrack(TrackId trackId) const;
    bool deleteBpmSegmentsForTracks(const QList<TrackId>& trackIds) const;

    QList<KeySegmentsPointer> getKeySegmentsForTrack(TrackId trackId) const;
    void saveTrackKeySegments(TrackId trackId, const QList<KeySegmentsPointer>& segmentList) const;
    bool deleteKeySegmentsForTrack(TrackId trackId) const;
    bool deleteKeySegmentsForTracks(const QList<TrackId>& trackIds) const;

  private:
    bool saveBpmSegment(TrackId trackId, BpmSegments* pSegment) const;
    bool saveKeySegment(TrackId trackId, KeySegments* pSegment) const;
};
